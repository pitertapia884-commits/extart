#pragma once

#include <gtk/gtk.h>

#include <cstdint>
#include <memory>
#include <string>

class BrowserEngine;
class BrowserWindow;
class Profile;

class Tab {
public:
    Tab(BrowserWindow& window, Profile& profile);
    ~Tab();

    Tab(const Tab&) = delete;
    Tab& operator=(const Tab&) = delete;

    GtkWidget* web_view() const;
    GtkWidget* tab_control() const;
    gpointer native_view() const;

    std::string current_uri() const;
    std::string current_title() const;
    bool can_go_back() const;
    bool can_go_forward() const;
    void go_back();
    void go_forward();
    void reload();

    void load_home();
    void load_uri(const char* uri);
    void set_title(const char* title);
    void set_active(bool active);
    void apply_config();
    void prepare_for_close();
    static void apply_config_to_all_tabs();

    // EXTART 0.4 memory management
    void touch_activity();
    void suspend();
    void resume();
    bool is_suspended() const;
    bool can_suspend() const;

    void find_text(const std::string& text);
    void find_next();
    void find_previous();
    void clear_find();

private:
    static void on_tab_selected(GtkButton* button, gpointer user_data);
    static void on_close_clicked(GtkButton* button, gpointer user_data);

    BrowserWindow& window_;
    std::unique_ptr<BrowserEngine> engine_;
    GtkWidget* tab_control_ = nullptr;
    GtkWidget* select_button_ = nullptr;
    GtkWidget* title_label_ = nullptr;

    std::string saved_uri_;
    std::string saved_title_;
    std::uint64_t last_activity_ = 0;
    bool suspended_ = false;
    bool active_ = false;
};
