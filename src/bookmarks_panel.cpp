#include "bookmarks_panel.hpp"

#include "bookmarks.hpp"

#include <glib.h>

BookmarksPanel::BookmarksPanel(Bookmarks* bookmarks)
    : bookmarks_(bookmarks) {
}

BookmarksPanel::~BookmarksPanel() = default;

GtkWidget* BookmarksPanel::create_panel() {
    GtkWidget* box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);

    // Sección: página actual
    current_page_box_ =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            6
        );

    GtkWidget* current_label =
        gtk_label_new("Current page");

    gtk_widget_add_css_class(
        current_label,
        "heading"
    );

    gtk_label_set_xalign(
        GTK_LABEL(current_label),
        0.0
    );

    GtkWidget* current_button_box =
        gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL,
            6
        );

    add_button_ =
        gtk_button_new_with_label(
            "Add bookmark"
        );

    gtk_widget_set_hexpand(
        add_button_,
        TRUE
    );

    g_signal_connect(
        add_button_,
        "clicked",
        G_CALLBACK(on_add_bookmark_clicked),
        this
    );

    gtk_box_append(
        GTK_BOX(current_button_box),
        add_button_
    );

    gtk_box_append(
        GTK_BOX(current_page_box_),
        current_label
    );

    gtk_box_append(
        GTK_BOX(current_page_box_),
        current_button_box
    );

    // Separador
    GtkWidget* separator =
        gtk_separator_new(
            GTK_ORIENTATION_HORIZONTAL
        );

    // Sección: lista de marcadores
    GtkWidget* all_label =
        gtk_label_new("Bookmarks");

    gtk_widget_add_css_class(
        all_label,
        "heading"
    );

    gtk_label_set_xalign(
        GTK_LABEL(all_label),
        0.0
    );

    // Lista scrollable
    GtkWidget* scrolled =
        gtk_scrolled_window_new();

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scrolled),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC
    );

    gtk_widget_set_vexpand(
        scrolled,
        TRUE
    );

    gtk_widget_set_hexpand(
        scrolled,
        TRUE
    );

    gtk_widget_set_size_request(
        scrolled,
        350,
        300
    );

    list_box_ =
        gtk_list_box_new();

    gtk_widget_add_css_class(
        list_box_,
        "navigation-sidebar"
    );

    g_signal_connect(
        list_box_,
        "row-activated",
        G_CALLBACK(on_row_activated),
        this
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scrolled),
        list_box_
    );

    // Label para estado vacío
    empty_label_ =
        gtk_label_new("No bookmarks yet");

    gtk_widget_add_css_class(
        empty_label_,
        "dim-label"
    );

    gtk_label_set_justify(
        GTK_LABEL(empty_label_),
        GTK_JUSTIFY_CENTER
    );

    // Stack para mostrar lista o vacío
    GtkWidget* stack =
        gtk_stack_new();

    gtk_stack_add_named(
        GTK_STACK(stack),
        scrolled,
        "list"
    );

    gtk_stack_add_named(
        GTK_STACK(stack),
        empty_label_,
        "empty"
    );

    gtk_stack_set_visible_child_name(
        GTK_STACK(stack),
        "empty"
    );

    gtk_widget_set_vexpand(
        stack,
        TRUE
    );

    gtk_widget_set_hexpand(
        stack,
        TRUE
    );

    // Armar panel
    gtk_box_append(
        GTK_BOX(box),
        current_page_box_
    );

    gtk_box_append(
        GTK_BOX(box),
        separator
    );

    gtk_box_append(
        GTK_BOX(box),
        all_label
    );

    gtk_box_append(
        GTK_BOX(box),
        stack
    );

    populate_list();
    update_add_button();

    return box;
}

void BookmarksPanel::set_current_page(
    const std::string& url,
    const std::string& title
) {
    current_url_ = url;
    current_title_ = title;

    update_add_button();
}

void BookmarksPanel::set_on_entry_activated(
    OnEntryActivated callback
) {
    on_entry_activated_ =
        std::move(callback);
}

void BookmarksPanel::set_on_bookmark_added(
    OnBookmarkAdded callback
) {
    on_bookmark_added_ =
        std::move(callback);
}

void BookmarksPanel::populate_list() {
    if (!list_box_) {
        return;
    }

    // Limpiar lista
    while (GtkWidget* child =
               gtk_widget_get_first_child(
                   list_box_
               )) {

        gtk_list_box_remove(
            GTK_LIST_BOX(list_box_),
            child
        );
    }

    const auto& bookmarks =
        bookmarks_->bookmarks();

    if (bookmarks.empty()) {
        return;
    }

    // Agregar cada bookmark
    for (const auto& bm :
         bookmarks) {

        GtkWidget* row =
            gtk_box_new(
                GTK_ORIENTATION_HORIZONTAL,
                8
            );

        gtk_widget_set_margin_start(
            row,
            8
        );

        gtk_widget_set_margin_end(
            row,
            8
        );

        gtk_widget_set_margin_top(
            row,
            6
        );

        gtk_widget_set_margin_bottom(
            row,
            6
        );

        // Título del bookmark
        GtkWidget* title_label =
            gtk_label_new(
                bm.title.c_str()
            );

        gtk_label_set_wrap(
            GTK_LABEL(title_label),
            TRUE
        );

        gtk_widget_set_hexpand(
            title_label,
            TRUE
        );

        gtk_label_set_xalign(
            GTK_LABEL(title_label),
            0.0
        );

        // Botón remover
        GtkWidget* remove_button =
            gtk_button_new_with_label("×");

        gtk_widget_add_css_class(
            remove_button,
            "flat"
        );

        gtk_widget_set_size_request(
            remove_button,
            30,
            30
        );

        g_signal_connect(
            remove_button,
            "clicked",
            G_CALLBACK(on_remove_clicked),
            this
        );

        // Guardar URL en el botón
        g_object_set_data_full(
            G_OBJECT(remove_button),
            "url",
            g_strdup(
                bm.url.c_str()
            ),
            g_free
        );

        gtk_box_append(
            GTK_BOX(row),
            title_label
        );

        gtk_box_append(
            GTK_BOX(row),
            remove_button
        );

        // GtkListBoxRow real
        GtkListBoxRow* list_row =
            GTK_LIST_BOX_ROW(
                gtk_list_box_row_new()
            );

        gtk_list_box_row_set_child(
            list_row,
            row
        );

        // Guardar URL en el row
        g_object_set_data_full(
            G_OBJECT(list_row),
            "url",
            g_strdup(
                bm.url.c_str()
            ),
            g_free
        );

        gtk_list_box_append(
            GTK_LIST_BOX(list_box_),
            GTK_WIDGET(list_row)
        );
    }
}

void BookmarksPanel::update_add_button() {
    if (!add_button_ ||
        !bookmarks_) {
        return;
    }

    bool is_bookmarked =
        bookmarks_->is_bookmarked(
            current_url_
        );

    if (is_bookmarked) {
        gtk_button_set_label(
            GTK_BUTTON(add_button_),
            "★ Bookmarked"
        );

        gtk_widget_set_sensitive(
            add_button_,
            FALSE
        );

        gtk_widget_add_css_class(
            add_button_,
            "suggested-action"
        );
    } else {
        gtk_button_set_label(
            GTK_BUTTON(add_button_),
            "☆ Add bookmark"
        );

        gtk_widget_set_sensitive(
            add_button_,
            !current_url_.empty()
        );

        gtk_widget_remove_css_class(
            add_button_,
            "suggested-action"
        );
    }
}

void BookmarksPanel::on_row_activated(
    GtkListBox*,
    GtkListBoxRow* row,
    gpointer user_data
) {
    auto* panel =
        static_cast<BookmarksPanel*>(
            user_data
        );

    if (!panel ||
        !panel->on_entry_activated_) {
        return;
    }

    const char* url =
        static_cast<const char*>(
            g_object_get_data(
                G_OBJECT(row),
                "url"
            )
        );

    if (url != nullptr) {
        panel->on_entry_activated_(
            std::string(url)
        );
    }
}

void BookmarksPanel::on_add_bookmark_clicked(
    GtkButton*,
    gpointer user_data
) {
    auto* panel =
        static_cast<BookmarksPanel*>(
            user_data
        );

    if (!panel ||
        panel->current_url_.empty() ||
        !panel->bookmarks_) {
        return;
    }

    panel->bookmarks_->add(
        panel->current_url_,
        panel->current_title_
    );

    panel->refresh();
    panel->update_add_button();

    if (panel->on_bookmark_added_) {
        panel->on_bookmark_added_();
    }
}

void BookmarksPanel::on_remove_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* panel =
        static_cast<BookmarksPanel*>(
            user_data
        );

    if (!panel ||
        !panel->bookmarks_) {
        return;
    }

    const char* url =
        static_cast<const char*>(
            g_object_get_data(
                G_OBJECT(button),
                "url"
            )
        );

    if (url != nullptr) {
        panel->bookmarks_->remove(
            std::string(url)
        );

        panel->refresh();
    }
}

void BookmarksPanel::refresh() {
    if (!list_box_) {
        return;
    }

    populate_list();

    // list_box_ -> scrolled -> stack
    GtkWidget* scrolled =
        gtk_widget_get_parent(
            list_box_
        );

    if (scrolled) {
        GtkWidget* stack =
            gtk_widget_get_parent(
                scrolled
            );

        if (stack &&
            GTK_IS_STACK(stack)) {

            if (bookmarks_->bookmarks().empty()) {
                gtk_stack_set_visible_child_name(
                    GTK_STACK(stack),
                    "empty"
                );
            } else {
                gtk_stack_set_visible_child_name(
                    GTK_STACK(stack),
                    "list"
                );
            }
        }
    }

    update_add_button();
}