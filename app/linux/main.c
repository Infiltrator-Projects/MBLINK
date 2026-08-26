// SPDX-License-Identifier: GPL-3.0-or-later
#include "about-dialog.h"
#include "link-gtk-shell.h"
#include "link-gtk-widgets.h"
#include "link/fuel_economy.h"
#include "link/workspace.h"
#include "mblink/mblink.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_engine_scan.h"
#include "mblink/mercedes_module_scan.h"
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
    MblinkMercedesEngineScan manufacturer_scan;
    bool manufacturer_scan_started;
    bool manufacturer_scan_active;
    bool manufacturer_scan_complete;
    bool manufacturer_scan_failed;
    MblinkMercedesModuleScan module_scan;
    bool module_scan_active;
    bool module_scan_complete;
    bool module_scan_failed;
    bool full_sweep_requested;
    bool module_scan_full;
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

static const MblinkMercedesEcuEndpointDefinition *engine_endpoint(void)
{
    const MblinkMercedesVehicleProfile *profile = mblink_mercedes_c207_om651_profile();
    return profile != NULL && profile->endpoint_count != 0U
        ? &profile->endpoints[0] : NULL;
}

static void reset_manufacturer_scan(MblinkLinuxContext *context)
{
    if (context == NULL) return;
    memset(&context->manufacturer_scan, 0, sizeof(context->manufacturer_scan));
    context->manufacturer_scan_started = false;
    context->manufacturer_scan_active = false;
    context->manufacturer_scan_complete = false;
    context->manufacturer_scan_failed = false;
    memset(&context->module_scan, 0, sizeof(context->module_scan));
    context->module_scan_active = false;
    context->module_scan_complete = false;
    context->module_scan_failed = false;
    context->module_scan_full = false;
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
    if (context->module_scan_active && context->module_scan_full) return "MERCEDES FULL SWEEP ACTIVE";
    if (context->manufacturer_scan_active || context->module_scan_active) return "MERCEDES FACTORY SCAN ACTIVE";
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

static void append_factory_dtc_list(GtkWidget *card,
                                    const MblinkUdsDtcList *list)
{
    size_t index;
    if (card == NULL || list == NULL) return;
    if (list->count == 0U) {
        link_gtk_card_append_detail(card, "Factory faults", "None reported");
        return;
    }
    for (index = 0U; index < list->count; ++index) {
        char label[48];
        char code[7];
        char value[96];
        if (!mblink_uds_dtc_format_hex(list->records[index].code,
                                       code, sizeof(code))) {
            (void)snprintf(code, sizeof(code), "??????");
        }
        (void)snprintf(label, sizeof(label), "Factory %zu", index + 1U);
        (void)snprintf(value, sizeof(value), "%s · status 0x%02X",
                       code, (unsigned int)list->records[index].status);
        link_gtk_card_append_detail(card, label, value);
    }
    if (list->truncated)
        link_gtk_card_append_detail(card, "Factory list", "Truncated at safe bounded capacity");
}

static void append_vehicle(GtkWidget *body, MblinkLinuxContext *context)
{
    const MblinkMercedesVehicleProfile *profile = mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
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
        "LINK now hands the normal Linux sequence to MBLINK's read-only Mercedes engine probe after SAE PID discovery, then restores the generic OBD-II channel before stored/pending/permanent SAE fault inventory and live polling.");
    gtk_box_append(GTK_BOX(body), identity);
    gtk_box_append(GTK_BOX(body), connection);
}

static void append_modules(GtkWidget *body, const MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new("MERCEDES MODULE INVENTORY", "Discovered control units");
    char summary[128];
    if (!context->connected) {
        link_gtk_card_append_status(card, "NOT SCANNED · LINK OFFLINE", "state-warning");
    } else if (context->module_scan_active) {
        (void)snprintf(summary, sizeof(summary), "%s · %zu modules found so far",
             context->module_scan_full ? "FULL SWEEP SCANNING" : "SCANNING",
             context->module_scan.module_count);
        link_gtk_card_append_status(card, summary, "state-warning");
    } else if (context->module_scan_complete) {
        (void)snprintf(summary, sizeof(summary), "%s · %zu responding module%s%s",
             context->module_scan_full ? "FULL SWEEP COMPLETE" : "COMPLETE",
             context->module_scan.module_count,
             context->module_scan.module_count == 1U ? "" : "s",
             context->module_scan.truncated ? " · result capacity reached" : "");
        link_gtk_card_append_status(card, summary, "state-success");
        for (size_t index = 0U; index < context->module_scan.module_count; ++index) {
  const MblinkMercedesModuleScanEntry *module = &context->module_scan.modules[index];
  char value[128];
  if (module->extended_id) {
      (void)snprintf(value, sizeof(value), "0x%08X → 0x%08X · %zu fault record%s",
                     (unsigned int)module->tx_can_id, (unsigned int)module->rx_can_id,
                     module->dtcs.count, module->dtcs.count == 1U ? "" : "s");
  } else {
      (void)snprintf(value, sizeof(value), "0x%03X → 0x%03X · %zu fault record%s",
                     (unsigned int)module->tx_can_id, (unsigned int)module->rx_can_id,
                     module->dtcs.count, module->dtcs.count == 1U ? "" : "s");
  }
  link_gtk_card_append_detail(card, mblink_mercedes_module_scan_module_name(module), value);
        }
    } else if (context->module_scan_failed) {
        link_gtk_card_append_status(card, "MODULE INVENTORY INCOMPLETE", "state-warning");
    } else {
        link_gtk_card_append_status(card, "MODULE SCAN PENDING", "state-warning");
    }
    link_gtk_card_append_note(card,
        "Normal discovery is bounded to 0x7E0–0x7E7. FULL SWEEP is an explicit read-only forensic pass across 0x600–0x7F7 plus ISO 15765 normal-fixed 29-bit targets. Responders are queried for F197 system-name evidence; unknown ECUs remain address-labelled rather than guessed.");
    gtk_box_append(GTK_BOX(body), card);
}

static void append_faults(GtkWidget *body, const MblinkLinuxContext *context)
{
    GtkWidget *mercedes = link_gtk_card_new("MERCEDES FACTORY", "Engine ECU factory diagnostics");
    GtkWidget *modules = link_gtk_card_new("ALL DISCOVERED MODULES", "Per-module Mercedes UDS fault memory");
    GtkWidget *obd = link_gtk_card_new("STANDARD SAE OBD-II", "Stored, pending and permanent faults");
    char summary[160];

    if (!context->connected) {
        link_gtk_card_append_status(mercedes, "NOT SCANNED · LINK OFFLINE", "state-warning");
    } else if (!context->manufacturer_scan_started) {
        link_gtk_card_append_status(mercedes, "FACTORY SCAN PENDING", "state-warning");
    } else if (context->manufacturer_scan_active) {
        (void)snprintf(summary, sizeof(summary), "SCANNING · %s",
             mblink_mercedes_engine_scan_stage_name(context->manufacturer_scan.stage));
        link_gtk_card_append_status(mercedes, summary, "state-warning");
    } else if (context->manufacturer_scan_complete) {
        (void)snprintf(summary, sizeof(summary), "COMPLETE · %zu factory DTC%s",
             context->manufacturer_scan.dtcs.count,
             context->manufacturer_scan.dtcs.count == 1U ? "" : "s");
        link_gtk_card_append_status(mercedes, summary, "state-success");
        if (context->manufacturer_scan.probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE)
  link_gtk_card_append_detail(mercedes, "VIN", context->manufacturer_scan.probe.vin);
        link_gtk_card_append_detail(mercedes, "Factory DTC result",
  mblink_mercedes_engine_dtc_result_name(context->manufacturer_scan.dtc_result));
        link_gtk_card_append_detail(mercedes, "CRD3 / Delphi signature",
  context->manufacturer_scan.crd3_evidence.om651_cdid3_delphi_signature
      ? "Matched" : "Not matched / insufficient evidence");
        if (context->manufacturer_scan.probe.ecu_hardware_number_available)
  link_gtk_card_append_detail(mercedes, "ECU hardware",
      context->manufacturer_scan.probe.ecu_hardware_number);
        if (context->manufacturer_scan.probe.ecu_software_number_available)
  link_gtk_card_append_detail(mercedes, "ECU software",
      context->manufacturer_scan.probe.ecu_software_number);
        if (context->manufacturer_scan.probe.ecu_spare_part_number_available)
  link_gtk_card_append_detail(mercedes, "ECU spare part",
      context->manufacturer_scan.probe.ecu_spare_part_number);
        if (context->manufacturer_scan.probe.ecu_system_name_available)
  link_gtk_card_append_detail(mercedes, "ECU system name",
      context->manufacturer_scan.probe.ecu_system_name);
        if (context->manufacturer_scan.probe.crd3_hardware_profile != NULL) {
  char profile_match[192];
  (void)snprintf(profile_match, sizeof(profile_match), "%s · %s · %s",
      context->manufacturer_scan.probe.crd3_hardware_profile->ecu_family,
      context->manufacturer_scan.probe.crd3_hardware_profile->microcontroller,
      mblink_mercedes_crd3_profile_match_name(
          context->manufacturer_scan.probe.crd3_hardware_match));
  link_gtk_card_append_detail(mercedes, "ECU source profile", profile_match);
        }
        append_factory_dtc_list(mercedes, &context->manufacturer_scan.dtcs);
    } else if (context->manufacturer_scan_failed) {
        link_gtk_card_append_status(mercedes, "ENGINE FACTORY SCAN UNAVAILABLE · MODULE PASS CONTINUES", "state-warning");
    }
    link_gtk_card_append_note(mercedes,
        "Mercedes engine-ECU UDS results remain separate from legislated SAE OBD-II DTCs.");

    if (!context->connected) {
        link_gtk_card_append_status(modules, "NOT SCANNED · LINK OFFLINE", "state-warning");
    } else if (context->module_scan_active) {
        (void)snprintf(summary, sizeof(summary), "SCANNING · %zu module%s found",
             context->module_scan.module_count,
             context->module_scan.module_count == 1U ? "" : "s");
        link_gtk_card_append_status(modules, summary, "state-warning");
    } else if (context->module_scan_complete) {
        const size_t total = mblink_mercedes_module_scan_total_dtc_count(&context->module_scan);
        (void)snprintf(summary, sizeof(summary), "COMPLETE · %zu modules · %zu factory fault record%s",
             context->module_scan.module_count, total, total == 1U ? "" : "s");
        link_gtk_card_append_status(modules, summary, "state-success");
        for (size_t module_index = 0U; module_index < context->module_scan.module_count; ++module_index) {
  const MblinkMercedesModuleScanEntry *module = &context->module_scan.modules[module_index];
  char module_label[96];
  char module_value[128];
  if (module->extended_id)
      (void)snprintf(module_label, sizeof(module_label), "%s · 0x%08X",
                     mblink_mercedes_module_scan_module_name(module), (unsigned int)module->tx_can_id);
  else
      (void)snprintf(module_label, sizeof(module_label), "%s · 0x%03X",
                     mblink_mercedes_module_scan_module_name(module), (unsigned int)module->tx_can_id);
  if (module->dtc_result == MBLINK_MERCEDES_MODULE_DTC_AVAILABLE)
      (void)snprintf(module_value, sizeof(module_value), "%zu fault record%s",
                     module->dtcs.count, module->dtcs.count == 1U ? "" : "s");
  else if (module->dtc_result == MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE)
      (void)snprintf(module_value, sizeof(module_value), "DTC read negative response · NRC 0x%02X",
                     (unsigned int)module->dtc_negative_response_code);
  else
      (void)snprintf(module_value, sizeof(module_value), "DTC read %s",
                     module->dtc_result == MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE ? "no response" : "unavailable");
  link_gtk_card_append_detail(modules, module_label, module_value);
  for (size_t dtc_index = 0U; dtc_index < module->dtcs.count; ++dtc_index) {
      char code[7];
      char label[96];
      char value[96];
      if (!mblink_uds_dtc_format_hex(module->dtcs.records[dtc_index].code, code, sizeof(code)))
          (void)snprintf(code, sizeof(code), "??????");
      (void)snprintf(label, sizeof(label), "  %s fault %zu", mblink_mercedes_module_scan_module_name(module), dtc_index + 1U);
      (void)snprintf(value, sizeof(value), "%s · status 0x%02X", code,
                     (unsigned int)module->dtcs.records[dtc_index].status);
      link_gtk_card_append_detail(modules, label, value);
  }
        }
    } else if (context->module_scan_failed) {
        link_gtk_card_append_status(modules, "MODULE FAULT INVENTORY INCOMPLETE", "state-warning");
    } else {
        link_gtk_card_append_status(modules, "MODULE FAULT SCAN PENDING", "state-warning");
    }
    link_gtk_card_append_note(modules,
        "Each responding ECU is queried independently with UDS ReadDTCInformation 0x19/0x02/0xFF. Fault memory is read only; MBLINK does not clear or alter any module.");

    if (!context->connected) {
        link_gtk_card_append_status(obd, "NOT SCANNED · LINK OFFLINE", "state-warning");
    } else if (!context->diagnostic_valid) {
        link_gtk_card_append_status(obd, "STARTING SCAN", "state-warning");
    } else if (context->diagnostic.stage == LINK_DIAGNOSTIC_FLOW_FAILED) {
        link_gtk_card_append_status(obd, "SCAN FAILED · RECONNECT TO RETRY", "state-warning");
    } else if (context->diagnostic_ready) {
        (void)snprintf(summary, sizeof(summary), "COMPLETE · %zu stored · %zu pending · %zu permanent",
             context->diagnostic.stored_dtcs.count, context->diagnostic.pending_dtcs.count,
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
    gtk_box_append(GTK_BOX(body), modules);
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
    case LINK_WORKSPACE_MODULES: append_modules(body, context); break;
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
        link_gtk_card_append_detail(card, "Linux transport", "LINK serial + BlueZ BLE ELM327 providers");
        link_gtk_card_append_detail(card, "Linux diagnostic flow", "SAE OBD-II + Mercedes read-only factory extension");
        link_gtk_card_append_detail(card, "Mercedes scan", "Engine fingerprint + bounded evidence-backed module discovery + per-module UDS DTC inventory");
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

static MblinkMercedesModuleScanResult begin_module_scan(
    MblinkLinuxContext *context)
{
    if (context == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    return context->module_scan_full
        ? mblink_mercedes_module_scan_begin_full(&context->module_scan)
        : mblink_mercedes_module_scan_begin(&context->module_scan);
}

static void request_full_sweep(void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (context != NULL) context->full_sweep_requested = true;
}

static bool manufacturer_begin(void *opaque)
{
    MblinkLinuxContext *context = opaque;
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    bool full_sweep;
    if (context == NULL || endpoint == NULL) return false;
    full_sweep = context->full_sweep_requested;
    context->full_sweep_requested = false;
    reset_manufacturer_scan(context);
    context->module_scan_full = full_sweep;
    context->manufacturer_scan_started = true;
    if (!mblink_mercedes_engine_scan_begin(&context->manufacturer_scan, endpoint)) {
        context->manufacturer_scan_failed = true;
        if (begin_module_scan(context) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
  context->module_scan_active = true;
  return true;
        }
        context->module_scan_failed = true;
        return false;
    }
    context->manufacturer_scan_active = true;
    return true;
}

static bool manufacturer_next_command(char *buffer,
                            size_t buffer_size,
                            size_t *written,
                            uint64_t *timeout_ms,
                            void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (context == NULL) return false;
    if (context->manufacturer_scan_active) {
        MblinkMercedesEcuProbeResult result = mblink_mercedes_engine_scan_command(
  &context->manufacturer_scan, buffer, buffer_size, written);
        if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK) {
  context->manufacturer_scan_active = false;
  context->manufacturer_scan_failed = true;
  return false;
        }
        if (timeout_ms != NULL) *timeout_ms = UINT64_C(4000);
        return true;
    }
    if (context->module_scan_active) {
        MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_command(
  &context->module_scan, buffer, buffer_size, written);
        if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
  context->module_scan_active = false;
  context->module_scan_failed = true;
  return false;
        }
        if (timeout_ms != NULL) *timeout_ms = mblink_mercedes_module_scan_timeout_ms(&context->module_scan);
        return true;
    }
    return false;
}

static bool manufacturer_accept_response(const LinkElm327Response *response,
                               bool *complete,
                               void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (complete != NULL) *complete = false;
    if (context == NULL || response == NULL) return false;

    if (context->manufacturer_scan_active) {
        MblinkMercedesEcuProbeResult result = mblink_mercedes_engine_scan_accept(
  &context->manufacturer_scan, (const MblinkElm327Response *)response);
        if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
  context->manufacturer_scan_active = false;
  context->manufacturer_scan_complete = true;
  if (begin_module_scan(context) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
      context->module_scan_active = true;
      return true;
  }
  context->module_scan_failed = true;
  if (complete != NULL) *complete = true;
  return true;
        }
        if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK) return true;
        context->manufacturer_scan_active = false;
        context->manufacturer_scan_failed = true;
        if (begin_module_scan(context) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
  context->module_scan_active = true;
  return true;
        }
        context->module_scan_failed = true;
        if (complete != NULL) *complete = true;
        return true;
    }

    if (context->module_scan_active) {
        MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_accept(
  &context->module_scan, (const MblinkElm327Response *)response);
        if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE) {
  context->module_scan_active = false;
  context->module_scan_complete = true;
  if (complete != NULL) *complete = true;
  return true;
        }
        if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) return true;
        context->module_scan_active = false;
        context->module_scan_failed = true;
        if (complete != NULL) *complete = true;
        return true;
    }
    return false;
}

static void manufacturer_finished(bool complete, void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (context == NULL) return;
    context->manufacturer_scan_active = false;
    context->module_scan_active = false;
    if (!complete) {
        if (context->manufacturer_scan_started && !context->manufacturer_scan_complete)
  context->manufacturer_scan_failed = true;
        if (!context->module_scan_complete) context->module_scan_failed = true;
    }
}

static const LinkGtkManufacturerExtension mblink_manufacturer_extension = {
    .begin = manufacturer_begin,
    .next_command = manufacturer_next_command,
    .accept_response = manufacturer_accept_response,
    .finished = manufacturer_finished
};

static void connection_changed(LinkTransport *transport,
                               bool connected,
                               const char *adapter_identity,
                               void *opaque)
{
    MblinkLinuxContext *context = opaque;
    context->connected = connected;
    context->transport = *transport;
    context->full_sweep_requested = false;
    (void)snprintf(context->adapter_identity, sizeof(context->adapter_identity), "%s",
                   connected && adapter_identity != NULL ? adapter_identity : "");
    reset_manufacturer_scan(context);
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
    descriptor.diagnostic_restart_action_label = "FULL SWEEP";
    descriptor.diagnostic_restart_action = request_full_sweep;
    descriptor.manufacturer_extension = &mblink_manufacturer_extension;
    descriptor.context = &context;
    return link_gtk_shell_run(argc, argv, &descriptor);
}
