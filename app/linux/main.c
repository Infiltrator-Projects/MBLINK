// SPDX-License-Identifier: GPL-3.0-or-later
#include "about-dialog.h"
#include "c207-replay.h"
#include "link-gtk-shell.h"
#include "link-gtk-widgets.h"
#include "link/dtc_knowledge.h"
#include "link/fuel_economy.h"
#include "link/workspace.h"
#include "mblink/mblink.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_engine_scan.h"
#include "mblink/fault_investigation.h"
#include "mblink/mercedes_module_scan.h"
#include "mblink/obd2.h"
#include "mblink/parameter.h"

#include <fontconfig/fontconfig.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum MblinkTemperatureUnit {
    MBLINK_TEMP_CELSIUS = 0,
    MBLINK_TEMP_FAHRENHEIT
} MblinkTemperatureUnit;

typedef enum MblinkPressureUnit {
    MBLINK_PRESSURE_KPA = 0,
    MBLINK_PRESSURE_BAR,
    MBLINK_PRESSURE_PSI
} MblinkPressureUnit;

typedef enum MblinkSpeedUnit {
    MBLINK_SPEED_KMH = 0,
    MBLINK_SPEED_MPH
} MblinkSpeedUnit;

typedef enum MblinkDistanceUnit {
    MBLINK_DISTANCE_KM = 0,
    MBLINK_DISTANCE_MILES
} MblinkDistanceUnit;

typedef enum MblinkFuelVolumeUnit {
    MBLINK_FUEL_VOLUME_LITRES = 0,
    MBLINK_FUEL_VOLUME_US_GALLONS,
    MBLINK_FUEL_VOLUME_IMPERIAL_GALLONS
} MblinkFuelVolumeUnit;

typedef enum MblinkFuelEconomyUnit {
    MBLINK_FUEL_ECONOMY_L_PER_100KM = 0,
    MBLINK_FUEL_ECONOMY_KM_PER_L,
    MBLINK_FUEL_ECONOMY_MPG_US,
    MBLINK_FUEL_ECONOMY_MPG_IMPERIAL
} MblinkFuelEconomyUnit;

typedef enum MblinkFuelRateUnit {
    MBLINK_FUEL_RATE_L_PER_HOUR = 0,
    MBLINK_FUEL_RATE_US_GAL_PER_HOUR,
    MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR
} MblinkFuelRateUnit;

typedef enum MblinkAirMassUnit {
    MBLINK_AIR_MASS_G_PER_SECOND = 0,
    MBLINK_AIR_MASS_LB_PER_MINUTE
} MblinkAirMassUnit;

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
    bool decoded_sample_valid[256];
    MblinkObd2DecodedPid decoded_samples[256];
    bool polling_enabled[256];
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

static const char mblink_css[] =
    "window { background: #050608; color: #e8ecef; font-family: \"MB Corpo S Title WEB\"; font-size: 14px; font-weight: 400; }"
    "window *, popover, popover * { font-family: \"MB Corpo S Title WEB\"; }"
    "button, button *, .link-toolbar-button, .link-toolbar-button *, .link-link-button, .link-link-button *, .link-save-session-button, .link-save-session-button *, .link-about-button, .link-about-button * { font-family: \"MB Corpo S Title WEB\"; font-size: 13px; font-weight: 700; }"
    "dropdown, dropdown *, .link-adapter-combo, .link-adapter-combo *, popover, popover * { font-family: \"MB Corpo S Title WEB\"; font-size: 13px; font-weight: 400; }"
    "entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-family: \"MB Corpo S Title WEB\"; font-size: 13px; font-weight: 400; }"
    ".link-toolbar-button { min-height: 40px; padding: 7px 14px; }"
    ".link-link-button { min-width: 100px; min-height: 42px; padding: 8px 18px; }"
    ".link-save-session-button { min-height: 42px; padding: 8px 14px; }"
    ".link-adapter-combo { min-height: 40px; }"
    ".link-section-title { font-size: 14px; }"
    ".link-section-summary { font-size: 12px; }"
    ".link-brand-subtitle { font-size: 12px; }"
    ".link-content-title { font-size: 30px; }"
    ".link-detail-label, .link-detail-value { font-size: 13px; }"
    ".link-card-note { font-size: 12px; }"
    ".link-settings-description, .mblink-settings-note { font-size: 12px; }"
    ".mblink-about-dialog { background: #050608; }"
    ".mblink-about-dialog stackswitcher { margin: 8px 14px 12px 14px; }"
    ".mblink-about-dialog stackswitcher button, .mblink-about-dialog stackswitcher button * { min-width: 100px; min-height: 42px; padding: 8px 18px; font-family: \"MB Corpo S Title WEB\"; font-size: 14px; font-weight: 700; }"
    ".mblink-about-dialog label, .mblink-about-dialog textview, .mblink-about-dialog textview text { font-family: \"MB Corpo S Title WEB\"; font-size: 14px; }"
    ".mblink-about-dialog textview, .mblink-about-dialog textview text { font-weight: 400; }"
    ".mblink-about-dialog scrolledwindow { min-width: 500px; min-height: 300px; }"
    ".link-connection-bar { background: #101318; border: 1px solid #353a40; }"
    ".link-brand { color: #eef1f3; font-family: \"MB Corpo A Title Cond WEB\"; font-weight: 400; }"
    ".link-brand-subtitle { color: #aeb6bd; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-brand-version { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-section-title { color: #e7ebee; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-section-summary { color: #899198; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-content-title { color: #e7ebee; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-content-summary { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-card { background: linear-gradient(135deg,#171b20,#0d1014); border: 1px solid #353a40; border-radius: 18px; padding: 20px; }"
    ".link-card-kicker { color: #8c949b; font-family: \"MB Corpo S Title WEB\"; font-size: 10px; font-weight: 700; letter-spacing: 2px; }"
    ".link-card-title { color: #eef1f3; font-family: \"MB Corpo S Title WEB\"; font-size: 20px; font-weight: 700; }"
    ".link-detail-label { color: #7e858c; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-detail-value { color: #eef1f3; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-card-note { color: #9ca4ab; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-status-chip { padding: 7px 11px; border-radius: 999px; border: 1px solid #3b4147; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-toolbar-label { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-toolbar-button, .link-toolbar-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-link-button { background: #d7dde2; color: #111418; border-radius: 10px; }"
    ".link-save-session-button, .link-save-session-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-connection-status { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-about-button, .link-about-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-settings-title { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-settings-description { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".state-warning { color: #d19e47; border-color: #72572f; }"
    ".state-success { color: #63ab7c; border-color: #365f45; }"
    ".mblink-settings-section { margin-top: 2px; }"
    ".mblink-settings-row { padding: 10px 0; }"
    ".mblink-settings-row dropdown { min-width: 210px; }"
    ".mblink-settings-note { color: #899198; font-family: \"MB Corpo S Title WEB\"; font-size: 11px; font-weight: 400; }";

static bool register_one_project_font(FcConfig *config, const char *filename)
{
    char *build_path;
    char *install_path;
    bool added = false;

    if (config == NULL || filename == NULL) return false;
    build_path = g_build_filename(MBLINK_FONT_BUILD_DIR, filename, NULL);
    install_path = g_build_filename(MBLINK_FONT_INSTALL_DIR, filename, NULL);
    if (build_path != NULL && g_file_test(build_path, G_FILE_TEST_IS_REGULAR))
        added = FcConfigAppFontAddFile(
            config, (const FcChar8 *)build_path) != FcFalse;
    if (!added && install_path != NULL &&
        g_file_test(install_path, G_FILE_TEST_IS_REGULAR))
        added = FcConfigAppFontAddFile(
            config, (const FcChar8 *)install_path) != FcFalse;
    g_free(build_path);
    g_free(install_path);
    return added;
}

static bool register_project_fonts(void)
{
    static const char *const fonts[] = {
        "mb_corpo_a_cond_regular.ttf",
        "mb_corpo_s_bold.ttf",
        "mb_corpo_s_regular.ttf"
    };
    FcConfig *config;
    size_t index;

    if (FcInit() == FcFalse) return false;
    config = FcConfigGetCurrent();
    if (config == NULL) return false;
    for (index = 0U; index < G_N_ELEMENTS(fonts); ++index) {
        if (!register_one_project_font(config, fonts[index]))
            return false;
    }
    return FcConfigBuildFonts(config) != FcFalse;
}

static uint64_t monotonic_ms(void)
{
    const gint64 value = g_get_monotonic_time();
    return value <= 0 ? 0U : (uint64_t)(value / 1000);
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

static bool effective_polling_enabled(
    const MblinkLinuxContext *context, uint8_t pid)
{
    return context != NULL &&
        (context->replay_mode || context->polling_enabled[pid]);
}

static void initialise_polling_policy(MblinkLinuxContext *context)
{
    static const uint8_t default_pids[] = {
        UINT8_C(0x0c), UINT8_C(0x0d), UINT8_C(0x05), UINT8_C(0x23),
        UINT8_C(0x11), UINT8_C(0x49), UINT8_C(0x4a), UINT8_C(0x46),
        UINT8_C(0x2f)
    };
    size_t index;
    if (context == NULL) return;
    memset(context->polling_enabled, 0, sizeof(context->polling_enabled));
    for (index = 0U;
         index < sizeof(default_pids) / sizeof(default_pids[0]); ++index) {
        context->polling_enabled[default_pids[index]] = true;
    }
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
    if (context == NULL || pid > UINT8_MAX) return;
    context->polling_enabled[pid] = gtk_check_button_get_active(button);
    ++context->presentation_revision;
    save_display_preferences(context);
}

static char *preferences_config_path(void)
{
    char *directory = g_build_filename(
        g_get_user_config_dir(), "the-first-infiltrator", NULL);
    char *path;
    if (directory == NULL) return NULL;
    (void)g_mkdir_with_parents(directory, 0700);
    path = g_build_filename(directory, "mblink.ini", NULL);
    g_free(directory);
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

        /*
         * Polling is a product preference, not a vehicle capability. Retain
         * the operator's choices by Mode 01 identifier while leaving new keys
         * at the safe default policy selected by initialise_polling_policy().
         */
        for (unsigned int pid = 1U; pid <= UINT8_MAX; ++pid) {
            char key[24];
            if (mblink_obd2_mode01_identifier_status((uint8_t)pid) !=
                    LINK_OBD2_IDENTIFIER_ASSIGNED ||
                (((uint8_t)pid & UINT8_C(0x1f)) == 0U)) {
                continue;
            }
            (void)snprintf(key, sizeof(key), "mode01_%02X", pid);
            if (g_key_file_has_key(key_file, "polling", key, NULL)) {
                context->polling_enabled[pid] =
                    g_key_file_get_boolean(
                        key_file, "polling", key, NULL) != FALSE;
            }
        }
    }
    g_key_file_unref(key_file);
    g_free(path);
}

static void save_display_preferences(const MblinkLinuxContext *context)
{
    GKeyFile *key_file;
    char *path;
    char *data;
    gsize length = 0U;
    if (context == NULL) return;
    path = preferences_config_path();
    if (path == NULL) return;
    key_file = g_key_file_new();
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
    for (unsigned int pid = 1U; pid <= UINT8_MAX; ++pid) {
        char key[24];
        if (mblink_obd2_mode01_identifier_status((uint8_t)pid) !=
                LINK_OBD2_IDENTIFIER_ASSIGNED ||
            (((uint8_t)pid & UINT8_C(0x1f)) == 0U)) {
            continue;
        }
        (void)snprintf(key, sizeof(key), "mode01_%02X", pid);
        g_key_file_set_boolean(
            key_file, "polling", key, context->polling_enabled[pid]);
    }
    data = g_key_file_to_data(key_file, &length, NULL);
    if (data != NULL) {
        (void)g_file_set_contents(path, data, (gssize)length, NULL);
        g_free(data);
    }
    g_key_file_unref(key_file);
    g_free(path);
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

static void format_sample(const LinkObd2Sample *sample,
                          const MblinkLinuxContext *context,
                          char *buffer,
                          size_t capacity)
{
    MblinkParameterSample parameter;
    const char *unit;
    if (buffer == NULL || capacity == 0U) return;
    if (sample == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }

    if (context != NULL) {
        switch (sample->unit) {
        case LINK_OBD2_UNIT_CELSIUS:
            if (context->temperature_unit == MBLINK_TEMP_FAHRENHEIT)
                (void)snprintf(
                    buffer, capacity, "%.1f °F",
                    sample->value * 9.0 / 5.0 + 32.0);
            else
                (void)snprintf(buffer, capacity, "%.1f °C", sample->value);
            return;

        case LINK_OBD2_UNIT_KPA:
            if (context->pressure_unit == MBLINK_PRESSURE_BAR)
                (void)snprintf(
                    buffer, capacity, "%.2f bar", sample->value / 100.0);
            else if (context->pressure_unit == MBLINK_PRESSURE_PSI)
                (void)snprintf(
                    buffer, capacity,
                    sample->value < 70.0 && sample->value > -70.0
                        ? "%.2f psi" : "%.1f psi",
                    sample->value * 0.14503773773020923);
            else
                (void)snprintf(
                    buffer, capacity,
                    sample->value < 100.0 && sample->value > -100.0
                        ? "%.2f kPa" : "%.1f kPa",
                    sample->value);
            return;

        case LINK_OBD2_UNIT_KMH:
            if (context->speed_unit == MBLINK_SPEED_MPH)
                (void)snprintf(
                    buffer, capacity, "%.1f mph",
                    sample->value * 0.621371192237334);
            else
                (void)snprintf(buffer, capacity, "%.1f km/h", sample->value);
            return;

        case LINK_OBD2_UNIT_GRAMS_PER_SECOND:
            if (context->air_mass_unit == MBLINK_AIR_MASS_LB_PER_MINUTE)
                (void)snprintf(
                    buffer, capacity, "%.2f lb/min",
                    sample->value * 0.1322773573109265);
            else
                (void)snprintf(buffer, capacity, "%.2f g/s", sample->value);
            return;

        case LINK_OBD2_UNIT_LITRES_PER_HOUR:
            if (context->fuel_rate_unit ==
                MBLINK_FUEL_RATE_US_GAL_PER_HOUR)
                (void)snprintf(
                    buffer, capacity, "%.2f US gal/h",
                    sample->value * 0.2641720523581484);
            else if (context->fuel_rate_unit ==
                     MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR)
                (void)snprintf(
                    buffer, capacity, "%.2f Imp gal/h",
                    sample->value * 0.2199692482990878);
            else
                (void)snprintf(buffer, capacity, "%.2f L/h", sample->value);
            return;

        default:
            break;
        }
    }

    /*
     * Prefer the shared LINK/MBLINK parameter formatter so Linux presents the
     * same precision and human-readable unit scaling as the iPhone. Canonical
     * telemetry remains unchanged; for example rail pressure stays kPa in the
     * decoded sample/export while the UI may render a large value in MPa.
     */
    if (mblink_parameter_from_obd2(sample, 0U, &parameter) &&
        mblink_parameter_format_sample(&parameter, buffer, capacity)) {
        return;
    }

    /* Preserve a generic fallback for typed samples not yet in the catalogue. */
    unit = link_obd2_unit_name(sample->unit);
    if (unit == NULL || unit[0] == '\0' || sample->unit == LINK_OBD2_UNIT_NONE)
        (void)snprintf(buffer, capacity, "%.2f", sample->value);
    else
        (void)snprintf(buffer, capacity, "%.2f %s", sample->value, unit);
}

static void format_decoded_pid(
    const MblinkObd2DecodedPid *decoded,
    char *buffer,
    size_t capacity)
{
    size_t index;
    size_t used = 0U;

    if (buffer == NULL || capacity == 0U) return;
    buffer[0] = '\0';
    if (decoded == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }

    if (decoded->signal_count != 0U) {
        const size_t shown =
            decoded->signal_count < 2U ? decoded->signal_count : 2U;
        for (index = 0U; index < shown; ++index) {
            const MblinkObd2DecodedSignal *signal = &decoded->signals[index];
            const int written = snprintf(
                buffer + used, capacity - used,
                "%s%s %.2f%s%s",
                index == 0U ? "" : " · ",
                signal->label != NULL ? signal->label : "value",
                signal->value,
                signal->unit != NULL && signal->unit[0] != '\0' ? " " : "",
                signal->unit != NULL ? signal->unit : "");
            if (written < 0) break;
            if ((size_t)written >= capacity - used) {
                used = capacity - 1U;
                break;
            }
            used += (size_t)written;
        }
        if (decoded->signal_count > shown && used + 8U < capacity) {
            (void)snprintf(
                buffer + used, capacity - used,
                " · +%zu", decoded->signal_count - shown);
        }
        return;
    }

    if (decoded->text_available && decoded->text[0] != '\0') {
        (void)snprintf(buffer, capacity, "%s", decoded->text);
        return;
    }

    if (decoded->raw_length != 0U) {
        const int first = snprintf(buffer, capacity, "RAW");
        if (first < 0 || (size_t)first >= capacity) return;
        used = (size_t)first;
        for (index = 0U;
             index < decoded->raw_length && index < 8U &&
             used + 4U < capacity;
             ++index) {
            const int written = snprintf(
                buffer + used, capacity - used, " %02X",
                (unsigned int)decoded->raw[index]);
            if (written < 0 || (size_t)written >= capacity - used) break;
            used += (size_t)written;
        }
        if (decoded->raw_length > 8U && used + 5U < capacity)
            (void)snprintf(buffer + used, capacity - used, " …");
        return;
    }

    (void)snprintf(buffer, capacity, "Decoded");
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
        char label[64];
        char classification[192];
        char definition[320];
        const char *code = list->entries[index].code;
        const bool resolved = link_dtc_resolve(code, &knowledge);

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

static void format_fuel_economy(
    double litres_per_100km,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    if (buffer == NULL || capacity == 0U || context == NULL) return;
    switch (context->fuel_economy_unit) {
    case MBLINK_FUEL_ECONOMY_KM_PER_L:
        if (litres_per_100km > 0.0)
            (void)snprintf(
                buffer, capacity, "%.2f km/L", 100.0 / litres_per_100km);
        else
            (void)snprintf(buffer, capacity, "—");
        break;
    case MBLINK_FUEL_ECONOMY_MPG_US:
        if (litres_per_100km > 0.0)
            (void)snprintf(
                buffer, capacity, "%.1f mpg (US)",
                235.214583 / litres_per_100km);
        else
            (void)snprintf(buffer, capacity, "—");
        break;
    case MBLINK_FUEL_ECONOMY_MPG_IMPERIAL:
        if (litres_per_100km > 0.0)
            (void)snprintf(
                buffer, capacity, "%.1f mpg (Imp)",
                282.480936 / litres_per_100km);
        else
            (void)snprintf(buffer, capacity, "—");
        break;
    case MBLINK_FUEL_ECONOMY_L_PER_100KM:
    default:
        (void)snprintf(
            buffer, capacity, "%.1f L/100 km", litres_per_100km);
        break;
    }
}

static void format_distance(
    double kilometres,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    if (context != NULL &&
        context->distance_unit == MBLINK_DISTANCE_MILES)
        (void)snprintf(
            buffer, capacity, "%.1f mi",
            kilometres * 0.621371192237334);
    else
        (void)snprintf(buffer, capacity, "%.1f km", kilometres);
}

static void format_fuel_volume(
    double litres,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    if (context != NULL &&
        context->fuel_volume_unit == MBLINK_FUEL_VOLUME_US_GALLONS)
        (void)snprintf(
            buffer, capacity, "%.2f US gal",
            litres * 0.2641720523581484);
    else if (context != NULL &&
             context->fuel_volume_unit ==
                 MBLINK_FUEL_VOLUME_IMPERIAL_GALLONS)
        (void)snprintf(
            buffer, capacity, "%.2f Imp gal",
            litres * 0.2199692482990878);
    else
        (void)snprintf(buffer, capacity, "%.2f L", litres);
}

static void format_fuel_rate(
    double litres_per_hour,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    if (context != NULL &&
        context->fuel_rate_unit == MBLINK_FUEL_RATE_US_GAL_PER_HOUR)
        (void)snprintf(
            buffer, capacity, "%.2f US gal/h",
            litres_per_hour * 0.2641720523581484);
    else if (context != NULL &&
             context->fuel_rate_unit ==
                 MBLINK_FUEL_RATE_IMPERIAL_GAL_PER_HOUR)
        (void)snprintf(
            buffer, capacity, "%.2f Imp gal/h",
            litres_per_hour * 0.2199692482990878);
    else
        (void)snprintf(buffer, capacity, "%.2f L/h", litres_per_hour);
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
    char trip[128];
    LinkFuelEconomySource display_source = snapshot.instantaneous_available
        ? snapshot.instantaneous_source
        : (snapshot.fuel_rate_available ? snapshot.fuel_rate_source : snapshot.average_source);

    if (snapshot.instantaneous_available) {
        format_fuel_economy(
            snapshot.instantaneous_l_per_100km,
            context, instantaneous, sizeof(instantaneous));
    } else if (context->connected && !snapshot.moving) {
        (void)snprintf(
            instantaneous, sizeof(instantaneous),
            "— · stationary / awaiting speed");
    } else {
        (void)snprintf(
            instantaneous, sizeof(instantaneous),
            "Waiting for measured fuel data");
    }
    if (snapshot.average_available)
        format_fuel_economy(
            snapshot.average_l_per_100km,
            context, average, sizeof(average));
    else
        (void)snprintf(
            average, sizeof(average), "Waiting for trip distance");
    if (snapshot.fuel_rate_available)
        format_fuel_rate(
            snapshot.fuel_rate_l_per_hour,
            context, rate, sizeof(rate));
    else
        (void)snprintf(rate, sizeof(rate), "Not available");
    {
        char volume[48];
        char distance[48];
        format_fuel_volume(
            snapshot.trip_fuel_litres, context,
            volume, sizeof(volume));
        format_distance(
            snapshot.trip_distance_km, context,
            distance, sizeof(distance));
        (void)snprintf(
            trip, sizeof(trip), "%s over %s", volume, distance);
    }

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
        "obd2.diesel.rail_pressure", "obd2.fuel.tank_level",
        "obd2.dpf.bank1_delta_pressure", "obd2.aftertreatment.egt_b1s1"
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
            format_sample(&context->samples[definition->key.identifier], context, value, sizeof(value));
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
    case LINK_WORKSPACE_SETTINGS:
        append_measurement_settings(body, context);
        append_application_settings(body, context);
        break;
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
    GApplication *application = g_application_get_default();
    (void)opaque;
    if (application != NULL) g_application_quit(application);
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
    context->connected = connected;
    context->transport = *transport;
    (void)snprintf(context->adapter_identity, sizeof(context->adapter_identity), "%s",
                   connected && adapter_identity != NULL ? adapter_identity : "");
    context->native_adapter_mode =
        connected && adapter_identity != NULL &&
        strstr(adapter_identity, "Mercedes me Adapter") != NULL;
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
        memset(
            context->decoded_sample_valid, 0,
            sizeof(context->decoded_sample_valid));
        memset(context->decoded_samples, 0, sizeof(context->decoded_samples));
        link_fuel_economy_init(&context->fuel_economy);
        return;
    }

    context->diagnostic = *flow;
    context->diagnostic_valid = true;

    if (event != NULL &&
        (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE ||
         event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_STRUCTURED)) {
        const uint64_t now_ms = monotonic_ms();

        for (size_t index = 0U;
             index < event->responder_decoded.count;
             ++index) {
            const LinkObd2ResponderDecodedPid *entry =
                &event->responder_decoded.entries[index];
            if (entry->decoded.definition == NULL) continue;
            const uint8_t pid = entry->decoded.definition->pid;
            context->decoded_samples[pid] = entry->decoded;
            context->decoded_sample_valid[pid] = true;
        }

        if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE) {
            context->samples[event->sample.pid] = event->sample;
            context->sample_valid[event->sample.pid] = true;
            (void)link_fuel_economy_observe_obd2(
                &context->fuel_economy, &event->sample, now_ms);
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
    format_fuel_economy(10.0, &context, value, sizeof(value));
    if (strcmp(value, "28.2 mpg (Imp)") != 0) return false;

    context.distance_unit = MBLINK_DISTANCE_MILES;
    format_distance(100.0, &context, value, sizeof(value));
    if (strcmp(value, "62.1 mi") != 0) return false;

    context.fuel_volume_unit = MBLINK_FUEL_VOLUME_US_GALLONS;
    format_fuel_volume(10.0, &context, value, sizeof(value));
    if (strcmp(value, "2.64 US gal") != 0) return false;

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
        "MBLINK settings verified: 8 independent measurement preferences + structured SAE display\n");
    return true;
}

int main(int argc, char **argv)
{
    MblinkLinuxContext context = {0};
    MblinkC207ReplayTransport replay;
    LinkGtkShellDescriptor descriptor = {0};
    bool replay_mode = false;
    bool replay_verify = false;
    bool settings_verify = false;
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

    if (!register_project_fonts()) {
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
    descriptor.app_id = "com.github.The-First-Infiltrator.MBLINK";
    descriptor.window_title = replay_mode
        ? "MBLINK · C207 Offline Replay"
        : "MBLINK · Mercedes-Benz Diagnostics";
    descriptor.brand_name = "MBLINK";
    descriptor.brand_subtitle = replay_mode
        ? "MERCEDES-BENZ · C207 / OM651 · OFFLINE REPLAY"
        : "MERCEDES-BENZ · C207 / OM651";
    descriptor.version = mblink_version();
    descriptor.emblem_resource = "/com/github/Infiltrator-Projects/MBLINK/mblink-emblem.png";
    descriptor.css = mblink_css;
    descriptor.render_section = render_section;
    descriptor.show_about = show_about;
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
    return link_gtk_shell_run(argc, argv, &descriptor);
}
