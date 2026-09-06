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

void DownloadManager::set_ask_download_location(bool enabled) {
    ask_download_location_ = enabled;
}

void DownloadManager::set_parent_window(GtkWindow* window) {
    parent_window_ = window;
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

    gchar* basename = g_path_get_basename(suggested.c_str());
    const std::string filename =
        basename != nullptr && *basename != '\0'
            ? basename
            : "download";
    g_free(basename);

    WebKitURIRequest* request =
        webkit_download_get_request(download);

    const char* uri =
        request != nullptr
            ? webkit_uri_request_get_uri(request)
            : nullptr;

    if (manager->ask_download_location_ && manager->parent_window_ != nullptr) {
        auto* pending = new PendingDestination{
            manager,
            static_cast<WebKitDownload*>(g_object_ref(download)),
            uri != nullptr ? uri : ""
        };

        GtkFileDialog* dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, "Guardar descarga");
        gtk_file_dialog_set_initial_name(dialog, filename.c_str());

        GFile* folder = g_file_new_for_path(manager->download_directory_.c_str());
        gtk_file_dialog_set_initial_folder(dialog, folder);
        g_object_unref(folder);

        gtk_file_dialog_save(
            dialog,
            manager->parent_window_,
            nullptr,
            on_destination_selected,
            pending
        );

        g_object_unref(dialog);

        // WebKitGTK 6.0 supports asynchronous destination selection here:
        // returning TRUE pauses the download until set_destination() is called.
        return TRUE;
    }

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

    webkit_download_set_destination(
        download,
        file_path.c_str()
    );

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

void DownloadManager::on_destination_selected(
    GObject* source,
    GAsyncResult* result,
    gpointer user_data
) {
    auto* pending = static_cast<PendingDestination*>(user_data);
    if (pending == nullptr) {
        return;
    }

    DownloadManager* manager = pending->manager;
    WebKitDownload* download = pending->download;

    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GError* error = nullptr;
    GFile* file = gtk_file_dialog_save_finish(dialog, result, &error);

    if (file == nullptr) {
        if (error != nullptr &&
            !g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED)) {
            g_warning("EXTART: unable to choose download destination: %s", error->message);
        }
        g_clear_error(&error);

        webkit_download_cancel(download);
        g_object_unref(download);
        delete pending;
        return;
    }

    char* path = g_file_get_path(file);
    if (path == nullptr || *path == '\0') {
        g_free(path);
        g_object_unref(file);
        webkit_download_cancel(download);
        g_object_unref(download);
        delete pending;
        return;
    }

    webkit_download_set_destination(download, path);

    if (manager != nullptr) {
        for (auto it = manager->downloads_.rbegin();
             it != manager->downloads_.rend();
             ++it) {
            if (!it->path.empty()) {
                continue;
            }

            if (pending->uri.empty() || it->uri == pending->uri) {
                it->filename = g_path_get_basename(path);
                it->path = path;
                break;
            }
        }
    }

    g_free(path);
    g_object_unref(file);
    g_object_unref(download);
    delete pending;
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
