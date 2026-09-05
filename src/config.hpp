#pragma once

#include <string>

class Config {
public:
    Config();

    void load();
    bool save() const;

    const std::string& search_engine() const;
    const std::string& download_directory() const;
    const std::string& theme() const;

    void set_search_engine(std::string value);
    void set_download_directory(std::string value);
    void set_theme(std::string value);

private:
    static std::string path();
    void reset_defaults();

    std::string search_engine_;
    std::string download_directory_;
    std::string theme_;
};
