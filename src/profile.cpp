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
}

Profile::~Profile() {
    g_clear_object(&network_session_);
}

WebKitNetworkSession* Profile::network_session() const {
    return network_session_;
}
