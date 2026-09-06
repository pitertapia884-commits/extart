#include "download_manager.hpp"

#include <glib.h>
#include <filesystem>
#include <cstring>
#include <ctime>

namespace fs = std::filesystem;

DownloadManager::DownloadManager() {
    const char* downloads = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);

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

DownloadManager::~DownloadManager() = default;

void DownloadManager::setup_for_web_view(WebKitWebView* web_view) {
    WebKitWebContext* context = webkit_web_view_get_context(web_view);

    g_signal_connect(
        context,
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
    if (downloads_.size() > 50) {
        downloads_.erase(
            downloads_.begin(),
            downloads_.end() - 50
        );
    }
}

void DownloadManager::on_download_started(
    WebKitWebContext*,
    WebKitDownload* download,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    // Obtener la respuesta de la descarga para conocer
    // el nombre de archivo sugerido.
    WebKitURIResponse* response =
        webkit_download_get_response(download);

    const char* suggested =
        response
            ? webkit_uri_response_get_suggested_filename(response)
            : nullptr;

    // Obtener la URI original desde la request.
    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    Download dl{};
    dl.filename = suggested ? suggested : "download";
    dl.uri = uri ? uri : "";
    dl.timestamp = std::time(nullptr);
    dl.completed = false;
    dl.size_bytes = 0;
    dl.downloaded_bytes = 0;

    manager->downloads_.push_back(dl);

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
}

gboolean DownloadManager::on_download_decide_destination(
    WebKitDownload* download,
    const gchar* suggested_filename,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    std::string filename =
        suggested_filename ? suggested_filename : "download";

    std::string path =
        manager->download_directory_ + "/" + filename;

    std::string destination = "file://" + path;

    webkit_download_set_destination(
        download,
        destination.c_str()
    );

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    // Actualizar la descarga que ya registramos
    // en on_download_started().
    for (auto& dl : manager->downloads_) {
        if (uri && dl.uri == uri && dl.path.empty()) {
            dl.filename = filename;
            dl.path = path;
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

    const char* destination =
        webkit_download_get_destination(download);

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    if (uri) {
        for (auto& dl : manager->downloads_) {
            if (dl.uri == uri) {
                dl.completed = true;

                if (destination) {
                    std::string dest(destination);

                    if (dest.rfind("file://", 0) == 0) {
                        dl.path = dest.substr(7);
                    }
                }

                break;
            }
        }
    }

    manager->clear_old_downloads();
}

void DownloadManager::on_download_failed(
    WebKitDownload*,
    gpointer user_data
) {
    auto* manager = static_cast<DownloadManager*>(user_data);

    manager->clear_old_downloads();
}