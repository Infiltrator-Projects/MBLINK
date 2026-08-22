// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mblink.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_om651.h"
#include "mblink/parameter.h"

#include <gtk/gtk.h>
#include <stddef.h>
#include <stdint.h>

typedef struct MblinkLinuxApp {
    GtkWidget *content_title;
    GtkWidget *content_summary;
    GtkWidget *content_kicker;
    GtkWidget *content_body;
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
    ".mblink-value {"
    "  color: #eef1f3;"
    "  font-weight: 700;"
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
    "  font-size: 20px;"
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
    ".mblink-warning {"
    "  color: #d19e47;"
    "  border-color: #72572f;"
    "}"
    ".mblink-success {"
    "  color: #63ab7c;"
    "  border-color: #365f45;"
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

static GtkWidget *make_chip(const char *text, const char *state_class)
{
    GtkWidget *chip = gtk_label_new(text);
    gtk_widget_add_css_class(chip, "mblink-chip");
    if (state_class != NULL) {
        gtk_widget_add_css_class(chip, state_class);
    }
    return chip;
}

static GtkWidget *make_card(const char *kicker, const char *title)
{
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(card, "mblink-card");
    if (kicker != NULL) {
        gtk_box_append(GTK_BOX(card),
                       make_left_label(kicker, "mblink-card-kicker"));
    }
    if (title != NULL) {
        gtk_box_append(GTK_BOX(card),
                       make_left_label(title, "mblink-card-title"));
    }
    return card;
}

static void append_detail(GtkWidget *card, const char *label, const char *value)
{
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    GtkWidget *name = make_left_label(label, "mblink-muted");
    GtkWidget *detail = make_left_label(value, "mblink-value");

    gtk_widget_set_hexpand(name, TRUE);
    gtk_label_set_xalign(GTK_LABEL(detail), 1.0F);
    gtk_label_set_selectable(GTK_LABEL(detail), TRUE);
    gtk_box_append(GTK_BOX(row), name);
    gtk_box_append(GTK_BOX(row), detail);
    gtk_box_append(GTK_BOX(card), row);
}

static void clear_box(GtkWidget *box)
{
    GtkWidget *child = gtk_widget_get_first_child(box);
    while (child != NULL) {
        gtk_box_remove(GTK_BOX(box), child);
        child = gtk_widget_get_first_child(box);
    }
}

static void append_vehicle_workspace(MblinkLinuxApp *app)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        profile != NULL && profile->endpoint_count != 0U
            ? &profile->endpoints[0]
            : NULL;
    GtkWidget *identity = make_card("VEHICLE EVIDENCE", "C207 E 250 CDI");
    GtkWidget *connection = make_card("CONNECTION", "Linux transport state");

    append_detail(identity, "Engine", profile != NULL
        ? profile->engine_family : "Unavailable");
    append_detail(identity, "Profile", profile != NULL
        ? profile->display_name : "Unavailable");
    append_detail(identity, "Engine ECU", endpoint != NULL
        ? endpoint->name : "Unavailable");
    append_detail(identity, "Definition", endpoint != NULL
        ? mblink_mercedes_definition_status_name(endpoint->status)
        : "Unavailable");
    append_detail(identity, "Physical CAN", "0x7E0 → 0x7E8");

    gtk_box_append(GTK_BOX(connection), make_chip(
        "TRANSPORT PROVIDER NOT CONFIGURED", "mblink-warning"));
    gtk_box_append(GTK_BOX(connection), make_left_label(
        "The portable diagnostic profile is live. A Linux BLE, serial or "
        "SocketCAN provider is still required before this front end can "
        "connect to a vehicle.", "mblink-command-note"));

    gtk_box_append(GTK_BOX(app->content_body), identity);
    gtk_box_append(GTK_BOX(app->content_body), connection);
}

static void append_modules_workspace(MblinkLinuxApp *app)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    GtkWidget *card = make_card("PORTABLE MERCEDES MODEL", "Known ECU endpoints");
    size_t index;

    if (profile == NULL || profile->endpoint_count == 0U) {
        gtk_box_append(GTK_BOX(card),
                       make_left_label("No endpoint definitions available",
                                       "mblink-command-note"));
    } else {
        for (index = 0U; index < profile->endpoint_count; ++index) {
            const MblinkMercedesEcuEndpointDefinition *endpoint =
                &profile->endpoints[index];
            append_detail(card, endpoint->name,
                          mblink_mercedes_definition_status_name(
                              endpoint->status));
        }
    }
    gtk_box_append(GTK_BOX(app->content_body), card);
}

static void append_faults_workspace(MblinkLinuxApp *app)
{
    GtkWidget *mercedes = make_card("MERCEDES ENGINE", "UDS 0x19 fault memory");
    GtkWidget *obd = make_card("STANDARD OBD-II", "Stored, pending and permanent faults");

    gtk_box_append(GTK_BOX(mercedes),
                   make_chip("NOT SCANNED · TRANSPORT OFFLINE", "mblink-warning"));
    gtk_box_append(GTK_BOX(mercedes), make_left_label(
        "The shared C core includes the read-only UDS ReadDTCInformation "
        "codec. No result is invented while the Linux transport is absent.",
        "mblink-command-note"));
    gtk_box_append(GTK_BOX(obd),
                   make_chip("NOT SCANNED · TRANSPORT OFFLINE", "mblink-warning"));
    gtk_box_append(GTK_BOX(app->content_body), mercedes);
    gtk_box_append(GTK_BOX(app->content_body), obd);
}

static void append_parameter_catalogue(MblinkLinuxApp *app, bool compact)
{
    GtkWidget *card = make_card(
        compact ? "PORTABLE PARAMETER TABLE" : "SHARED PARAMETER CATALOGUE",
        compact ? "Standard OBD-II definitions" : "Live-data definitions");
    const size_t count = mblink_parameter_obd2_definition_count();
    size_t index;

    for (index = 0U; index < count; ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_at(index);
        char identifier[24];
        if (definition == NULL) {
            continue;
        }
        (void)g_snprintf(identifier, sizeof(identifier), "0x%02X · %s",
                         (unsigned int)definition->key.identifier,
                         definition->short_name);
        append_detail(card, compact ? identifier : definition->name,
                      compact ? definition->name : identifier);
    }
    gtk_box_append(GTK_BOX(app->content_body), card);
}

static void append_dashboard_workspace(MblinkLinuxApp *app)
{
    static const char *keys[] = {
        "obd2.engine.rpm",
        "obd2.vehicle.speed",
        "obd2.engine.coolant",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    };
    GtkWidget *card = make_card("AT-A-GLANCE", "Powertrain dashboard");
    size_t index;

    gtk_box_append(GTK_BOX(card), make_chip(
        "WAITING FOR LIVE TRANSPORT", "mblink-warning"));
    for (index = 0U; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_for_stable_key(keys[index]);
        if (definition != NULL) {
            append_detail(card, definition->name, "N/A");
        }
    }
    gtk_box_append(GTK_BOX(app->content_body), card);
}

static void append_graphs_workspace(MblinkLinuxApp *app)
{
    GtkWidget *card = make_card("INSTRUMENT TRACES", "Signal history");
    gtk_box_append(GTK_BOX(card), make_chip(
        "NO LIVE SAMPLE HISTORY", "mblink-warning"));
    gtk_box_append(GTK_BOX(card), make_left_label(
        "Graph definitions and bounded history are owned by the portable "
        "parameter and telemetry layers. Traces will render after a Linux "
        "transport provider supplies real samples.", "mblink-command-note"));
    gtk_box_append(GTK_BOX(app->content_body), card);
}

static void append_log_workspace(MblinkLinuxApp *app)
{
    GtkWidget *card = make_card("SESSION RECORDER", "Diagnostic evidence");
    gtk_box_append(GTK_BOX(card), make_chip(
        "NO ACTIVE SESSION", "mblink-warning"));
    gtk_box_append(GTK_BOX(card), make_left_label(
        "The shared recorder preserves raw command/response evidence and "
        "telemetry. Linux will not present fabricated session data while its "
        "transport edge is unavailable.", "mblink-command-note"));
    gtk_box_append(GTK_BOX(app->content_body), card);
}

static void append_settings_workspace(MblinkLinuxApp *app)
{
    GtkWidget *system = make_card("MBLINK", "System identity");
    GtkWidget *manufacturer = make_card(
        "MERCEDES CAPABILITY MAP", "Evidence-gated manufacturer targets");
    char target_count[32];

    (void)g_snprintf(target_count, sizeof(target_count), "%zu definitions",
                     mblink_mercedes_om651_signal_count());
    append_detail(system, "Version", mblink_version());
    append_detail(system, "Application ID",
                  "com.github.The-First-Infiltrator.MBLINK");
    append_detail(system, "Author", "Shannon Smith");
    append_detail(system, "Licence", "GPL-3.0-or-later");
    append_detail(system, "Portable core", mblink_self_check()
        ? "Validated" : "Invalid metadata");
    append_detail(manufacturer, "OM651 targets", target_count);
    append_detail(manufacturer, "State", "Corroborated · mapping pending");
    gtk_box_append(GTK_BOX(app->content_body), system);
    gtk_box_append(GTK_BOX(app->content_body), manufacturer);
}

static void render_workspace(MblinkLinuxApp *app,
                             MblinkWorkspaceSection section)
{
    clear_box(app->content_body);
    switch (section) {
    case MBLINK_WORKSPACE_VEHICLE:
        append_vehicle_workspace(app);
        break;
    case MBLINK_WORKSPACE_MODULES:
        append_modules_workspace(app);
        break;
    case MBLINK_WORKSPACE_FAULTS:
        append_faults_workspace(app);
        break;
    case MBLINK_WORKSPACE_LIVE_DATA:
        append_parameter_catalogue(app, false);
        break;
    case MBLINK_WORKSPACE_TABLE:
        append_parameter_catalogue(app, true);
        break;
    case MBLINK_WORKSPACE_DASHBOARD:
        append_dashboard_workspace(app);
        break;
    case MBLINK_WORKSPACE_GRAPHS:
        append_graphs_workspace(app);
        break;
    case MBLINK_WORKSPACE_LOG:
        append_log_workspace(app);
        break;
    case MBLINK_WORKSPACE_SETTINGS:
        append_settings_workspace(app);
        break;
    case MBLINK_WORKSPACE_SECTION_COUNT:
        break;
    }
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
    render_workspace(app, section);
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
    GtkWidget *brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *identity = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *emblem = gtk_image_new_from_resource(
        "/com/github/The-First-Infiltrator/MBLINK/mblink-emblem.png");
    GtkWidget *labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *title = make_left_label("MBLINK", "mblink-title");
    GtkWidget *subtitle = make_left_label("MERCEDES DIAGNOSTIC COMMAND",
                                          "mblink-kicker");
    GtkWidget *target = make_left_label("C207 · OM651 · DELPHI CRD3.x",
                                        "mblink-muted");
    GtkWidget *version_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *version_label = gtk_label_new("BUILD");
    GtkWidget *version = gtk_label_new(mblink_version());

    gtk_image_set_pixel_size(GTK_IMAGE(emblem), 72);
    gtk_widget_set_valign(emblem, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(brand, "mblink-brand");
    gtk_widget_add_css_class(version_box, "mblink-chip");
    gtk_widget_add_css_class(version_label, "mblink-kicker");
    gtk_widget_add_css_class(version, "mblink-muted");

    gtk_box_append(GTK_BOX(labels), title);
    gtk_box_append(GTK_BOX(labels), subtitle);
    gtk_box_append(GTK_BOX(labels), target);
    gtk_box_append(GTK_BOX(identity), emblem);
    gtk_box_append(GTK_BOX(identity), labels);

    gtk_box_append(GTK_BOX(version_box), version_label);
    gtk_box_append(GTK_BOX(version_box), version);
    gtk_widget_set_halign(version_box, GTK_ALIGN_START);

    gtk_box_append(GTK_BOX(brand), identity);
    gtk_box_append(GTK_BOX(brand), version_box);
    return brand;
}

static GtkWidget *build_sidebar(MblinkLinuxApp *app)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *brand = build_brand_header();
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *list = gtk_list_box_new();
    size_t index;

    gtk_widget_set_size_request(sidebar, 350, -1);
    gtk_widget_add_css_class(sidebar, "mblink-sidebar");
    gtk_box_append(GTK_BOX(sidebar), brand);

    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    set_margins(list, 6);
    g_signal_connect(list, "row-selected", G_CALLBACK(row_selected), app);

    for (index = 0U; index < mblink_workspace_section_count(); ++index) {
        const MblinkWorkspaceSectionDescriptor *descriptor =
            mblink_workspace_section_at(index);
        if (descriptor != NULL) {
            gtk_list_box_append(GTK_LIST_BOX(list), workspace_row(descriptor));
        }
    }

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), list);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(sidebar), scroll);
    return sidebar;
}

static GtkWidget *build_status_strip(void)
{
    GtkWidget *strip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(strip), make_chip("C207 E 250 CDI", NULL));
    gtk_box_append(GTK_BOX(strip), make_chip("OM651", NULL));
    gtk_box_append(GTK_BOX(strip), make_chip("CRD3.x", NULL));
    gtk_box_append(GTK_BOX(strip), make_chip("0x7E0 → 0x7E8", NULL));
    gtk_box_append(GTK_BOX(strip), make_chip(
        "LINUX TRANSPORT OFFLINE", "mblink-warning"));
    return strip;
}

static GtkWidget *build_content(MblinkLinuxApp *app)
{
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    GtkWidget *status = build_status_strip();
    GtkWidget *heading = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *scroll = gtk_scrolled_window_new();

    app->content_kicker = make_left_label("DIAGNOSTIC WORKSPACE",
                                          "mblink-card-kicker");
    app->content_title = make_left_label("Vehicle", "mblink-card-title");
    app->content_summary = make_left_label(
        "Vehicle identity, adapter and connection information",
        "mblink-card-summary");
    app->content_body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);

    gtk_box_append(GTK_BOX(heading), app->content_kicker);
    gtk_box_append(GTK_BOX(heading), app->content_title);
    gtk_box_append(GTK_BOX(heading), app->content_summary);
    set_margins(app->content_body, 2);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),
                                  app->content_body);
    gtk_widget_set_vexpand(scroll, TRUE);

    set_margins(content, 28);
    gtk_box_append(GTK_BOX(content), status);
    gtk_box_append(GTK_BOX(content), heading);
    gtk_box_append(GTK_BOX(content), scroll);
    render_workspace(app, MBLINK_WORKSPACE_VEHICLE);
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

    gtk_window_set_title(GTK_WINDOW(window),
                         "MBLINK · Mercedes Diagnostic Command");
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
#if GLIB_CHECK_VERSION(2, 74, 0)
        G_APPLICATION_DEFAULT_FLAGS);
#else
        G_APPLICATION_FLAGS_NONE);
#endif
    g_signal_connect(application, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
