#pragma once
#include <string>

class Config {
public:
    static Config& get();

    std::string search_engine;
    std::string download_dir;

    void load();
    void save();

private:
    Config();
    static std::string config_path();
};