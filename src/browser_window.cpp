#include "browser_window.hpp"

#include "config.hpp"
#include "download_manager.hpp"
#include "extart_application.hpp"
#include "profile.hpp"
#include "tab.hpp"
#include "history.hpp"
#include "bookmarks.hpp"
#include "history_panel.hpp"
#include "bookmarks_panel.hpp"
#include "downloads_panel.hpp"

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

```
return false;
```

}

bool is_url_candidate(const std::string& text) {
if (text.empty() || has_whitespace(text)) {
return false;
}

```
gchar* scheme = g_uri_parse_scheme(text.c_str());

if (scheme != nullptr) {
    g_free(scheme);
    return true;
}

return text == "localhost" ||
       text.rfind("localhost:", 0) == 0 ||
       text.find('.') != std::string::npos ||
       text.find(':') != std::string::npos;
```

}

} // namespace

BrowserWindow::BrowserWindow(
ExtartApplication& application,
GtkApplication* gtk_application,
Profile& profile,
Config& config
)
: application_(application),
profile_(profile),
config_(config),
history_(new History()),
bookmarks_(new Bookmarks()),
download_manager_(std::make_unique<DownloadManager>()) {

```
window_ = gtk_application_window_new(gtk_application);

gtk_window_set_title(
    GTK_WINDOW(window_),
    "EXTART"
);

gtk_window_set_default_size(
    GTK_WINDOW(window_),
    1200,
    800
);

GtkWidget* root =
    gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        0
    );

gtk_widget_add_css_class(
    root,
    "browser-root"
);

tab_bar_ =
    gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        2
    );

gtk_widget_add_css_class(
    tab_bar_,
    "tabbar"
);

GtkWidget* new_tab_button =
    gtk_button_new_with_label("+");

gtk_widget_add_css_class(
    new_tab_button,
    "newtab-btn"
);

gtk_widget_set_hexpand(
    tab_bar_,
    TRUE
);

GtkWidget* navigation =
    gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        4
    );

gtk_widget_add_css_class(
    navigation,
    "navbar"
);

gtk_widget_set_margin_start(
    navigation,
    6
);

gtk_widget_set_margin_end(
    navigation,
    6
);

gtk_widget_set_margin_top(
    navigation,
    6
);

gtk_widget_set_margin_bottom(
    navigation,
    6
);

GtkWidget* back =
    gtk_button_new_with_label("←");

GtkWidget* forward =
    gtk_button_new_with_label("→");

GtkWidget* reload =
    gtk_button_new_with_label("↻");

GtkWidget* home =
    gtk_button_new_with_label("⌂");

bookmarks_button_ =
    gtk_button_new_with_label("☆");

gtk_widget_set_tooltip_text(
    bookmarks_button_,
    "Bookmarks (Ctrl+D)"
);

history_button_ =
    gtk_button_new_with_label("⏱");

gtk_widget_set_tooltip_text(
    history_button_,
    "History (Ctrl+H)"
);

downloads_button_ =
    gtk_button_new_with_label("↓");

gtk_widget_set_tooltip_text(
    downloads_button_,
    "Downloads (Ctrl+J)"
);

url_bar_ =
    gtk_entry_new();

gtk_widget_set_hexpand(
    url_bar_,
    TRUE
);

gtk_entry_set_placeholder_text(
    GTK_ENTRY(url_bar_),
    "Search or enter address"
);

gtk_box_append(
    GTK_BOX(tab_bar_),
    new_tab_button
);

gtk_box_append(
    GTK_BOX(navigation),
    back
);

gtk_box_append(
    GTK_BOX(navigation),
    forward
);

gtk_box_append(
    GTK_BOX(navigation),
    reload
);

gtk_box_append(
    GTK_BOX(navigation),
    home
);

gtk_box_append(
    GTK_BOX(navigation),
    bookmarks_button_
);

gtk_box_append(
    GTK_BOX(navigation),
    history_button_
);

gtk_box_append(
    GTK_BOX(navigation),
    downloads_button_
);

gtk_box_append(
    GTK_BOX(navigation),
    url_bar_
);

content_stack_ =
    gtk_stack_new();

gtk_widget_set_hexpand(
    content_stack_,
    TRUE
);

gtk_widget_set_vexpand(
    content_stack_,
    TRUE
);

gtk_box_append(
    GTK_BOX(root),
    tab_bar_
);

gtk_box_append(
    GTK_BOX(root),
    navigation
);

gtk_box_append(
    GTK_BOX(root),
    content_stack_
);

gtk_window_set_child(
    GTK_WINDOW(window_),
    root
);

g_signal_connect(
    url_bar_,
    "activate",
    G_CALLBACK(on_address_activate),
    this
);

g_signal_connect(
    back,
    "clicked",
    G_CALLBACK(on_back_clicked),
    this
);

g_signal_connect(
    forward,
    "clicked",
    G_CALLBACK(on_forward_clicked),
    this
);

g_signal_connect(
    reload,
    "clicked",
    G_CALLBACK(on_reload_clicked),
    this
);

g_signal_connect(
    home,
    "clicked",
    G_CALLBACK(on_home_clicked),
    this
);

g_signal_connect(
    new_tab_button,
    "clicked",
    G_CALLBACK(on_new_tab_clicked),
    this
);

g_signal_connect(
    bookmarks_button_,
    "clicked",
    G_CALLBACK(on_bookmarks_button_clicked_cb),
    this
);

g_signal_connect(
    history_button_,
    "clicked",
    G_CALLBACK(on_history_button_clicked_cb),
    this
);

g_signal_connect(
    downloads_button_,
    "clicked",
    G_CALLBACK(on_downloads_button_clicked_cb),
    this
);

setup_keyboard_shortcuts();
setup_ui_panels();

open_tab();

gtk_window_present(
    GTK_WINDOW(window_)
);
```

}

BrowserWindow::~BrowserWindow() {
history_panel_.reset();
bookmarks_panel_.reset();
downloads_panel_.reset();

```
delete history_;
history_ = nullptr;

delete bookmarks_;
bookmarks_ = nullptr;
```

}

GtkWidget* BrowserWindow::widget() const {
return window_;
}

DownloadManager* BrowserWindow::download_manager() const {
return download_manager_.get();
}

Tab& BrowserWindow::open_tab() {
auto tab =
std::make_unique<Tab>(
*this,
profile_
);

```
Tab& result = *tab;

GtkWidget* previous_control =
    tabs_.empty()
        ? nullptr
        : tabs_.back()->tab_control();

gtk_box_insert_child_after(
    GTK_BOX(tab_bar_),
    result.tab_control(),
    previous_control
);

gtk_stack_add_child(
    GTK_STACK(content_stack_),
    result.web_view()
);

tabs_.push_back(
    std::move(tab)
);

select_tab(&result);

result.load_home();

return result;
```

}

Tab* BrowserWindow::active_tab() const {
return active_tab_;
}

void BrowserWindow::select_tab(Tab* tab) {
if (tab == nullptr) {
return;
}

```
active_tab_ = tab;

for (const auto& candidate : tabs_) {
    candidate->set_active(
        candidate.get() == tab
    );
}

gtk_stack_set_visible_child(
    GTK_STACK(content_stack_),
    tab->web_view()
);

const char* uri =
    webkit_web_view_get_uri(
        tab->view()
    );

gtk_editable_set_text(
    GTK_EDITABLE(url_bar_),
    uri ? uri : ""
);

if (bookmarks_panel_) {
    const char* title =
        webkit_web_view_get_title(
            tab->view()
        );

    bookmarks_panel_->set_current_page(
        uri ? uri : "",
        title ? title : ""
    );
}
```

}

void BrowserWindow::close_tab(Tab* tab) {
auto it =
std::find_if(
tabs_.begin(),
tabs_.end(),
[tab](const std::unique_ptr<Tab>& candidate) {
return candidate.get() == tab;
}
);

```
if (it == tabs_.end()) {
    return;
}

if (tabs_.size() == 1) {
    tab->load_home();
    return;
}

const bool was_active =
    active_tab_ == tab;

const std::size_t index =
    static_cast<std::size_t>(
        std::distance(
            tabs_.begin(),
            it
        )
    );

gtk_box_remove(
    GTK_BOX(tab_bar_),
    tab->tab_control()
);

gtk_stack_remove(
    GTK_STACK(content_stack_),
    tab->web_view()
);

tabs_.erase(it);

if (was_active) {
    const std::size_t next_index =
        index == tabs_.size()
            ? index - 1
            : index;

    select_tab(
        tabs_[next_index].get()
    );
}
```

}

void BrowserWindow::tab_uri_changed(
Tab* tab,
const char* uri
) {
if (tab != active_tab_) {
return;
}

```
gtk_editable_set_text(
    GTK_EDITABLE(url_bar_),
    uri ? uri : ""
);

if (bookmarks_panel_) {
    const char* title =
        webkit_web_view_get_title(
            tab->view()
        );

    bookmarks_panel_->set_current_page(
        uri ? uri : "",
        title ? title : ""
    );
}
```

}

void BrowserWindow::navigate_from_entry() {
Tab* tab = active_tab();

```
if (tab == nullptr) {
    return;
}

const std::string input =
    gtk_editable_get_text(
        GTK_EDITABLE(url_bar_)
    );

if (input.empty()) {
    return;
}

std::string uri;

if (is_url_candidate(input)) {
    gchar* scheme =
        g_uri_parse_scheme(
            input.c_str()
        );

    if (scheme == nullptr) {
        uri =
            "https://" + input;
    } else {
        uri = input;
        g_free(scheme);
    }
} else {
    gchar* escaped =
        g_uri_escape_string(
            input.c_str(),
            nullptr,
            FALSE
        );

    uri =
        config_.search_engine() +
        escaped;

    g_free(escaped);
}

tab->load_uri(
    uri.c_str()
);
```

}

void BrowserWindow::on_address_activate(
GtkEntry*,
gpointer user_data
) {
static_cast<BrowserWindow*>(
user_data
)->navigate_from_entry();
}

void BrowserWindow::on_back_clicked(
GtkButton*,
gpointer user_data
) {
Tab* tab =
static_cast<BrowserWindow*>(
user_data
)->active_tab();

```
if (tab != nullptr &&
    webkit_web_view_can_go_back(
        tab->view()
    )) {

    webkit_web_view_go_back(
        tab->view()
    );
}
```

}

void BrowserWindow::on_forward_clicked(
GtkButton*,
gpointer user_data
) {
Tab* tab =
static_cast<BrowserWindow*>(
user_data
)->active_tab();

```
if (tab != nullptr &&
    webkit_web_view_can_go_forward(
        tab->view()
    )) {

    webkit_web_view_go_forward(
        tab->view()
    );
}
```

}

void BrowserWindow::on_reload_clicked(
GtkButton*,
gpointer user_data
) {
Tab* tab =
static_cast<BrowserWindow*>(
user_data
)->active_tab();

```
if (tab != nullptr) {
    webkit_web_view_reload(
        tab->view()
    );
}
```

}

void BrowserWindow::on_home_clicked(
GtkButton*,
gpointer user_data
) {
Tab* tab =
static_cast<BrowserWindow*>(
user_data
)->active_tab();

```
if (tab != nullptr) {
    tab->load_home();
}
```

}

void BrowserWindow::on_new_tab_clicked(
GtkButton*,
gpointer user_data
) {
static_cast<BrowserWindow*>(
user_data
)->open_tab();
}

void BrowserWindow::on_bookmarks_button_clicked_cb(
GtkButton*,
gpointer user_data
) {
static_cast<BrowserWindow*>(
user_data
)->on_bookmarks_button_clicked();
}

void BrowserWindow::on_history_button_clicked_cb(
GtkButton*,
gpointer user_data
) {
static_cast<BrowserWindow*>(
user_data
)->on_history_button_clicked();
}

void BrowserWindow::on_downloads_button_clicked_cb(
GtkButton*,
gpointer user_data
) {
static_cast<BrowserWindow*>(
user_data
)->on_downloads_button_clicked();
}

void BrowserWindow::setup_keyboard_shortcuts() {
GtkEventController* key_controller =
gtk_event_controller_key_new();

```
g_signal_connect(
    key_controller,
    "key-pressed",
    G_CALLBACK(on_key_pressed),
    this
);

gtk_widget_add_controller(
    window_,
    key_controller
);
```

}

gboolean BrowserWindow::on_key_pressed(
GtkEventControllerKey* controller,
guint keyval,
guint keycode,
GdkModifierType state,
gpointer user_data
) {
(void)controller;
(void)keycode;

```
auto* window =
    static_cast<BrowserWindow*>(
        user_data
    );

gboolean is_ctrl =
    state & GDK_CONTROL_MASK;

gboolean is_shift =
    state & GDK_SHIFT_MASK;

if (is_ctrl && keyval == GDK_KEY_t) {
    window->open_tab();
    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_w) {
    if (window->active_tab_) {
        window->close_tab(
            window->active_tab_
        );
    }

    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_Tab) {
    if (window->tabs_.empty()) {
        return TRUE;
    }

    auto it =
        std::find_if(
            window->tabs_.begin(),
            window->tabs_.end(),
            [window](const std::unique_ptr<Tab>& tab) {
                return tab.get() ==
                       window->active_tab_;
            }
        );

    if (it != window->tabs_.end()) {
        auto next =
            std::next(it);

        if (next == window->tabs_.end()) {
            next =
                window->tabs_.begin();
        }

        window->select_tab(
            next->get()
        );
    }

    return TRUE;
}

if (is_ctrl &&
    is_shift &&
    keyval == GDK_KEY_ISO_Left_Tab) {

    if (window->tabs_.empty()) {
        return TRUE;
    }

    auto it =
        std::find_if(
            window->tabs_.begin(),
            window->tabs_.end(),
            [window](const std::unique_ptr<Tab>& tab) {
                return tab.get() ==
                       window->active_tab_;
            }
        );

    if (it != window->tabs_.end()) {
        auto prev =
            std::prev(it);

        if (it == window->tabs_.begin()) {
            prev =
                std::prev(
                    window->tabs_.end()
                );
        }

        window->select_tab(
            prev->get()
        );
    }

    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_l) {
    gtk_widget_grab_focus(
        window->url_bar_
    );

    gtk_editable_select_region(
        GTK_EDITABLE(window->url_bar_),
        0,
        -1
    );

    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_f) {
    if (window->active_tab_) {
        g_warning(
            "Ctrl+F: Búsqueda en página activada"
        );
    }

    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_h) {
    window->on_history_button_clicked();
    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_d) {
    window->on_bookmarks_button_clicked();
    return TRUE;
}

if (is_ctrl && keyval == GDK_KEY_j) {
    window->on_downloads_button_clicked();
    return TRUE;
}

return FALSE;
```

}

void BrowserWindow::setup_ui_panels() {
history_panel_ =
std::make_unique<HistoryPanel>(
history_
);

```
bookmarks_panel_ =
    std::make_unique<BookmarksPanel>(
        bookmarks_
    );

downloads_panel_ =
    std::make_unique<DownloadsPanel>(
        download_manager_.get()
    );

history_panel_->set_on_entry_activated(
    [this](const std::string& url) {
        Tab* tab = active_tab();

        if (tab == nullptr) {
            tab = &open_tab();
        }

        tab->load_uri(
            url.c_str()
        );

        if (history_popover_) {
            gtk_widget_set_visible(
                history_popover_,
                FALSE
            );
        }
    }
);

bookmarks_panel_->set_on_entry_activated(
    [this](const std::string& url) {
        Tab* tab = active_tab();

        if (tab == nullptr) {
            tab = &open_tab();
        }

        tab->load_uri(
            url.c_str()
        );

        if (bookmarks_popover_) {
            gtk_widget_set_visible(
                bookmarks_popover_,
                FALSE
            );
        }
    }
);

bookmarks_panel_->set_on_bookmark_added(
    [this]() {
        if (bookmarks_panel_) {
            bookmarks_panel_->refresh();
        }
    }
);
```

}

void BrowserWindow::on_history_button_clicked() {
if (!history_popover_) {
history_popover_ =
gtk_popover_new();

```
    GtkWidget* panel =
        history_panel_->create_panel();

    gtk_popover_set_child(
        GTK_POPOVER(history_popover_),
        panel
    );

    gtk_widget_set_size_request(
        panel,
        420,
        500
    );

    gtk_widget_set_parent(
        history_popover_,
        history_button_
    );

    history_panel_->refresh();

    gtk_popover_popup(
        GTK_POPOVER(history_popover_)
    );

    return;
}

history_panel_->refresh();

if (gtk_widget_get_visible(
        history_popover_
    )) {

    gtk_widget_set_visible(
        history_popover_,
        FALSE
    );
} else {
    gtk_popover_popup(
        GTK_POPOVER(history_popover_)
    );
}
```

}

void BrowserWindow::on_bookmarks_button_clicked() {
if (!bookmarks_popover_) {
bookmarks_popover_ =
gtk_popover_new();

```
    GtkWidget* panel =
        bookmarks_panel_->create_panel();

    gtk_popover_set_child(
        GTK_POPOVER(bookmarks_popover_),
        panel
    );

    gtk_widget_set_size_request(
        panel,
        420,
        500
    );

    gtk_widget_set_parent(
        bookmarks_popover_,
        bookmarks_button_
    );

    if (active_tab_) {
        const char* uri =
            webkit_web_view_get_uri(
                active_tab_->view()
            );

        const char* title =
            webkit_web_view_get_title(
                active_tab_->view()
            );

        bookmarks_panel_->set_current_page(
            uri ? uri : "",
            title ? title : ""
        );
    }

    bookmarks_panel_->refresh();

    gtk_popover_popup(
        GTK_POPOVER(bookmarks_popover_)
    );

    return;
}

if (active_tab_) {
    const char* uri =
        webkit_web_view_get_uri(
            active_tab_->view()
        );

    const char* title =
        webkit_web_view_get_title(
            active_tab_->view()
        );

    bookmarks_panel_->set_current_page(
        uri ? uri : "",
        title ? title : ""
    );
}

bookmarks_panel_->refresh();

if (gtk_widget_get_visible(
        bookmarks_popover_
    )) {

    gtk_widget_set_visible(
        bookmarks_popover_,
        FALSE
    );
} else {
    gtk_popover_popup(
        GTK_POPOVER(bookmarks_popover_)
    );
}
```

}

void BrowserWindow::on_downloads_button_clicked() {
if (!downloads_popover_) {
downloads_popover_ =
gtk_popover_new();

```
    GtkWidget* panel =
        downloads_panel_->create_panel();

    gtk_popover_set_child(
        GTK_POPOVER(downloads_popover_),
        panel
    );

    gtk_widget_set_size_request(
        panel,
        450,
        500
    );

    gtk_widget_set_parent(
        downloads_popover_,
        downloads_button_
    );

    downloads_panel_->refresh();

    gtk_popover_popup(
        GTK_POPOVER(downloads_popover_)
    );

    return;
}

downloads_panel_->refresh();

if (gtk_widget_get_visible(
        downloads_popover_
    )) {

    gtk_widget_set_visible(
        downloads_popover_,
        FALSE
    );
} else {
    gtk_popover_popup(
        GTK_POPOVER(downloads_popover_)
    );
}
```

}
