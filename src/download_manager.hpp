#pragma once

#include <webkit/webkit.h>
#include <string>
#include <vector>
#include <ctime>

struct Download {
    std::string filename;
    std::string path;
    std::string uri;
    uint64_t size_bytes = 0;
    uint64_t downloaded_bytes = 0;
    time_t timestamp;
    bool completed = false;
};

class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();
    
    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;
    
    void setup_for_web_view(WebKitWebView* web_view);
    const std::vector<Download>& downloads() const;
    const std::string& download_directory() const;
    void set_download_directory(const std::string& path);
    void clear_old_downloads();
    
private:
    static void on_download_started(WebKitWebContext* context, WebKitDownload* download, 
                                   gpointer user_data);
    static gboolean on_download_decide_destination(WebKitDownload* download, 
                                                  const gchar* suggested_filename,
                                                  gpointer user_data);
    static void on_download_finished(WebKitDownload* download, gpointer user_data);
    static void on_download_failed(WebKitDownload* download, gpointer user_data);
    
    std::vector<Download> downloads_;
    std::string download_directory_;
};
