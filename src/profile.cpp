#include "profile.hpp"

#include <glib.h>

#include <system_error>

namespace {
std::filesystem::path user_path(const char* base, const char* leaf) {
    return std::filesystem::path(base) / "extart" / leaf;
}
}

Profile::Profile()
    : data_directory_(user_path(g_get_user_data_dir(), "web-data")),
      cache_directory_(user_path(g_get_user_cache_dir(), "web-cache")),
      cookies_path_(user_path(g_get_user_data_dir(), "cookies.sqlite")) {
    std::error_code error;
    std::filesystem::create_directories(data_directory_, error);
    if (error) return;
    std::filesystem::create_directories(cache_directory_, error);
}
