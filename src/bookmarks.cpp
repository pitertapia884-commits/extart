#include "bookmarks.hpp"

#include <glib.h>
#include <fstream>

Bookmarks::Bookmarks() = default;

std::string Bookmarks::path() {
    gchar* value = g_build_filename(g_get_user_data_dir(), "extart", "bookmarks.csv", nullptr);
    std::string result(value);
    g_free(value);
    return result;
}

void Bookmarks::add(const std::string& url, const std::string& title) {
    if (url.empty()) {
        return;
    }
    
    // Evitar duplicados
    for (const auto& bm : bookmarks_) {
        if (bm.url == url) {
            return;
        }
    }
    
    bookmarks_.push_back({title.empty() ? url : title, url});
    save();
}

void Bookmarks::remove(const std::string& url) {
    for (auto it = bookmarks_.begin(); it != bookmarks_.end(); ++it) {
        if (it->url == url) {
            bookmarks_.erase(it);
            save();
            return;
        }
    }
}

void Bookmarks::load() {
    bookmarks_.clear();
    std::ifstream file(path());
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t pos = line.find('|');
        if (pos != std::string::npos) {
            std::string title = line.substr(0, pos);
            std::string url = line.substr(pos + 1);
            bookmarks_.push_back({title, url});
        }
    }
}

bool Bookmarks::save() const {
    gchar* directory = g_build_filename(g_get_user_data_dir(), "extart", nullptr);
    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_free(directory);
        return false;
    }
    g_free(directory);
    
    std::ofstream file(path());
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& bm : bookmarks_) {
        file << bm.title << "|" << bm.url << "\n";
    }
    
    return true;
}

const std::vector<Bookmark>& Bookmarks::bookmarks() const {
    return bookmarks_;
}

bool Bookmarks::is_bookmarked(const std::string& url) const {
    for (const auto& bm : bookmarks_) {
        if (bm.url == url) {
            return true;
        }
    }
    return false;
}
