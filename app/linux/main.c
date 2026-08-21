// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mblink.h"

#include <gtk/gtk.h>
#include <stddef.h>

typedef struct MblinkLinuxApp {
    GtkWidget *content_title;
    GtkWidget *content_summary;
    GtkWidget *content_kicker;
} MblinkLinuxApp;

static const char mblink_css[] =
    "window, .mblink-root {"
    "  background: #050608;"
    "  color: #e8ecef;"
    "}"
    ".mblink-sidebar {"
    "  background: #0a0c0f;"
    "  border-right: 1px solid #34383d;"
    "}"
    ".mblink-brand {"
    "  padding: 20px 18px 18px 18px;"
    "  background: linear-gradient(135deg, #15191e, #090b0e);"
    "  border-bottom: 1px solid #34383d;"
    "}"
    ".mblink-title {"
    "  font-size: 30px;"
    "  font-weight: 900;"
    "  letter-spacing: 3px;"
    "  color: #eef1f3;"
    "}"
    ".mblink-kicker {"
    "  font-size: 10px;"
    "  font-weight: 800;"
    "  letter-spacing: 2px;"
    "  color: #b9c0c6;"
    "}"
    ".mblink-muted {"
    "  color: #7e858c;"
    "}"
    ".mblink-sidebar list {"
    "  background: transparent;"
    "}"
    ".mblink-sidebar row {"
    "  margin: 5px 10px;"
    "  border-radius: 12px;"
    "  border: 1px solid transparent;"
    "}"
    ".mblink-sidebar row:hover {"
    "  background: #15191e;"
    "  border-color: #34383d;"
    "}"
    ".mblink-sidebar row:selected {"
    "  background: #1a1f24;"
    "  border-color: #8d969e;"
    "}"
    ".mblink-card {"
    "  background: linear-gradient(135deg, #171b20, #0d1014);"
    "  border: 1px solid #353a40;"
    "  border-radius: 18px;"
    "  padding: 22px;"
    "}"
    ".mblink-card-title {"
    "  font-size: 26px;"
    "  font-weight: 800;"
    "  color: #eef1f3;"
    "}"
    ".mblink-card-kicker {"
    "  font-size: 10px;"
    "  font-weight: 800;"
    "  letter-spacing: 2px;"
    "  color: #8c949b;"
    "}"
    ".mblink-card-summary {"
    "  font-size: 15px;"
    "  color: #bdc4ca;"
    "}"
    ".mblink-chip {"
    "  padding: 7px 11px;"
    "  border-radius: 999px;"
    "  border: 1px solid #3b4147;"
    "  background: #111419;"
    "  color: #dfe4e8;"
    "  font-size: 11px;"
    "  font-weight: 700;"
    "}"
    ".mblink-rule {"
    "  background: #353a40;"
    "}"
    ".mblink-workspace-title {"
    "  font-weight: 700;"
    "  color: #e7ebee;"
    "}"
    ".mblink-workspace-summary {"
    "  color: #7f878e;"
    "}"
    ".mblink-command-note {"
    "  color: #8d959c;"
    "  font-size: 12px;"
    "}";

static void set_margins(GtkWidget *widget, int margin)
{
    gtk_widget_set_margin_top(widget, margin);
    gtk_widget_set_margin_bottom(widget, margin);
    gtk_widget_set_margin_start(widget, margin);
    gtk_widget_set_margin_end(widget, margin);
}

static GtkWidget *make_left_label(const char *text, const char *css_class)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    if (css_class != NULL) {
        gtk_widget_add_css_class(label, css_class);
    }
    return label;
}

static GtkWidget *workspace_row(const MblinkWorkspaceSectionDescriptor *descriptor)
{
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *title = make_left_label(descriptor->title,
                                       "mblink-workspace-title");
    GtkWidget *summary = make_left_label(descriptor->summary,
                                         "mblink-workspace-summary");

    set_margins(box, 11);
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), summary);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    g_object_set_data(G_OBJECT(row), "mblink-section",
                      GINT_TO_POINTER((int)descriptor->section));
    return row;
}

static void update_content(MblinkLinuxApp *app, MblinkWorkspaceSection section)
{
    const MblinkWorkspaceSectionDescriptor *descriptor =
        mblink_workspace_section(section);

    if (descriptor == NULL) {
        return;
    }

    gtk_label_set_text(GTK_LABEL(app->content_kicker), "DIAGNOSTIC WORKSPACE");
    gtk_label_set_text(GTK_LABEL(app->content_title), descriptor->title);
    gtk_label_set_text(GTK_LABEL(app->content_summary), descriptor->summary);
}

static void row_selected(GtkListBox *list_box,
                         GtkListBoxRow *row,
                         gpointer user_data)
{
    MblinkLinuxApp *app = user_data;
    gpointer value;

    (void)list_box;
    if (row == NULL) {
        return;
    }

    value = g_object_get_data(G_OBJECT(row), "mblink-section");
    update_content(app, (MblinkWorkspaceSection)GPOINTER_TO_INT(value));
}

static GtkWidget *build_brand_header(void)
{
    GtkWidget *brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *title = make_left_label("MBLINK", "mblink-title");
    GtkWidget *subtitle = make_left_label("MERCEDES DIAGNOSTIC COMMAND",
                                          "mblink-kicker");
    GtkWidget *target = make_left_label("C207 · OM651 · DELPHI CRD3.x",
                                        "mblink-muted");
    GtkWidget *version_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *version_label = gtk_label_new("BUILD");
    GtkWidget *version = gtk_label_new(mblink_version());

    gtk_widget_add_css_class(brand, "mblink-brand");
    gtk_widget_add_css_class(version_box, "mblink-chip");
    gtk_widget_add_css_class(version_label, "mblink-kicker");
    gtk_widget_add_css_class(version, "mblink-muted");

    gtk_box_append(GTK_BOX(version_box), version_label);
    gtk_box_append(GTK_BOX(version_box), version);
    gtk_widget_set_halign(version_box, GTK_ALIGN_START);

    gtk_box_append(GTK_BOX(brand), title);
    gtk_box_append(GTK_BOX(brand), subtitle);
    gtk_box_append(GTK_BOX(brand), target);
    gtk_box_append(GTK_BOX(brand), version_box);
    return brand;
}

static GtkWidget *build_sidebar(MblinkLinuxApp *app)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *brand = build_brand_header();
    GtkWidget *list = gtk_list_box_new();
    size_t index;

    gtk_widget_set_size_request(sidebar, 330, -1);
    gtk_widget_add_css_class(sidebar, "mblink-sidebar");
    gtk_box_append(GTK_BOX(sidebar), brand);

    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    gtk_widget_set_vexpand(list, TRUE);
    set_margins(list, 6);
    g_signal_connect(list, "row-selected", G_CALLBACK(row_selected), app);

    for (index = 0U; index < mblink_workspace_section_count(); ++index) {
        const MblinkWorkspaceSectionDescriptor *descriptor =
            mblink_workspace_section_at(index);
        if (descriptor != NULL) {
            gtk_list_box_append(GTK_LIST_BOX(list), workspace_row(descriptor));
        }
    }

    gtk_box_append(GTK_BOX(sidebar), list);
    return sidebar;
}

static GtkWidget *build_status_strip(void)
{
    GtkWidget *strip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *vehicle = gtk_label_new("C207 E 250 CDI");
    GtkWidget *engine = gtk_label_new("OM651");
    GtkWidget *ecu = gtk_label_new("CRD3.x");
    GtkWidget *endpoint = gtk_label_new("0x7E0 → 0x7E8");

    gtk_widget_add_css_class(vehicle, "mblink-chip");
    gtk_widget_add_css_class(engine, "mblink-chip");
    gtk_widget_add_css_class(ecu, "mblink-chip");
    gtk_widget_add_css_class(endpoint, "mblink-chip");

    gtk_box_append(GTK_BOX(strip), vehicle);
    gtk_box_append(GTK_BOX(strip), engine);
    gtk_box_append(GTK_BOX(strip), ecu);
    gtk_box_append(GTK_BOX(strip), endpoint);
    return strip;
}

static GtkWidget *build_content(MblinkLinuxApp *app)
{
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    GtkWidget *status = build_status_strip();
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *rule = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *notice = make_left_label(
        "The Linux shell shares the same portable C workspace, protocol engine, "
        "Mercedes evidence model and parameter catalogue as the iPhone build. "
        "Transport-specific screens fill in as the portable layers become live.",
        "mblink-command-note");

    app->content_kicker = make_left_label("DIAGNOSTIC WORKSPACE",
                                          "mblink-card-kicker");
    app->content_title = make_left_label("Vehicle", "mblink-card-title");
    app->content_summary = make_left_label(
        "Vehicle identity, adapter and connection information",
        "mblink-card-summary");

    gtk_widget_add_css_class(card, "mblink-card");
    gtk_widget_add_css_class(rule, "mblink-rule");

    gtk_box_append(GTK_BOX(card), app->content_kicker);
    gtk_box_append(GTK_BOX(card), app->content_title);
    gtk_box_append(GTK_BOX(card), app->content_summary);
    gtk_box_append(GTK_BOX(card), rule);
    gtk_box_append(GTK_BOX(card), notice);

    set_margins(content, 28);
    gtk_box_append(GTK_BOX(content), status);
    gtk_box_append(GTK_BOX(content), card);
    return content;
}

static void install_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();

#if GTK_CHECK_VERSION(4, 12, 0)
    gtk_css_provider_load_from_string(provider, mblink_css);
#else
    gtk_css_provider_load_from_data(provider, mblink_css, -1);
#endif
    if (display != NULL) {
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

static void activate(GtkApplication *application, gpointer user_data)
{
    MblinkLinuxApp *app = user_data;
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *sidebar;
    GtkWidget *content;

    install_css();

    gtk_window_set_title(GTK_WINDOW(window), "MBLINK · Mercedes Diagnostic Command");
    gtk_window_set_default_size(GTK_WINDOW(window), 1180, 760);

    gtk_widget_add_css_class(root, "mblink-root");
    sidebar = build_sidebar(app);
    content = build_content(app);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_widget_set_vexpand(content, TRUE);

    gtk_box_append(GTK_BOX(root), sidebar);
    gtk_box_append(GTK_BOX(root), content);
    gtk_window_set_child(GTK_WINDOW(window), root);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *application;
    MblinkLinuxApp app = { 0 };
    int status;

    application = gtk_application_new(
        "com.github.The-First-Infiltrator.MBLINK",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(application, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
