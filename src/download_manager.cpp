#include "download_manager.hpp"
#include "download_backend.hpp"

#include <glib.h>
#include <utility>

DownloadManager::DownloadManager() {
    const char* downloads = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (downloads != nullptr) download_directory_ = downloads;
    else {
        gchar* fallback = g_build_filename(g_get_home_dir(), "Downloads", nullptr);
        download_directory_ = fallback;
        g_free(fallback);
    }
    if (download_directory_.empty()) download_directory_ = g_get_home_dir();
}

DownloadManager::~DownloadManager() = default;

void DownloadManager::set_backend(std::unique_ptr<DownloadBackend> backend) {
    backend_ = std::move(backend);
    if (backend_) backend_->attach(*this);
}

void DownloadManager::clear_backend() { backend_.reset(); }
const std::vector<Download>& DownloadManager::downloads() const { return downloads_; }
const std::string& DownloadManager::download_directory() const { return download_directory_; }
void DownloadManager::set_download_directory(const std::string& path) { if (!path.empty()) download_directory_ = path; }
void DownloadManager::set_ask_download_location(bool enabled) { ask_download_location_ = enabled; }
void DownloadManager::set_parent_window(GtkWindow* window) { parent_window_ = window; }
bool DownloadManager::ask_download_location() const { return ask_download_location_; }
GtkWindow* DownloadManager::parent_window() const { return parent_window_; }
void DownloadManager::add_download(Download entry) { downloads_.push_back(std::move(entry)); }

void DownloadManager::mark_download_path(const std::string& uri, const std::string& path) {
    for (auto it = downloads_.rbegin(); it != downloads_.rend(); ++it) {
        if (!it->path.empty()) continue;
        if (uri.empty() || it->uri == uri) {
            it->path = path;
            if (!path.empty()) {
                gchar* basename = g_path_get_basename(path.c_str());
                if (basename != nullptr && *basename != '\0') it->filename = basename;
                g_free(basename);
            }
            break;
        }
    }
}

void DownloadManager::mark_download_finished(const std::string& uri, const std::string& path) {
    for (auto it = downloads_.rbegin(); it != downloads_.rend(); ++it) {
        if (it->completed) continue;
        if (uri.empty() || it->uri == uri) {
            it->completed = true;
            if (!path.empty()) it->path = path;
            break;
        }
    }
}

void DownloadManager::report_download_failed(const std::string& message) {
    g_warning("EXTART: download failed: %s", message.c_str());
}

void DownloadManager::clear_old_downloads() {
    if (downloads_.size() <= 50) return;
    downloads_.erase(downloads_.begin(), downloads_.end() - 50);
}
