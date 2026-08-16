#include "config.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstdlib>

namespace fs = std::filesystem;

Config::Config() {
    search_engine = "https://duckduckgo.com/?q=";
    download_dir  = std::string(getenv("HOME")) + "/Downloads";
}

Config& Config::get() {
    static Config instance;
    return instance;
}

std::string Config::config_path() {
    return std::string(getenv("HOME")) + "/.config/extart/config.json";
}

void Config::load() {
    std::ifstream f(config_path());
    if (!f.is_open()) return;

    std::string line, key, value;
    while (std::getline(f, line)) {
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        key   = line.substr(0, colon);
        value = line.substr(colon + 1);

        // Limpiar comillas y espacios
        auto clean = [](std::string& s) {
            s.erase(remove(s.begin(), s.end(), '"'), s.end());
            s.erase(remove(s.begin(), s.end(), ' '), s.end());
            s.erase(remove(s.begin(), s.end(), ','), s.end());
        };
        clean(key); clean(value);

        if (key == "search_engine") search_engine = value;
        if (key == "download_dir")  download_dir  = value;
    }
}

void Config::save() {
    std::string path = config_path();
    fs::create_directories(fs::path(path).parent_path());

    std::ofstream f(path);
    f << "{\n";
    f << "  \"search_engine\": \"" << search_engine << "\",\n";
    f << "  \"download_dir\": \"" << download_dir << "\"\n";
    f << "}\n";
}