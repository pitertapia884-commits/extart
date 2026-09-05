#pragma once

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <string>
#include <vector>

struct DownloadItem {
    WebKitDownload* download;
    GtkWidget* row;
    GtkWidget* label_name;
    GtkWidget* progress_bar;
    GtkWidget* label_speed;
    GtkWidget* label_status;
    guint64 last_received;
    gint64 last_time;
};

class DownloadManager {
public:
    static DownloadManager& get();

    void add_download(WebKitDownload* download);
    void show_panel(GtkWidget* parent_window);
    void hide_panel();
    bool is_visible();

private:
    DownloadManager();

    GtkWidget* panel;
    GtkWidget* list_box;
    std::vector<DownloadItem*> items;

    static void on_progress(WebKitDownload* dl, GParamSpec* pspec, gpointer data);
    static void on_finished(WebKitDownload* dl, gpointer data);
    static void on_failed(WebKitDownload* dl, GError* error, gpointer data);
    static void on_cancel(GtkWidget* btn, gpointer data);
};