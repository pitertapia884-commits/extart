#include "config.hpp"

#include <glib.h>

Config::Config() {
    reset_defaults();
}

void Config::reset_defaults() {
    search_engine_ = "https://www.google.com/search?q=";
    const char* downloads = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (downloads != nullptr) {
        download_directory_ = downloads;
    } else {
        download_directory_.clear();
    }
    if (download_directory_.empty()) {
        gchar* fallback = g_build_filename(g_get_home_dir(), "Downloads", nullptr);
        download_directory_ = fallback;
        g_free(fallback);
    }
    theme_ = "system";
}

std::string Config::path() {
    gchar* value = g_build_filename(g_get_user_config_dir(), "extart", "settings.ini", nullptr);
    std::string result(value);
    g_free(value);
    return result;
}

void Config::load() {
    reset_defaults();

    GKeyFile* key_file = g_key_file_new();
    GError* error = nullptr;
    if (!g_key_file_load_from_file(key_file, path().c_str(), G_KEY_FILE_NONE, &error)) {
        g_clear_error(&error);
        g_key_file_unref(key_file);
        return;
    }

    auto read_string = [key_file](const char* key, std::string& target) {
        GError* read_error = nullptr;
        gchar* value = g_key_file_get_string(key_file, "General", key, &read_error);
        if (value != nullptr && value[0] != '\0') {
            target = value;
        }
        g_free(value);
        g_clear_error(&read_error);
    };

    read_string("search-engine", search_engine_);
    read_string("download-directory", download_directory_);
    read_string("theme", theme_);

    if (theme_ != "system" && theme_ != "light" && theme_ != "dark") {
        theme_ = "system";
    }

    g_key_file_unref(key_file);
}

bool Config::save() const {
    gchar* directory = g_build_filename(g_get_user_config_dir(), "extart", nullptr);
    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_free(directory);
        return false;
    }
    g_free(directory);

    GKeyFile* key_file = g_key_file_new();
    g_key_file_set_string(key_file, "General", "search-engine", search_engine_.c_str());
    g_key_file_set_string(key_file, "General", "download-directory", download_directory_.c_str());
    g_key_file_set_string(key_file, "General", "theme", theme_.c_str());

    gsize length = 0;
    GError* error = nullptr;
    gchar* data = g_key_file_to_data(key_file, &length, &error);
    const bool saved = data != nullptr &&
        g_file_set_contents(path().c_str(), data, static_cast<gssize>(length), &error);

    g_free(data);
    g_clear_error(&error);
    g_key_file_unref(key_file);
    return saved;
}

const std::string& Config::search_engine() const {
    return search_engine_;
}

const std::string& Config::download_directory() const {
    return download_directory_;
}

const std::string& Config::theme() const {
    return theme_;
}

void Config::set_search_engine(std::string value) {
    if (!value.empty()) {
        search_engine_ = std::move(value);
    }
}

void Config::set_download_directory(std::string value) {
    if (!value.empty()) {
        download_directory_ = std::move(value);
    }
}

void Config::set_theme(std::string value) {
    if (value == "system" || value == "light" || value == "dark") {
        theme_ = std::move(value);
    }
}
