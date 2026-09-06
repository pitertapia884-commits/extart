#include "bookmarks_panel.hpp"

#include <utility>

BookmarksPanel::BookmarksPanel(Bookmarks* bookmarks)
    : bookmarks_(bookmarks) {
}

GtkWidget* BookmarksPanel::create_panel() {
    if (panel_ != nullptr) {
        populate_list();
        return panel_;
    }

    panel_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(panel_, 8);
    gtk_widget_set_margin_bottom(panel_, 8);
    gtk_widget_set_margin_start(panel_, 8);
    gtk_widget_set_margin_end(panel_, 8);

    GtkWidget* header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_hexpand(header, TRUE);

    title_label_ = gtk_label_new("Marcadores");
    gtk_widget_set_hexpand(title_label_, TRUE);
    gtk_label_set_xalign(GTK_LABEL(title_label_), 0.0);

    add_button_ = gtk_button_new_with_label("☆");
    gtk_widget_set_tooltip_text(add_button_, "Agregar marcador");
    g_signal_connect(add_button_, "clicked", G_CALLBACK(on_add_clicked), this);

    gtk_box_append(GTK_BOX(header), title_label_);
    gtk_box_append(GTK_BOX(header), add_button_);
    gtk_box_append(GTK_BOX(panel_), header);

    list_box_ = gtk_list_box_new();
    gtk_list_box_set_selection_mode(
        GTK_LIST_BOX(list_box_), GTK_SELECTION_NONE);
    gtk_widget_set_vexpand(list_box_, TRUE);
    gtk_widget_set_hexpand(list_box_, TRUE);
    gtk_box_append(GTK_BOX(panel_), list_box_);

    populate_list();
    update_add_button();

    return panel_;
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
    on_entry_activated_ = std::move(callback);
}

void BookmarksPanel::set_on_bookmark_added(
    OnBookmarkAdded callback
) {
    on_bookmark_added_ = std::move(callback);
}

void BookmarksPanel::populate_list() {
    if (!list_box_ || !bookmarks_) {
        return;
    }

    while (GtkWidget* child =
               gtk_widget_get_first_child(list_box_)) {
        gtk_list_box_remove(
            GTK_LIST_BOX(list_box_),
            child
        );
    }

    const auto& bookmarks = bookmarks_->bookmarks();

    for (const auto& bm : bookmarks) {
        GtkWidget* row =
            gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

        gtk_widget_set_margin_start(row, 8);
        gtk_widget_set_margin_end(row, 8);
        gtk_widget_set_margin_top(row, 6);
        gtk_widget_set_margin_bottom(row, 6);

        GtkWidget* title_label =
            gtk_label_new(bm.title.c_str());

        gtk_label_set_wrap(GTK_LABEL(title_label), TRUE);
        gtk_widget_set_hexpand(title_label, TRUE);
        gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);

        GtkWidget* open_button =
            gtk_button_new_with_label("Abrir");

        GtkWidget* remove_button =
            gtk_button_new_with_label("×");
        gtk_widget_set_tooltip_text(
            remove_button,
            "Eliminar marcador"
        );

        g_object_set_data_full(
            G_OBJECT(open_button),
            "bookmark-url",
            g_strdup(bm.url.c_str()),
            g_free
        );

        g_object_set_data_full(
            G_OBJECT(remove_button),
            "bookmark-url",
            g_strdup(bm.url.c_str()),
            g_free
        );

        g_signal_connect(
            open_button,
            "clicked",
            G_CALLBACK(on_open_clicked),
            this
        );

        g_signal_connect(
            remove_button,
            "clicked",
            G_CALLBACK(on_remove_clicked),
            this
        );

        gtk_box_append(GTK_BOX(row), title_label);
        gtk_box_append(GTK_BOX(row), open_button);
        gtk_box_append(GTK_BOX(row), remove_button);

        gtk_list_box_append(
            GTK_LIST_BOX(list_box_),
            row
        );
    }
}

void BookmarksPanel::update_add_button() {
    if (!add_button_) {
        return;
    }

    const bool bookmarked =
        bookmarks_ != nullptr &&
        !current_url_.empty() &&
        bookmarks_->is_bookmarked(current_url_);

    gtk_button_set_label(
        GTK_BUTTON(add_button_),
        bookmarked ? "★" : "☆"
    );

    gtk_widget_set_tooltip_text(
        add_button_,
        bookmarked
            ? "Eliminar marcador"
            : "Agregar marcador"
    );
}

void BookmarksPanel::on_add_clicked(
    GtkButton*,
    gpointer user_data
) {
    auto* self = static_cast<BookmarksPanel*>(user_data);
    if (!self || !self->bookmarks_ || self->current_url_.empty()) {
        return;
    }

    if (self->bookmarks_->is_bookmarked(self->current_url_)) {
        self->bookmarks_->remove(self->current_url_);
    } else {
        self->bookmarks_->add(
            self->current_url_,
            self->current_title_
        );

        if (self->on_bookmark_added_) {
            self->on_bookmark_added_(self->current_url_);
        }
    }

    self->populate_list();
    self->update_add_button();
}

void BookmarksPanel::on_open_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* self = static_cast<BookmarksPanel*>(user_data);
    if (!self || !button || !self->on_entry_activated_) {
        return;
    }

    const char* url = static_cast<const char*>(
        g_object_get_data(G_OBJECT(button), "bookmark-url")
    );

    if (url && *url) {
        self->on_entry_activated_(url);
    }
}

void BookmarksPanel::on_remove_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* self = static_cast<BookmarksPanel*>(user_data);
    if (!self || !button || !self->bookmarks_) {
        return;
    }

    const char* url = static_cast<const char*>(
        g_object_get_data(G_OBJECT(button), "bookmark-url")
    );

    if (!url || !*url) {
        return;
    }

    self->bookmarks_->remove(url);
    self->populate_list();
    self->update_add_button();
}
