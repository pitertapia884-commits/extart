#include "download_manager.h"
#include <string>
#include <ctime>

DownloadManager::DownloadManager() {
    panel = nullptr;
    list_box = nullptr;
}

DownloadManager& DownloadManager::get() {
    static DownloadManager instance;
    return instance;
}

void DownloadManager::add_download(WebKitDownload* download) {
    DownloadItem* item = new DownloadItem();
    item->download = download;
    item->last_received = 0;
    item->last_time = g_get_monotonic_time();

    // Fila de la descarga
    item->row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(item->row, 8);
    gtk_widget_set_margin_end(item->row, 8);
    gtk_widget_set_margin_top(item->row, 6);
    gtk_widget_set_margin_bottom(item->row, 6);

    GtkWidget* top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    const char* dest = webkit_download_get_destination(download);
    std::string name = dest ? std::string(dest) : "descarga";
    name = name.substr(name.find_last_of("/") + 1);

    item->label_name = gtk_label_new(name.c_str());
    gtk_widget_set_hexpand(item->label_name, TRUE);
    gtk_widget_set_halign(item->label_name, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(item->label_name), PANGO_ELLIPSIZE_MIDDLE);

    GtkWidget* btn_cancel = gtk_button_new_with_label("✕");
    gtk_widget_add_css_class(btn_cancel, "tab-close");

    gtk_box_append(GTK_BOX(top_row), item->label_name);
    gtk_box_append(GTK_BOX(top_row), btn_cancel);

    item->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(item->progress_bar), 0.0);

    GtkWidget* bottom_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    item->label_speed = gtk_label_new("0 KB/s");
    gtk_widget_set_halign(item->label_speed, GTK_ALIGN_START);
    gtk_widget_set_hexpand(item->label_speed, TRUE);

    item->label_status = gtk_label_new("Descargando...");
    gtk_widget_set_halign(item->label_status, GTK_ALIGN_END);

    gtk_box_append(GTK_BOX(bottom_row), item->label_speed);
    gtk_box_append(GTK_BOX(bottom_row), item->label_status);

    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    gtk_box_append(GTK_BOX(item->row), top_row);
    gtk_box_append(GTK_BOX(item->row), item->progress_bar);
    gtk_box_append(GTK_BOX(item->row), bottom_row);
    gtk_box_append(GTK_BOX(item->row), sep);

    g_signal_connect(download, "notify::estimated-progress", G_CALLBACK(on_progress), item);
    g_signal_connect(download, "finished", G_CALLBACK(on_finished), item);
    g_signal_connect(download, "failed", G_CALLBACK(on_failed), item);
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_cancel), item);

    items.push_back(item);

    // Si el panel ya existe, agregar directo
    if (list_box) {
        gtk_box_append(GTK_BOX(list_box), item->row);
        gtk_widget_set_visible(panel, TRUE);
    }
}
void DownloadManager::show_panel(GtkWidget* parent_window) {
    if (panel) {
        gtk_widget_set_visible(panel, TRUE);
        return;
    }

    panel = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(panel), "Descargas — EXTART");
    gtk_window_set_default_size(GTK_WINDOW(panel), 380, 400);
    gtk_window_set_transient_for(GTK_WINDOW(panel), GTK_WINDOW(parent_window));

    GtkWidget* scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), list_box);
    gtk_window_set_child(GTK_WINDOW(panel), scroll);

    // Re-agregar items existentes si los hay
    for (auto* item : items) {
        gtk_box_append(GTK_BOX(list_box), item->row);
    }

    gtk_widget_set_visible(panel, TRUE);
}

void DownloadManager::hide_panel() {
    if (panel) gtk_widget_set_visible(panel, FALSE);
}

bool DownloadManager::is_visible() {
    return panel && gtk_widget_get_visible(panel);
}

void DownloadManager::on_progress(WebKitDownload* dl, GParamSpec* pspec, gpointer data) {
    DownloadItem* item = (DownloadItem*)data;

    double progress = webkit_download_get_estimated_progress(dl);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(item->progress_bar), progress);

    // Calcular velocidad
    guint64 received = webkit_download_get_received_data_length(dl);
    gint64 now = g_get_monotonic_time();
    gint64 elapsed = now - item->last_time;

    if (elapsed > 500000) { // cada 0.5 segundos
        guint64 diff = received - item->last_received;
        double speed = (double)diff / (elapsed / 1000000.0);

        char speed_str[64];
        if (speed > 1024 * 1024)
            snprintf(speed_str, sizeof(speed_str), "%.1f MB/s", speed / (1024 * 1024));
        else
            snprintf(speed_str, sizeof(speed_str), "%.0f KB/s", speed / 1024);

        gtk_label_set_text(GTK_LABEL(item->label_speed), speed_str);

        item->last_received = received;
        item->last_time = now;
    }

    // Porcentaje en el estado
    char status_str[32];
    snprintf(status_str, sizeof(status_str), "%.0f%%", progress * 100);
    gtk_label_set_text(GTK_LABEL(item->label_status), status_str);
}

void DownloadManager::on_finished(WebKitDownload* dl, gpointer data) {
    DownloadItem* item = (DownloadItem*)data;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(item->progress_bar), 1.0);
    gtk_label_set_text(GTK_LABEL(item->label_status), "Completado ✓");
    gtk_label_set_text(GTK_LABEL(item->label_speed), "");
}

void DownloadManager::on_failed(WebKitDownload* dl, GError* error, gpointer data) {
    DownloadItem* item = (DownloadItem*)data;
    gtk_label_set_text(GTK_LABEL(item->label_status), "Error ✗");
    gtk_label_set_text(GTK_LABEL(item->label_speed), "");
}

void DownloadManager::on_cancel(GtkWidget* btn, gpointer data) {
    DownloadItem* item = (DownloadItem*)data;
    webkit_download_cancel(item->download);
    gtk_label_set_text(GTK_LABEL(item->label_status), "Cancelado");
    gtk_widget_set_sensitive(btn, FALSE);
}