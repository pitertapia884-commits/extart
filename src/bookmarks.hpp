#pragma once

#include <string>
#include <vector>

struct Bookmark {
    std::string title;
    std::string url;
};

class Bookmarks {
public:
    Bookmarks();
    
    void add(const std::string& url, const std::string& title);
    void remove(const std::string& url);
    void load();
    bool save() const;
    const std::vector<Bookmark>& bookmarks() const;
    bool is_bookmarked(const std::string& url) const;
    
private:
    static std::string path();
    
    std::vector<Bookmark> bookmarks_;
};
