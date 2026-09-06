#pragma once

#include <gtk/gtk.h>
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
    time_t timestamp = 0;
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
    void set_ask_download_location(bool enabled);
    void set_parent_window(GtkWindow* window);
    void clear_old_downloads();

private:
    struct PendingDestination {
        DownloadManager* manager = nullptr;
        WebKitDownload* download = nullptr;
        std::string uri;
    };

    static void on_download_started(WebKitNetworkSession* session,
                                    WebKitDownload* download,
                                    gpointer user_data);
    static gboolean on_download_decide_destination(WebKitDownload* download,
                                                    const gchar* suggested_filename,
                                                    gpointer user_data);
    static void on_destination_selected(GObject* source,
                                        GAsyncResult* result,
                                        gpointer user_data);
    static void on_download_finished(WebKitDownload* download,
                                     gpointer user_data);
    static void on_download_failed(WebKitDownload* download,
                                   GError* error,
                                   gpointer user_data);

    std::vector<Download> downloads_;
    std::string download_directory_;
    WebKitNetworkSession* network_session_ = nullptr;
    GtkWindow* parent_window_ = nullptr;
    bool ask_download_location_ = false;
};
