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
#include <mozilla/gfx/2D.h>
#include <mozilla/gfx/Factory.h>
#include <gfxContext.h>
#include <WindowRenderer.h>
#include <PuppetWidget.h>
#include <cstring>
#include <memory>

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

    render_widget_ = gtk_drawing_area_new();
    gtk_widget_set_hexpand(render_widget_, TRUE);
    gtk_widget_set_vexpand(render_widget_, TRUE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(render_widget_),
                                   draw_render_surface, this, nullptr);
    render_source_id_ = g_timeout_add(16, render_tick, this);

    // Force creation of Gecko's native WindowRenderer. PuppetWidget uses a
    // fallback renderer in the parent process at this embedding boundary;
    // keeping this explicit prevents the GTK bridge from silently operating
    // without a compositor object.
    if (gecko_widget_->GetWindowRenderer() == nullptr) {
        gecko_widget_->Show(false);
        gecko_widget_->Release();
        gecko_widget_ = nullptr;
        windowless_browser_->Close();
        windowless_browser_->Release();
        windowless_browser_ = nullptr;
        navigation_->Release();
        navigation_ = nullptr;
    }
}

GeckoEngine::~GeckoEngine() {
    if (render_source_id_ != 0) {
        g_source_remove(render_source_id_);
        render_source_id_ = 0;
    }
    render_target_ = nullptr;
    render_pixels_.reset();
    render_widget_ = nullptr;

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
    return render_widget_;
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


void GeckoEngine::ensure_render_target(int width, int height) {
    if (width <= 0 || height <= 0 || !gecko_widget_) return;
    if (render_target_ && render_width_ == width && render_height_ == height) return;

    if (render_target_) {
        render_target_->Release();
        render_target_ = nullptr;
    }
    render_width_ = width;
    render_height_ = height;
    render_stride_ = static_cast<std::size_t>(width) * 4u;
    render_pixels_ = std::make_unique<unsigned char[]>(
        render_stride_ * static_cast<std::size_t>(height));
    std::memset(render_pixels_.get(), 0,
                render_stride_ * static_cast<std::size_t>(height));

    RefPtr<mozilla::gfx::DrawTarget> target =
        mozilla::gfx::Factory::CreateDrawTargetForData(
            mozilla::gfx::BackendType::CAIRO,
            render_pixels_.get(),
            mozilla::gfx::IntSize(width, height),
            static_cast<int32_t>(render_stride_),
            mozilla::gfx::SurfaceFormat::B8G8R8A8);
    if (!target) return;

    render_target_ = target.get();
    render_target_->AddRef();
    gecko_widget_->Resize(width, height, true);
}

void GeckoEngine::render_frame() {
    if (!gecko_widget_ || !render_target_) return;

    auto* puppet =
        static_cast<mozilla::widget::PuppetWidget*>(gecko_widget_);
    if (!puppet) return;

    auto* renderer = gecko_widget_->GetWindowRenderer();
    if (!renderer) return;

    if (auto* fallback = renderer->AsFallback()) {
        gfxContext context(render_target_);
        fallback->SetTarget(&context);
        puppet->Paint();
        fallback->SetTarget(nullptr);
    }
}

void GeckoEngine::draw_render_surface(GtkDrawingArea*, cairo_t* cr,
                                      int width, int height,
                                      gpointer user_data) {
    auto* engine = static_cast<GeckoEngine*>(user_data);
    if (!engine) return;

    engine->ensure_render_target(width, height);
    engine->render_frame();
    if (!engine->render_pixels_) return;

    cairo_surface_t* surface = cairo_image_surface_create_for_data(
        engine->render_pixels_.get(),
        CAIRO_FORMAT_ARGB32,
        engine->render_width_,
        engine->render_height_,
        static_cast<int>(engine->render_stride_));
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    cairo_surface_destroy(surface);
}

gboolean GeckoEngine::render_tick(gpointer user_data) {
    auto* engine = static_cast<GeckoEngine*>(user_data);
    if (!engine || !engine->render_widget_) return G_SOURCE_REMOVE;
    engine->render_frame();
    gtk_widget_queue_draw(engine->render_widget_);
    return G_SOURCE_CONTINUE;
}
