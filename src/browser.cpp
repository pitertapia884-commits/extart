#include "browser.h"
#include <string>

Browser* Browser::_instance = nullptr;

Browser::Browser(GtkApplication* app) {
    _instance = this;
    build_ui(app);
}

Browser* Browser::instance() {
    return _instance;
}

WebKitWebView* Browser::get_current_webview() {
    int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    if (page < 0) return nullptr;
    GtkWidget* wv = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page);
    if (!WEBKIT_IS_WEB_VIEW(wv)) return nullptr;
    return WEBKIT_WEB_VIEW(wv);
}
void Browser::new_tab(const std::string& url) {
    WebKitWebContext* ctx = webkit_web_context_new();
    GtkWidget* wv = GTK_WIDGET(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "web-context", ctx,
        nullptr));
    g_object_unref(ctx);

    WebKitSettings* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(wv));
    webkit_settings_set_enable_smooth_scrolling(settings, FALSE);
    webkit_settings_set_javascript_can_access_clipboard(settings, FALSE);

    gtk_widget_set_vexpand(wv, TRUE);
    gtk_widget_set_hexpand(wv, TRUE);

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(wv), url.c_str());
    g_signal_connect(wv, "load-changed", G_CALLBACK(on_load_changed), NULL);

    GtkWidget* tab_box   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget* tab_label = gtk_label_new("Nueva pestaña");
    GtkWidget* tab_close = gtk_button_new_with_label("✕");
    gtk_widget_add_css_class(tab_close, "tab-close");

    gtk_box_append(GTK_BOX(tab_box), tab_label);
    gtk_box_append(GTK_BOX(tab_box), tab_close);

    int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    int index = gtk_notebook_insert_page(GTK_NOTEBOOK(notebook), wv, tab_box, n - 1);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), index);

    g_object_set_data(G_OBJECT(wv), "tab-label", tab_label);
    g_signal_connect_swapped(tab_close, "clicked", G_CALLBACK(on_close_tab), wv);

    gtk_widget_set_visible(wv, TRUE);
}

void Browser::on_close_tab(GtkWidget* wv, GtkWidget* button) {
    Browser* b = Browser::instance();
    int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(b->notebook));

    if (n <= 2) return;

    int index = gtk_notebook_page_num(GTK_NOTEBOOK(b->notebook), wv);
    if (index < 0) return;

    int target = -1;
    if (index > 0) {
        GtkWidget* left = gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), index - 1);
        if (WEBKIT_IS_WEB_VIEW(left)) target = index - 1;
    }
    if (target < 0 && index < n - 1) {
        GtkWidget* right = gtk_notebook_get_nth_page(GTK_NOTEBOOK(b->notebook), index + 1);
        if (WEBKIT_IS_WEB_VIEW(right)) target = index + 1;
    }

    b->switching_tab = true;
    if (target >= 0)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(b->notebook), target);
    gtk_notebook_remove_page(GTK_NOTEBOOK(b->notebook), index);
    b->switching_tab = false;
}

void Browser::build_ui(GtkApplication* app) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "EXTART");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(navbar, 6);
    gtk_widget_set_margin_end(navbar, 6);
    gtk_widget_set_margin_top(navbar, 6);
    gtk_widget_set_margin_bottom(navbar, 6);
    gtk_widget_add_css_class(navbar, "navbar");

    GtkWidget* btn_back    = gtk_button_new_with_label("←");
    GtkWidget* btn_forward = gtk_button_new_with_label("→");
    GtkWidget* btn_reload  = gtk_button_new_with_label("↺");
    GtkWidget* btn_home    = gtk_button_new_with_label("⌂");

    url_bar = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(url_bar), "");
    gtk_widget_set_hexpand(url_bar, TRUE);

    g_signal_connect(url_bar,     "activate", G_CALLBACK(on_navigate), NULL);
    g_signal_connect(btn_back,    "clicked",  G_CALLBACK(on_back),     NULL);
    g_signal_connect(btn_forward, "clicked",  G_CALLBACK(on_forward),  NULL);
    g_signal_connect(btn_reload,  "clicked",  G_CALLBACK(on_reload),   NULL);
    g_signal_connect(btn_home,    "clicked",  G_CALLBACK(on_home),     NULL);

    gtk_box_append(GTK_BOX(navbar), btn_back);
    gtk_box_append(GTK_BOX(navbar), btn_forward);
    gtk_box_append(GTK_BOX(navbar), btn_reload);
    gtk_box_append(GTK_BOX(navbar), btn_home);
    gtk_box_append(GTK_BOX(navbar), url_bar);

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_widget_set_hexpand(notebook, TRUE);

    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_switch_page), NULL);

    gtk_box_append(GTK_BOX(vbox), navbar);
    gtk_box_append(GTK_BOX(vbox), notebook);

    gtk_window_set_child(GTK_WINDOW(window), vbox);

    load_css();

    GtkWidget* placeholder = gtk_label_new("");
    GtkWidget* plus_btn = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(plus_btn, "newtab-btn");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), placeholder, plus_btn);
    gtk_widget_set_visible(placeholder, TRUE);
    g_signal_connect(plus_btn, "clicked", G_CALLBACK(on_new_tab), NULL);

    new_tab();

    gtk_window_present(GTK_WINDOW(window));
}

void Browser::load_css() {
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "../style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

void Browser::on_navigate(GtkWidget* widget, gpointer user_data) {
    Browser* b = Browser::instance();
    WebKitWebView* wv = b->get_current_webview();
    if (!wv) return;

    const char* url = gtk_editable_get_text(GTK_EDITABLE(b->url_bar));
    std::string uri(url);

    if (uri.find("http://") == 0 || uri.find("https://") == 0 || uri.find("file://") == 0) {
        // URL completa
    } else if (uri.find(".") != std::string::npos && uri.find(" ") == std::string::npos) {
        uri = "https://" + uri;
    } else {
        std::string encoded;
        for (char c : uri) {
            if (c == ' ') encoded += "+";
            else encoded += c;
        }
        uri = "https://duckduckgo.com/?q=" + encoded;
    }

    webkit_web_view_load_uri(wv, uri.c_str());
}

void Browser::on_back(GtkWidget* widget, gpointer user_data) {
    WebKitWebView* wv = Browser::instance()->get_current_webview();
    if (wv) webkit_web_view_go_back(wv);
}

void Browser::on_forward(GtkWidget* widget, gpointer user_data) {
    WebKitWebView* wv = Browser::instance()->get_current_webview();
    if (wv) webkit_web_view_go_forward(wv);
}

void Browser::on_reload(GtkWidget* widget, gpointer user_data) {
    WebKitWebView* wv = Browser::instance()->get_current_webview();
    if (wv) webkit_web_view_reload(wv);
}

void Browser::on_home(GtkWidget* widget, gpointer user_data) {
    WebKitWebView* wv = Browser::instance()->get_current_webview();
    if (wv) webkit_web_view_load_uri(wv, "file:///home/arch/extart/home.html");
}

void Browser::on_new_tab(GtkWidget* widget, gpointer user_data) {
    Browser::instance()->new_tab();
}

void Browser::on_load_changed(WebKitWebView* wv, WebKitLoadEvent event, gpointer user_data) {
    Browser* b = Browser::instance();
    if (!b) return;

    WebKitWebView* active = b->get_current_webview();
    if (wv != active) return;

    if (event == WEBKIT_LOAD_COMMITTED) {
        const char* uri = webkit_web_view_get_uri(wv);
        if (uri) gtk_editable_set_text(GTK_EDITABLE(b->url_bar), uri);
    }

    if (event == WEBKIT_LOAD_FINISHED) {
        const char* title = webkit_web_view_get_title(wv);
        GtkWidget* label = GTK_WIDGET(g_object_get_data(G_OBJECT(wv), "tab-label"));
        if (label && title) gtk_label_set_text(GTK_LABEL(label), title);
    }
}

void Browser::on_switch_page(GtkNotebook* nb, GtkWidget* page, guint page_num, gpointer user_data) {
    Browser* b = Browser::instance();

    if (b->switching_tab) return;

    if (!WEBKIT_IS_WEB_VIEW(page)) {
        int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));
        for (int i = n - 1; i >= 0; i--) {
            GtkWidget* p = gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb), i);
            if (WEBKIT_IS_WEB_VIEW(p)) {
                b->switching_tab = true;
                gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), i);
                b->switching_tab = false;
                break;
            }
        }
        return;
    }

    WebKitWebView* wv = WEBKIT_WEB_VIEW(page);
    const char* uri = webkit_web_view_get_uri(wv);
    if (uri) gtk_editable_set_text(GTK_EDITABLE(b->url_bar), uri);
}
// viva el yuri