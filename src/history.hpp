#pragma once

#include <string>
#include <vector>
#include <ctime>

struct HistoryEntry {
    std::string url;
    std::string title;
    time_t timestamp;
};

class History {
public:
    History();
    
    void add(const std::string& url, const std::string& title);
    void load();
    bool save() const;
    const std::vector<HistoryEntry>& entries() const;
    void clear();
    
private:
    static std::string path();
    
    std::vector<HistoryEntry> entries_;
};
