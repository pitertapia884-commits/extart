#pragma once

#include <gtk/gtk.h>
#include <functional>
#include <string>

class History;

class HistoryPanel {
public:
    using OnEntryActivated = std::function<void(const std::string& url)>;
    
    HistoryPanel(History* history);
    ~HistoryPanel();
    
    GtkWidget* create_panel();
    void set_on_entry_activated(OnEntryActivated callback);
    void refresh();
    void clear_all();
    
private:
    static void on_row_activated(GtkListBox* box, GtkListBoxRow* row, gpointer user_data);
    static void on_clear_clicked(GtkButton* button, gpointer user_data);
    void populate_list();
    
    History* history_;
    GtkWidget* list_box_ = nullptr;
    GtkWidget* empty_label_ = nullptr;
    GtkWidget* search_entry_ = nullptr;
    OnEntryActivated on_entry_activated_;
    std::string search_text_;
};
