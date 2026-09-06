#pragma once

#include <gtk/gtk.h>
#include <functional>
#include <string>

class Bookmarks;

class BookmarksPanel {
public:
    using OnEntryActivated = std::function<void(const std::string& url)>;
    using OnBookmarkAdded = std::function<void(const std::string& url)>;

    explicit BookmarksPanel(Bookmarks* bookmarks);
    ~BookmarksPanel() = default;

    BookmarksPanel(const BookmarksPanel&) = delete;
    BookmarksPanel& operator=(const BookmarksPanel&) = delete;

    GtkWidget* create_panel();
    void set_current_page(const std::string& url, const std::string& title);
    void set_on_entry_activated(OnEntryActivated callback);
    void set_on_bookmark_added(OnBookmarkAdded callback);
    void refresh();

private:
    static void on_add_clicked(GtkButton* button, gpointer user_data);
    static void on_open_clicked(GtkButton* button, gpointer user_data);
    static void on_remove_clicked(GtkButton* button, gpointer user_data);

    void populate_list();
    void update_add_button();

    Bookmarks* bookmarks_ = nullptr;
    std::string current_url_;
    std::string current_title_;

    GtkWidget* panel_ = nullptr;
    GtkWidget* title_label_ = nullptr;
    GtkWidget* list_box_ = nullptr;
    GtkWidget* add_button_ = nullptr;

    OnEntryActivated on_entry_activated_;
    OnBookmarkAdded on_bookmark_added_;
};
