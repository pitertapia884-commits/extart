#include "config.hpp"

#include <glib.h>

#include <utility>

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

    homepage_ = "extart://home/";
    ask_download_location_ = false;
    restore_session_ = false;
    javascript_enabled_ = true;
    images_enabled_ = true;
    sound_enabled_ = true;
    popups_enabled_ = true;
}

std::string Config::path() {
    gchar* value = g_build_filename(
        g_get_user_config_dir(), "extart", "settings.ini", nullptr);
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
        gchar* value = g_key_file_get_string(
            key_file, "General", key, &read_error);
        if (value != nullptr && value[0] != '\0') {
            target = value;
        }
        g_free(value);
        g_clear_error(&read_error);
    };

    auto read_boolean = [key_file](const char* key, bool& target) {
        GError* read_error = nullptr;
        target = g_key_file_get_boolean(key_file, "General", key, &read_error);
        g_clear_error(&read_error);
    };

    read_string("search-engine", search_engine_);
    read_string("download-directory", download_directory_);
    read_string("homepage", homepage_);
    read_boolean("ask-download-location", ask_download_location_);
    read_boolean("restore-session", restore_session_);
    read_boolean("javascript", javascript_enabled_);
    read_boolean("images", images_enabled_);
    read_boolean("sound", sound_enabled_);
    read_boolean("popups", popups_enabled_);

    if (homepage_.empty()) {
        homepage_ = "extart://home/";
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
    g_key_file_set_string(key_file, "General", "homepage", homepage_.c_str());
    g_key_file_set_boolean(key_file, "General", "ask-download-location", ask_download_location_);
    g_key_file_set_boolean(key_file, "General", "restore-session", restore_session_);
    g_key_file_set_boolean(key_file, "General", "javascript", javascript_enabled_);
    g_key_file_set_boolean(key_file, "General", "images", images_enabled_);
    g_key_file_set_boolean(key_file, "General", "sound", sound_enabled_);
    g_key_file_set_boolean(key_file, "General", "popups", popups_enabled_);

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

const std::string& Config::search_engine() const { return search_engine_; }
const std::string& Config::download_directory() const { return download_directory_; }
const std::string& Config::homepage() const { return homepage_; }

bool Config::ask_download_location() const { return ask_download_location_; }
bool Config::restore_session() const { return restore_session_; }
bool Config::javascript_enabled() const { return javascript_enabled_; }
bool Config::images_enabled() const { return images_enabled_; }
bool Config::sound_enabled() const { return sound_enabled_; }
bool Config::popups_enabled() const { return popups_enabled_; }

void Config::set_search_engine(std::string value) {
    if (!value.empty()) search_engine_ = std::move(value);
}

void Config::set_download_directory(std::string value) {
    if (!value.empty()) download_directory_ = std::move(value);
}

void Config::set_homepage(std::string value) {
    if (!value.empty()) homepage_ = std::move(value);
}

void Config::set_ask_download_location(bool value) { ask_download_location_ = value; }
void Config::set_restore_session(bool value) { restore_session_ = value; }
void Config::set_javascript_enabled(bool value) { javascript_enabled_ = value; }
void Config::set_images_enabled(bool value) { images_enabled_ = value; }
void Config::set_sound_enabled(bool value) { sound_enabled_ = value; }
void Config::set_popups_enabled(bool value) { popups_enabled_ = value; }
