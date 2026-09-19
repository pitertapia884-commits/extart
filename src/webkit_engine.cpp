#include "browser_engine.hpp"

#include "config.hpp"
#include "download_backend.hpp"
#include "download_manager.hpp"
#include "profile.hpp"

#include <webkit/webkit.h>

#include <utility>

namespace {

WebKitUserContentManager* create_whatsapp_compatibility_manager() {
    auto* manager = webkit_user_content_manager_new();
    static const char* const script_source = R"JS(
(function () {
    const host = window.location.hostname;
    if (!host || !(host === "whatsapp.com" || host.endsWith(".whatsapp.com"))) return;
    const chromeUA = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
    const override = (object, property, value) => { try { Object.defineProperty(object, property, { configurable: true, get: () => value }); } catch (_) {} };
    override(navigator, "userAgent", chromeUA);
    override(navigator, "appVersion", chromeUA.substring(8));
    override(navigator, "platform", "Linux x86_64");
    override(navigator, "vendor", "Google Inc.");
    if (!navigator.userAgentData) {
        const brands = [{ brand: "Not_A Brand", version: "99" }, { brand: "Chromium", version: "140" }, { brand: "Google Chrome", version: "140" }];
        override(navigator, "userAgentData", { brands, mobile: false, platform: "Linux", getHighEntropyValues: async () => ({ brands, mobile: false, platform: "Linux", platformVersion: "0.0.0", architecture: "x86", bitness: "64", model: "", uaFullVersion: "140.0.0.0", fullVersionList: brands }) });
    }
    if (!window.chrome) { try { Object.defineProperty(window, "chrome", { configurable: true, value: { runtime: {} } }); } catch (_) {} }
})();
)JS";
    const char* const allow_list[] = { "https://whatsapp.com/*", "https://*.whatsapp.com/*", nullptr };
    auto* script = webkit_user_script_new(script_source, WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, allow_list, nullptr);
    webkit_user_content_manager_add_script(manager, script);
    webkit_user_script_unref(script);
    return manager;
}

} // namespace

WebKitEngine::WebKitEngine(Profile& profile, const Config& config)
: profile_(profile) {
    WebKitUserContentManager* user_content_manager = create_whatsapp_compatibility_manager();
    web_view_ = GTK_WIDGET(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "web-context", profile.web_context(),
        "network-session", profile.network_session(),
        "user-content-manager", user_content_manager,
        nullptr));
    g_object_unref(user_content_manager);

    gtk_widget_set_hexpand(web_view_, TRUE);
    gtk_widget_set_vexpand(web_view_, TRUE);
    apply_config(config);
    find_controller_ = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(web_view_));
    g_signal_connect(web_view_, "load-changed", G_CALLBACK(on_load_changed), this);
}

WebKitEngine::~WebKitEngine() {
    find_controller_ = nullptr;
    web_view_ = nullptr;
}

GtkWidget* WebKitEngine::widget() const { return web_view_; }
gpointer WebKitEngine::native_handle() const { return web_view_; }
void WebKitEngine::set_callbacks(Callbacks callbacks) { callbacks_ = std::move(callbacks); }

std::string WebKitEngine::current_uri() const {
    if (web_view_ == nullptr) return {};
    const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(web_view_));
    return uri ? uri : "";
}

std::string WebKitEngine::current_title() const {
    if (web_view_ == nullptr) return {};
    const char* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(web_view_));
    return title ? title : "";
}

bool WebKitEngine::can_go_back() const {
    return web_view_ != nullptr && webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view_));
}

bool WebKitEngine::can_go_forward() const {
    return web_view_ != nullptr && webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view_));
}

void WebKitEngine::go_back() {
    if (can_go_back()) webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view_));
}

void WebKitEngine::go_forward() {
    if (can_go_forward()) webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view_));
}

void WebKitEngine::reload() {
    if (web_view_ != nullptr) webkit_web_view_reload(WEBKIT_WEB_VIEW(web_view_));
}

void WebKitEngine::load_html(const std::string& html, const char* base_uri) {
    if (web_view_ == nullptr) return;
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view_), html.c_str(), base_uri);
}

void WebKitEngine::load_uri(const char* uri) {
    if (web_view_ == nullptr || uri == nullptr || *uri == '\0') return;
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view_), uri);
}

void WebKitEngine::stop_loading() {
    if (web_view_ != nullptr) webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(web_view_));
}

void WebKitEngine::find_text(const std::string& text) {
    if (find_controller_ == nullptr) return;
    last_search_ = text;
    if (text.empty()) {
        webkit_find_controller_search_finish(find_controller_);
        return;
    }
    webkit_find_controller_search(
        find_controller_,
        text.c_str(),
        WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE,
        G_MAXUINT);
}

void WebKitEngine::find_next() {
    if (find_controller_ != nullptr && !last_search_.empty())
        webkit_find_controller_search_next(find_controller_);
}

void WebKitEngine::find_previous() {
    if (find_controller_ != nullptr && !last_search_.empty())
        webkit_find_controller_search_previous(find_controller_);
}

void WebKitEngine::clear_find() {
    if (find_controller_ == nullptr) return;
    webkit_find_controller_search_finish(find_controller_);
    last_search_.clear();
}

void WebKitEngine::apply_config(const Config& config) {
    if (web_view_ == nullptr) return;
    WebKitSettings* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view_));
    if (settings == nullptr) return;

    webkit_settings_set_enable_javascript(settings, config.javascript_enabled());
    webkit_settings_set_auto_load_images(settings, config.images_enabled());
    webkit_settings_set_enable_media(settings, config.sound_enabled());
    webkit_settings_set_enable_webaudio(settings, config.sound_enabled());
    webkit_settings_set_javascript_can_open_windows_automatically(settings, config.popups_enabled());
    webkit_settings_set_enable_page_cache(settings, TRUE);
    webkit_settings_set_enable_site_specific_quirks(settings, FALSE);
    webkit_settings_set_user_agent(
        settings,
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36");
}

void WebKitEngine::setup_downloads(DownloadManager& manager) {
    if (web_view_ == nullptr) return;
    manager.set_backend(make_webkit_download_backend(profile_.network_session()));
}

void WebKitEngine::clear_site_data() {
    WebKitWebsiteDataManager* manager =
        webkit_network_session_get_website_data_manager(profile_.network_session());

    if (manager == nullptr) return;

    webkit_website_data_manager_clear(
        manager,
        WEBKIT_WEBSITE_DATA_ALL,
        0,
        nullptr,
        nullptr,
        nullptr);
}

void WebKitEngine::on_load_changed(WebKitWebView* view, int event, gpointer user_data) {
    auto* engine = static_cast<WebKitEngine*>(user_data);
    if (engine == nullptr) return;

    if (event == WEBKIT_LOAD_COMMITTED && engine->callbacks_.uri_changed)
        engine->callbacks_.uri_changed(webkit_web_view_get_uri(view));

    if (event == WEBKIT_LOAD_FINISHED) {
        if (engine->callbacks_.title_changed)
            engine->callbacks_.title_changed(webkit_web_view_get_title(view));
        if (engine->callbacks_.load_finished)
            engine->callbacks_.load_finished();
    }
}

std::unique_ptr<BrowserEngine> make_browser_engine(Profile& profile, const Config& config) {
    return std::make_unique<WebKitEngine>(profile, config);
}
