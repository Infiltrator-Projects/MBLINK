// SPDX-License-Identifier: GPL-3.0-or-later
#include "about-dialog.h"
#include "link-gtk-shell.h"
#include "link-gtk-widgets.h"
#include "link/fuel_economy.h"
#include "link/workspace.h"
#include "mblink/mblink.h"
#include "mblink/mercedes.h"
#include "mblink/parameter.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct MblinkLinuxContext {
    bool connected;
    char adapter_identity[160];
    LinkTransport transport;
    bool diagnostic_valid;
    bool diagnostic_active;
    bool diagnostic_ready;
    LinkDiagnosticFlow diagnostic;
    bool sample_valid[256];
    LinkObd2Sample samples[256];
    LinkFuelEconomy fuel_economy;
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

static uint64_t monotonic_ms(void)
{
    const gint64 value = g_get_monotonic_time();
    return value <= 0 ? 0U : (uint64_t)(value / 1000);
}

static const char *connection_text(const MblinkLinuxContext *context)
{
    return context->connected ? "LINKED · ELM327 VERIFIED" : "NOT LINKED";
}

static const char *diagnostic_text(const MblinkLinuxContext *context)
{
    if (!context->connected) return "LINK OFFLINE";
    if (!context->diagnostic_valid) return "STARTING DIAGNOSTICS";
    if (context->diagnostic.stage == LINK_DIAGNOSTIC_FLOW_FAILED) return "DIAGNOSTIC SESSION FAILED";
    if (context->diagnostic_ready) return "LIVE DIAGNOSTICS ACTIVE";
    return link_diagnostic_flow_stage_name(context->diagnostic.stage);
}

static const char *fuel_source_text(LinkFuelEconomySource source)
{
    switch (source) {
    case LINK_FUEL_ECONOMY_SOURCE_FACTORY_DIRECT: return "Mercedes factory direct";
    case LINK_FUEL_ECONOMY_SOURCE_FACTORY_COUNTERS: return "Mercedes factory counters";
    case LINK_FUEL_ECONOMY_SOURCE_FACTORY_RATE: return "Mercedes factory fuel rate";
    case LINK_FUEL_ECONOMY_SOURCE_SAE_OBD2: return "SAE OBD-II PID 0x5E + 0x0D";
    case LINK_FUEL_ECONOMY_SOURCE_ESTIMATED: return "Estimated";
    case LINK_FUEL_ECONOMY_SOURCE_MIXED: return "Mixed measured sources";
    case LINK_FUEL_ECONOMY_SOURCE_NONE: return "Unavailable";
    }
    return "Unavailable";
}

static void format_sample(const LinkObd2Sample *sample,
                          char *buffer,
                          size_t capacity)
{
    const char *unit;
    if (buffer == NULL || capacity == 0U) return;
    if (sample == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }
    unit = link_obd2_unit_name(sample->unit);
    if (unit == NULL || unit[0] == '\0' || sample->unit == LINK_OBD2_UNIT_NONE)
        (void)snprintf(buffer, capacity, "%.2f", sample->value);
    else
        (void)snprintf(buffer, capacity, "%.2f %s", sample->value, unit);
}

static void append_dtc_list(GtkWidget *card,
                            const char *prefix,
                            const LinkObd2DtcList *list)
{
    size_t index;
    if (card == NULL || prefix == NULL || list == NULL) return;
    if (list->count == 0U) {
        char label[48];
        (void)snprintf(label, sizeof(label), "%s faults", prefix);
        link_gtk_card_append_detail(card, label, "None reported");
        return;
    }
    for (index = 0U; index < list->count; ++index) {
        char label[48];
        (void)snprintf(label, sizeof(label), "%s %zu", prefix, index + 1U);
        link_gtk_card_append_detail(card, label, list->entries[index].code);
    }
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
    link_gtk_card_append_detail(connection, "Diagnostic flow", diagnostic_text(context));
    link_gtk_card_append_note(connection,
        "LINK carries the Linux connection directly into ELM initialisation, supported-PID discovery, stored/pending/permanent OBD-II fault inventory and live polling.");
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
        "Mercedes-specific endpoint definitions remain in MBLINK; transport, standard OBD-II sequencing and live polling are owned by LINK.");
    gtk_box_append(GTK_BOX(body), card);
}

static void append_faults(GtkWidget *body, const MblinkLinuxContext *context)
{
    GtkWidget *mercedes = link_gtk_card_new("MERCEDES ENGINE", "Manufacturer profile status");
    GtkWidget *obd = link_gtk_card_new("STANDARD OBD-II", "Stored, pending and permanent faults");
    char summary[160];

    link_gtk_card_append_status(mercedes,
        context->connected ? "PROFILE READY" : "LINK OFFLINE",
        context->connected ? "state-success" : "state-warning");
    link_gtk_card_append_note(mercedes,
        "The C207/OM651 profile is available for Mercedes-specific read-only probing; the automatic Linux pass below is the shared standards-based inventory.");

    if (!context->connected) {
        link_gtk_card_append_status(obd, "NOT SCANNED · LINK OFFLINE", "state-warning");
    } else if (!context->diagnostic_valid) {
        link_gtk_card_append_status(obd, "STARTING SCAN", "state-warning");
    } else if (context->diagnostic.stage == LINK_DIAGNOSTIC_FLOW_FAILED) {
        link_gtk_card_append_status(obd, "SCAN FAILED · RECONNECT TO RETRY", "state-warning");
    } else if (context->diagnostic_ready) {
        (void)snprintf(summary, sizeof(summary),
                       "COMPLETE · %zu stored · %zu pending · %zu permanent",
                       context->diagnostic.stored_dtcs.count,
                       context->diagnostic.pending_dtcs.count,
                       context->diagnostic.permanent_dtcs.count);
        link_gtk_card_append_status(obd, summary, "state-success");
        append_dtc_list(obd, "Stored", &context->diagnostic.stored_dtcs);
        append_dtc_list(obd, "Pending", &context->diagnostic.pending_dtcs);
        append_dtc_list(obd, "Permanent", &context->diagnostic.permanent_dtcs);
    } else {
        (void)snprintf(summary, sizeof(summary), "SCAN IN PROGRESS · %s",
                       link_diagnostic_flow_stage_name(context->diagnostic.stage));
        link_gtk_card_append_status(obd, summary, "state-warning");
    }

    gtk_box_append(GTK_BOX(body), mercedes);
    gtk_box_append(GTK_BOX(body), obd);
}

static void append_parameters(GtkWidget *body,
                              bool compact,
                              const MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new(compact ? "PARAMETER TABLE" : "LIVE DATA CATALOGUE",
                                        compact ? "Real standard OBD-II samples" : "Available shared diagnostic parameters");
    size_t count = mblink_parameter_obd2_definition_count();
    size_t index;
    for (index = 0U; index < count; ++index) {
        const MblinkParameterDefinition *definition = mblink_parameter_obd2_definition_at(index);
        char key[64];
        char value[96];
        uint8_t pid;
        if (definition == NULL) continue;
        pid = (uint8_t)definition->key.identifier;
        (void)snprintf(key, sizeof(key), "PID 0x%02X · %s",
                       (unsigned int)pid, definition->short_name);
        if (context->sample_valid[pid]) {
            format_sample(&context->samples[pid], value, sizeof(value));
        } else if (context->diagnostic_valid &&
                   !link_obd2_pid_set_contains(&context->diagnostic.supported_pids, pid)) {
            (void)snprintf(value, sizeof(value), "Not supported by vehicle");
        } else if (context->diagnostic_active || context->diagnostic_ready) {
            (void)snprintf(value, sizeof(value), "Waiting for sample");
        } else {
            (void)snprintf(value, sizeof(value), "No live session");
        }
        link_gtk_card_append_detail(card, compact ? key : definition->name, value);
    }
    gtk_box_append(GTK_BOX(body), card);
}

static void append_fuel_economy(GtkWidget *body,
                                const MblinkLinuxContext *context)
{
    LinkFuelEconomySnapshot snapshot =
        link_fuel_economy_snapshot(&context->fuel_economy, monotonic_ms());
    GtkWidget *card = link_gtk_card_new("FUEL ECONOMY", "Fuel use and trip consumption");
    char instantaneous[64];
    char average[64];
    char rate[64];
    char trip[96];
    LinkFuelEconomySource display_source = snapshot.instantaneous_available
        ? snapshot.instantaneous_source
        : (snapshot.fuel_rate_available ? snapshot.fuel_rate_source : snapshot.average_source);

    if (snapshot.instantaneous_available) {
        (void)snprintf(instantaneous, sizeof(instantaneous), "%.1f L/100 km",
                       snapshot.instantaneous_l_per_100km);
    } else if (context->connected && !snapshot.moving) {
        (void)snprintf(instantaneous, sizeof(instantaneous), "— · stationary / awaiting speed");
    } else {
        (void)snprintf(instantaneous, sizeof(instantaneous), "Waiting for measured fuel data");
    }
    if (snapshot.average_available)
        (void)snprintf(average, sizeof(average), "%.1f L/100 km", snapshot.average_l_per_100km);
    else
        (void)snprintf(average, sizeof(average), "Waiting for trip distance");
    if (snapshot.fuel_rate_available)
        (void)snprintf(rate, sizeof(rate), "%.2f L/h", snapshot.fuel_rate_l_per_hour);
    else
        (void)snprintf(rate, sizeof(rate), "Not available");
    (void)snprintf(trip, sizeof(trip), "%.2f L over %.1f km",
                   snapshot.trip_fuel_litres, snapshot.trip_distance_km);

    link_gtk_card_append_status(card,
        snapshot.instantaneous_available || snapshot.fuel_rate_available
            ? "MEASURED FUEL DATA ACTIVE" : "WAITING FOR FUEL DATA",
        snapshot.instantaneous_available || snapshot.fuel_rate_available
            ? "state-success" : "state-warning");
    link_gtk_card_append_detail(card, "Instantaneous", instantaneous);
    link_gtk_card_append_detail(card, "Trip average", average);
    link_gtk_card_append_detail(card, "Fuel rate", rate);
    link_gtk_card_append_detail(card, "Trip", trip);
    link_gtk_card_append_detail(card, "Current source", fuel_source_text(display_source));
    link_gtk_card_append_detail(card, "Mercedes factory source",
        "No fuel-consumption DID is enabled until its address and scaling are evidence-backed");
    link_gtk_card_append_note(card,
        "LINK prefers a verified Mercedes factory value when one is supplied by the MBLINK profile. Until then it uses measured SAE PID 0x5E fuel rate with PID 0x0D vehicle speed; estimates are never presented as measured data.");
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
        context->diagnostic_ready ? "LIVE SAMPLES" : diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");
    for (index = 0U; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        const MblinkParameterDefinition *definition = mblink_parameter_obd2_definition_for_stable_key(keys[index]);
        char value[96];
        if (definition == NULL) continue;
        if (context->sample_valid[definition->key.identifier])
            format_sample(&context->samples[definition->key.identifier], value, sizeof(value));
        else
            (void)snprintf(value, sizeof(value), "Waiting");
        link_gtk_card_append_detail(card, definition->name, value);
    }
    gtk_box_append(GTK_BOX(body), card);
    append_fuel_economy(body, context);
}

static void append_generic_status(GtkWidget *body,
                                  const char *kicker,
                                  const char *title,
                                  const char *note,
                                  const MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new(kicker, title);
    link_gtk_card_append_status(card, diagnostic_text(context),
                                context->diagnostic_ready ? "state-success" : "state-warning");
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
    case LINK_WORKSPACE_LIVE_DATA: append_parameters(body, false, context); break;
    case LINK_WORKSPACE_TABLE: append_parameters(body, true, context); break;
    case LINK_WORKSPACE_DASHBOARD: append_dashboard(body, context); break;
    case LINK_WORKSPACE_GRAPHS:
        append_generic_status(body, "INSTRUMENT TRACES", "Signal history",
                              "Time-series traces receive real LINK telemetry samples from the active Linux diagnostic flow.", context); break;
    case LINK_WORKSPACE_LOG:
        append_generic_status(body, "SESSION RECORDER", "Diagnostic evidence",
                              "The shared recorder/evidence path can consume the same real diagnostic events without inventing data.", context); break;
    case LINK_WORKSPACE_SETTINGS: {
        GtkWidget *card = link_gtk_card_new("MBLINK", "System identity");
        link_gtk_card_append_detail(card, "Version", mblink_version());
        link_gtk_card_append_detail(card, "Product", "Mercedes-Benz diagnostics");
        link_gtk_card_append_detail(card, "Portable core", mblink_self_check() ? "Validated" : "Invalid metadata");
        link_gtk_card_append_detail(card, "Linux transport", "LINK serial ELM327 provider");
        link_gtk_card_append_detail(card, "Linux diagnostic flow", "Automatic PID + DTC + live polling");
        link_gtk_card_append_detail(card, "Fuel economy", "Factory-priority + SAE measured fallback");
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
    if (connected)
        link_fuel_economy_reset_trip(&context->fuel_economy, monotonic_ms());
    else
        link_fuel_economy_init(&context->fuel_economy);
}

static void diagnostic_changed(const LinkDiagnosticFlow *flow,
                               const LinkDiagnosticFlowEvent *event,
                               bool active,
                               bool ready,
                               void *opaque)
{
    MblinkLinuxContext *context = opaque;
    context->diagnostic_active = active;
    context->diagnostic_ready = ready;
    if (flow == NULL) {
        context->diagnostic_valid = false;
        memset(&context->diagnostic, 0, sizeof(context->diagnostic));
        memset(context->sample_valid, 0, sizeof(context->sample_valid));
        memset(context->samples, 0, sizeof(context->samples));
        link_fuel_economy_init(&context->fuel_economy);
        return;
    }
    context->diagnostic = *flow;
    context->diagnostic_valid = true;
    if (event != NULL && event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE) {
        const uint64_t now_ms = monotonic_ms();
        context->samples[event->sample.pid] = event->sample;
        context->sample_valid[event->sample.pid] = true;
        (void)link_fuel_economy_observe_obd2(
            &context->fuel_economy, &event->sample, now_ms);
        link_fuel_economy_tick(&context->fuel_economy, now_ms);
    }
}

int main(int argc, char **argv)
{
    MblinkLinuxContext context = {0};
    LinkGtkShellDescriptor descriptor = {0};
    link_fuel_economy_init(&context.fuel_economy);
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
    descriptor.diagnostic_changed = diagnostic_changed;
    descriptor.context = &context;
    return link_gtk_shell_run(argc, argv, &descriptor);
}
