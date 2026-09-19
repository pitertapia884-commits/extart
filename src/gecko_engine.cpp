#include "gecko_engine.hpp"

#include "config.hpp"
#include "download_manager.hpp"
#include "gecko_runtime.hpp"

#include <nsIAppShellService.h>
#include <nsIBaseWindow.h>
#include <nsIDocShell.h>
#include <nsIURI.h>
#include <nsIWebNavigation.h>
#include <nsIWindowlessBrowser.h>
#include <nsIScriptSecurityManager.h>
#include <nsNetUtil.h>
#include <nsString.h>
#include <nsIWidget.h>
#include <mozilla/dom/LoadURIOptions.h>

#include <utility>

namespace {
GeckoRuntime& process_runtime(GeckoBackend& backend) {
    static GeckoRuntime runtime;
    if (!runtime.initialized()) {
        runtime.initialize(backend);
    }
    return runtime;
}
}

GeckoEngine::GeckoEngine(GeckoBackend& backend, Profile& profile, const Config& config)
    : backend_(backend), profile_(profile) {
    (void)config;

    runtime_ = &process_runtime(backend_);

    if (!runtime_->initialized() || !runtime_->app_shell()) {
        return;
    }

    nsCOMPtr<nsIWindowlessBrowser> browser;
    nsresult rv = runtime_->app_shell()->CreateWindowlessBrowser(
        false, 0, getter_AddRefs(browser));
    if (NS_FAILED(rv) || !browser) {
        return;
    }

    windowless_browser_ = browser.get();
    windowless_browser_->AddRef();

    nsCOMPtr<nsIWebNavigation> navigation =
        do_QueryInterface(windowless_browser_, &rv);
    if (NS_FAILED(rv) || !navigation) {
        windowless_browser_->Close();
        windowless_browser_->Release();
        windowless_browser_ = nullptr;
        return;
    }

    navigation_ = navigation.get();
    navigation_->AddRef();

    // CreateWindowlessBrowser() gives nsWebBrowser a real Gecko PuppetWidget.
    // It has no GTK widget, but it is the actual widget driving Gecko's
    // rendering/compositor path. Keep it so the GTK bridge can use it.
    nsCOMPtr<nsIDocShell> doc_shell;
    rv = windowless_browser_->GetDocShell(getter_AddRefs(doc_shell));
    if (NS_FAILED(rv) || !doc_shell) {
        return;
    }

    nsCOMPtr<nsIBaseWindow> base_window = do_QueryInterface(doc_shell, &rv);
    if (NS_FAILED(rv) || !base_window) {
        return;
    }

    nsCOMPtr<nsIWidget> widget;
    rv = base_window->GetMainWidget(getter_AddRefs(widget));
    if (NS_FAILED(rv) || !widget) {
        return;
    }

    gecko_widget_ = widget.get();
    gecko_widget_->AddRef();

    gecko_widget_->Resize(1, 1, true);
    gecko_widget_->Show(true);
}

GeckoEngine::~GeckoEngine() {
    if (gecko_widget_) {
        gecko_widget_->Show(false);
        gecko_widget_->Release();
        gecko_widget_ = nullptr;
    }

    if (windowless_browser_) {
        if (navigation_) {
            navigation_->Release();
            navigation_ = nullptr;
        }
        windowless_browser_->Close();
        windowless_browser_->Release();
        windowless_browser_ = nullptr;
    }
}

GtkWidget* GeckoEngine::widget() const {
    // A Gecko PuppetWidget is not a GtkWidget. Returning it here would be an
    // invalid cast and would make GTK own something it does not own.
    // The GTK4 rendering bridge is the next stage.
    return nullptr;
}

gpointer GeckoEngine::native_handle() const {
    return gecko_widget_;
}

void GeckoEngine::set_callbacks(Callbacks callbacks) {
    callbacks_ = std::move(callbacks);
}

std::string GeckoEngine::current_uri() const {
    if (!navigation_) {
        return current_uri_;
    }

    nsCOMPtr<nsIURI> uri;
    if (NS_FAILED(navigation_->GetCurrentURI(getter_AddRefs(uri))) || !uri) {
        return current_uri_;
    }

    nsAutoCString spec;
    if (NS_FAILED(uri->GetSpec(spec))) {
        return current_uri_;
    }

    current_uri_ = spec.get();
    return current_uri_;
}

std::string GeckoEngine::current_title() const {
    return current_title_;
}

bool GeckoEngine::can_go_back() const {
    bool value = false;
    if (navigation_) {
        navigation_->GetCanGoBack(&value);
    }
    return value;
}

bool GeckoEngine::can_go_forward() const {
    bool value = false;
    if (navigation_) {
        navigation_->GetCanGoForward(&value);
    }
    return value;
}

void GeckoEngine::go_back() {
    if (navigation_ && can_go_back()) {
        navigation_->GoBack(false, true);
    }
}

void GeckoEngine::go_forward() {
    if (navigation_ && can_go_forward()) {
        navigation_->GoForward(false, true);
    }
}

void GeckoEngine::reload() {
    if (navigation_) {
        navigation_->Reload(nsIWebNavigation::LOAD_FLAGS_NONE);
    }
}

void GeckoEngine::load_html(const std::string& html, const char* base_uri) {
    if (!navigation_) {
        return;
    }

    nsCOMPtr<nsIURI> base;
    if (base_uri && *base_uri) {
        NS_NewURI(getter_AddRefs(base), nsCString(base_uri));
    }

    nsCOMPtr<nsIInputStream> stream;
    if (NS_FAILED(NS_NewCStringInputStream(getter_AddRefs(stream), html))) {
        return;
    }

    mozilla::dom::LoadURIOptions options;
    nsCOMPtr<nsIScriptSecurityManager> security_manager =
        do_GetService("@mozilla.org/scriptsecuritymanager;1");
    if (!security_manager ||
        NS_FAILED(security_manager->GetSystemPrincipal(
            getter_AddRefs(options.mTriggeringPrincipal)))) {
        return;
    }

    options.mLoadFlags = nsIWebNavigation::LOAD_FLAGS_NONE;
    navigation_->LoadURIFromStream(stream, base, nullptr, nullptr, options);
}

void GeckoEngine::load_uri(const char* uri) {
    if (!navigation_ || !uri || *uri == '\0') {
        return;
    }

    nsCOMPtr<nsIURI> target;
    if (NS_FAILED(NS_NewURI(getter_AddRefs(target), nsCString(uri))) || !target) {
        return;
    }

    mozilla::dom::LoadURIOptions options;
    nsCOMPtr<nsIScriptSecurityManager> security_manager =
        do_GetService("@mozilla.org/scriptsecuritymanager;1");
    if (!security_manager ||
        NS_FAILED(security_manager->GetSystemPrincipal(
            getter_AddRefs(options.mTriggeringPrincipal)))) {
        return;
    }

    options.mLoadFlags = nsIWebNavigation::LOAD_FLAGS_NONE;
    navigation_->LoadURI(target, options);
    current_uri_ = uri;

    if (callbacks_.uri_changed) {
        callbacks_.uri_changed(current_uri_.c_str());
    }
}

void GeckoEngine::stop_loading() {
    if (navigation_) {
        navigation_->Stop(nsIWebNavigation::STOP_ALL);
    }
}

void GeckoEngine::find_text(const std::string& text) {
    last_search_ = text;
}

void GeckoEngine::find_next() {}

void GeckoEngine::find_previous() {}

void GeckoEngine::clear_find() {
    last_search_.clear();
}

void GeckoEngine::apply_config(const Config&) {}

void GeckoEngine::setup_downloads(DownloadManager&) {}

void GeckoEngine::clear_site_data() {}
