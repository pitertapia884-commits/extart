#include "history.hpp"

#include <glib.h>

#include <fstream>
#include <string>

History::History() = default;

std::string History::path() {
gchar* value = g_build_filename(
g_get_user_data_dir(),
"extart",
"history.csv",
nullptr
);


std::string result(value);
g_free(value);

return result;


}

void History::add(
const std::string& url,
const std::string& title
) {
if (url.empty()) {
return;
}


if (url == "about:blank" ||
    url.rfind("extart://", 0) == 0) {
    return;
}

// Evitar registrar repetidamente la misma URL
// si el usuario recarga la página.
if (!entries_.empty() &&
    entries_.back().url == url) {

    entries_.back().title =
        title.empty()
            ? entries_.back().title
            : title;

    entries_.back().timestamp =
        std::time(nullptr);

    save();
    return;
}

entries_.push_back({
    url,
    title.empty() ? url : title,
    std::time(nullptr)
});

// Mantener solamente las últimas 1000 entradas.
if (entries_.size() > 1000) {
    entries_.erase(
        entries_.begin(),
        entries_.begin() +
            (entries_.size() - 1000)
    );
}

save();


}

void History::load() {
entries_.clear();


std::ifstream file(path());

if (!file.is_open()) {
    return;
}

std::string line;

while (std::getline(file, line)) {
    if (line.empty()) {
        continue;
    }

    const size_t pos1 =
        line.find('|');

    if (pos1 == std::string::npos) {
        continue;
    }

    const size_t pos2 =
        line.find('|', pos1 + 1);

    if (pos2 == std::string::npos) {
        continue;
    }

    try {
        const std::string url =
            line.substr(0, pos1);

        const std::string title =
            line.substr(
                pos1 + 1,
                pos2 - pos1 - 1
            );

        const time_t timestamp =
            static_cast<time_t>(
                std::stoll(
                    line.substr(pos2 + 1)
                )
            );

        if (!url.empty()) {
            entries_.push_back({
                url,
                title,
                timestamp
            });
        }
    } catch (...) {
        // Ignorar entradas corruptas.
        continue;
    }
}

if (entries_.size() > 1000) {
    entries_.erase(
        entries_.begin(),
        entries_.end() - 1000
    );
}


}

bool History::save() const {
gchar* directory = g_build_filename(
g_get_user_data_dir(),
"extart",
nullptr
);


if (g_mkdir_with_parents(directory, 0700) != 0) {
    g_free(directory);
    return false;
}

g_free(directory);

std::ofstream file(path());

if (!file.is_open()) {
    return false;
}

for (const auto& entry : entries_) {
    file << entry.url
         << "|"
         << entry.title
         << "|"
         << entry.timestamp
         << "\n";
}

return true;


}

const std::vector<HistoryEntry>& History::entries() const {
return entries_;
}

void History::clear() {
entries_.clear();
save();
}
