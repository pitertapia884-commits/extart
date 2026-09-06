#include "session_manager.hpp"

#include <webkit/webkit.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::string session_path() {
    const char* config_dir = g_get_user_config_dir();
    return (std::filesystem::path(config_dir) / "extart" / "session.ini").string();
}

void collect_web_views(GtkWidget* widget, std::vector<WebKitWebView*>& result) {
    if (widget == nullptr) {
        return;
    }

    if (WEBKIT_IS_WEB_VIEW(widget)) {
        result.push_back(WEBKIT_WEB_VIEW(widget));
        return;
    }

    GtkWidget* child = gtk_widget_get_first_child(widget);
    while (child != nullptr) {
        collect_web_views(child, result);
        child = gtk_widget_get_next_sibling(child);
    }
}

GtkButton* find_new_tab_button(GtkWidget* widget) {
    if (widget == nullptr) {
        return nullptr;
    }

    if (GTK_IS_BUTTON(widget)) {
        const char* label = gtk_button_get_label(GTK_BUTTON(widget));
        if (label != nullptr && std::string(label) == "+") {
            return GTK_BUTTON(widget);
        }
    }

    GtkWidget* child = gtk_widget_get_first_child(widget);
    while (child != nullptr) {
        if (GtkButton* button = find_new_tab_button(child)) {
            return button;
        }
        child = gtk_widget_get_next_sibling(child);
    }

    return nullptr;
}

bool valid_session_uri(const char* uri) {
    if (uri == nullptr || *uri == '\0') {
        return false;
    }

    const std::string value(uri);
    return value != "about:blank" && value.rfind("extart://", 0) != 0;
}

} // namespace

void session_save(GtkWidget* window) {
    std::vector<WebKitWebView*> views;
    collect_web_views(window, views);

    const std::filesystem::path path(session_path());
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        return;
    }

    std::ofstream file(path);
    if (!file) {
        return;
    }

    file << "[Session]\n";

    int index = 0;
    for (WebKitWebView* view : views) {
        const char* uri = webkit_web_view_get_uri(view);
        if (!valid_session_uri(uri)) {
            continue;
        }

        file << "tab" << index++ << "=" << uri << "\n";
    }
}

void session_restore(GtkWidget* window) {
    std::ifstream file(session_path());
    if (!file) {
        return;
    }

    std::vector<std::string> uris;
    std::string line;
    while (std::getline(file, line)) {
        const std::string prefix = "tab";
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos || line.rfind(prefix, 0) != 0) {
            continue;
        }

        const std::string uri = line.substr(equals + 1);
        if (valid_session_uri(uri.c_str())) {
            uris.push_back(uri);
        }
    }

    if (uris.empty()) {
        return;
    }

    std::vector<WebKitWebView*> views;
    collect_web_views(window, views);
    if (views.empty()) {
        return;
    }

    webkit_web_view_load_uri(views.front(), uris.front().c_str());

    for (std::size_t index = 1; index < uris.size(); ++index) {
        GtkButton* new_tab = find_new_tab_button(window);
        if (new_tab == nullptr) {
            break;
        }

        gtk_widget_activate(GTK_WIDGET(new_tab));

        views.clear();
        collect_web_views(window, views);
        if (views.size() <= index) {
            break;
        }

        webkit_web_view_load_uri(views[index], uris[index].c_str());
    }
}
