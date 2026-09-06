#include "download_manager.hpp"

#include <glib.h>

#include <ctime>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

DownloadManager::DownloadManager() {
const char* downloads =
g_get_user_special_dir(
G_USER_DIRECTORY_DOWNLOAD
);


if (downloads != nullptr) {
    download_directory_ = downloads;
} else {
    gchar* fallback =
        g_build_filename(
            g_get_home_dir(),
            "Downloads",
            nullptr
        );

    download_directory_ = fallback;
    g_free(fallback);
}

if (download_directory_.empty()) {
    download_directory_ =
        g_get_home_dir();
}


}

DownloadManager::~DownloadManager() = default;

void DownloadManager::setup_for_web_view(
WebKitWebView* web_view
) {
if (web_view == nullptr) {
return;
}


WebKitWebContext* context =
    webkit_web_view_get_context(web_view);

if (context == nullptr) {
    return;
}

// El contexto puede ser compartido por varias pestañas.
// Solo necesitamos conectar download-started una vez.
if (g_object_get_data(
        G_OBJECT(context),
        "extart-download-manager"
    ) == this) {

    return;
}

g_object_set_data(
    G_OBJECT(context),
    "extart-download-manager",
    this
);

g_signal_connect(
    context,
    "download-started",
    G_CALLBACK(on_download_started),
    this
);


}

const std::vector<Download>&
DownloadManager::downloads() const {
return downloads_;
}

const std::string&
DownloadManager::download_directory() const {
return download_directory_;
}

void DownloadManager::set_download_directory(
const std::string& path
) {
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
WebKitWebContext*,
WebKitDownload* download,
gpointer user_data
) {
auto* manager =
static_cast<DownloadManager*>(user_data);


if (manager == nullptr || download == nullptr) {
    return;
}

WebKitURIResponse* response =
    webkit_download_get_response(download);

const char* suggested =
    response
        ? webkit_uri_response_get_suggested_filename(
              response
          )
        : nullptr;

WebKitURIRequest* request =
    webkit_download_get_request(download);

const char* uri =
    request
        ? webkit_uri_request_get_uri(request)
        : nullptr;

Download dl{};

dl.filename =
    suggested && *suggested
        ? suggested
        : "download";

dl.uri =
    uri ? uri : "";

dl.timestamp =
    std::time(nullptr);

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

g_message(
    "EXTART: download detected: %s",
    dl.filename.c_str()
);


}

gboolean DownloadManager::on_download_decide_destination(
WebKitDownload* download,
const gchar* suggested_filename,
gpointer user_data
) {
auto* manager =
static_cast<DownloadManager*>(user_data);


if (manager == nullptr || download == nullptr) {
    return FALSE;
}

std::string filename =
    suggested_filename && *suggested_filename
        ? suggested_filename
        : "download";

fs::path directory(
    manager->download_directory_
);

fs::path file_path =
    directory / filename;

// Crear Downloads si todavía no existe.
std::error_code ec;

fs::create_directories(
    directory,
    ec
);

if (ec) {
    g_warning(
        "EXTART: unable to create download directory: %s",
        ec.message().c_str()
    );

    return FALSE;
}

const std::string path =
    file_path.string();

gchar* destination =
    g_filename_to_uri(
        path.c_str(),
        nullptr,
        nullptr
    );

if (destination == nullptr) {
    g_warning(
        "EXTART: unable to create download destination"
    );

    return FALSE;
}

webkit_download_set_destination(
    download,
    destination
);

g_free(destination);

WebKitURIRequest* request =
    webkit_download_get_request(download);

const char* uri =
    request
        ? webkit_uri_request_get_uri(request)
        : nullptr;

if (uri != nullptr) {
    for (auto& dl : manager->downloads_) {
        if (dl.uri == uri &&
            dl.path.empty()) {

            dl.filename = filename;
            dl.path = path;
            break;
        }
    }
}

return TRUE;


}

void DownloadManager::on_download_finished(
WebKitDownload* download,
gpointer user_data
) {
auto* manager =
static_cast<DownloadManager*>(user_data);


if (manager == nullptr || download == nullptr) {
    return;
}

const char* destination =
    webkit_download_get_destination(download);

WebKitURIRequest* request =
    webkit_download_get_request(download);

const char* uri =
    request
        ? webkit_uri_request_get_uri(request)
        : nullptr;

if (uri != nullptr) {
    for (auto& dl : manager->downloads_) {
        if (dl.uri == uri) {
            dl.completed = true;

            if (destination != nullptr) {
                gchar* path =
                    g_filename_from_uri(
                        destination,
                        nullptr,
                        nullptr
                    );

                if (path != nullptr) {
                    dl.path = path;
                    g_free(path);
                }
            }

            break;
        }
    }
}

manager->clear_old_downloads();

g_message(
    "EXTART: download finished"
);


}

void DownloadManager::on_download_failed(
WebKitDownload*,
gpointer user_data
) {
auto* manager =
static_cast<DownloadManager*>(user_data);


if (manager == nullptr) {
    return;
}

manager->clear_old_downloads();

g_warning(
    "EXTART: download failed"
);


}
