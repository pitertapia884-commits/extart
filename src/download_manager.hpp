#pragma once

#include <gtk/gtk.h>

#include <ctime>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class DownloadBackend;

struct Download {
    std::string filename;
    std::string path;
    std::string uri;
    uint64_t size_bytes = 0;
    uint64_t downloaded_bytes = 0;
    time_t timestamp = 0;
    bool completed = false;
};

class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();

    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;

    void set_backend(std::unique_ptr<DownloadBackend> backend);
    void clear_backend();

    const std::vector<Download>& downloads() const;
    const std::string& download_directory() const;
    void set_download_directory(const std::string& path);
    void set_ask_download_location(bool enabled);
    void set_parent_window(GtkWindow* window);
    void clear_old_downloads();

    bool ask_download_location() const;
    GtkWindow* parent_window() const;

    void add_download(Download entry);
    void mark_download_path(const std::string& uri, const std::string& path);
    void mark_download_finished(const std::string& uri, const std::string& path);
    void report_download_failed(const std::string& message);

private:
    std::vector<Download> downloads_;
    std::string download_directory_;
    GtkWindow* parent_window_ = nullptr;
    bool ask_download_location_ = false;
    std::unique_ptr<DownloadBackend> backend_;
};
