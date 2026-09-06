#include "download_manager.hpp"

#include <glib.h>

#include <ctime>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

DownloadManager::DownloadManager() {
    const char* downloads =
        g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);

    if (downloads != nullptr) {
        download_directory_ = downloads;
    } else {
        gchar* fallback =
            g_build_filename(g_get_home_dir(), "Downloads", nullptr);

        download_directory_ = fallback;
        g_free(fallback);
    }

    if (download_directory_.empty()) {
        download_directory_ = g_get_home_dir();
    }
}

DownloadManager::~DownloadManager() {
    if (network_session_ != nullptr) {
        g_signal_handlers_disconnect_by_data(
            network_session_,
            this
        );
        network_session_ = nullptr;
    }
}

void DownloadManager::setup_for_web_view(WebKitWebView* web_view) {
    if (web_view == nullptr) {
        return;
    }

    WebKitNetworkSession* session =
        webkit_web_view_get_network_session(web_view);

    if (session == nullptr) {
        return;
    }

    // All EXTART tabs share the same NetworkSession.
    // Connect the download signal only once.
    if (network_session_ == session) {
        return;
    }

    if (network_session_ != nullptr) {
        g_signal_handlers_disconnect_by_data(
            network_session_,
            this
        );
    }

    network_session_ = session;

    g_signal_connect(
        network_session_,
        "download-started",
        G_CALLBACK(on_download_started),
        this
    );
}

const std::vector<Download>& DownloadManager::downloads() const {
    return downloads_;
}

const std::string& DownloadManager::download_directory() const {
    return download_directory_;
}

void DownloadManager::set_download_directory(const std::string& path) {
    if (!path.empty()) {
        download_directory_ = path;
    }
}

void DownloadManager::clear_old_downloads() {
    if (downloads_.size() <= 50) {
        return;
    }

    downloads_.erase(
        downloads_.begin(),
        downloads_.end() - 50
    );
}

void DownloadManager::on_download_started(
    WebKitNetworkSession*,
    WebKitDownload* download,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    if (manager == nullptr || download == nullptr) {
        return;
    }

    WebKitURIResponse* response =
        webkit_download_get_response(download);

    const char* suggested =
        response != nullptr
            ? webkit_uri_response_get_suggested_filename(response)
            : nullptr;

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request != nullptr
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    Download entry{};
    entry.filename =
        suggested != nullptr && *suggested != '\0'
            ? suggested
            : "download";
    entry.uri = uri != nullptr ? uri : "";
    entry.timestamp = std::time(nullptr);
    entry.completed = false;

    manager->downloads_.push_back(entry);

    g_signal_connect(
        download,
        "decide-destination",
        G_CALLBACK(on_download_decide_destination),
        manager
    );

    g_signal_connect(
        download,
        "finished",
        G_CALLBACK(on_download_finished),
        manager
    );

    g_signal_connect(
        download,
        "failed",
        G_CALLBACK(on_download_failed),
        manager
    );

    g_message(
        "EXTART: download detected: %s",
        entry.filename.c_str()
    );
}

gboolean DownloadManager::on_download_decide_destination(
    WebKitDownload* download,
    const gchar* suggested_filename,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    if (manager == nullptr || download == nullptr) {
        return FALSE;
    }

    const std::string suggested =
        suggested_filename != nullptr && *suggested_filename != '\0'
            ? suggested_filename
            : "download";

    // Keep the file inside the configured download directory.
    gchar* basename = g_path_get_basename(suggested.c_str());
    const std::string filename =
        basename != nullptr && *basename != '\0'
            ? basename
            : "download";
    g_free(basename);

    fs::path directory(manager->download_directory_);
    std::error_code ec;

    fs::create_directories(directory, ec);

    if (ec) {
        g_warning(
            "EXTART: unable to create download directory: %s",
            ec.message().c_str()
        );
        return FALSE;
    }

    const fs::path file_path = directory / filename;

    // WebKitGTK 6.0 expects a filesystem path here, not a file:// URI.
    webkit_download_set_destination(
        download,
        file_path.c_str()
    );

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request != nullptr
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    // Match the newest unfinished entry. Matching only by URI can associate
    // the wrong entry when the same URL is downloaded more than once.
    for (auto it = manager->downloads_.rbegin();
         it != manager->downloads_.rend();
         ++it) {
        if (!it->path.empty()) {
            continue;
        }

        if (uri == nullptr || it->uri == uri) {
            it->filename = filename;
            it->path = file_path.string();
            break;
        }
    }

    return TRUE;
}

void DownloadManager::on_download_finished(
    WebKitDownload* download,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    if (manager == nullptr || download == nullptr) {
        return;
    }

    const char* destination =
        webkit_download_get_destination(download);

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request != nullptr
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    for (auto it = manager->downloads_.rbegin();
         it != manager->downloads_.rend();
         ++it) {
        if (it->completed) {
            continue;
        }

        if (uri == nullptr || it->uri == uri) {
            it->completed = true;

            if (destination != nullptr && *destination != '\0') {
                it->path = destination;
            }

            break;
        }
    }

    manager->clear_old_downloads();

    g_message("EXTART: download finished");
}

void DownloadManager::on_download_failed(
    WebKitDownload* download,
    GError* error,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    if (manager == nullptr) {
        return;
    }

    if (error != nullptr) {
        g_warning(
            "EXTART: download failed: %s",
            error->message
        );
    } else {
        g_warning("EXTART: download failed");
    }

    (void)download;
    manager->clear_old_downloads();
}
