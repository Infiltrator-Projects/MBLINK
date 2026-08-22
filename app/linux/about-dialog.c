// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file about-dialog.c
 * @brief Standard GTK4 About surface for the MBLINK product face.
 */
#include "about-dialog.h"

#include "mblink/mblink.h"

static gboolean destroy_about_on_close(GtkWindow *window, gpointer user_data)
{
    (void)user_data;
    gtk_window_destroy(window);
    return TRUE;
}

void mblink_linux_show_about(GtkWindow *parent)
{
    static const char *authors[] = {
        "Shannon Smith",
        NULL
    };
    static const char license_text[] =
        "MBLINK is free software licensed under the GNU General Public "
        "License version 3 or, at your option, any later version "
        "(GPL-3.0-or-later).\n\n"
        "See LICENSE in the source package for the complete licence text.";
    GtkWidget *widget = gtk_about_dialog_new();
    GtkAboutDialog *about = GTK_ABOUT_DIALOG(widget);
    GdkTexture *logo = gdk_texture_new_from_resource(
        "/com/github/The-First-Infiltrator/MBLINK/mblink-emblem.png");

    gtk_window_set_title(GTK_WINDOW(widget), "About MBLINK");
    gtk_window_set_transient_for(GTK_WINDOW(widget), parent);
    gtk_window_set_modal(GTK_WINDOW(widget), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(widget), TRUE);
    gtk_about_dialog_set_program_name(about, "MBLINK");
    gtk_about_dialog_set_version(about, mblink_version());
    gtk_about_dialog_set_comments(
        about,
        "Mercedes-oriented vehicle diagnostics built on the shared LINK engine.");
    gtk_about_dialog_set_authors(about, authors);
    gtk_about_dialog_set_website(
        about, "https://github.com/The-First-Infiltrator/MBLINK");
    gtk_about_dialog_set_website_label(about, "Website");
    gtk_about_dialog_set_copyright(about, "Copyright © 2026 Shannon Smith");
    gtk_about_dialog_set_license(about, license_text);
    gtk_about_dialog_set_wrap_license(about, TRUE);
    if (logo != NULL) {
        gtk_about_dialog_set_logo(about, GDK_PAINTABLE(logo));
        g_object_unref(logo);
    }
    g_signal_connect(widget, "close-request",
                     G_CALLBACK(destroy_about_on_close), NULL);
    gtk_window_present(GTK_WINDOW(widget));
}
