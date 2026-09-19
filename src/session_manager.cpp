#include "session_manager.hpp"

#include "browser_window.hpp"

#include <glib.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::string session_path() {
    const char* config_dir = g_get_user_config_dir();
    return (std::filesystem::path(config_dir) / "extart" / "session.ini").string();
}

bool valid_session_uri(const std::string& uri) {
    return !uri.empty() && uri != "about:blank" && uri.rfind("extart://", 0) != 0;
}

} // namespace

void session_save(const BrowserWindow& window) {
    const std::filesystem::path path(session_path());
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) return;

    std::ofstream file(path);
    if (!file) return;

    file << "[Session]\n";
    int index = 0;
    for (const std::string& uri : window.session_uris()) {
        if (!valid_session_uri(uri)) continue;
        file << "tab" << index++ << "=" << uri << "\n";
    }
}

void session_restore(BrowserWindow& window) {
    std::ifstream file(session_path());
    if (!file) return;

    std::vector<std::string> uris;
    std::string line;
    while (std::getline(file, line)) {
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos || line.rfind("tab", 0) != 0) continue;
        const std::string uri = line.substr(equals + 1);
        if (valid_session_uri(uri)) uris.push_back(uri);
    }
    window.restore_session_uris(uris);
}
