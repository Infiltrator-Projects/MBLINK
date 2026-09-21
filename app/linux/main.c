// SPDX-License-Identifier: GPL-3.0-or-later
#include "c207-replay.h"
#include "session_trace.h"
#include "style.h"

#include "link-gtk-shell.h"
#include "link-gtk-widgets.h"
#include "link/dtc_knowledge.h"
#include "link/fuel_economy.h"
#include "link/i18n.h"
#include "link/units.h"
#include "link/workspace.h"
#include "link/dashboard.h"
#include "link/selection.h"
#include "link/discover.h"
#include "link/uds_services.h"
#include "mblink/mblink.h"
#include "mblink/uds_dtc.h"
#include "mblink/project_info.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_engine_scan.h"
#include "mblink/fault_investigation.h"
#include "mblink/mercedes_module_scan.h"
#include "mblink/obd2.h"
#include "mblink/parameter.h"

#include "infiltratr/arithmetic.h"
#include "infiltratr/core.h"
#include "infiltratr/posix.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MblinkLiteralTranslation {
    const char *english;
    const char *german;
    const char *polish;
} MblinkLiteralTranslation;

static const char *mblink_translate_text(const char *text, void *context)
{
    static const MblinkLiteralTranslation translations[] = {
        {"MERCEDES PROFILE", "MERCEDES-PROFIL", "PROFIL MERCEDES"},
        {"MERCEDES ENGINE", "MERCEDES-MOTOR", "SILNIK MERCEDES"},
        {"Mercedes factory direct", "Mercedes-Werkswert direkt", "Bezpośrednia wartość fabryczna Mercedes"},
        {"Mercedes factory counters", "Mercedes-Werkszähler", "Fabryczne liczniki Mercedes"},
        {"Mercedes factory fuel rate", "Mercedes-Werks-Kraftstoffrate", "Fabryczny przepływ paliwa Mercedes"},
        {"Mercedes factory source", "Mercedes-Werksquelle", "Fabryczne źródło Mercedes"},
        {"Mercedes-Benz diagnostics", "Mercedes-Benz-Diagnose", "Diagnostyka Mercedes-Benz"},
        {"MERCEDES-BENZ · C207 / OM651", "MERCEDES-BENZ · C207 / OM651", "MERCEDES-BENZ · C207 / OM651"}
    };
    const char *locale = link_i18n_locale();
    size_t index;
    int language = 0;
    (void)context;
    if (text == NULL) return "";
    if (infiltratr_ascii_starts_with_ci(locale, "de")) language = 1;
    else if (infiltratr_ascii_starts_with_ci(locale, "pl")) language = 2;
    if (language == 0) return text;
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(translations); ++index) {
        if (strcmp(text, translations[index].english) == 0)
            return language == 1 ? translations[index].german
                                 : translations[index].polish;
    }
    return text;
}

/*
 * MBLINK keeps its stable preference names for on-disk/UI compatibility, but
 * LINK owns the dimension types and conversion semantics.
 */
typedef LinkTemperatureUnit MblinkTemperatureUnit;
#define MBLINK_TEMP_CELSIUS LINK_TEMPERATURE_CELSIUS
#define MBLINK_TEMP_FAHRENHEIT LINK_TEMPERATURE_FAHRENHEIT

typedef LinkPressureUnit MblinkPressureUnit;
#define MBLINK_PRESSURE_KPA LINK_PRESSURE_KPA
#define MBLINK_PRESSURE_BAR LINK_PRESSURE_BAR
#define MBLINK_PRESSURE_PSI LINK_PRESSURE_PSI

typedef LinkSpeedUnit MblinkSpeedUnit;
#define MBLINK_SPEED_KMH LINK_SPEED_KMH
#define MBLINK_SPEED_MPH LINK_SPEED_MPH

typedef LinkDistanceUnit MblinkDistanceUnit;
#define MBLINK_DISTANCE_KM LINK_DISTANCE_KM
#define MBLINK_DISTANCE_MILES LINK_DISTANCE_MILES

typedef LinkFuelVolumeUnit MblinkFuelVolumeUnit;
#define MBLINK_FUEL_VOLUME_LITRES LINK_FUEL_VOLUME_LITRES
#define MBLINK_FUEL_VOLUME_US_GALLONS LINK_FUEL_VOLUME_US_GALLONS
#define MBLINK_FUEL_VOLUME_IMPERIAL_GALLONS LINK_FUEL_VOLUME_IMPERIAL_GALLONS

typedef LinkFuelEconomyUnit MblinkFuelEconomyUnit;
#define MBLINK_FUEL_ECONOMY_L_PER_100KM LINK_FUEL_ECONOMY_L_PER_100KM
#define MBLINK_FUEL_ECONOMY_KM_PER_L LINK_FUEL_ECONOMY_KM_PER_L
#define MBLINK_FUEL_ECONOMY_MPG_US LINK_FUEL_ECONOMY_MPG_US
#define MBLINK_FUEL_ECONOMY_MPG_IMPERIAL LINK_FUEL_ECONOMY_MPG_IMPERIAL

typedef LinkFuelRateUnit MblinkFuelRateUnit;
#define MBLINK_FUEL_RATE_L_PER_HOUR LINK_FUEL_RATE_L_PER_HOUR
#define MBLINK_FUEL_RATE_US_GAL_PER_HOUR LINK_FUEL_RATE_US_GAL_PER_HOUR
#define MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR LINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR

typedef LinkAirMassUnit MblinkAirMassUnit;
#define MBLINK_AIR_MASS_G_PER_SECOND LINK_AIR_MASS_G_PER_SECOND
#define MBLINK_AIR_MASS_LB_PER_MINUTE LINK_AIR_MASS_LB_PER_MINUTE

typedef enum MblinkPreferenceKind {
    MBLINK_PREF_TEMPERATURE = 0,
    MBLINK_PREF_PRESSURE,
    MBLINK_PREF_SPEED,
    MBLINK_PREF_DISTANCE,
    MBLINK_PREF_FUEL_VOLUME,
    MBLINK_PREF_FUEL_ECONOMY,
    MBLINK_PREF_FUEL_RATE,
    MBLINK_PREF_AIR_MASS
} MblinkPreferenceKind;

typedef struct MblinkLinuxContext {
    bool connected;
    bool replay_mode;
    bool replay_verify;
    bool replay_verify_emitted;
    bool manufacturer_ui_dirty;
    bool native_adapter_mode;
    char adapter_identity[160];
    LinkTransport transport;
    bool diagnostic_valid;
    bool diagnostic_active;
    bool diagnostic_ready;
    LinkDiagnosticFlow diagnostic;
    bool sample_valid[256];
    LinkObd2Sample samples[256];
    bool sample_responder_valid[256];
    uint32_t sample_responder[256];
    bool sample_responder_extended[256];
    bool decoded_sample_valid[256];
    MblinkObd2DecodedPid decoded_samples[256];
    bool decoded_sample_responder_valid[256];
    uint32_t decoded_sample_responder[256];
    bool decoded_sample_responder_extended[256];
    MblinkLinuxSessionTrace session_trace;
    LinkParameterSelection selected_parameters;
    LinkDashboardPresentationMode dashboard_mode;
    MblinkTemperatureUnit temperature_unit;
    MblinkPressureUnit pressure_unit;
    MblinkSpeedUnit speed_unit;
    MblinkDistanceUnit distance_unit;
    MblinkFuelVolumeUnit fuel_volume_unit;
    MblinkFuelEconomyUnit fuel_economy_unit;
    MblinkFuelRateUnit fuel_rate_unit;
    MblinkAirMassUnit air_mass_unit;
    uint64_t presentation_revision;
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
    bool module_scan_full;
    bool full_sweep_requested;
} MblinkLinuxContext;

static void save_display_preferences(const MblinkLinuxContext *context);

static uint64_t monotonic_ms(void)
{
    uint64_t nanoseconds = 0U;
    if (!infiltratr_monotonic_nanoseconds(&nanoseconds)) return 0U;
    return nanoseconds / UINT64_C(1000000);
}

static const MblinkMercedesEcuEndpointDefinition *engine_endpoint(void)
{
    return mblink_mercedes_generic_engine_endpoint();
}

static const MblinkMercedesVehicleProfile *active_vehicle_profile(
    const MblinkLinuxContext *context)
{
    if (context != NULL &&
        context->manufacturer_scan.probe.identified_profile != NULL) {
        return context->manufacturer_scan.probe.identified_profile;
    }
    return mblink_mercedes_generic_profile();
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
    context->manufacturer_ui_dirty = false;
}

static const char *connection_text(const MblinkLinuxContext *context)
{
    if (context->connected && context->replay_mode)
        return "OFFLINE REPLAY · ELM327/VGATE CAPTURE";
    if (context->connected && context->native_adapter_mode)
        return "LINKED · MERCEDES ME ADAPTER · NATIVE";
    return context->connected ? "LINKED · DIAGNOSTIC ADAPTER VERIFIED" : "NOT LINKED";
}

static const char *diagnostic_text(const MblinkLinuxContext *context)
{
    if (!context->connected) return "LINK OFFLINE";
    if (context->native_adapter_mode)
        return "MERCEDES ME NATIVE CAPTURE · PROTOCOL LEARNING";
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

static LinkParameterKey standard_parameter_key(uint8_t pid)
{
    LinkParameterKey key = {
        LINK_PARAMETER_PROTOCOL_OBD2,
        LINK_PARAMETER_MODULE_STANDARD_OBD2,
        (uint32_t)pid
    };
    return key;
}

static bool effective_polling_enabled(
    const MblinkLinuxContext *context, uint8_t pid)
{
    const LinkParameterKey key = standard_parameter_key(pid);
    return context != NULL &&
        link_parameter_selection_contains(&context->selected_parameters, &key);
}

static void configure_selected_graphs(MblinkLinuxContext *context)
{
    uint8_t pids[LINK_PARAMETER_SELECTION_MAX_ITEMS];
    size_t count;
    if (context == NULL) return;
    count = link_parameter_selection_copy_obd2_pids(
        &context->selected_parameters, pids, G_N_ELEMENTS(pids));
    (void)mblink_linux_trace_configure_graph_pids(
        &context->session_trace, pids, count);
}

static void initialise_polling_policy(MblinkLinuxContext *context)
{
    if (context == NULL) return;
    if (!link_parameter_selection_init(&context->selected_parameters, ""))
        return;

    /*
     * Offline replay is a deterministic diagnostic fixture, not a persisted
     * vehicle profile. Give it LINK's familiar demonstration set so the replay
     * remains useful while real vehicles stay strictly OFF-by-default.
     */
    if (context->replay_mode) {
        size_t count = 0U;
        const uint8_t *default_pids =
            link_scheduler_default_obd2_pids(&count);
        size_t index;
        for (index = 0U; default_pids != NULL && index < count; ++index) {
            const LinkParameterKey key =
                standard_parameter_key(default_pids[index]);
            (void)link_parameter_selection_set(
                &context->selected_parameters, &key, true);
        }
    }
    configure_selected_graphs(context);
}

static bool mblink_polling_enabled(uint8_t pid, void *opaque)
{
    return effective_polling_enabled(
        (const MblinkLinuxContext *)opaque, pid);
}

static void polling_toggled(GtkCheckButton *button, gpointer opaque)
{
    MblinkLinuxContext *context = opaque;
    const guint pid = GPOINTER_TO_UINT(
        g_object_get_data(G_OBJECT(button), "mblink-pid"));
    LinkParameterKey key;
    if (context == NULL || pid > UINT8_MAX) return;

    key = standard_parameter_key((uint8_t)pid);
    if (link_parameter_selection_set(
            &context->selected_parameters,
            &key,
            gtk_check_button_get_active(button) != FALSE) !=
        LINK_PARAMETER_SELECTION_RESULT_OK) {
        return;
    }

    configure_selected_graphs(context);
    ++context->presentation_revision;
    save_display_preferences(context);
}

static char *mblink_path_join_alloc(const char *left, const char *right)
{
    size_t capacity;
    char *path;

    if (left == NULL || right == NULL ||
        !infiltratr_size_add_checked(strlen(left), strlen(right), &capacity) ||
        !infiltratr_size_add_checked(capacity, 2U, &capacity)) {
        return NULL;
    }
    path = malloc(capacity);
    if (path == NULL) return NULL;
    if (!infiltratr_path_join(path, capacity, left, right)) {
        free(path);
        return NULL;
    }
    return path;
}

static char *preferences_config_path(void)
{
    char *base = NULL;
    char *directory;
    char *path;

    if (!infiltratr_xdg_config_home_alloc(&base)) return NULL;
    directory = mblink_path_join_alloc(base, "the-first-infiltrator");
    free(base);
    if (directory == NULL) return NULL;
    if (infiltratr_mkdir_parents(directory, 0700U) != 0) {
        free(directory);
        return NULL;
    }
    path = mblink_path_join_alloc(directory, "mblink.ini");
    free(directory);
    return path;
}

static int key_file_integer_or_default(
    GKeyFile *key_file,
    const char *group,
    const char *key,
    int fallback,
    int maximum)
{
    GError *error = NULL;
    gint value;
    if (key_file == NULL || group == NULL || key == NULL)
        return fallback;
    value = g_key_file_get_integer(key_file, group, key, &error);
    if (error != NULL) {
        g_error_free(error);
        return fallback;
    }
    if (value < 0 || value > maximum) return fallback;
    return (int)value;
}

static char *polling_group_for_vehicle(const char *vehicle_identifier)
{
    if (vehicle_identifier == NULL || vehicle_identifier[0] == '\0')
        return NULL;
    return g_strdup_printf("polling-vin-%s", vehicle_identifier);
}

static void load_polling_selection_for_vehicle(
    MblinkLinuxContext *context,
    const char *vehicle_identifier)
{
    LinkParameterSelection loaded;
    GKeyFile *key_file;
    char *path;
    char *group;

    if (context == NULL || vehicle_identifier == NULL ||
        vehicle_identifier[0] == '\0' ||
        strcmp(context->selected_parameters.vehicle_identifier,
               vehicle_identifier) == 0) {
        return;
    }
    if (!link_parameter_selection_init(&loaded, vehicle_identifier))
        return;

    path = preferences_config_path();
    group = polling_group_for_vehicle(vehicle_identifier);
    if (path != NULL && group != NULL) {
        key_file = g_key_file_new();
        if (g_key_file_load_from_file(
                key_file, path, G_KEY_FILE_NONE, NULL)) {
            for (unsigned int pid = 1U; pid <= UINT8_MAX; ++pid) {
                char key_name[24];
                LinkParameterKey key;
                GError *error = NULL;
                gboolean enabled;

                if (mblink_obd2_mode01_identifier_status((uint8_t)pid) !=
                        LINK_OBD2_IDENTIFIER_ASSIGNED ||
                    (((uint8_t)pid & UINT8_C(0x1f)) == 0U)) {
                    continue;
                }
                (void)snprintf(
                    key_name, sizeof(key_name), "mode01_%02X", pid);
                if (!g_key_file_has_key(
                        key_file, group, key_name, NULL)) {
                    continue;
                }
                enabled = g_key_file_get_boolean(
                    key_file, group, key_name, &error);
                if (error != NULL) {
                    g_error_free(error);
                    continue;
                }
                if (!enabled) continue;
                key = standard_parameter_key((uint8_t)pid);
                (void)link_parameter_selection_set(
                    &loaded, &key, true);
            }
        }
        g_key_file_unref(key_file);
    }
    g_free(group);
    free(path);

    context->selected_parameters = loaded;
    configure_selected_graphs(context);
    ++context->presentation_revision;
}

static void initialise_display_preferences(MblinkLinuxContext *context)
{
    GKeyFile *key_file;
    char *path;
    if (context == NULL) return;

    context->temperature_unit = MBLINK_TEMP_CELSIUS;
    context->pressure_unit = MBLINK_PRESSURE_KPA;
    context->speed_unit = MBLINK_SPEED_KMH;
    context->distance_unit = MBLINK_DISTANCE_KM;
    context->fuel_volume_unit = MBLINK_FUEL_VOLUME_LITRES;
    context->fuel_economy_unit = MBLINK_FUEL_ECONOMY_L_PER_100KM;
    context->fuel_rate_unit = MBLINK_FUEL_RATE_L_PER_HOUR;
    context->air_mass_unit = MBLINK_AIR_MASS_G_PER_SECOND;
    /* Combined preserves the established Linux cockpit appearance by default. */
    context->dashboard_mode = LINK_DASHBOARD_PRESENTATION_COMBINED;
    context->presentation_revision = 1U;

    path = preferences_config_path();
    if (path == NULL) return;
    key_file = g_key_file_new();
    if (g_key_file_load_from_file(
            key_file, path, G_KEY_FILE_NONE, NULL)) {
        context->temperature_unit = (MblinkTemperatureUnit)
            key_file_integer_or_default(
                key_file, "units", "temperature",
                MBLINK_TEMP_CELSIUS, MBLINK_TEMP_FAHRENHEIT);
        context->pressure_unit = (MblinkPressureUnit)
            key_file_integer_or_default(
                key_file, "units", "pressure",
                MBLINK_PRESSURE_KPA, MBLINK_PRESSURE_PSI);
        context->speed_unit = (MblinkSpeedUnit)
            key_file_integer_or_default(
                key_file, "units", "speed",
                MBLINK_SPEED_KMH, MBLINK_SPEED_MPH);
        context->distance_unit = (MblinkDistanceUnit)
            key_file_integer_or_default(
                key_file, "units", "distance",
                MBLINK_DISTANCE_KM, MBLINK_DISTANCE_MILES);
        context->fuel_volume_unit = (MblinkFuelVolumeUnit)
            key_file_integer_or_default(
                key_file, "units", "fuel_volume",
                MBLINK_FUEL_VOLUME_LITRES,
                MBLINK_FUEL_VOLUME_IMPERIAL_GALLONS);
        context->fuel_economy_unit = (MblinkFuelEconomyUnit)
            key_file_integer_or_default(
                key_file, "units", "fuel_economy",
                MBLINK_FUEL_ECONOMY_L_PER_100KM,
                MBLINK_FUEL_ECONOMY_MPG_IMPERIAL);
        context->fuel_rate_unit = (MblinkFuelRateUnit)
            key_file_integer_or_default(
                key_file, "units", "fuel_rate",
                MBLINK_FUEL_RATE_L_PER_HOUR,
                MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR);
        context->air_mass_unit = (MblinkAirMassUnit)
            key_file_integer_or_default(
                key_file, "units", "air_mass",
                MBLINK_AIR_MASS_G_PER_SECOND,
                MBLINK_AIR_MASS_LB_PER_MINUTE);
        context->dashboard_mode = (LinkDashboardPresentationMode)
            key_file_integer_or_default(
                key_file, "display", "dashboard_mode",
                LINK_DASHBOARD_PRESENTATION_COMBINED,
                LINK_DASHBOARD_PRESENTATION_COMBINED);
    }
    g_key_file_unref(key_file);
    free(path);
}

static void save_display_preferences(const MblinkLinuxContext *context)
{
    GKeyFile *key_file;
    char *path;
    char *data;
    char *group;
    gsize length = 0U;
    if (context == NULL) return;
    path = preferences_config_path();
    if (path == NULL) return;

    key_file = g_key_file_new();
    (void)g_key_file_load_from_file(
        key_file, path, G_KEY_FILE_KEEP_COMMENTS, NULL);

    g_key_file_set_integer(
        key_file, "units", "temperature", context->temperature_unit);
    g_key_file_set_integer(
        key_file, "units", "pressure", context->pressure_unit);
    g_key_file_set_integer(
        key_file, "units", "speed", context->speed_unit);
    g_key_file_set_integer(
        key_file, "units", "distance", context->distance_unit);
    g_key_file_set_integer(
        key_file, "units", "fuel_volume", context->fuel_volume_unit);
    g_key_file_set_integer(
        key_file, "units", "fuel_economy", context->fuel_economy_unit);
    g_key_file_set_integer(
        key_file, "units", "fuel_rate", context->fuel_rate_unit);
    g_key_file_set_integer(
        key_file, "units", "air_mass", context->air_mass_unit);
    g_key_file_set_integer(
        key_file, "display", "dashboard_mode", context->dashboard_mode);

    group = polling_group_for_vehicle(
        context->selected_parameters.vehicle_identifier);
    if (group != NULL) {
        for (unsigned int pid = 1U; pid <= UINT8_MAX; ++pid) {
            char key_name[24];
            const LinkParameterKey key =
                standard_parameter_key((uint8_t)pid);
            if (mblink_obd2_mode01_identifier_status((uint8_t)pid) !=
                    LINK_OBD2_IDENTIFIER_ASSIGNED ||
                (((uint8_t)pid & UINT8_C(0x1f)) == 0U)) {
                continue;
            }
            (void)snprintf(
                key_name, sizeof(key_name), "mode01_%02X", pid);
            g_key_file_set_boolean(
                key_file, group, key_name,
                link_parameter_selection_contains(
                    &context->selected_parameters, &key));
        }
        g_free(group);
    }

    data = g_key_file_to_data(key_file, &length, NULL);
    if (data != NULL) {
        (void)infiltratr_atomic_file_write_bytes(
            path, INFILTRATR_ATOMIC_FILE_PRESERVE_PERMISSIONS,
            data, (size_t)length);
        g_free(data);
    }
    g_key_file_unref(key_file);
    free(path);
}

static uint64_t mblink_presentation_revision(void *opaque)
{
    const MblinkLinuxContext *context = opaque;
    return context != NULL ? context->presentation_revision : 0U;
}

static void preference_changed(
    GtkDropDown *dropdown,
    GParamSpec *spec,
    gpointer opaque)
{
    MblinkLinuxContext *context = opaque;
    const guint selected = gtk_drop_down_get_selected(dropdown);
    const guint kind = GPOINTER_TO_UINT(
        g_object_get_data(G_OBJECT(dropdown), "mblink-pref-kind"));
    (void)spec;
    if (context == NULL) return;

    switch ((MblinkPreferenceKind)kind) {
    case MBLINK_PREF_TEMPERATURE:
        if (selected <= MBLINK_TEMP_FAHRENHEIT)
            context->temperature_unit = (MblinkTemperatureUnit)selected;
        break;
    case MBLINK_PREF_PRESSURE:
        if (selected <= MBLINK_PRESSURE_PSI)
            context->pressure_unit = (MblinkPressureUnit)selected;
        break;
    case MBLINK_PREF_SPEED:
        if (selected <= MBLINK_SPEED_MPH)
            context->speed_unit = (MblinkSpeedUnit)selected;
        break;
    case MBLINK_PREF_DISTANCE:
        if (selected <= MBLINK_DISTANCE_MILES)
            context->distance_unit = (MblinkDistanceUnit)selected;
        break;
    case MBLINK_PREF_FUEL_VOLUME:
        if (selected <= MBLINK_FUEL_VOLUME_IMPERIAL_GALLONS)
            context->fuel_volume_unit = (MblinkFuelVolumeUnit)selected;
        break;
    case MBLINK_PREF_FUEL_ECONOMY:
        if (selected <= MBLINK_FUEL_ECONOMY_MPG_IMPERIAL)
            context->fuel_economy_unit = (MblinkFuelEconomyUnit)selected;
        break;
    case MBLINK_PREF_FUEL_RATE:
        if (selected <= MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR)
            context->fuel_rate_unit = (MblinkFuelRateUnit)selected;
        break;
    case MBLINK_PREF_AIR_MASS:
        if (selected <= MBLINK_AIR_MASS_LB_PER_MINUTE)
            context->air_mass_unit = (MblinkAirMassUnit)selected;
        break;
    }
    ++context->presentation_revision;
    save_display_preferences(context);
}

static void dashboard_mode_changed(
    GtkDropDown *dropdown,
    GParamSpec *spec,
    gpointer opaque)
{
    MblinkLinuxContext *context = opaque;
    const guint selected = gtk_drop_down_get_selected(dropdown);
    (void)spec;
    if (context == NULL ||
        selected > LINK_DASHBOARD_PRESENTATION_COMBINED ||
        context->dashboard_mode == (LinkDashboardPresentationMode)selected) {
        return;
    }
    context->dashboard_mode = (LinkDashboardPresentationMode)selected;
    ++context->presentation_revision;
    save_display_preferences(context);
}

static void mblink_link_unit_preferences(
    const MblinkLinuxContext *context,
    LinkUnitPreferences *preferences)
{
    link_unit_preferences_metric(preferences);
    if (context == NULL || preferences == NULL) return;
    preferences->temperature = (LinkTemperatureUnit)context->temperature_unit;
    preferences->pressure = (LinkPressureUnit)context->pressure_unit;
    preferences->speed = (LinkSpeedUnit)context->speed_unit;
    preferences->distance = (LinkDistanceUnit)context->distance_unit;
    preferences->fuel_volume = (LinkFuelVolumeUnit)context->fuel_volume_unit;
    preferences->fuel_economy = (LinkFuelEconomyUnit)context->fuel_economy_unit;
    preferences->fuel_rate = (LinkFuelRateUnit)context->fuel_rate_unit;
    preferences->air_mass = (LinkAirMassUnit)context->air_mass_unit;
}

static void format_sample(const LinkObd2Sample *sample,
                          const MblinkLinuxContext *context,
                          char *buffer,
                          size_t capacity)
{
    MblinkParameterSample parameter;
    LinkUnitPreferences preferences;

    if (buffer == NULL || capacity == 0U) return;
    if (sample == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }

    mblink_link_unit_preferences(context, &preferences);
    if (link_units_format_obd2_with_preferences(
            sample, &preferences, buffer, capacity)) {
        return;
    }

    if (mblink_parameter_from_obd2(sample, 0U, &parameter) &&
        mblink_parameter_format_sample(&parameter, buffer, capacity)) {
        return;
    }

    (void)snprintf(buffer, capacity, "—");
}

static void format_decoded_pid(
    const MblinkObd2DecodedPid *decoded,
    char *buffer,
    size_t capacity)
{
    if (buffer == NULL || capacity == 0U) return;
    if (decoded == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }
    if (!link_obd2_format_decoded_summary(
            decoded, 2U, 8U, buffer, capacity)) {
        (void)snprintf(buffer, capacity, "Decoded");
    }
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
        LinkDtcKnowledge knowledge = {0};
        MblinkMercedesReferenceDtcKnowledge mercedes_reference = {0};
        char label[64];
        char classification[192];
        char definition[512];
        const char *code = list->entries[index].code;
        const bool resolved = link_dtc_resolve(code, &knowledge);
        const bool has_mercedes_reference =
            resolved && !knowledge.definition_known &&
            knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC &&
            mblink_mercedes_reference_dtc_resolve(
                code, &mercedes_reference);

        (void)snprintf(
            label, sizeof(label), "%s %zu · code", prefix, index + 1U);
        link_gtk_card_append_detail(
            card, label,
            resolved && knowledge.code[0] != '\0' ? knowledge.code : code);

        (void)snprintf(label, sizeof(label), "%s %zu · state", prefix, index + 1U);
        link_gtk_card_append_detail(card, label, prefix);

        if (resolved && knowledge.definition_known) {
            (void)snprintf(
                label, sizeof(label), "%s %zu · meaning", prefix, index + 1U);
            link_gtk_card_append_detail(card, label, knowledge.title);

            (void)snprintf(
                classification, sizeof(classification), "%s · %s · %s",
                link_dtc_system_name(knowledge.system),
                knowledge.category,
                link_dtc_origin_name(knowledge.origin));
            (void)snprintf(
                label, sizeof(label), "%s %zu · classification",
                prefix, index + 1U);
            link_gtk_card_append_detail(card, label, classification);

            (void)snprintf(
                definition, sizeof(definition),
                "%s · raw code preserved",
                link_dtc_source_name(knowledge.source));
            (void)snprintf(
                label, sizeof(label), "%s %zu · definition",
                prefix, index + 1U);
            link_gtk_card_append_detail(card, label, definition);
        } else if (has_mercedes_reference) {
            (void)snprintf(
                label, sizeof(label), "%s %zu · meaning", prefix, index + 1U);
            link_gtk_card_append_detail(
                card, label, mercedes_reference.title);

            (void)snprintf(
                classification, sizeof(classification), "%s · %s · %s",
                link_dtc_system_name(knowledge.system),
                mercedes_reference.area,
                mercedes_reference.ambiguous
                    ? "Mercedes manufacturer reference · ambiguous"
                    : "Mercedes manufacturer reference");
            (void)snprintf(
                label, sizeof(label), "%s %zu · classification",
                prefix, index + 1U);
            link_gtk_card_append_detail(card, label, classification);

            (void)snprintf(
                definition, sizeof(definition), "%s · %s · raw code preserved",
                mercedes_reference.source,
                mercedes_reference.applicability);
            (void)snprintf(
                label, sizeof(label), "%s %zu · definition",
                prefix, index + 1U);
            link_gtk_card_append_detail(card, label, definition);
        } else {
            (void)snprintf(
                label, sizeof(label), "%s %zu · definition",
                prefix, index + 1U);
            link_gtk_card_append_detail(
                card, label,
                resolved &&
                knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC
                    ? "Manufacturer-specific definition unknown · raw code preserved"
                    : "Definition unknown · raw code preserved");
        }
    }
}

static void append_readiness_monitor(
    GtkWidget *card,
    const char *name,
    bool supported,
    bool incomplete)
{
    if (card == NULL || name == NULL || !supported) return;
    link_gtk_card_append_detail(
        card, name, incomplete ? "Not ready / incomplete" : "Ready");
}

static void append_diagnostic_context(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *card =
        link_gtk_card_new("DIAGNOSTIC CONTEXT", "Readiness and stored-fault freeze-frame");
    const LinkObd2Readiness *readiness;
    size_t freeze_count = 0U;
    const LinkObd2Sample *freeze_samples;
    char summary[160];
    size_t index;

    if (body == NULL || context == NULL) return;

    if (!context->diagnostic_valid) {
        link_gtk_card_append_status(
            card, "NOT COLLECTED", "state-warning");
        link_gtk_card_append_note(
            card,
            "Diagnostic context is collected as part of a completed standard fault investigation.");
        gtk_box_append(GTK_BOX(body), card);
        return;
    }

    readiness = link_diagnostic_flow_readiness(&context->diagnostic);
    if (!context->diagnostic.readiness_attempted) {
        link_gtk_card_append_status(
            card, "READINESS NOT YET COLLECTED", "state-warning");
    } else if (readiness == NULL) {
        link_gtk_card_append_status(
            card, "READINESS UNAVAILABLE / UNSUPPORTED", "state-warning");
    } else {
        static const char *spark_names[8] = {
            "Catalyst", "Heated catalyst", "Evaporative system",
            "Secondary air", "A/C refrigerant", "Oxygen sensor",
            "Oxygen sensor heater", "EGR / VVT"
        };
        static const char *diesel_names[8] = {
            "NMHC catalyst", "NOx / SCR", NULL, "Boost pressure",
            NULL, "Exhaust gas sensor", "Particulate filter", "EGR / VVT"
        };
        const char *const *names =
            readiness->compression_ignition ? diesel_names : spark_names;
        static const char *continuous_names[3] = {
            "Misfire", "Fuel system", "Comprehensive components"
        };

        (void)snprintf(
            summary, sizeof(summary),
            "READINESS CAPTURED · MIL %s · %u confirmed DTC%s · %s ignition",
            readiness->mil_on ? "ON" : "off",
            (unsigned int)readiness->confirmed_dtc_count,
            readiness->confirmed_dtc_count == 1U ? "" : "s",
            readiness->compression_ignition ? "compression" : "spark");
        link_gtk_card_append_status(card, summary, "state-success");

        for (index = 0U; index < 3U; ++index) {
            append_readiness_monitor(
                card, continuous_names[index],
                (readiness->continuous_supported &
                    (uint8_t)(1U << index)) != 0U,
                (readiness->continuous_incomplete &
                    (uint8_t)(1U << index)) != 0U);
        }
        for (index = 0U; index < 8U; ++index) {
            if (names[index] == NULL) continue;
            append_readiness_monitor(
                card, names[index],
                (readiness->noncontinuous_supported &
                    (uint8_t)(1U << index)) != 0U,
                (readiness->noncontinuous_incomplete &
                    (uint8_t)(1U << index)) != 0U);
        }
    }

    freeze_samples = link_diagnostic_flow_freeze_frame_samples(
        &context->diagnostic, &freeze_count);
    if (!context->diagnostic.freeze_frame_requested) {
        link_gtk_card_append_detail(
            card, "Mode 02 freeze-frame",
            context->diagnostic.standard_diagnostic_context_complete
                ? "Not required · no stored OBD fault reported"
                : "Not yet requested");
    } else if (freeze_samples == NULL || freeze_count == 0U) {
        link_gtk_card_append_detail(
            card, "Mode 02 freeze-frame",
            context->diagnostic.freeze_frame_complete
                ? "Unavailable / unsupported for stored fault"
                : "Collection in progress");
    } else {
        link_gtk_card_append_detail(
            card, "Mode 02 freeze-frame",
            "Frame 0 · captured fault context (NOT current live data)");
        for (index = 0U; index < freeze_count; ++index) {
            char label[96];
            char value[96];
            (void)snprintf(
                label, sizeof(label), "Freeze PID 0x%02X · %s",
                (unsigned int)freeze_samples[index].pid,
                link_obd2_pid_name(freeze_samples[index].pid));
            format_sample(
                &freeze_samples[index], context, value, sizeof(value));
            link_gtk_card_append_detail(card, label, value);
        }
    }

    link_gtk_card_append_note(
        card,
        "Readiness is standards-defined diagnostic state. Mode 02 values are the ECU's stored frame-zero fault context and are deliberately kept separate from current Live Data.");
    gtk_box_append(GTK_BOX(body), card);
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

static const MblinkMercedesModuleScanEntry *replay_orc_module(
    const MblinkLinuxContext *context)
{
    size_t index;
    if (context == NULL || !context->replay_mode) return NULL;
    for (index = 0U; index < context->module_scan.module_count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            &context->module_scan.modules[index];
        if (module->kind == MBLINK_MERCEDES_MODULE_RESTRAINTS &&
            module->dtcs.count != 0U) {
            return module;
        }
    }
    return NULL;
}

static void append_vehicle(GtkWidget *body, MblinkLinuxContext *context)
{
    const MblinkMercedesVehicleProfile *profile = active_vehicle_profile(context);
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    GtkWidget *identity = link_gtk_card_new(
        "VEHICLE EVIDENCE", "Automatic Mercedes VIN / Baumuster identification");
    GtkWidget *connection = link_gtk_card_new("CONNECTION", "Linux diagnostic link");
    MblinkMercedesVinDecode decoded;
    const bool vin_decoded =
        context->manufacturer_scan.probe.vin_result ==
            MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE &&
        mblink_mercedes_vin_decode(
            context->manufacturer_scan.probe.vin, &decoded);
    char detail[192];

    if (vin_decoded) {
        link_gtk_card_append_detail(
            identity, "WMI",
            decoded.wmi_definition != NULL
                ? decoded.wmi_definition->manufacturer : decoded.wmi);
        if (decoded.wmi_definition != NULL)
            link_gtk_card_append_detail(
                identity, "WMI country",
                decoded.wmi_definition->wmi_country);
        if (decoded.baumuster_available)
            link_gtk_card_append_detail(identity, "Baumuster", decoded.baumuster);

        if (decoded.baumuster_definition != NULL) {
            link_gtk_card_append_detail(
                identity, "Chassis",
                decoded.baumuster_definition->chassis_family);
            link_gtk_card_append_detail(
                identity, "Body",
                decoded.baumuster_definition->body_style);
            link_gtk_card_append_detail(
                identity, "Model",
                decoded.baumuster_definition->model);
            link_gtk_card_append_detail(
                identity, "Engine",
                decoded.baumuster_definition->engine_code);
            link_gtk_card_append_detail(
                identity, "Fuel",
                mblink_mercedes_fuel_type_name(
                    decoded.baumuster_definition->fuel));
            if (decoded.baumuster_definition->displacement_cc != 0U) {
                (void)snprintf(
                    detail, sizeof(detail), "%u cc",
                    decoded.baumuster_definition->displacement_cc);
                link_gtk_card_append_detail(identity, "Displacement", detail);
            }
            if (decoded.baumuster_definition->rated_power_kw != 0U) {
                (void)snprintf(
                    detail, sizeof(detail), "%u kW",
                    decoded.baumuster_definition->rated_power_kw);
                link_gtk_card_append_detail(identity, "Catalogue power", detail);
            }
        } else {
            link_gtk_card_append_detail(
                identity, "Series", decoded.series_number);
            link_gtk_card_append_detail(
                identity, "Model",
                "Baumuster not yet present in offline catalogue");
        }

        link_gtk_card_append_detail(
            identity, "Steering",
            mblink_mercedes_steering_name(decoded.steering));
        if (decoded.plant_definition != NULL) {
            (void)snprintf(
                detail, sizeof(detail), "%s, %s",
                decoded.plant_definition->plant,
                decoded.plant_definition->country);
            link_gtk_card_append_detail(identity, "Assembly plant", detail);
        } else if (decoded.plant_code != '\0') {
            detail[0] = decoded.plant_code;
            detail[1] = '\0';
            link_gtk_card_append_detail(identity, "Assembly plant code", detail);
        }
        link_gtk_card_append_detail(
            identity, "Production serial", decoded.serial_number);
    } else {
        link_gtk_card_append_detail(
            identity, "Platform",
            profile != NULL ? profile->chassis_code : "Unidentified");
        link_gtk_card_append_detail(
            identity, "Engine family",
            profile != NULL ? profile->engine_family : "Unidentified");
        link_gtk_card_append_detail(
            identity, "VIN decode", "Waiting for standard VIN DID F190");
    }

    link_gtk_card_append_detail(
        identity, "Diagnostic profile",
        profile != NULL ? profile->display_name : "Mercedes generic");
    link_gtk_card_append_detail(
        identity, "Engine ECU",
        endpoint != NULL ? endpoint->name : "Unavailable");
    link_gtk_card_append_detail(
        identity, "Physical CAN", "0x7E0 → 0x7E8");

    if (context->replay_mode) {
        const MblinkMercedesModuleScanEntry *orc =
            replay_orc_module(context);
        link_gtk_card_append_status(
            identity,
            orc != NULL
                ? "REPLAY ORC FAULT CAPTURED · SEATBELT / AIRBAG"
                : "OFFLINE C207 REPLAY · WAITING FOR INJECTED ORC FAULT",
            "state-warning");
        if (orc != NULL) {
            char code[7];
            char value[128];
            if (!mblink_uds_dtc_format_hex(
                    orc->dtcs.records[0].code,
                    code, sizeof(code))) {
                (void)snprintf(code, sizeof(code), "??????");
            }
            (void)snprintf(
                value, sizeof(value),
                "%s · status 0x%02X · synthetic replay evidence",
                code, (unsigned int)orc->dtcs.records[0].status);
            link_gtk_card_append_detail(
                identity, "Injected ORC UDS fault", value);
        }
    }
    link_gtk_card_append_status(
        connection, connection_text(context),
        context->connected ? "state-success" : "state-warning");
    link_gtk_card_append_detail(
        connection, "Adapter",
        context->connected && context->adapter_identity[0] != '\0'
            ? context->adapter_identity
            : "Select an adapter above and press LINK UP");
    link_gtk_card_append_detail(
        connection, "Diagnostic flow", diagnostic_text(context));
    link_gtk_card_append_note(
        connection,
        context->replay_mode
            ? "Replay mode uses the captured C207/Vgate response shapes with a synthetic VIN serial suffix. The injected ORC restraint fault is test-only and is never presented as a real fault from the vehicle."
            : "MBLINK decodes the Mercedes VIN/FIN offline first, then uses live ECU identity to confirm the controller actually installed. Chassis/model assumptions do not override live evidence.");
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
             context->module_scan_full
                 ? "FULL FORENSIC SWEEP"
                 : "GATEWAY CENSUS",
             context->module_scan.module_count);
        link_gtk_card_append_status(card, summary, "state-warning");
    } else if (context->module_scan_complete) {
        const size_t classified =
            mblink_mercedes_module_scan_classified_count(
                &context->module_scan);
        (void)snprintf(summary, sizeof(summary),
             "%s · %zu responding · %zu classified · %zu unresolved%s",
             context->module_scan_full
                 ? "FULL FORENSIC COMPLETE"
                 : "GATEWAY CENSUS COMPLETE",
             context->module_scan.module_count,
             classified,
             context->module_scan.module_count - classified,
             context->module_scan.truncated
                 ? " · result capacity reached" : "");
        link_gtk_card_append_status(card, summary, "state-success");
        for (size_t index = 0U; index < context->module_scan.module_count; ++index) {
  const MblinkMercedesModuleScanEntry *module = &context->module_scan.modules[index];
  const size_t fault_count =
      mblink_mercedes_module_scan_entry_dtc_count(module);
  const char *protocol = mblink_mercedes_diagnostic_protocol_name(
      mblink_mercedes_module_scan_entry_protocol(module));
  char value[160];
  if (module->extended_id) {
      (void)snprintf(value, sizeof(value),
                     "0x%08X → 0x%08X · %s · %zu fault record%s",
                     (unsigned int)module->tx_can_id,
                     (unsigned int)module->rx_can_id,
                     protocol, fault_count,
                     fault_count == 1U ? "" : "s");
  } else {
      (void)snprintf(value, sizeof(value),
                     "0x%03X → 0x%03X · %s · %zu fault record%s",
                     (unsigned int)module->tx_can_id,
                     (unsigned int)module->rx_can_id,
                     protocol, fault_count,
                     fault_count == 1U ? "" : "s");
  }
  link_gtk_card_append_detail(
      card, mblink_mercedes_module_scan_module_name(module), value);
  if (module->definition != NULL)
      link_gtk_card_append_detail(
          card, "  Mercedes component",
          module->definition->component_designation);
  if (module->identity_available)
      link_gtk_card_append_detail(
          card, "  ECU system name", module->identity);
  if (module->spare_part_number_available)
      link_gtk_card_append_detail(
          card, "  Spare part", module->spare_part_number);
  if (module->software_number_available)
      link_gtk_card_append_detail(
          card, "  Software", module->software_number);
  if (module->hardware_number_available)
      link_gtk_card_append_detail(
          card, "  Hardware", module->hardware_number);
        }
    } else if (context->module_scan_failed) {
        link_gtk_card_append_status(card, "MODULE INVENTORY INCOMPLETE", "state-warning");
    } else {
        link_gtk_card_append_status(card, "MODULE SCAN PENDING", "state-warning");
    }
    link_gtk_card_append_note(card,
        "Normal discovery uses the full Mercedes-owned 11/29-bit target plan with bounded probes. Source-backed routes select their real diagnostic protocol: UDS endpoints use UDS identity/fault reads while confirmed KW2C3PE/KWP2000 endpoints use only KWP read services. FULL SWEEP retains slower read-only fallbacks. Unknown responders remain unresolved rather than guessed.");
    gtk_box_append(GTK_BOX(body), card);
}

static void append_module_fault_rows(
    GtkWidget *card,
    const MblinkLinuxContext *context)
{
    size_t module_index;
    if (card == NULL || context == NULL) return;

    for (module_index = 0U;
         module_index < context->module_scan.module_count;
         ++module_index) {
        const MblinkMercedesModuleScanEntry *module =
            &context->module_scan.modules[module_index];
        char module_label[96];
        char module_value[128];

        if (module->extended_id) {
            (void)snprintf(module_label, sizeof(module_label),
                "%s · 0x%08X",
                mblink_mercedes_module_scan_module_name(module),
                (unsigned int)module->tx_can_id);
        } else {
            (void)snprintf(module_label, sizeof(module_label),
                "%s · 0x%03X",
                mblink_mercedes_module_scan_module_name(module),
                (unsigned int)module->tx_can_id);
        }

        if (module->dtc_result == MBLINK_MERCEDES_MODULE_DTC_AVAILABLE) {
            const size_t fault_count =
                mblink_mercedes_module_scan_entry_dtc_count(module);
            (void)snprintf(module_value, sizeof(module_value),
                "%zu %s fault record%s",
                fault_count,
                mblink_mercedes_diagnostic_protocol_name(
                    mblink_mercedes_module_scan_entry_protocol(module)),
                fault_count == 1U ? "" : "s");
        } else if (module->dtc_result ==
                   MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE) {
            (void)snprintf(module_value, sizeof(module_value),
                "DTC read negative response · NRC 0x%02X",
                (unsigned int)module->dtc_negative_response_code);
        } else {
            (void)snprintf(module_value, sizeof(module_value),
                "DTC read %s",
                module->dtc_result == MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE
                    ? "no response" : "unavailable");
        }
        link_gtk_card_append_detail(card, module_label, module_value);

        if (mblink_mercedes_module_scan_entry_protocol(module) ==
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
            for (size_t dtc_index = 0U;
                 dtc_index < module->kwp_dtcs.count;
                 ++dtc_index) {
                char label[96];
                char value[384];
                const MblinkKwp2000Dtc *record =
                    &module->kwp_dtcs.entries[dtc_index];
                const char *module_key = module->definition != NULL
                    ? module->definition->key : NULL;
                const MblinkMercedesKwpDtcDefinition *definition =
                    mblink_mercedes_kwp_dtc_find(module_key, record->code);
                (void)snprintf(label, sizeof(label),
                    "  %s fault %zu",
                    mblink_mercedes_module_scan_module_name(module),
                    dtc_index + 1U);
                if (definition != NULL) {
                    (void)snprintf(
                        value, sizeof(value),
                        "%04X — %s · KWP2000 status 0x%02X · %s",
                        (unsigned int)record->code,
                        definition->description,
                        (unsigned int)record->status,
                        mblink_mercedes_definition_status_name(
                            definition->status));
                } else {
                    if (!mblink_mercedes_kwp_dtc_format(
                            module_key, record->code, record->status,
                            value, sizeof(value))) {
                        (void)snprintf(value, sizeof(value),
                            "%04X · raw KWP2000 status 0x%02X",
                            (unsigned int)record->code,
                            (unsigned int)record->status);
                    }
                }
                link_gtk_card_append_detail(card, label, value);
                if (definition != NULL) {
                    (void)snprintf(
                        label, sizeof(label),
                        "  %s %04X subsystem",
                        mblink_mercedes_module_scan_module_name(module),
                        (unsigned int)record->code);
                    link_gtk_card_append_detail(
                        card, label, definition->subsystem);
                    (void)snprintf(
                        label, sizeof(label),
                        "  %s %04X applicability",
                        mblink_mercedes_module_scan_module_name(module),
                        (unsigned int)record->code);
                    link_gtk_card_append_detail(
                        card, label, definition->applicability);
                    (void)snprintf(
                        label, sizeof(label),
                        "  %s %04X provenance",
                        mblink_mercedes_module_scan_module_name(module),
                        (unsigned int)record->code);
                    link_gtk_card_append_detail(
                        card, label, definition->provenance);
                }
            }
        } else {
            for (size_t dtc_index = 0U;
                 dtc_index < module->dtcs.count;
                 ++dtc_index) {
                char label[96];
                char value[MBLINK_MERCEDES_DTC_TEXT_LENGTH];
                (void)snprintf(label, sizeof(label),
                    "  %s fault %zu",
                    mblink_mercedes_module_scan_module_name(module),
                    dtc_index + 1U);
                if (!mblink_mercedes_uds_dtc_format(
                        module->definition != NULL
                            ? module->definition->key : NULL,
                        module->dtcs.records[dtc_index].code,
                        module->dtcs.records[dtc_index].status,
                        value, sizeof(value))) {
                    (void)snprintf(value, sizeof(value),
                        "%06X · raw UDS status 0x%02X",
                        (unsigned int)module->dtcs.records[dtc_index].code,
                        (unsigned int)module->dtcs.records[dtc_index].status);
                }
                link_gtk_card_append_detail(card, label, value);

                if (context->replay_mode &&
                    module->kind == MBLINK_MERCEDES_MODULE_RESTRAINTS) {
                    link_gtk_card_append_detail(
                        card,
                        "  REPLAY INJECTION",
                        "Seatbelt / airbag restraint fault · synthetic test evidence");
                }
            }
        }
    }
}

static void append_faults(GtkWidget *body, const MblinkLinuxContext *context)
{
    GtkWidget *mercedes = link_gtk_card_new("MERCEDES FACTORY", "Engine ECU factory diagnostics");
    GtkWidget *modules = link_gtk_card_new("ALL DISCOVERED MODULES", "Per-module Mercedes UDS/KWP2000 fault memory");
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
        link_gtk_card_append_detail(mercedes, "Identified engine family",
  context->manufacturer_scan.probe.identified_profile != NULL
      ? context->manufacturer_scan.probe.identified_profile->engine_family
      : "Unidentified");
        link_gtk_card_append_detail(mercedes, "CRD3 extension",
  context->manufacturer_scan.probe.crd3_fingerprint_attempted
      ? (context->manufacturer_scan.crd3_evidence.om651_cdid3_delphi_signature
          ? "Attempted · OM651/CDID3 signature matched"
          : "Attempted · no exact OM651/CDID3 signature")
      : "Not selected by vehicle/ECU evidence");
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
        const size_t total =
            mblink_mercedes_module_scan_total_dtc_count(
                &context->module_scan);
        (void)snprintf(summary, sizeof(summary),
             "SCANNING · %zu module%s found · %zu factory fault record%s captured",
             context->module_scan.module_count,
             context->module_scan.module_count == 1U ? "" : "s",
             total,
             total == 1U ? "" : "s");
        link_gtk_card_append_status(modules, summary, "state-warning");
        append_module_fault_rows(modules, context);
    } else if (context->module_scan_complete) {
        const size_t total =
            mblink_mercedes_module_scan_total_dtc_count(
                &context->module_scan);
        const size_t classified =
            mblink_mercedes_module_scan_classified_count(
                &context->module_scan);
        (void)snprintf(summary, sizeof(summary),
             "COMPLETE · %zu modules · %zu classified · %zu factory fault record%s",
             context->module_scan.module_count, classified,
             total, total == 1U ? "" : "s");
        link_gtk_card_append_status(modules, summary, "state-success");
        append_module_fault_rows(modules, context);
    } else if (context->module_scan_failed) {
        link_gtk_card_append_status(modules, "MODULE FAULT INVENTORY INCOMPLETE", "state-warning");
    } else {
        link_gtk_card_append_status(modules, "MODULE FAULT SCAN PENDING", "state-warning");
    }
    link_gtk_card_append_note(modules,
        "Each responding ECU is queried independently with UDS ReadDTCInformation 0x19/0x02/0xFF. Fault memory is read only; MBLINK does not clear or alter any module.");

    {
        const size_t standard_fault_count =
            context->diagnostic.stored_dtcs.count +
            context->diagnostic.pending_dtcs.count +
            context->diagnostic.permanent_dtcs.count;
        const MblinkFaultScanPresentationState scan_state =
            mblink_fault_scan_presentation_state(
                context->diagnostic_valid,
                context->diagnostic_active,
                context->diagnostic_valid &&
                    link_diagnostic_flow_standard_context_complete(
                        &context->diagnostic),
                context->diagnostic_valid &&
                    context->diagnostic.stage == LINK_DIAGNOSTIC_FLOW_FAILED,
                standard_fault_count);

        switch (scan_state) {
        case MBLINK_FAULT_SCAN_NOT_SCANNED:
            link_gtk_card_append_status(
                obd,
                context->connected ? "NOT SCANNED" : "NOT SCANNED · LINK OFFLINE",
                "state-warning");
            break;
        case MBLINK_FAULT_SCAN_IN_PROGRESS:
            (void)snprintf(summary, sizeof(summary), "SCAN IN PROGRESS · %s",
                context->diagnostic_valid
                    ? link_diagnostic_flow_stage_name(context->diagnostic.stage)
                    : "starting");
            link_gtk_card_append_status(obd, summary, "state-warning");
            break;
        case MBLINK_FAULT_SCAN_FAILED:
            link_gtk_card_append_status(
                obd, "SCAN FAILED · RECONNECT TO RETRY", "state-warning");
            break;
        case MBLINK_FAULT_SCAN_CLEAN:
        case MBLINK_FAULT_SCAN_FAULTS_PRESENT:
            (void)snprintf(
                summary, sizeof(summary),
                "COMPLETE · %zu stored · %zu pending · %zu permanent",
                context->diagnostic.stored_dtcs.count,
                context->diagnostic.pending_dtcs.count,
                context->diagnostic.permanent_dtcs.count);
            link_gtk_card_append_status(
                obd, summary,
                scan_state == MBLINK_FAULT_SCAN_CLEAN
                    ? "state-success" : "state-warning");
            append_dtc_list(obd, "Stored", &context->diagnostic.stored_dtcs);
            append_dtc_list(obd, "Pending", &context->diagnostic.pending_dtcs);
            append_dtc_list(obd, "Permanent", &context->diagnostic.permanent_dtcs);
            break;
        }
    }

    gtk_box_append(GTK_BOX(body), mercedes);
    gtk_box_append(GTK_BOX(body), modules);
    gtk_box_append(GTK_BOX(body), obd);
    append_diagnostic_context(body, context);
}

static void append_parameters(GtkWidget *body,
                              bool compact,
                              MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new(
        compact ? "PARAMETER TABLE" : "FULL SAE LIVE DATA CATALOGUE",
        compact ? "Enabled standard OBD-II samples"
                : "Every assigned SAE Mode 01 live-data identifier");
    const size_t count = mblink_obd2_pid_definition_count();
    size_t index;

    for (index = 0U; index < count; ++index) {
        const MblinkObd2PidDefinition *definition =
            mblink_obd2_pid_definition_at(index);
        char key[128];
        char value[192];
        uint8_t pid;

        if (definition == NULL ||
            definition->mode != UINT8_C(0x01) ||
            (definition->pid & UINT8_C(0x1f)) == 0U) {
            continue;
        }

        pid = definition->pid;
        if (compact && !effective_polling_enabled(context, pid))
            continue;

        (void)snprintf(
            key, sizeof(key), "PID 0x%02X · %s",
            (unsigned int)pid,
            definition->name != NULL ? definition->name : "SAE parameter");

        if (!effective_polling_enabled(context, pid)) {
            (void)snprintf(value, sizeof(value), "Polling off");
        } else if (context->sample_valid[pid]) {
            format_sample(
                &context->samples[pid], context, value, sizeof(value));
        } else if (context->decoded_sample_valid[pid]) {
            format_decoded_pid(
                &context->decoded_samples[pid], value, sizeof(value));
        } else if (context->diagnostic_valid &&
                   !link_obd2_pid_set_contains(
                       &context->diagnostic.supported_pids, pid)) {
            (void)snprintf(value, sizeof(value), "Not supported by vehicle");
        } else if (context->diagnostic_active || context->diagnostic_ready) {
            (void)snprintf(value, sizeof(value), "Waiting for sample");
        } else {
            (void)snprintf(value, sizeof(value), "No live session");
        }

        if (compact) {
            link_gtk_card_append_detail(card, key, value);
        } else {
            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            GtkWidget *name = link_gtk_left_label(
                definition->name != NULL
                    ? definition->name : "SAE parameter",
                "link-detail-label");
            GtkWidget *detail =
                link_gtk_left_label(value, "link-detail-value");
            GtkWidget *toggle = gtk_check_button_new_with_label("Poll");
            gtk_widget_set_hexpand(name, TRUE);
            gtk_label_set_xalign(GTK_LABEL(detail), 1.0F);
            gtk_check_button_set_active(
                GTK_CHECK_BUTTON(toggle),
                effective_polling_enabled(context, pid));
            g_object_set_data(
                G_OBJECT(toggle), "mblink-pid",
                GUINT_TO_POINTER((guint)pid));
            g_signal_connect(
                toggle, "toggled", G_CALLBACK(polling_toggled), context);
            gtk_box_append(GTK_BOX(row), name);
            gtk_box_append(GTK_BOX(row), detail);
            gtk_box_append(GTK_BOX(row), toggle);
            gtk_box_append(GTK_BOX(card), row);
        }
    }
    gtk_box_append(GTK_BOX(body), card);
}

static void append_fuel_economy(GtkWidget *body,
                                const MblinkLinuxContext *context)
{
    LinkFuelEconomySnapshot snapshot =
        link_fuel_economy_snapshot(&context->fuel_economy, monotonic_ms());
    GtkWidget *card = link_gtk_card_new("FUEL ECONOMY", "Fuel use and trip consumption");
    LinkUnitPreferences preferences;
    LinkFuelEconomyDisplay display;
    LinkFuelEconomySource display_source = snapshot.instantaneous_available
        ? snapshot.instantaneous_source
        : (snapshot.fuel_rate_available ? snapshot.fuel_rate_source : snapshot.average_source);

    mblink_link_unit_preferences(context, &preferences);
    if (!link_fuel_economy_format_display(
            &snapshot, &preferences, context->connected, &display)) {
        (void)snprintf(display.instantaneous, sizeof(display.instantaneous), "Not available");
        (void)snprintf(display.average, sizeof(display.average), "Not available");
        (void)snprintf(display.fuel_rate, sizeof(display.fuel_rate), "Not available");
        (void)snprintf(display.trip, sizeof(display.trip), "Not available");
    }

    link_gtk_card_append_status(card,
        snapshot.instantaneous_available || snapshot.fuel_rate_available
            ? "MEASURED FUEL DATA ACTIVE" : "WAITING FOR FUEL DATA",
        snapshot.instantaneous_available || snapshot.fuel_rate_available
            ? "state-success" : "state-warning");
    link_gtk_card_append_detail(card, "Instantaneous", display.instantaneous);
    link_gtk_card_append_detail(card, "Trip average", display.average);
    link_gtk_card_append_detail(card, "Fuel rate", display.fuel_rate);
    link_gtk_card_append_detail(card, "Trip", display.trip);
    link_gtk_card_append_detail(card, "Current source", fuel_source_text(display_source));
    link_gtk_card_append_detail(card, "Mercedes factory source",
        "No fuel-consumption DID is enabled until its address and scaling are evidence-backed");
    link_gtk_card_append_note(card,
        "LINK prefers a verified Mercedes factory value when one is supplied by the MBLINK profile. Until then it uses measured SAE PID 0x5E fuel rate with PID 0x0D vehicle speed; estimates are never presented as measured data.");
    gtk_box_append(GTK_BOX(body), card);
}

typedef struct MblinkCockpitGaugeDraw {
    double fraction;
    bool available;
} MblinkCockpitGaugeDraw;

static void mblink_cockpit_gauge_draw(
    GtkDrawingArea *area,
    cairo_t *cr,
    int width,
    int height,
    gpointer opaque)
{
    const MblinkCockpitGaugeDraw *gauge = opaque;
    const double pi = 3.14159265358979323846;
    const double start = pi * 0.75;
    const double span = pi * 1.50;
    const double size = width < height ? (double)width : (double)height;
    const double cx = (double)width / 2.0;
    const double cy = (double)height / 2.0;
    const double radius = size * 0.39;
    cairo_pattern_t *face;
    double fraction;

    (void)area;
    if (cr == NULL || gauge == NULL || width <= 0 || height <= 0)
        return;

    face = cairo_pattern_create_radial(
        cx - radius * 0.20, cy - radius * 0.24, radius * 0.06,
        cx, cy, radius);
    cairo_pattern_add_color_stop_rgb(face, 0.0, 0.13, 0.15, 0.17);
    cairo_pattern_add_color_stop_rgb(face, 0.70, 0.055, 0.065, 0.075);
    cairo_pattern_add_color_stop_rgb(face, 1.0, 0.020, 0.024, 0.028);
    cairo_arc(cr, cx, cy, radius - 8.0, 0.0, pi * 2.0);
    cairo_set_source(cr, face);
    cairo_fill(cr);
    cairo_pattern_destroy(face);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 7.0);
    cairo_set_source_rgb(cr, 0.20, 0.23, 0.25);
    cairo_arc(cr, cx, cy, radius, start, start + span);
    cairo_stroke(cr);

    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.75, 0.79, 0.82, 0.38);
    cairo_arc(cr, cx, cy, radius - 12.0, 0.0, pi * 2.0);
    cairo_stroke(cr);

    if (!gauge->available)
        return;
    fraction = gauge->fraction;
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    cairo_set_line_width(cr, 7.0);
    cairo_set_source_rgb(cr, 0.0, 0.678, 0.937);
    cairo_arc(cr, cx, cy, radius, start, start + (span * fraction));
    cairo_stroke(cr);
}

static double mblink_cockpit_fraction(
    const MblinkParameterDefinition *definition,
    double value)
{
    LinkDashboardGaugeRange range;
    double fraction = 0.0;
    if (!link_dashboard_gauge_range_for_parameter(definition, &range) ||
        !link_dashboard_gauge_fraction(&range, value, &fraction)) {
        return 0.0;
    }
    return fraction;
}

static GtkWidget *mblink_cockpit_gauge_new(
    const char *title,
    const char *pid_text,
    const char *value_text,
    bool available,
    double fraction)
{
    GtkWidget *gauge = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *overlay = gtk_overlay_new();
    GtkWidget *drawing = gtk_drawing_area_new();
    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    GtkWidget *value = gtk_label_new(value_text);
    GtkWidget *pid = gtk_label_new(pid_text);
    GtkWidget *title_label = gtk_label_new(title);
    MblinkCockpitGaugeDraw *draw = g_new0(MblinkCockpitGaugeDraw, 1);

    draw->fraction = fraction;
    draw->available = available;
    gtk_widget_add_css_class(gauge, "mblink-cockpit-gauge");
    gtk_widget_set_size_request(gauge, 174, -1);
    gtk_widget_set_size_request(drawing, 154, 154);
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(drawing),
        mblink_cockpit_gauge_draw, draw, g_free);

    gtk_widget_add_css_class(value, "mblink-gauge-value");
    gtk_widget_add_css_class(pid, "mblink-gauge-pid");
    gtk_widget_add_css_class(title_label, "mblink-gauge-title");
    gtk_label_set_xalign(GTK_LABEL(value), 0.5F);
    gtk_label_set_xalign(GTK_LABEL(pid), 0.5F);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.5F);

    gtk_box_append(GTK_BOX(center), value);
    gtk_box_append(GTK_BOX(center), pid);
    gtk_widget_set_halign(center, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center, GTK_ALIGN_CENTER);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), drawing);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), center);
    gtk_box_append(GTK_BOX(gauge), overlay);
    gtk_box_append(GTK_BOX(gauge), title_label);
    return gauge;
}

static void append_dashboard(GtkWidget *body, MblinkLinuxContext *context)
{
    static const struct {
        const char *stable_key;
        const char *title;
    } gauges[] = {
        { "obd2.engine.rpm", "Engine speed" },
        { "obd2.vehicle.speed", "Road speed" },
        { "obd2.engine.coolant", "Coolant" },
        { "obd2.diesel.rail_pressure", "Fuel rail" },
        { "obd2.fuel.tank_level", "Fuel level" }
    };
    GtkWidget *cockpit = link_gtk_card_new(
        "COCKPIT", "Selected live powertrain instruments");
    GtkWidget *flow = gtk_flow_box_new();
    GtkWidget *selected = link_gtk_card_new(
        "SELECTED LIVE DATA", "Every selected standard diagnostic channel");
    size_t index;
    size_t gauge_count = 0U;
    size_t selected_detail_count = 0U;

    gtk_widget_add_css_class(cockpit, "mblink-cockpit-card");
    gtk_widget_add_css_class(flow, "mblink-cockpit-flow");
    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 2);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 5);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 10);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 10);

    link_gtk_card_append_status(
        cockpit,
        context->diagnostic_ready ? "LIVE COCKPIT" : diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");

    {
        static const char *mode_names[] = {
            "Numbers", "Dials", "Combined", NULL
        };
        GtkWidget *mode_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        GtkWidget *mode_label = gtk_label_new("DISPLAY");
        GtkWidget *mode = gtk_drop_down_new_from_strings(mode_names);
        gtk_widget_set_hexpand(mode_label, TRUE);
        gtk_widget_set_halign(mode_label, GTK_ALIGN_START);
        gtk_drop_down_set_selected(
            GTK_DROP_DOWN(mode), (guint)context->dashboard_mode);
        g_signal_connect(
            mode, "notify::selected",
            G_CALLBACK(dashboard_mode_changed), context);
        gtk_box_append(GTK_BOX(mode_row), mode_label);
        gtk_box_append(GTK_BOX(mode_row), mode);
        gtk_box_append(GTK_BOX(cockpit), mode_row);
    }

    for (index = 0U; index < G_N_ELEMENTS(gauges); ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_for_stable_key(
                gauges[index].stable_key);
        char value[96];
        char pid_text[24];
        uint8_t pid;
        bool available;
        double fraction = 0.0;

        if (definition == NULL ||
            definition->key.identifier > UINT8_MAX) {
            continue;
        }
        pid = (uint8_t)definition->key.identifier;
        if (!effective_polling_enabled(context, pid))
            continue;
        available = context->sample_valid[pid];
        if (available) {
            format_sample(
                &context->samples[pid], context, value, sizeof(value));
            fraction = mblink_cockpit_fraction(
                definition, context->samples[pid].value);
        } else if (context->decoded_sample_valid[pid]) {
            format_decoded_pid(
                &context->decoded_samples[pid], value, sizeof(value));
        } else {
            (void)snprintf(value, sizeof(value), "Waiting for sample");
        }
        (void)snprintf(
            pid_text, sizeof(pid_text), "PID 0x%02X",
            (unsigned int)pid);
        if (context->dashboard_mode == LINK_DASHBOARD_PRESENTATION_NUMBERS) {
            link_gtk_card_append_detail(cockpit, gauges[index].title, value);
        } else {
            gtk_flow_box_append(
                GTK_FLOW_BOX(flow),
                mblink_cockpit_gauge_new(
                    gauges[index].title, pid_text, value,
                    available, fraction));
        }
        ++gauge_count;
    }

    if (context->dashboard_mode != LINK_DASHBOARD_PRESENTATION_NUMBERS &&
        gauge_count != 0U) {
        gtk_box_append(GTK_BOX(cockpit), flow);
    }
    if (link_parameter_selection_count(&context->selected_parameters) == 0U) {
        link_gtk_card_append_note(
            cockpit,
            "No live channels are selected. Choose channels in Table; a new vehicle profile starts with every channel OFF.");
    } else {
        link_gtk_card_append_note(
            cockpit,
            "The cockpit follows the same explicit channel selection as Table and Graphs. Values remain in Waiting state until measured; no interpolation or synthetic live values are introduced.");
    }
    gtk_box_append(GTK_BOX(body), cockpit);

    for (index = 0U; index < mblink_parameter_obd2_definition_count(); ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_at(index);
        bool featured = false;
        char label[128];
        char value[192];
        uint8_t pid;
        size_t gauge_index;

        if (definition == NULL ||
            definition->key.protocol != LINK_PARAMETER_PROTOCOL_OBD2 ||
            definition->key.module != LINK_PARAMETER_MODULE_STANDARD_OBD2 ||
            definition->key.identifier > UINT8_MAX) {
            continue;
        }
        pid = (uint8_t)definition->key.identifier;
        if ((pid & UINT8_C(0x1f)) == 0U ||
            !effective_polling_enabled(context, pid)) {
            continue;
        }
        for (gauge_index = 0U;
             gauge_index < G_N_ELEMENTS(gauges);
             ++gauge_index) {
            if (definition->stable_key != NULL &&
                strcmp(
                    definition->stable_key,
                    gauges[gauge_index].stable_key) == 0) {
                featured = true;
                break;
            }
        }
        if (featured) continue;

        (void)snprintf(
            label, sizeof(label), "PID 0x%02X · %s",
            (unsigned int)pid,
            definition->name != NULL ? definition->name : "SAE parameter");
        if (context->sample_valid[pid]) {
            format_sample(
                &context->samples[pid], context, value, sizeof(value));
        } else if (context->decoded_sample_valid[pid]) {
            format_decoded_pid(
                &context->decoded_samples[pid], value, sizeof(value));
        } else {
            (void)snprintf(value, sizeof(value), "Waiting for sample");
        }
        link_gtk_card_append_detail(selected, label, value);
        ++selected_detail_count;
    }

    if (selected_detail_count != 0U) {
        link_gtk_card_append_note(
            selected,
            "These channels are selected explicitly and therefore remain visible while waiting for their first sample.");
        gtk_box_append(GTK_BOX(body), selected);
    }
    append_fuel_economy(body, context);
}

static void append_graphs(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *summary = link_gtk_card_new(
        "GRAPH", "Selected telemetry history");
    GtkWidget *flow = gtk_flow_box_new();
    size_t index;
    size_t rendered = 0U;

    gtk_widget_add_css_class(flow, "mblink-trace-flow");
    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 1);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 3);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 12);

    link_gtk_card_append_status(
        summary, diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");

    for (index = 0U;
         index < context->session_trace.graph_count;
         ++index) {
        const uint8_t pid = context->session_trace.graph_pids[index];
        const size_t count =
            context->session_trace.graph_history_count[index];
        const MblinkObd2PidDefinition *definition;
        GtkWidget *trace_card;
        char kicker[48];
        char trace[192];
        char current[96];
        char range[224];
        char responder[48];

        definition = mblink_obd2_pid_definition(UINT8_C(0x01), pid);
        if (context->sample_valid[pid]) {
            format_sample(
                &context->samples[pid], context,
                current, sizeof(current));
        } else if (context->decoded_sample_valid[pid]) {
            format_decoded_pid(
                &context->decoded_samples[pid],
                current, sizeof(current));
        } else {
            (void)snprintf(
                current, sizeof(current), "Waiting for sample");
        }

        if (count != 0U && context->sample_valid[pid]) {
            char minimum_text[96];
            char maximum_text[96];
            double minimum;
            double maximum;
            size_t start;
            size_t sample_index;
            LinkObd2Sample boundary_sample;

            mblink_linux_trace_format_sparkline(
                context->session_trace.graph_history[index], count,
                context->session_trace.graph_history_next[index],
                trace, sizeof(trace));
            start = count <
                    G_N_ELEMENTS(
                        context->session_trace.graph_history[index])
                ? 0U
                : context->session_trace.graph_history_next[index];
            minimum =
                context->session_trace.graph_history[index][start];
            maximum = minimum;
            for (sample_index = 1U;
                 sample_index < count;
                 ++sample_index) {
                const double sample_value =
                    context->session_trace.graph_history[index][
                        (start + sample_index) %
                        G_N_ELEMENTS(
                            context->session_trace.graph_history[index])];
                if (sample_value < minimum) minimum = sample_value;
                if (sample_value > maximum) maximum = sample_value;
            }

            boundary_sample = context->samples[pid];
            boundary_sample.value = minimum;
            format_sample(
                &boundary_sample, context,
                minimum_text, sizeof(minimum_text));
            boundary_sample.value = maximum;
            format_sample(
                &boundary_sample, context,
                maximum_text, sizeof(maximum_text));
            (void)snprintf(
                range, sizeof(range), "%s — %s",
                minimum_text, maximum_text);
        } else {
            (void)snprintf(
                trace, sizeof(trace), "Waiting for first sample");
            (void)snprintf(
                range, sizeof(range), "Waiting for sample");
        }

        if (context->sample_responder_valid[pid]) {
            (void)snprintf(
                responder, sizeof(responder),
                context->sample_responder_extended[pid]
                    ? "ECU 0x%08X" : "ECU 0x%03X",
                (unsigned int)context->sample_responder[pid]);
        } else {
            (void)snprintf(
                responder, sizeof(responder), "Waiting for responder");
        }

        (void)snprintf(
            kicker, sizeof(kicker), "PID 0x%02X · %zu sample%s",
            (unsigned int)pid, count, count == 1U ? "" : "s");
        trace_card = link_gtk_card_new(
            kicker,
            definition != NULL && definition->name != NULL
                ? definition->name : "SAE parameter");
        gtk_widget_add_css_class(trace_card, "mblink-trace-card");
        link_gtk_card_append_detail(
            trace_card, "Current", current);
        link_gtk_card_append_detail(
            trace_card, "Session range", range);
        link_gtk_card_append_detail(
            trace_card, "Responder", responder);
        link_gtk_card_append_note(trace_card, trace);
        gtk_flow_box_append(GTK_FLOW_BOX(flow), trace_card);
        ++rendered;
    }

    if (rendered == 0U) {
        link_gtk_card_append_note(
            summary,
            "No live channels are selected. Choose channels in Table; selected channels appear here immediately, even before the first sample arrives.");
    } else {
        link_gtk_card_append_note(
            summary,
            "Graphs use the same explicit selection as Dashboard and Table. Each selected channel remains visible while waiting and retains only history collected under that selection.");
    }
    gtk_box_append(GTK_BOX(body), summary);
    if (rendered != 0U)
        gtk_box_append(GTK_BOX(body), flow);
}

static size_t live_sample_count(const MblinkLinuxContext *context)
{
    size_t pid;
    size_t count = 0U;
    if (context == NULL) return 0U;
    for (pid = 0U; pid < 256U; ++pid) {
        if (context->sample_valid[pid] || context->decoded_sample_valid[pid])
            ++count;
    }
    return count;
}

static void append_session_log(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *card = link_gtk_card_new(
        "SESSION RECORDER", "Diagnostic evidence");
    char value[128];

    link_gtk_card_append_status(
        card, diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");
    link_gtk_card_append_detail(
        card, "Connection",
        context->connected ? connection_text(context) : "Not linked");
    link_gtk_card_append_detail(
        card, "Adapter",
        context->adapter_identity[0] != '\0'
            ? context->adapter_identity : "No adapter identity captured");

    (void)snprintf(value, sizeof(value), "%zu",
                   context->module_scan.module_count);
    link_gtk_card_append_detail(card, "Responding Mercedes modules", value);

    (void)snprintf(
        value, sizeof(value), "%zu",
        mblink_mercedes_module_scan_total_dtc_count(&context->module_scan));
    link_gtk_card_append_detail(card, "Mercedes fault records", value);

    (void)snprintf(value, sizeof(value), "%zu", live_sample_count(context));
    link_gtk_card_append_detail(card, "Live standard data items", value);

    (void)snprintf(value, sizeof(value), "%zu",
                   context->manufacturer_scan.dtcs.count);
    link_gtk_card_append_detail(card, "Engine manufacturer fault records", value);

    if (context->session_trace.session_log_count != 0U) {
        size_t index;
        for (index = 0U;
             index < context->session_trace.session_log_count;
             ++index) {
            const size_t slot = mblink_linux_trace_log_ordered_slot(
                &context->session_trace, index);
            const uint64_t elapsed = context->session_trace.session_log_time_ms[slot];
            char when[32];
            (void)snprintf(
                when, sizeof(when), "+%llu.%03llus",
                (unsigned long long)(elapsed / UINT64_C(1000)),
                (unsigned long long)(elapsed % UINT64_C(1000)));
            link_gtk_card_append_detail(
                card, when, context->session_trace.session_log[slot]);
        }
    } else {
        link_gtk_card_append_note(
            card, "No chronological session events have been recorded yet.");
    }

    link_gtk_card_append_note(
        card,
        "The bounded event history above is presentation-only. Save Session remains the authoritative full evidence bundle.");
    gtk_box_append(GTK_BOX(body), card);
    append_diagnostic_context(body, context);
}

static void append_preference_dropdown(
    GtkWidget *card,
    const char *title,
    const char *description,
    const char *const *choices,
    guint selected,
    MblinkPreferenceKind kind,
    MblinkLinuxContext *context)
{
    GtkWidget *row;
    GtkWidget *copy;
    GtkWidget *dropdown;

    if (card == NULL || choices == NULL || context == NULL) return;
    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    copy = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    /*
     * Use GTK's string constructor so MBLINK never has to manage a
     * GtkDropDown model reference here. This permanently removes the ownership
     * mistake that produced the field g_list_model_get_n_items() crashes.
     */
    dropdown = gtk_drop_down_new_from_strings(choices);

    gtk_widget_add_css_class(row, "link-settings-row");
    gtk_widget_add_css_class(row, "mblink-settings-row");
    gtk_widget_add_css_class(copy, "link-settings-copy");
    gtk_widget_add_css_class(dropdown, "link-settings-dropdown");
    gtk_widget_set_hexpand(copy, TRUE);
    gtk_box_append(
        GTK_BOX(copy),
        link_gtk_left_label(title, "link-settings-title"));
    gtk_box_append(
        GTK_BOX(copy),
        link_gtk_left_label(description, "link-settings-description"));
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), selected);
    g_object_set_data(
        G_OBJECT(dropdown), "mblink-pref-kind",
        GUINT_TO_POINTER((guint)kind));
    g_signal_connect(
        dropdown, "notify::selected",
        G_CALLBACK(preference_changed), context);
    gtk_box_append(GTK_BOX(row), copy);
    gtk_box_append(GTK_BOX(row), dropdown);
    gtk_box_append(GTK_BOX(card), row);
}

static void append_measurement_settings(
    GtkWidget *body,
    MblinkLinuxContext *context)
{
    static const char *temperature[] =
        { "Celsius (°C)", "Fahrenheit (°F)", NULL };
    static const char *pressure[] =
        { "Kilopascal (kPa)", "Bar (bar)", "PSI (psi)", NULL };
    static const char *speed[] =
        { "Kilometres per hour (km/h)", "Miles per hour (mph)", NULL };
    static const char *distance[] =
        { "Kilometres (km)", "Miles (mi)", NULL };
    static const char *fuel_volume[] =
        { "Litres (L)", "US gallons (US gal)", "Imperial gallons (Imp gal)", NULL };
    static const char *fuel_economy[] = {
        "Litres per 100 km (L/100 km)",
        "Kilometres per litre (km/L)",
        "Miles per gallon — US (mpg US)",
        "Miles per gallon — Imperial (mpg Imp)",
        NULL
    };
    static const char *fuel_rate[] = {
        "Litres per hour (L/h)",
        "US gallons per hour (US gal/h)",
        "Imperial gallons per hour (Imp gal/h)",
        NULL
    };
    static const char *air_mass[] =
        { "Grams per second (g/s)", "Pounds per minute (lb/min)", NULL };
    GtkWidget *card =
        link_gtk_card_new("MEASUREMENT", "Units & display formats");

    gtk_widget_add_css_class(card, "mblink-settings-section");
    append_preference_dropdown(
        card, "Temperature",
        "Coolant, intake-air, ambient and exhaust-gas temperatures.",
        temperature, (guint)context->temperature_unit,
        MBLINK_PREF_TEMPERATURE, context);
    append_preference_dropdown(
        card, "Pressure",
        "Boost, manifold, fuel-rail and differential-pressure readings.",
        pressure, (guint)context->pressure_unit,
        MBLINK_PREF_PRESSURE, context);
    append_preference_dropdown(
        card, "Speed",
        "Vehicle and wheel-speed presentation.",
        speed, (guint)context->speed_unit,
        MBLINK_PREF_SPEED, context);
    append_preference_dropdown(
        card, "Distance",
        "Trip distance and distance-based summaries.",
        distance, (guint)context->distance_unit,
        MBLINK_PREF_DISTANCE, context);
    append_preference_dropdown(
        card, "Fuel volume",
        "Trip fuel quantity and other volumetric fuel values.",
        fuel_volume, (guint)context->fuel_volume_unit,
        MBLINK_PREF_FUEL_VOLUME, context);
    append_preference_dropdown(
        card, "Fuel economy",
        "Instantaneous and trip-average consumption.",
        fuel_economy, (guint)context->fuel_economy_unit,
        MBLINK_PREF_FUEL_ECONOMY, context);
    append_preference_dropdown(
        card, "Fuel flow",
        "Measured volumetric fuel-rate display.",
        fuel_rate, (guint)context->fuel_rate_unit,
        MBLINK_PREF_FUEL_RATE, context);
    append_preference_dropdown(
        card, "Air mass flow",
        "Mass-air-flow sensor presentation.",
        air_mass, (guint)context->air_mass_unit,
        MBLINK_PREF_AIR_MASS, context);
    link_gtk_card_append_note(
        card,
        "RPM, percentages and volts retain their conventional engineering units. These choices change presentation only; captured diagnostic values and exported evidence remain canonical.");
    gtk_box_append(GTK_BOX(body), card);
}

static void append_application_settings(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *card =
        link_gtk_card_new("APPLICATION", "MBLINK system information");
    link_gtk_card_append_detail(card, "Version", mblink_version());
    link_gtk_card_append_detail(
        card, "Product", "Mercedes-Benz diagnostics");
    link_gtk_card_append_detail(
        card, "Portable core",
        mblink_self_check() ? "Validated" : "Invalid metadata");
    link_gtk_card_append_detail(
        card, "Transport",
        "LINK native ELM/Bluetooth + Tactrix OpenPort 2.0 USB");
    link_gtk_card_append_detail(
        card, "Diagnostic mode",
        "SAE OBD-II + Mercedes read-only factory extension");
    link_gtk_card_append_detail(
        card, "Vehicle discovery",
        "VIN/ECU evidence + bounded module census");
    link_gtk_card_append_note(
        card,
        "Display preferences are saved automatically for this user account and apply immediately to Linux presentation.");
    gtk_box_append(GTK_BOX(body), card);
    (void)context;
}

static void append_services(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *procedure =
        link_gtk_card_new("SERVICES", "Mercedes-Benz procedure policy");
    GtkWidget *catalogue =
        link_gtk_card_new("UDS CATALOGUE", "LINK generic service codecs");
    GtkWidget *reports =
        link_gtk_card_new("READ DTC INFORMATION", "Complete 0x19 report catalogue");
    size_t index;

    link_gtk_card_append_status(
        procedure,
        context != NULL && context->diagnostic_ready
            ? "NO ACTIVE CONTROL PROCEDURE ENABLED"
            : "CONNECT TO EVALUATE VEHICLE-SPECIFIC READS",
        "state-warning");
    link_gtk_card_append_note(
        procedure,
        "Generic UDS codec availability is not a Mercedes capability claim and is not permission to transmit. MBLINK exposes a vehicle procedure only after its target, prerequisites, request sequence and safety behaviour are independently verified.");
    gtk_box_append(GTK_BOX(body), procedure);

    for (index = 0U; index < link_uds_standard_service_count(); ++index) {
        const LinkUdsServiceDefinition *definition =
            link_uds_standard_service_at(index);
        char key[16];
        char value[256];
        uint8_t payload[1];
        link_safety_result safety;
        const char *effect;
        const char *reason;
        if (definition == NULL) continue;
        payload[0] = definition->service;
        safety = link_safety_classify(payload, sizeof(payload));
        effect = link_uds_service_effect_name(definition->effect);
        reason = link_safety_reason_string(safety.reason);
        (void)snprintf(
            key, sizeof(key), "0x%02X", (unsigned int)definition->service);
        (void)snprintf(
            value, sizeof(value), "%s · %s · %s",
            definition->name != NULL ? definition->name : "Unknown",
            effect != NULL ? effect : "unknown",
            safety.decision == LINK_SAFETY_ALLOW_READ_ONLY
                ? "read-only transmission allowed"
                : (reason != NULL ? reason : "blocked by policy"));
        link_gtk_card_append_detail(catalogue, key, value);
    }
    link_gtk_card_append_note(
        catalogue,
        "A blocked entry still has a standards-shaped codec; it remains non-transmissible through Discover. In particular, service 0x14 DTC clearing and service 0x29 Authentication stay gated.");
    gtk_box_append(GTK_BOX(body), catalogue);

    for (index = 0U; index < link_uds_dtc_report_definition_count(); ++index) {
        const LinkUdsDtcReportDefinition *definition =
            link_uds_dtc_report_definition_at(index);
        char key[16];
        char value[256];
        if (definition == NULL) continue;
        (void)snprintf(
            key, sizeof(key), "19 %02X", (unsigned int)definition->subfunction);
        (void)snprintf(
            value, sizeof(value), "%s · %s",
            definition->name != NULL ? definition->name : "Unknown",
            definition->withdrawn_in_2020
                ? "legacy compatibility / withdrawn in 2020"
                : "current generic report type");
        link_gtk_card_append_detail(reports, key, value);
    }
    link_gtk_card_append_note(
        reports,
        "Snapshot and extended-data response tails remain raw when their record sizes depend on ECU/DID definitions. The generic layer validates only the fixed envelope it can prove.");
    gtk_box_append(GTK_BOX(body), reports);
}

static void render_section(size_t section, GtkWidget *body, void *opaque)
{
    MblinkLinuxContext *context = opaque;
    switch ((LinkWorkspaceSection)section) {
    case LINK_WORKSPACE_VEHICLE:
        append_vehicle(body, context);
        append_modules(body, context);
        break;
    case LINK_WORKSPACE_FAULTS:
        append_faults(body, context);
        break;
    case LINK_WORKSPACE_TABLE:
        append_parameters(body, true, context);
        break;
    case LINK_WORKSPACE_DASHBOARD:
        append_dashboard(body, context);
        break;
    case LINK_WORKSPACE_GRAPHS:
        append_graphs(body, context);
        break;
    case LINK_WORKSPACE_TESTS:
        append_diagnostic_context(body, context);
        break;
    case LINK_WORKSPACE_SERVICES:
        append_services(body, context);
        break;
    case LINK_WORKSPACE_LOG:
        append_session_log(body, context);
        break;
    case LINK_WORKSPACE_SETTINGS:
        append_measurement_settings(body, context);
        append_application_settings(body, context);
        break;
    case LINK_WORKSPACE_SECTION_COUNT:
        break;
    case LINK_WORKSPACE_OBD:
    case LINK_WORKSPACE_MODULES:
    case LINK_WORKSPACE_LIVE_DATA:
        break; /* compatibility-only internal IDs; never primary navigation */
    }
}

static const char *mblink_linux_build_label(const char *profile)
{
    if (g_strcmp0(profile, "native") == 0)
        return "Native / local machine compile";
    if (g_strcmp0(profile, "generic") == 0)
        return "Generic / APT package";
    return "Source / development build";
}

static MblinkMercedesModuleScanResult begin_module_scan(
    MblinkLinuxContext *context)
{
    if (context == NULL)
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    return context->module_scan_full
        ? mblink_mercedes_module_scan_begin_full(&context->module_scan)
        : mblink_mercedes_module_scan_begin_mobile_census(&context->module_scan);
}

static void request_full_sweep(void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (context == NULL) return;
    context->full_sweep_requested = true;
}

static bool manufacturer_begin(void *opaque)
{
    MblinkLinuxContext *context = opaque;
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    if (context == NULL || endpoint == NULL) return false;
    {
        const bool full_sweep = context->full_sweep_requested;
        reset_manufacturer_scan(context);
        context->module_scan_full = full_sweep;
        context->full_sweep_requested = false;
    }
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
        const bool vin_before =
            context->manufacturer_scan.probe.vin_result ==
                MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE;
        const bool supplier_before =
            context->manufacturer_scan.probe.crd3_supplier_available;
        MblinkMercedesEcuProbeResult result =
            mblink_mercedes_engine_scan_accept(
                &context->manufacturer_scan,
                (const MblinkElm327Response *)response);
        if (context->manufacturer_scan.probe.vin_result ==
                MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE) {
            load_polling_selection_for_vehicle(
                context, context->manufacturer_scan.probe.vin);
        }
        if (vin_before !=
                (context->manufacturer_scan.probe.vin_result ==
                 MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE) ||
            supplier_before !=
                context->manufacturer_scan.probe.crd3_supplier_available) {
            context->manufacturer_ui_dirty = true;
        }
        if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
            context->manufacturer_scan_active = false;
            context->manufacturer_scan_complete = true;
            context->manufacturer_ui_dirty = true;
            if (begin_module_scan(context) ==
                MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
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
        context->manufacturer_ui_dirty = true;
        if (begin_module_scan(context) ==
            MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
            context->module_scan_active = true;
            return true;
        }
        context->module_scan_failed = true;
        if (complete != NULL) *complete = true;
        return true;
    }

    if (context->module_scan_active) {
        const size_t modules_before = context->module_scan.module_count;
        const size_t dtcs_before =
            mblink_mercedes_module_scan_total_dtc_count(
                &context->module_scan);
        const size_t classified_before =
            mblink_mercedes_module_scan_classified_count(
                &context->module_scan);
        MblinkMercedesModuleScanResult result =
            mblink_mercedes_module_scan_accept(
                &context->module_scan,
                (const MblinkElm327Response *)response);
        const size_t modules_after = context->module_scan.module_count;
        const size_t dtcs_after =
            mblink_mercedes_module_scan_total_dtc_count(
                &context->module_scan);
        const size_t classified_after =
            mblink_mercedes_module_scan_classified_count(
                &context->module_scan);

        if (modules_before != modules_after ||
            dtcs_before != dtcs_after ||
            classified_before != classified_after) {
            context->manufacturer_ui_dirty = true;
        }
        if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE) {
            context->module_scan_active = false;
            context->module_scan_complete = true;
            context->manufacturer_ui_dirty = true;
            if (complete != NULL) *complete = true;
            return true;
        }
        if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) return true;
        context->module_scan_active = false;
        context->module_scan_failed = true;
        context->manufacturer_ui_dirty = true;
        if (complete != NULL) *complete = true;
        return true;
    }
    return false;
}

static gboolean replay_verify_quit(gpointer opaque)
{
    (void)opaque;
    /*
     * This callback exists only for --replay-c207-verify.  Once the rendered
     * ORC success marker has been emitted, terminate the verifier cleanly
     * instead of leaving a GUI main loop alive until CI's timeout expires.
     */
    exit(EXIT_SUCCESS);
    return G_SOURCE_REMOVE;
}

static bool manufacturer_progress_changed(void *opaque)
{
    MblinkLinuxContext *context = opaque;
    bool changed;
    if (context == NULL) return false;
    changed = context->manufacturer_ui_dirty;
    context->manufacturer_ui_dirty = false;

    /*
     * CI's replay verifier quits on the first rendered state containing the
     * synthetic ORC fault.  Scheduling the quit through the GLib idle queue
     * lets the shared shell repaint the current Vehicle/Faults view first.
     */
    if (changed && context->replay_verify &&
        !context->replay_verify_emitted &&
        replay_orc_module(context) != NULL) {
        context->replay_verify_emitted = true;
        (void)printf(
            "MBLINK replay verified: ORC seatbelt/airbag fault B00013 visible\n");
        (void)fflush(stdout);
        (void)g_idle_add(replay_verify_quit, NULL);
    }
    return changed;
}

static void manufacturer_finished(bool complete, void *opaque)
{
    MblinkLinuxContext *context = opaque;
    if (context == NULL) return;
    mblink_linux_trace_append_log(
        &context->session_trace, monotonic_ms(),
        complete ? "Mercedes manufacturer scan complete"
                 : "Mercedes manufacturer scan incomplete");
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
    .progress_changed = manufacturer_progress_changed,
    .finished = manufacturer_finished
};

static void connection_changed(LinkTransport *transport,
                               bool connected,
                               const char *adapter_identity,
                               void *opaque)
{
    MblinkLinuxContext *context = opaque;
    char log_message[192];
    context->connected = connected;
    context->transport = *transport;
    infiltratr_copy_string(
        context->adapter_identity, sizeof(context->adapter_identity),
        connected && adapter_identity != NULL ? adapter_identity : "");
    if (connected) {
        mblink_linux_trace_clear_log(&context->session_trace, monotonic_ms());
        (void)snprintf(
            log_message, sizeof(log_message), "Connected · %s",
            context->adapter_identity[0] != '\0'
                ? context->adapter_identity : "diagnostic adapter");
        mblink_linux_trace_append_log(&context->session_trace, monotonic_ms(), log_message);
    } else {
        mblink_linux_trace_append_log(&context->session_trace, monotonic_ms(), "Disconnected");
    }
    context->native_adapter_mode =
        connected && adapter_identity != NULL &&
        strstr(adapter_identity, "Mercedes me Adapter") != NULL;
    reset_manufacturer_scan(context);
    if (connected) {
        (void)link_parameter_selection_set_vehicle(
            &context->selected_parameters, "");
        configure_selected_graphs(context);
        ++context->presentation_revision;
    } else {
        mblink_linux_trace_reset_graph(&context->session_trace);
    }
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
    const bool was_ready = context->diagnostic_ready;
    context->diagnostic_active = active;
    context->diagnostic_ready = ready;

    if (flow == NULL) {
        mblink_linux_trace_append_log(&context->session_trace, monotonic_ms(), "Diagnostic flow reset");
        context->diagnostic_valid = false;
        memset(&context->diagnostic, 0, sizeof(context->diagnostic));
        memset(context->sample_valid, 0, sizeof(context->sample_valid));
        memset(context->samples, 0, sizeof(context->samples));
        memset(
            context->sample_responder_valid, 0,
            sizeof(context->sample_responder_valid));
        memset(context->sample_responder, 0, sizeof(context->sample_responder));
        memset(
            context->sample_responder_extended, 0,
            sizeof(context->sample_responder_extended));
        memset(
            context->decoded_sample_valid, 0,
            sizeof(context->decoded_sample_valid));
        memset(context->decoded_samples, 0, sizeof(context->decoded_samples));
        memset(
            context->decoded_sample_responder_valid, 0,
            sizeof(context->decoded_sample_responder_valid));
        memset(
            context->decoded_sample_responder, 0,
            sizeof(context->decoded_sample_responder));
        memset(
            context->decoded_sample_responder_extended, 0,
            sizeof(context->decoded_sample_responder_extended));
        mblink_linux_trace_reset_graph(&context->session_trace);
        link_fuel_economy_init(&context->fuel_economy);
        return;
    }

    context->diagnostic = *flow;
    context->diagnostic_valid = true;

    if (ready && !was_ready)
        mblink_linux_trace_append_log(&context->session_trace, monotonic_ms(), "Live diagnostics ready");
    if (event != NULL) {
        const char *event_text = mblink_linux_trace_event_text(event->kind);
        if (event_text != NULL)
            mblink_linux_trace_append_log(&context->session_trace, monotonic_ms(), event_text);
    }

    if (event != NULL &&
        (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE ||
         event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_STRUCTURED)) {
        const uint64_t now_ms = monotonic_ms();
        bool graph_sample_dirty[256] = {false};

        for (size_t index = 0U;
             index < event->responder_decoded.count;
             ++index) {
            const LinkObd2ResponderDecodedPid *entry =
                &event->responder_decoded.entries[index];
            if (!entry->responder_id_available ||
                entry->decoded.definition == NULL) {
                continue;
            }
            const uint8_t pid = entry->decoded.definition->pid;
            if (!mblink_linux_trace_prefer_responder(
                    entry->responder_id, entry->extended_id,
                    context->decoded_sample_responder_valid[pid],
                    context->decoded_sample_responder[pid],
                    context->decoded_sample_responder_extended[pid])) {
                continue;
            }
            context->decoded_samples[pid] = entry->decoded;
            context->decoded_sample_valid[pid] = true;
            context->decoded_sample_responder_valid[pid] = true;
            context->decoded_sample_responder[pid] = entry->responder_id;
            context->decoded_sample_responder_extended[pid] =
                entry->extended_id;
        }

        if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE) {
            const LinkObd2Sample *fuel_sample = NULL;
            for (size_t index = 0U;
                 index < event->responder_samples.count;
                 ++index) {
                const LinkObd2ResponderSample *entry =
                    &event->responder_samples.samples[index];
                if (!entry->responder_id_available) continue;
                const uint8_t pid = entry->sample.pid;
                if (!mblink_linux_trace_prefer_responder(
                        entry->responder_id, entry->extended_id,
                        context->sample_responder_valid[pid],
                        context->sample_responder[pid],
                        context->sample_responder_extended[pid])) {
                    continue;
                }
                context->samples[pid] = entry->sample;
                context->sample_valid[pid] = true;
                context->sample_responder_valid[pid] = true;
                context->sample_responder[pid] = entry->responder_id;
                context->sample_responder_extended[pid] =
                    entry->extended_id;
                graph_sample_dirty[pid] = true;
                fuel_sample = &context->samples[pid];
            }
            for (size_t pid = 0U; pid < 256U; ++pid) {
                if (graph_sample_dirty[pid]) {
                    mblink_linux_trace_record_graph(
                        &context->session_trace, (uint8_t)pid,
                        context->samples[pid].value);
                }
            }
            if (fuel_sample != NULL) {
                (void)link_fuel_economy_observe_obd2(
                    &context->fuel_economy, fuel_sample, now_ms);
            }
        }
        link_fuel_economy_tick(&context->fuel_economy, now_ms);
    }
}

static void append_session_state_json(GString *json, void *opaque)
{
    const MblinkLinuxContext *context = opaque;
    if (json == NULL || context == NULL) return;

    g_string_append_printf(
        json,
        "{\"replay_mode\":%s,"
        "\"manufacturer_scan\":{"
        "\"started\":%s,\"active\":%s,\"complete\":%s,\"failed\":%s,"
        "\"engine_fault_records\":%zu},"
        "\"module_scan\":{"
        "\"active\":%s,\"complete\":%s,\"failed\":%s,"
        "\"modules_found\":%zu,\"fault_records\":%zu}}",
        context->replay_mode ? "true" : "false",
        context->manufacturer_scan_started ? "true" : "false",
        context->manufacturer_scan_active ? "true" : "false",
        context->manufacturer_scan_complete ? "true" : "false",
        context->manufacturer_scan_failed ? "true" : "false",
        context->manufacturer_scan.dtcs.count,
        context->module_scan_active ? "true" : "false",
        context->module_scan_complete ? "true" : "false",
        context->module_scan_failed ? "true" : "false",
        context->module_scan.module_count,
        mblink_mercedes_module_scan_total_dtc_count(&context->module_scan));
}

static bool verify_display_preferences(void)
{
    MblinkLinuxContext context = {0};
    LinkObd2Sample sample = {0};
    char value[96];
    LinkParameterKey rpm_key;

    initialise_polling_policy(&context);
    if (link_parameter_selection_count(
            &context.selected_parameters) != 0U) {
        return false;
    }
    rpm_key = standard_parameter_key(UINT8_C(0x0c));
    if (link_parameter_selection_set(
            &context.selected_parameters, &rpm_key, true) !=
        LINK_PARAMETER_SELECTION_RESULT_OK) {
        return false;
    }
    configure_selected_graphs(&context);
    if (context.session_trace.graph_count != 1U ||
        context.session_trace.graph_pids[0] != UINT8_C(0x0c)) {
        return false;
    }

    /* Arrival order must never make 7E9 displace the selected 7E8 stream. */
    if (!mblink_linux_trace_prefer_responder(
            UINT32_C(0x7e8), false, true, UINT32_C(0x7e9), false)) {
        return false;
    }
    if (mblink_linux_trace_prefer_responder(
            UINT32_C(0x7e9), false, true, UINT32_C(0x7e8), false)) {
        return false;
    }
    if (!mblink_linux_trace_prefer_responder(
            UINT32_C(0x7e8), false, true, UINT32_C(0x7e8), false)) {
        return false;
    }

    context.temperature_unit = MBLINK_TEMP_FAHRENHEIT;
    sample.unit = LINK_OBD2_UNIT_CELSIUS;
    sample.value = 100.0;
    format_sample(&sample, &context, value, sizeof(value));
    if (strcmp(value, "212.0 °F") != 0) return false;

    context.pressure_unit = MBLINK_PRESSURE_BAR;
    sample.unit = LINK_OBD2_UNIT_KPA;
    sample.value = 250.0;
    format_sample(&sample, &context, value, sizeof(value));
    if (strcmp(value, "2.50 bar") != 0) return false;

    context.speed_unit = MBLINK_SPEED_MPH;
    sample.unit = LINK_OBD2_UNIT_KMH;
    sample.value = 100.0;
    format_sample(&sample, &context, value, sizeof(value));
    if (strcmp(value, "62.1 mph") != 0) return false;

    context.air_mass_unit = MBLINK_AIR_MASS_LB_PER_MINUTE;
    sample.unit = LINK_OBD2_UNIT_GRAMS_PER_SECOND;
    sample.value = 10.0;
    format_sample(&sample, &context, value, sizeof(value));
    if (strcmp(value, "1.32 lb/min") != 0) return false;

    context.fuel_rate_unit = MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR;
    sample.unit = LINK_OBD2_UNIT_LITRES_PER_HOUR;
    sample.value = 10.0;
    format_sample(&sample, &context, value, sizeof(value));
    if (strcmp(value, "2.20 Imp gal/h") != 0) return false;

    context.fuel_economy_unit = MBLINK_FUEL_ECONOMY_MPG_IMPERIAL;
    context.distance_unit = MBLINK_DISTANCE_MILES;
    context.fuel_volume_unit = MBLINK_FUEL_VOLUME_US_GALLONS;
    {
        LinkUnitPreferences preferences;
        LinkFuelEconomySnapshot fuel = {0};
        LinkFuelEconomyDisplay display;
        fuel.instantaneous_available = true;
        fuel.instantaneous_l_per_100km = 10.0;
        fuel.average_available = true;
        fuel.average_l_per_100km = 10.0;
        fuel.fuel_rate_available = true;
        fuel.fuel_rate_l_per_hour = 10.0;
        fuel.trip_fuel_litres = 10.0;
        fuel.trip_distance_km = 100.0;
        fuel.moving = true;
        mblink_link_unit_preferences(&context, &preferences);
        if (!link_fuel_economy_format_display(
                &fuel, &preferences, true, &display)) {
            return false;
        }
        if (strcmp(display.instantaneous, "28.2 mpg (Imp)") != 0)
            return false;
        if (strcmp(display.trip, "2.64 US gal over 62.1 mi") != 0)
            return false;
    }

    sample.pid = UINT8_C(0x0c);
    sample.unit = LINK_OBD2_UNIT_RPM;
    sample.value = 1500.0;
    mblink_linux_trace_record_graph(
        &context.session_trace, sample.pid, sample.value);
    if (context.session_trace.graph_history_count[0] != 1U) return false;
    mblink_linux_trace_reset_graph(&context.session_trace);
    if (context.session_trace.graph_history_count[0] != 0U ||
        context.session_trace.graph_history_next[0] != 0U ||
        context.session_trace.graph_history[0][0] != 0.0) {
        return false;
    }

    {
        MblinkObd2DecodedPid decoded = {0};
        decoded.signal_count = 3U;
        decoded.signals[0].label = "commanded boost A";
        decoded.signals[0].value = 120.0;
        decoded.signals[0].unit = "kPa";
        decoded.signals[1].label = "actual boost A";
        decoded.signals[1].value = 125.0;
        decoded.signals[1].unit = "kPa";
        decoded.signals[2].label = "boost A control status";
        decoded.signals[2].value = 1.0;
        decoded.signals[2].unit = "state";
        format_decoded_pid(&decoded, value, sizeof(value));
        if (strstr(value, "commanded boost A 120.00 kPa") == NULL)
            return false;
        if (strstr(value, "+1") == NULL)
            return false;
    }

    (void)printf(
        "MBLINK settings verified: OFF-by-default vehicle selection + 8 measurement preferences + structured SAE display + selected graph-session reset\n");
    return true;
}


static const char *mblink_navigation_icon_resource(size_t section, void *opaque)
{
    (void)opaque;
    switch ((LinkWorkspaceSection)section) {
    case LINK_WORKSPACE_VEHICLE:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-vehicle.png";
    case LINK_WORKSPACE_FAULTS:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-errors.png";
    case LINK_WORKSPACE_TABLE:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-table.png";
    case LINK_WORKSPACE_DASHBOARD:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-dashboard.png";
    case LINK_WORKSPACE_GRAPHS:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-graph.png";
    case LINK_WORKSPACE_TESTS:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-tests.png";
    case LINK_WORKSPACE_SERVICES:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-services.png";
    case LINK_WORKSPACE_LOG:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-log.png";
    case LINK_WORKSPACE_SETTINGS:
        return "/com/github/Infiltrator-Projects/MBLINK/nav-settings.png";
    default:
        return NULL;
    }
}

int main(int argc, char **argv)
{
    MblinkLinuxContext context = {0};
    MblinkC207ReplayTransport replay;
    LinkGtkShellDescriptor descriptor = {0};
    LinkAboutInfo about_info = {0};
    const InfiltratrProjectInfo *project_info;
    bool replay_mode = false;
    bool replay_verify = false;
    bool settings_verify = false;
    char *runtime_css;
    int status;
    int index;

    for (index = 1; index < argc; ++index) {
        const bool replay_argument =
            strcmp(argv[index], "--replay-c207") == 0 ||
            strcmp(argv[index], "--replay-c207-verify") == 0;
        const bool settings_argument =
            strcmp(argv[index], "--ui-settings-verify") == 0;
        if (replay_argument || settings_argument) {
            int move;
            if (replay_argument) {
                replay_mode = true;
                if (strcmp(argv[index], "--replay-c207-verify") == 0)
                    replay_verify = true;
            } else {
                settings_verify = true;
            }
            for (move = index; move + 1 < argc; ++move)
                argv[move] = argv[move + 1];
            --argc;
            --index;
        }
    }

    if (!mblink_linux_style_register_fonts()) {
        (void)fprintf(stderr,
            "MBLINK: bundled MB Corpo font set unavailable; using platform fallback fonts.\n");
    }
    if (settings_verify)
        return verify_display_preferences() ? 0 : 5;

    context.replay_mode = replay_mode;
    context.replay_verify = replay_verify;
    initialise_polling_policy(&context);
    initialise_display_preferences(&context);
    link_fuel_economy_init(&context.fuel_economy);

    project_info = mblink_project_info();
    about_info.product_name = project_info->program_name;
    about_info.version = project_info->version;
    about_info.description = project_info->comments;
    about_info.build =
        mblink_linux_build_label(project_info->build_profile);
    about_info.authors = "Shannon Smith";
    about_info.copyright = project_info->copyright_text;
    about_info.website = project_info->website;
    about_info.license_name = project_info->license_id;
    about_info.license_text =
        "MBLINK is free software licensed under the GNU General Public "
        "License version 3 or, at your option, any later version "
        "(GPL-3.0-or-later).\n\n"
        "See LICENSE in the source package for the complete licence text.";
    about_info.credits = "Shannon Smith — Author and project maintainer";

    descriptor.app_id = "com.github.The-First-Infiltrator.MBLINK";
    descriptor.window_title = replay_mode
        ? "MBLINK · C207 Offline Replay"
        : "MBLINK · Mercedes-Benz Diagnostics";
    descriptor.brand_name = "MBLINK";
    descriptor.brand_subtitle = replay_mode
        ? "MERCEDES-BENZ · C207 / OM651 · OFFLINE REPLAY"
        : "MERCEDES-BENZ · LINK DIAGNOSTICS";
    descriptor.version = mblink_version();
    descriptor.emblem_resource = "/com/github/Infiltrator-Projects/MBLINK/mblink-emblem.png";
    descriptor.navigation_icon_resource = mblink_navigation_icon_resource;
    descriptor.use_client_side_titlebar = true;
    runtime_css = g_strconcat(
        mblink_linux_style_base_css(),
        mblink_linux_style_metrics_css(), NULL);
    if (runtime_css == NULL) return 6;
    descriptor.css = runtime_css;
    descriptor.translate_text = mblink_translate_text;
    descriptor.render_section = render_section;
    descriptor.about = &about_info;
    descriptor.connection_changed = connection_changed;
    descriptor.diagnostic_changed = diagnostic_changed;
    descriptor.polling_enabled = mblink_polling_enabled;
    descriptor.presentation_revision = mblink_presentation_revision;
    descriptor.append_session_state_json = append_session_state_json;
    descriptor.diagnostic_restart_action_label = "DEEP RESCAN";
    descriptor.diagnostic_restart_action = request_full_sweep;
    descriptor.manufacturer_extension = &mblink_manufacturer_extension;
    if (replay_mode) {
        mblink_c207_replay_init(&replay);
        descriptor.transport_provider = mblink_c207_replay_provider();
        descriptor.transport_provider_context = &replay;
        descriptor.auto_connect = true;
    }
    descriptor.context = &context;
    status = link_gtk_shell_run(argc, argv, &descriptor);
    g_free(runtime_css);
    return status;
}
