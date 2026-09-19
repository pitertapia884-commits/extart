#include "download_backend.hpp"
#include "download_manager.hpp"
#include <webkit/webkit.h>
#include <filesystem>
#include <ctime>
namespace fs = std::filesystem;

class WebKitDownloadBackend final : public DownloadBackend {
public:
    explicit WebKitDownloadBackend(WebKitNetworkSession* session) : session_(session) {}
    ~WebKitDownloadBackend() override { if (session_) g_signal_handlers_disconnect_by_data(session_, this); }
    void attach(DownloadManager& manager) override { manager_ = &manager; if (session_) g_signal_connect(session_, "download-started", G_CALLBACK(on_download_started), this); }
private:
    struct PendingDestination { WebKitDownload* download; std::string uri; WebKitDownloadBackend* backend; };
    static void on_download_started(WebKitNetworkSession*, WebKitDownload* download, gpointer data) {
        auto* b = static_cast<WebKitDownloadBackend*>(data); if (!b || !b->manager_ || !download) return;
        WebKitURIResponse* response = webkit_download_get_response(download);
        WebKitURIRequest* request = webkit_download_get_request(download);
        const char* suggested = response ? webkit_uri_response_get_suggested_filename(response) : nullptr;
        const char* uri = request ? webkit_uri_request_get_uri(request) : nullptr;
        Download entry{}; entry.filename = suggested && *suggested ? suggested : "download"; entry.uri = uri ? uri : ""; entry.timestamp = std::time(nullptr);
        b->manager_->add_download(entry);
        g_signal_connect(download, "decide-destination", G_CALLBACK(on_download_decide_destination), b);
        g_signal_connect(download, "finished", G_CALLBACK(on_download_finished), b);
        g_signal_connect(download, "failed", G_CALLBACK(on_download_failed), b);
    }
    static gboolean on_download_decide_destination(WebKitDownload* download, const gchar* suggested_filename, gpointer data) {
        auto* b = static_cast<WebKitDownloadBackend*>(data); if (!b || !b->manager_ || !download) return FALSE;
        const std::string suggested = suggested_filename && *suggested_filename ? suggested_filename : "download";
        gchar* basename = g_path_get_basename(suggested.c_str());
        const std::string filename = basename && *basename ? basename : "download"; g_free(basename);
        WebKitURIRequest* request = webkit_download_get_request(download); const char* uri = request ? webkit_uri_request_get_uri(request) : nullptr;
        if (b->manager_->ask_download_location() && b->manager_->parent_window()) {
            auto* pending = new PendingDestination{static_cast<WebKitDownload*>(g_object_ref(download)), uri ? uri : "", b};
            GtkFileDialog* dialog = gtk_file_dialog_new(); gtk_file_dialog_set_title(dialog, "Guardar descarga"); gtk_file_dialog_set_initial_name(dialog, filename.c_str());
            GFile* folder = g_file_new_for_path(b->manager_->download_directory().c_str()); gtk_file_dialog_set_initial_folder(dialog, folder); g_object_unref(folder);
            gtk_file_dialog_save(dialog, b->manager_->parent_window(), nullptr, on_destination_selected, pending); g_object_unref(dialog); return TRUE;
        }
        fs::path directory(b->manager_->download_directory()); std::error_code error; fs::create_directories(directory, error); if (error) return FALSE;
        const fs::path file_path = directory / filename; webkit_download_set_destination(download, file_path.c_str()); b->manager_->mark_download_path(uri ? uri : "", file_path.string()); return TRUE;
    }
    static void on_destination_selected(GObject* source, GAsyncResult* result, gpointer data) {
        auto* pending = static_cast<PendingDestination*>(data); if (!pending) return; GError* error = nullptr;
        GFile* file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), result, &error);
        if (!file) { if (error && !g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED)) g_warning("EXTART: unable to choose download destination: %s", error->message); g_clear_error(&error); webkit_download_cancel(pending->download); g_object_unref(pending->download); delete pending; return; }
        char* path = g_file_get_path(file); if (!path || !*path) { g_free(path); g_object_unref(file); webkit_download_cancel(pending->download); g_object_unref(pending->download); delete pending; return; }
        webkit_download_set_destination(pending->download, path); if (pending->backend && pending->backend->manager_) pending->backend->manager_->mark_download_path(pending->uri, path);
        g_free(path); g_object_unref(file); g_object_unref(pending->download); delete pending;
    }
    static void on_download_finished(WebKitDownload* download, gpointer data) {
        auto* b = static_cast<WebKitDownloadBackend*>(data); if (!b || !b->manager_ || !download) return; WebKitURIRequest* request = webkit_download_get_request(download); const char* uri = request ? webkit_uri_request_get_uri(request) : nullptr; const char* destination = webkit_download_get_destination(download); b->manager_->mark_download_finished(uri ? uri : "", destination ? destination : ""); b->manager_->clear_old_downloads();
    }
    static void on_download_failed(WebKitDownload*, GError* error, gpointer data) { auto* b = static_cast<WebKitDownloadBackend*>(data); if (!b || !b->manager_) return; b->manager_->report_download_failed(error ? error->message : "download failed"); b->manager_->clear_old_downloads(); }
    WebKitNetworkSession* session_ = nullptr; DownloadManager* manager_ = nullptr;
};

std::unique_ptr<DownloadBackend> make_webkit_download_backend(WebKitNetworkSession* session) { return std::make_unique<WebKitDownloadBackend>(session); }
