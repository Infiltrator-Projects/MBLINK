// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mblink.h"

#include <gtk/gtk.h>
#include <stddef.h>

typedef struct MblinkLinuxApp {
    GtkWidget *content_title;
    GtkWidget *content_summary;
} MblinkLinuxApp;

static void set_margins(GtkWidget *widget, int margin)
{
    gtk_widget_set_margin_top(widget, margin);
    gtk_widget_set_margin_bottom(widget, margin);
    gtk_widget_set_margin_start(widget, margin);
    gtk_widget_set_margin_end(widget, margin);
}

static GtkWidget *workspace_row(const MblinkWorkspaceSectionDescriptor *descriptor)
{
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *title = gtk_label_new(descriptor->title);
    GtkWidget *summary = gtk_label_new(descriptor->summary);

    gtk_label_set_xalign(GTK_LABEL(title), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(summary), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(summary), TRUE);
    gtk_widget_add_css_class(title, "heading");
    gtk_widget_add_css_class(summary, "dim-label");
    set_margins(box, 10);

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

static GtkWidget *build_sidebar(MblinkLinuxApp *app)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *title = gtk_label_new("MBLINK");
    GtkWidget *subtitle = gtk_label_new("Mercedes-Benz diagnostics");
    GtkWidget *version = gtk_label_new(mblink_version());
    GtkWidget *list = gtk_list_box_new();
    size_t index;

    gtk_widget_set_size_request(sidebar, 300, -1);
    set_margins(sidebar, 12);

    gtk_label_set_xalign(GTK_LABEL(title), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(version), 0.0F);
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_add_css_class(subtitle, "heading");
    gtk_widget_add_css_class(version, "dim-label");

    gtk_box_append(GTK_BOX(brand), title);
    gtk_box_append(GTK_BOX(brand), subtitle);
    gtk_box_append(GTK_BOX(brand), version);
    gtk_box_append(GTK_BOX(sidebar), brand);

    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    gtk_widget_add_css_class(list, "navigation-sidebar");
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

static GtkWidget *build_content(MblinkLinuxApp *app)
{
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *notice = gtk_label_new(
        "Linux shell foundation: the shared C workspace is live. "
        "Transport, UDS and Mercedes module discovery will populate the same "
        "screens as the iPhone build as those portable layers land.");

    app->content_title = gtk_label_new("Vehicle");
    app->content_summary = gtk_label_new(
        "Vehicle identity, adapter and connection information");

    gtk_label_set_xalign(GTK_LABEL(app->content_title), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(app->content_summary), 0.0F);
    gtk_label_set_xalign(GTK_LABEL(notice), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(app->content_summary), TRUE);
    gtk_label_set_wrap(GTK_LABEL(notice), TRUE);
    gtk_widget_add_css_class(app->content_title, "title-1");
    gtk_widget_add_css_class(app->content_summary, "heading");
    gtk_widget_add_css_class(notice, "dim-label");

    set_margins(content, 24);
    gtk_box_append(GTK_BOX(content), app->content_title);
    gtk_box_append(GTK_BOX(content), app->content_summary);
    gtk_box_append(GTK_BOX(content), notice);
    return content;
}

static void activate(GtkApplication *application, gpointer user_data)
{
    MblinkLinuxApp *app = user_data;
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *sidebar;
    GtkWidget *content;

    gtk_window_set_title(GTK_WINDOW(window), "MBLINK");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 720);

    sidebar = build_sidebar(app);
    content = build_content(app);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_widget_set_vexpand(content, TRUE);

    gtk_box_append(GTK_BOX(root), sidebar);
    gtk_box_append(GTK_BOX(root), separator);
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
