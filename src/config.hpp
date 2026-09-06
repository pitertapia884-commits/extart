#pragma once

#include <string>

class Config {
public:
    Config();

    void load();
    bool save() const;

    const std::string& search_engine() const;
    const std::string& download_directory() const;
    const std::string& homepage() const;

    bool ask_download_location() const;
    bool restore_session() const;
    bool javascript_enabled() const;
    bool images_enabled() const;
    bool sound_enabled() const;
    bool popups_enabled() const;

    void set_search_engine(std::string value);
    void set_download_directory(std::string value);
    void set_homepage(std::string value);
    void set_ask_download_location(bool value);
    void set_restore_session(bool value);
    void set_javascript_enabled(bool value);
    void set_images_enabled(bool value);
    void set_sound_enabled(bool value);
    void set_popups_enabled(bool value);

private:
    static std::string path();
    void reset_defaults();

    std::string search_engine_;
    std::string download_directory_;
    std::string homepage_;
    bool ask_download_location_ = false;
    bool restore_session_ = false;
    bool javascript_enabled_ = true;
    bool images_enabled_ = true;
    bool sound_enabled_ = true;
    bool popups_enabled_ = true;
};
