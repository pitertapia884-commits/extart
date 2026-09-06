#include "profile.hpp"

#include <glib.h>
#include <string>

namespace {
std::string build_path(const char* base, const char* leaf) {
    gchar* path = g_build_filename(base, "extart", leaf, nullptr);
    std::string result(path);
    g_free(path);
    return result;
}
}

Profile::Profile() {
    const std::string data_directory = build_path(g_get_user_data_dir(), "web-data");
    const std::string cache_directory = build_path(g_get_user_cache_dir(), "web-cache");
    const std::string cookies_path = build_path(g_get_user_data_dir(), "cookies.sqlite");

    g_mkdir_with_parents(data_directory.c_str(), 0700);
    g_mkdir_with_parents(cache_directory.c_str(), 0700);

    network_session_ = webkit_network_session_new(
        data_directory.c_str(), cache_directory.c_str());

    WebKitCookieManager* cookies = webkit_network_session_get_cookie_manager(network_session_);
    webkit_cookie_manager_set_persistent_storage(
        cookies,
        cookies_path.c_str(),
        WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);

    // Give WebKit's web processes an explicit memory-pressure policy instead
    // of relying only on the machine-wide defaults. This asks WebKit to start
    // reclaiming non-critical memory well before an 8 GiB machine is exhausted,
    // without imposing a hard kill threshold on normal pages.
    WebKitMemoryPressureSettings* pressure = webkit_memory_pressure_settings_new();
    webkit_memory_pressure_settings_set_memory_limit(pressure, 2048);
    webkit_memory_pressure_settings_set_conservative_threshold(pressure, 0.40);
    webkit_memory_pressure_settings_set_strict_threshold(pressure, 0.65);
    webkit_memory_pressure_settings_set_kill_threshold(pressure, 0.0);
    webkit_memory_pressure_settings_set_poll_interval(pressure, 15.0);

    web_context_ = WEBKIT_WEB_CONTEXT(g_object_new(
        WEBKIT_TYPE_WEB_CONTEXT,
        "memory-pressure-settings", pressure,
        nullptr));

    webkit_memory_pressure_settings_free(pressure);

    // DOCUMENT_BROWSER keeps a moderate amount of cache. DOCUMENT_VIEWER
    // disables remote caching and made normal browsing unnecessarily slow.
    webkit_web_context_set_cache_model(
        web_context_,
        WEBKIT_CACHE_MODEL_DOCUMENT_BROWSER
    );
}

Profile::~Profile() {
    g_clear_object(&web_context_);
    g_clear_object(&network_session_);
}

WebKitNetworkSession* Profile::network_session() const {
    return network_session_;
}

WebKitWebContext* Profile::web_context() const {
    return web_context_;
}
