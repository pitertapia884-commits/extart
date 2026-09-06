#pragma once

#include <gtk/gtk.h>

#include <memory>
#include <vector>

class Config;
class ExtartApplication;
class Profile;
class Tab;
class DownloadManager;
class History;
class Bookmarks;
class HistoryPanel;
class BookmarksPanel;
class DownloadsPanel;

class BrowserWindow {
public:
BrowserWindow(ExtartApplication& application,
GtkApplication* gtk_application,
Profile& profile,
Config& config);


~BrowserWindow();

GtkWidget* widget() const;

void prepare_for_shutdown();

void select_tab(Tab* tab);
void close_tab(Tab* tab);

void tab_uri_changed(Tab* tab, const char* uri);
void tab_load_finished(Tab* tab);

DownloadManager* download_manager() const;


private:
Tab* active_tab() const;
Tab& open_tab();
void navigate_from_entry();
void setup_keyboard_shortcuts();
void setup_ui_panels();


static void on_address_activate(GtkEntry* entry, gpointer user_data);
static void on_back_clicked(GtkButton* button, gpointer user_data);
static void on_forward_clicked(GtkButton* button, gpointer user_data);
static void on_reload_clicked(GtkButton* button, gpointer user_data);
static void on_home_clicked(GtkButton* button, gpointer user_data);
static void on_new_tab_clicked(GtkButton* button, gpointer user_data);

static void on_history_button_clicked_cb(GtkButton* button, gpointer user_data);
static void on_bookmarks_button_clicked_cb(GtkButton* button, gpointer user_data);
static void on_downloads_button_clicked_cb(GtkButton* button, gpointer user_data);

static gboolean on_key_pressed(GtkEventControllerKey* controller,
                               guint keyval,
                               guint keycode,
                               GdkModifierType state,
                               gpointer user_data);

ExtartApplication& application_;
Profile& profile_;
Config& config_;

std::unique_ptr<History> history_;
std::unique_ptr<Bookmarks> bookmarks_;

GtkWidget* window_ = nullptr;
GtkWidget* tab_bar_ = nullptr;
GtkWidget* content_stack_ = nullptr;
GtkWidget* url_bar_ = nullptr;

std::vector<std::unique_ptr<Tab>> tabs_;
Tab* active_tab_ = nullptr;

std::unique_ptr<DownloadManager> download_manager_;

std::unique_ptr<HistoryPanel> history_panel_;
std::unique_ptr<BookmarksPanel> bookmarks_panel_;
std::unique_ptr<DownloadsPanel> downloads_panel_;

GtkWidget* history_popover_ = nullptr;
GtkWidget* bookmarks_popover_ = nullptr;
GtkWidget* downloads_popover_ = nullptr;

GtkWidget* history_button_ = nullptr;
GtkWidget* bookmarks_button_ = nullptr;
GtkWidget* downloads_button_ = nullptr;

void on_history_button_clicked();
void on_bookmarks_button_clicked();
void on_downloads_button_clicked();


};