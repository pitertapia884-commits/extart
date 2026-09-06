#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <memory>

struct Download;
class DownloadManager;

class DownloadsPanel {
public:
    DownloadsPanel(DownloadManager* dm);
    ~DownloadsPanel();
    
    GtkWidget* create_panel();
    void refresh();
    void clear_completed();
    
private:
    static void on_open_clicked(GtkButton* button, gpointer user_data);
    static void on_open_folder_clicked(GtkButton* button, gpointer user_data);
    static void on_remove_clicked(GtkButton* button, gpointer user_data);
    void populate_list();
    void open_download(const Download& download);
    void open_folder(const Download& download);
    
    DownloadManager* dm_;
    GtkWidget* list_box_ = nullptr;
    GtkWidget* empty_label_ = nullptr;
    std::vector<std::pair<Download, GtkWidget*>> download_rows_;
};
