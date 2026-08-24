// SPDX-License-Identifier: GPL-3.0-or-later
#include "about-dialog.h"
#include "link-gtk-shell.h"
#include "link-gtk-widgets.h"
#include "link/workspace.h"
#include "mblink/mblink.h"
#include "mblink/mercedes.h"
#include "mblink/parameter.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct MblinkLinuxContext {
    bool connected;
    char adapter_identity[160];
    LinkTransport transport;
} MblinkLinuxContext;

static const char mblink_css[] =
    "window { background: #050608; color: #e8ecef; }"
    ".link-connection-bar { background: #101318; border: 1px solid #353a40; }"
    ".link-link-button { background: #d7dde2; color: #111418; border-radius: 10px; }"
    ".link-brand { color: #eef1f3; }"
    ".link-brand-subtitle { color: #aeb6bd; }"
    ".link-section-title { color: #e7ebee; }"
    ".link-section-summary { color: #899198; }"
    ".link-card { background: linear-gradient(135deg,#171b20,#0d1014); border: 1px solid #353a40; border-radius: 18px; padding: 20px; }"
    ".link-card-kicker { color: #8c949b; font-size: 10px; font-weight: 800; letter-spacing: 2px; }"
    ".link-card-title { color: #eef1f3; font-size: 20px; font-weight: 800; }"
    ".link-detail-label { color: #7e858c; }"
    ".link-detail-value { color: #eef1f3; font-weight: 700; }"
    ".link-card-note { color: #9ca4ab; }"
    ".link-status-chip { padding: 7px 11px; border-radius: 999px; border: 1px solid #3b4147; font-weight: 700; }"
    ".state-warning { color: #d19e47; border-color: #72572f; }"
    ".state-success { color: #63ab7c; border-color: #365f45; }";

static const char *connection_text(const MblinkLinuxContext *context)
{
    return context->connected ? "LINKED · ELM327 VERIFIED" : "NOT LINKED";
}

static void append_vehicle(GtkWidget *body, MblinkLinuxContext *context)
{
    const MblinkMercedesVehicleProfile *profile = mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        profile != NULL && profile->endpoint_count != 0U ? &profile->endpoints[0] : NULL;
    GtkWidget *identity = link_gtk_card_new("VEHICLE EVIDENCE", "C207 E 250 CDI");
    GtkWidget *connection = link_gtk_card_new("CONNECTION", "Linux diagnostic link");

    link_gtk_card_append_detail(identity, "Platform", profile != NULL ? profile->chassis_code : "Unavailable");
    link_gtk_card_append_detail(identity, "Engine", profile != NULL ? profile->engine_family : "Unavailable");
    link_gtk_card_append_detail(identity, "Profile", profile != NULL ? profile->display_name : "Unavailable");
    link_gtk_card_append_detail(identity, "Engine ECU", endpoint != NULL ? endpoint->name : "Unavailable");
    link_gtk_card_append_detail(identity, "Definition", endpoint != NULL ? mblink_mercedes_definition_status_name(endpoint->status) : "Unavailable");
    link_gtk_card_append_detail(identity, "Physical CAN", "0x7E0 → 0x7E8");

    link_gtk_card_append_status(connection, connection_text(context),
                                context->connected ? "state-success" : "state-warning");
    link_gtk_card_append_detail(connection, "Adapter",
                                context->connected && context->adapter_identity[0] != '\0'
                                    ? context->adapter_identity : "Select an adapter above and press LINK UP");
    link_gtk_card_append_note(connection,
        "LINK owns the Linux serial transport and validates the ELM327 identity before the application reports a live link.");
    gtk_box_append(GTK_BOX(body), identity);
    gtk_box_append(GTK_BOX(body), connection);
}

static void append_modules(GtkWidget *body)
{
    const MblinkMercedesVehicleProfile *profile = mblink_mercedes_c207_om651_profile();
    GtkWidget *card = link_gtk_card_new("MERCEDES PROFILE", "Known ECU endpoints");
    size_t index;
    if (profile == NULL || profile->endpoint_count == 0U) {
        link_gtk_card_append_status(card, "NO ENDPOINT DEFINITIONS", "state-warning");
    } else {
        for (index = 0U; index < profile->endpoint_count; ++index) {
            const MblinkMercedesEcuEndpointDefinition *endpoint = &profile->endpoints[index];
            link_gtk_card_append_detail(card, endpoint->name,
                                        mblink_mercedes_definition_status_name(endpoint->status));
        }
    }
    link_gtk_card_append_note(card,
        "Additional Mercedes modules belong here as their addresses and read-only identities are evidence-backed.");
    gtk_box_append(GTK_BOX(body), card);
}

static void append_faults(GtkWidget *body, const MblinkLinuxContext *context)
{
    GtkWidget *mercedes = link_gtk_card_new("MERCEDES ENGINE", "UDS 0x19 fault memory");
    GtkWidget *obd = link_gtk_card_new("STANDARD OBD-II", "Stored, pending and permanent faults");
    const char *status = context->connected ? "LINK READY · SCAN NOT STARTED" : "NOT SCANNED · LINK OFFLINE";
    link_gtk_card_append_status(mercedes, status, context->connected ? "state-success" : "state-warning");
    link_gtk_card_append_note(mercedes,
        "Read-only Mercedes fault acquisition uses the shared LINK UDS engine; no fault data is invented before a scan.");
    link_gtk_card_append_status(obd, status, context->connected ? "state-success" : "state-warning");
    gtk_box_append(GTK_BOX(body), mercedes);
    gtk_box_append(GTK_BOX(body), obd);
}

static void append_parameters(GtkWidget *body, bool compact)
{
    GtkWidget *card = link_gtk_card_new(compact ? "PARAMETER TABLE" : "LIVE DATA CATALOGUE",
                                        compact ? "Standard OBD-II definitions" : "Available shared diagnostic parameters");
    size_t count = mblink_parameter_obd2_definition_count();
    size_t index;
    for (index = 0U; index < count; ++index) {
        const MblinkParameterDefinition *definition = mblink_parameter_obd2_definition_at(index);
        char key[48];
        if (definition == NULL) continue;
        (void)snprintf(key, sizeof(key), "PID 0x%02X · %s",
                       (unsigned int)definition->key.identifier, definition->short_name);
        link_gtk_card_append_detail(card, compact ? key : definition->name,
                                    compact ? definition->name : key);
    }
    gtk_box_append(GTK_BOX(body), card);
}

static void append_dashboard(GtkWidget *body, const MblinkLinuxContext *context)
{
    static const char *keys[] = {
        "obd2.engine.rpm", "obd2.vehicle.speed", "obd2.engine.coolant",
        "obd2.diesel.rail_pressure", "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    };
    GtkWidget *card = link_gtk_card_new("AT-A-GLANCE", "Powertrain dashboard");
    size_t index;
    link_gtk_card_append_status(card,
        context->connected ? "LINK READY · WAITING FOR LIVE SAMPLES" : "WAITING FOR LINK",
        context->connected ? "state-success" : "state-warning");
    for (index = 0U; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        const MblinkParameterDefinition *definition = mblink_parameter_obd2_definition_for_stable_key(keys[index]);
        if (definition != NULL) link_gtk_card_append_detail(card, definition->name, "N/A");
    }
    gtk_box_append(GTK_BOX(body), card);
}

static void append_generic_status(GtkWidget *body,
                                  const char *kicker,
                                  const char *title,
                                  const char *note,
                                  const MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new(kicker, title);
    link_gtk_card_append_status(card, context->connected ? "LINK READY" : "LINK OFFLINE",
                                context->connected ? "state-success" : "state-warning");
    link_gtk_card_append_note(card, note);
    gtk_box_append(GTK_BOX(body), card);
}

static void render_section(size_t section, GtkWidget *body, void *opaque)
{
    MblinkLinuxContext *context = opaque;
    switch ((LinkWorkspaceSection)section) {
    case LINK_WORKSPACE_VEHICLE: append_vehicle(body, context); break;
    case LINK_WORKSPACE_MODULES: append_modules(body); break;
    case LINK_WORKSPACE_FAULTS: append_faults(body, context); break;
    case LINK_WORKSPACE_LIVE_DATA: append_parameters(body, false); break;
    case LINK_WORKSPACE_TABLE: append_parameters(body, true); break;
    case LINK_WORKSPACE_DASHBOARD: append_dashboard(body, context); break;
    case LINK_WORKSPACE_GRAPHS:
        append_generic_status(body, "INSTRUMENT TRACES", "Signal history",
                              "Time-series traces populate from real LINK telemetry samples.", context); break;
    case LINK_WORKSPACE_LOG:
        append_generic_status(body, "SESSION RECORDER", "Diagnostic evidence",
                              "The shared recorder preserves raw diagnostic evidence and telemetry for export.", context); break;
    case LINK_WORKSPACE_SETTINGS: {
        GtkWidget *card = link_gtk_card_new("MBLINK", "System identity");
        link_gtk_card_append_detail(card, "Version", mblink_version());
        link_gtk_card_append_detail(card, "Product", "Mercedes-Benz diagnostics");
        link_gtk_card_append_detail(card, "Portable core", mblink_self_check() ? "Validated" : "Invalid metadata");
        link_gtk_card_append_detail(card, "Linux transport", "LINK serial ELM327 provider");
        gtk_box_append(GTK_BOX(body), card);
        break;
    }
    case LINK_WORKSPACE_SECTION_COUNT: break;
    }
}

static void show_about(GtkWindow *window, void *context)
{
    (void)context;
    mblink_linux_show_about(window);
}

static void connection_changed(LinkTransport *transport,
                               bool connected,
                               const char *adapter_identity,
                               void *opaque)
{
    MblinkLinuxContext *context = opaque;
    context->connected = connected;
    context->transport = *transport;
    (void)snprintf(context->adapter_identity, sizeof(context->adapter_identity), "%s",
                   connected && adapter_identity != NULL ? adapter_identity : "");
}

int main(int argc, char **argv)
{
    MblinkLinuxContext context = {0};
    LinkGtkShellDescriptor descriptor = {0};
    descriptor.app_id = "com.github.The-First-Infiltrator.MBLINK";
    descriptor.window_title = "MBLINK · Mercedes-Benz Diagnostics";
    descriptor.brand_name = "MBLINK";
    descriptor.brand_subtitle = "MERCEDES-BENZ · C207 / OM651";
    descriptor.version = mblink_version();
    descriptor.emblem_resource = "/com/github/The-First-Infiltrator/MBLINK/mblink-emblem.png";
    descriptor.css = mblink_css;
    descriptor.render_section = render_section;
    descriptor.show_about = show_about;
    descriptor.connection_changed = connection_changed;
    descriptor.context = &context;
    return link_gtk_shell_run(argc, argv, &descriptor);
}
