#include "browser_window.hpp"

#include "config.hpp"
#include "extart_application.hpp"
#include "profile.hpp"
#include "tab.hpp"

#include <glib.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

namespace {
bool has_whitespace(const std::string& text) {
    for (const unsigned char character : text) {
        if (std::isspace(character)) {
            return true;
        }
    }
    return false;
}

bool is_url_candidate(const std::string& text) {
    if (text.empty() || has_whitespace(text)) {
        return false;
    }

    gchar* scheme = g_uri_parse_scheme(text.c_str());
    if (scheme != nullptr) {
        g_free(scheme);
        return true;
    }

    return text == "localhost" || text.rfind("localhost:", 0) == 0 ||
        text.find('.') != std::string::npos || text.find(':') != std::string::npos;
}
}

BrowserWindow::BrowserWindow(ExtartApplication& application, GtkApplication* gtk_application,
                             Profile& profile, Config& config)
    : application_(application), profile_(profile), config_(config) {
    window_ = gtk_application_window_new(gtk_application);
    gtk_window_set_title(GTK_WINDOW(window_), "EXTART");
    gtk_window_set_default_size(GTK_WINDOW(window_), 1200, 800);

    GtkWidget* root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(root, "browser-root");

    tab_bar_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_add_css_class(tab_bar_, "tabbar");
    GtkWidget* new_tab_button = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(new_tab_button, "newtab-btn");
    gtk_widget_set_hexpand(tab_bar_, TRUE);

    GtkWidget* navigation = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_add_css_class(navigation, "navbar");
    gtk_widget_set_margin_start(navigation, 6);
    gtk_widget_set_margin_end(navigation, 6);
    gtk_widget_set_margin_top(navigation, 6);
    gtk_widget_set_margin_bottom(navigation, 6);

    GtkWidget* back = gtk_button_new_with_label("←");
    GtkWidget* forward = gtk_button_new_with_label("→");
    GtkWidget* reload = gtk_button_new_with_label("↻");
    GtkWidget* home = gtk_button_new_with_label("⌂");
    url_bar_ = gtk_entry_new();
    gtk_widget_set_hexpand(url_bar_, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(url_bar_), "Search or enter address");

    gtk_box_append(GTK_BOX(tab_bar_), new_tab_button);
    gtk_box_append(GTK_BOX(navigation), back);
    gtk_box_append(GTK_BOX(navigation), forward);
    gtk_box_append(GTK_BOX(navigation), reload);
    gtk_box_append(GTK_BOX(navigation), home);
    gtk_box_append(GTK_BOX(navigation), url_bar_);

    content_stack_ = gtk_stack_new();
    gtk_widget_set_hexpand(content_stack_, TRUE);
    gtk_widget_set_vexpand(content_stack_, TRUE);

    gtk_box_append(GTK_BOX(root), tab_bar_);
    gtk_box_append(GTK_BOX(root), navigation);
    gtk_box_append(GTK_BOX(root), content_stack_);
    gtk_window_set_child(GTK_WINDOW(window_), root);

    g_signal_connect(url_bar_, "activate", G_CALLBACK(on_address_activate), this);
    g_signal_connect(back, "clicked", G_CALLBACK(on_back_clicked), this);
    g_signal_connect(forward, "clicked", G_CALLBACK(on_forward_clicked), this);
    g_signal_connect(reload, "clicked", G_CALLBACK(on_reload_clicked), this);
    g_signal_connect(home, "clicked", G_CALLBACK(on_home_clicked), this);
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_clicked), this);

    open_tab();
    gtk_window_present(GTK_WINDOW(window_));
}

BrowserWindow::~BrowserWindow() = default;

GtkWidget* BrowserWindow::widget() const {
    return window_;
}

Tab& BrowserWindow::open_tab() {
    auto tab = std::make_unique<Tab>(*this, profile_);
    Tab& result = *tab;
    GtkWidget* previous_control = tabs_.empty() ? nullptr : tabs_.back()->tab_control();
    gtk_box_insert_child_after(GTK_BOX(tab_bar_), result.tab_control(), previous_control);
    gtk_stack_add_child(GTK_STACK(content_stack_), result.web_view());
    tabs_.push_back(std::move(tab));
    select_tab(&result);
    result.load_home();
    return result;
}

Tab* BrowserWindow::active_tab() const {
    return active_tab_;
}

void BrowserWindow::select_tab(Tab* tab) {
    if (tab == nullptr) {
        return;
    }
    active_tab_ = tab;
    for (const auto& candidate : tabs_) {
        candidate->set_active(candidate.get() == tab);
    }
    gtk_stack_set_visible_child(GTK_STACK(content_stack_), tab->web_view());
    const char* uri = webkit_web_view_get_uri(tab->view());
    gtk_editable_set_text(GTK_EDITABLE(url_bar_), uri ? uri : "");
}

void BrowserWindow::close_tab(Tab* tab) {
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
        [tab](const std::unique_ptr<Tab>& candidate) { return candidate.get() == tab; });
    if (it == tabs_.end()) {
        return;
    }

    if (tabs_.size() == 1) {
        tab->load_home();
        return;
    }

    const bool was_active = active_tab_ == tab;
    const std::size_t index = static_cast<std::size_t>(std::distance(tabs_.begin(), it));
    gtk_box_remove(GTK_BOX(tab_bar_), tab->tab_control());
    gtk_stack_remove(GTK_STACK(content_stack_), tab->web_view());
    tabs_.erase(it);

    if (was_active) {
        const std::size_t next_index = index == tabs_.size() ? index - 1 : index;
        select_tab(tabs_[next_index].get());
    }
}

void BrowserWindow::tab_uri_changed(Tab* tab, const char* uri) {
    if (tab == active_tab_) {
        gtk_editable_set_text(GTK_EDITABLE(url_bar_), uri ? uri : "");
    }
}

void BrowserWindow::navigate_from_entry() {
    Tab* tab = active_tab();
    if (tab == nullptr) {
        return;
    }

    const std::string input = gtk_editable_get_text(GTK_EDITABLE(url_bar_));
    if (input.empty()) {
        return;
    }

    std::string uri;
    if (is_url_candidate(input)) {
        gchar* scheme = g_uri_parse_scheme(input.c_str());
        if (scheme == nullptr) {
            uri = "https://" + input;
        } else {
            uri = input;
            g_free(scheme);
        }
    } else {
        gchar* escaped = g_uri_escape_string(input.c_str(), nullptr, FALSE);
        uri = config_.search_engine() + escaped;
        g_free(escaped);
    }

    tab->load_uri(uri.c_str());
}

void BrowserWindow::on_address_activate(GtkEntry*, gpointer user_data) {
    static_cast<BrowserWindow*>(user_data)->navigate_from_entry();
}

void BrowserWindow::on_back_clicked(GtkButton*, gpointer user_data) {
    Tab* tab = static_cast<BrowserWindow*>(user_data)->active_tab();
    if (tab != nullptr && webkit_web_view_can_go_back(tab->view())) {
        webkit_web_view_go_back(tab->view());
    }
}

void BrowserWindow::on_forward_clicked(GtkButton*, gpointer user_data) {
    Tab* tab = static_cast<BrowserWindow*>(user_data)->active_tab();
    if (tab != nullptr && webkit_web_view_can_go_forward(tab->view())) {
        webkit_web_view_go_forward(tab->view());
    }
}

void BrowserWindow::on_reload_clicked(GtkButton*, gpointer user_data) {
    Tab* tab = static_cast<BrowserWindow*>(user_data)->active_tab();
    if (tab != nullptr) {
        webkit_web_view_reload(tab->view());
    }
}

void BrowserWindow::on_home_clicked(GtkButton*, gpointer user_data) {
    Tab* tab = static_cast<BrowserWindow*>(user_data)->active_tab();
    if (tab != nullptr) {
        tab->load_home();
    }
}

void BrowserWindow::on_new_tab_clicked(GtkButton*, gpointer user_data) {
    static_cast<BrowserWindow*>(user_data)->open_tab();
}
