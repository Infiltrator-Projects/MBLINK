#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""One-shot consumer migration after reusable MBLINK behaviour moved to LINK."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
LINK_SHA = "e8121c5507a2df4df57bd0bf8bd5f1b236e79023"


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def write(path: str, text: str) -> None:
    target = ROOT / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text, encoding="utf-8")


def replace_once(path: str, old: str, new: str) -> None:
    text = read(path)
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected one exact replacement, found {count}: {old[:120]!r}")
    write(path, text.replace(old, new, 1))


def replace_braced(path: str, start_marker: str, replacement: str) -> None:
    text = read(path)
    start = text.find(start_marker)
    if start < 0:
        raise SystemExit(f"{path}: start marker not found: {start_marker!r}")
    if text.find(start_marker, start + 1) >= 0:
        raise SystemExit(f"{path}: start marker is ambiguous: {start_marker!r}")
    opening = text.find("{", start)
    if opening < 0:
        raise SystemExit(f"{path}: opening brace not found after {start_marker!r}")
    depth = 0
    in_string = False
    escape = False
    index = opening
    while index < len(text):
        ch = text[index]
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
        else:
            if ch == '"':
                in_string = True
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    end = index + 1
                    write(path, text[:start] + replacement.rstrip() + text[end:])
                    return
        index += 1
    raise SystemExit(f"{path}: unterminated braced block for {start_marker!r}")


# Pin the exact tested shared implementation first.
subprocess.run(["git", "-C", str(ROOT / "src/link"), "fetch", "origin", LINK_SHA], check=True)
subprocess.run(["git", "-C", str(ROOT / "src/link"), "checkout", "--detach", LINK_SHA], check=True)

# ---------------------------------------------------------------------------
# Linux: keep the MBLINK-facing session_trace API, but make LINK authoritative.
# ---------------------------------------------------------------------------
SESSION_TRACE_H = r'''// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_LINUX_SESSION_TRACE_H
#define MBLINK_LINUX_SESSION_TRACE_H

#include "link/diagnostic_flow.h"
#include "link/session_trace.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MBLINK_LINUX_GRAPH_TRACE_COUNT 8U
#define MBLINK_LINUX_GRAPH_HISTORY_CAPACITY LINK_SESSION_TRACE_GRAPH_HISTORY_CAPACITY
#define MBLINK_LINUX_SESSION_LOG_CAPACITY LINK_SESSION_TRACE_LOG_CAPACITY
#define MBLINK_LINUX_SESSION_LOG_MESSAGE_CAPACITY LINK_SESSION_TRACE_LOG_MESSAGE_CAPACITY

typedef LinkSessionTrace MblinkLinuxSessionTrace;

extern const uint8_t
    mblink_linux_graph_pids[MBLINK_LINUX_GRAPH_TRACE_COUNT];

size_t mblink_linux_graph_trace_index(uint8_t pid);
void mblink_linux_trace_reset_graph(MblinkLinuxSessionTrace *trace);
void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value);
void mblink_linux_trace_format_sparkline(
    const double *history, size_t count, size_t next,
    char *output, size_t output_size);

void mblink_linux_trace_clear_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms);
void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message);
size_t mblink_linux_trace_log_ordered_slot(
    const MblinkLinuxSessionTrace *trace, size_t ordered_index);

bool mblink_linux_trace_prefer_responder(
    uint32_t candidate, bool candidate_extended,
    bool current_valid, uint32_t current, bool current_extended);
const char *mblink_linux_trace_event_text(LinkDiagnosticFlowEventKind kind);

#endif
'''

SESSION_TRACE_C = r'''// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include "link/diagnostic_request.h"

#include <string.h>

const uint8_t mblink_linux_graph_pids[MBLINK_LINUX_GRAPH_TRACE_COUNT] = {
    UINT8_C(0x0c), UINT8_C(0x0d), UINT8_C(0x05), UINT8_C(0x23),
    UINT8_C(0x2f), UINT8_C(0x11), UINT8_C(0x46), UINT8_C(0x49)
};

static void ensure_graph_configuration(MblinkLinuxSessionTrace *trace)
{
    if (trace == NULL) return;
    if (trace->graph_count == MBLINK_LINUX_GRAPH_TRACE_COUNT &&
        memcmp(trace->graph_pids, mblink_linux_graph_pids,
               MBLINK_LINUX_GRAPH_TRACE_COUNT) == 0) {
        return;
    }
    memcpy(trace->graph_pids, mblink_linux_graph_pids,
           MBLINK_LINUX_GRAPH_TRACE_COUNT);
    trace->graph_count = MBLINK_LINUX_GRAPH_TRACE_COUNT;
}

size_t mblink_linux_graph_trace_index(uint8_t pid)
{
    size_t index;
    for (index = 0U; index < MBLINK_LINUX_GRAPH_TRACE_COUNT; ++index) {
        if (mblink_linux_graph_pids[index] == pid) return index;
    }
    return MBLINK_LINUX_GRAPH_TRACE_COUNT;
}

void mblink_linux_trace_reset_graph(MblinkLinuxSessionTrace *trace)
{
    ensure_graph_configuration(trace);
    link_session_trace_reset_graph(trace);
}

void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value)
{
    ensure_graph_configuration(trace);
    link_session_trace_record_graph(trace, pid, value);
}

void mblink_linux_trace_format_sparkline(
    const double *history, size_t count, size_t next,
    char *output, size_t output_size)
{
    link_session_trace_format_sparkline(
        history, count, next, output, output_size);
}

void mblink_linux_trace_clear_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms)
{
    link_session_trace_clear_log(trace, now_ms);
}

void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message)
{
    link_session_trace_append_log(trace, now_ms, message);
}

size_t mblink_linux_trace_log_ordered_slot(
    const MblinkLinuxSessionTrace *trace, size_t ordered_index)
{
    return link_session_trace_log_ordered_slot(trace, ordered_index);
}

const char *mblink_linux_trace_event_text(LinkDiagnosticFlowEventKind kind)
{
    return link_diagnostic_flow_event_text(kind);
}

bool mblink_linux_trace_prefer_responder(
    uint32_t candidate,
    bool candidate_extended,
    bool current_valid,
    uint32_t current,
    bool current_extended)
{
    return link_diagnostic_response_route_preferred(
        candidate,
        candidate_extended,
        current_valid,
        current,
        current_extended,
        UINT32_C(0x7e8),
        false);
}
'''
write("app/linux/session_trace.h", SESSION_TRACE_H)
write("app/linux/session_trace.c", SESSION_TRACE_C)

# ---------------------------------------------------------------------------
# Linux units: preserve MBLINK settings/UI, move conversion ownership to LINK.
# ---------------------------------------------------------------------------
replace_once(
    "app/linux/main.c",
    '#include "link/fuel_economy.h"\n#include "link/workspace.h"',
    '#include "link/fuel_economy.h"\n#include "link/units.h"\n#include "link/workspace.h"')

old_unit_types = r'''typedef enum MblinkTemperatureUnit {
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
'''
new_unit_types = r'''/*
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
'''
replace_once("app/linux/main.c", old_unit_types, new_unit_types)

unit_pref_helper = r'''static void mblink_link_unit_preferences(
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

'''
replace_once(
    "app/linux/main.c",
    "static void format_sample(const LinkObd2Sample *sample,\n",
    unit_pref_helper + "static void format_sample(const LinkObd2Sample *sample,\n")

FORMAT_SAMPLE = r'''static void format_sample(const LinkObd2Sample *sample,
                          const MblinkLinuxContext *context,
                          char *buffer,
                          size_t capacity)
{
    MblinkParameterSample parameter;
    const char *unit;
    LinkUnitPreferences preferences;
    double display;
    const char *display_unit = NULL;

    if (buffer == NULL || capacity == 0U) return;
    if (sample == NULL) {
        (void)snprintf(buffer, capacity, "Waiting");
        return;
    }

    if (context != NULL) {
        mblink_link_unit_preferences(context, &preferences);
        if (link_units_convert_obd2_with_preferences(
                sample->unit, sample->value, &preferences,
                &display, &display_unit)) {
            switch (sample->unit) {
            case LINK_OBD2_UNIT_CELSIUS:
                (void)snprintf(
                    buffer, capacity, "%.1f %s", display,
                    preferences.temperature == LINK_TEMPERATURE_FAHRENHEIT
                        ? "°F" : "°C");
                return;
            case LINK_OBD2_UNIT_KPA:
                if (preferences.pressure == LINK_PRESSURE_BAR)
                    (void)snprintf(buffer, capacity, "%.2f bar", display);
                else if (preferences.pressure == LINK_PRESSURE_PSI)
                    (void)snprintf(
                        buffer, capacity,
                        sample->value < 70.0 && sample->value > -70.0
                            ? "%.2f psi" : "%.1f psi", display);
                else
                    (void)snprintf(
                        buffer, capacity,
                        sample->value < 100.0 && sample->value > -100.0
                            ? "%.2f kPa" : "%.1f kPa", display);
                return;
            case LINK_OBD2_UNIT_KMH:
                (void)snprintf(buffer, capacity, "%.1f %s", display, display_unit);
                return;
            case LINK_OBD2_UNIT_KILOMETRES:
                (void)snprintf(buffer, capacity, "%.1f %s", display, display_unit);
                return;
            case LINK_OBD2_UNIT_GRAMS_PER_SECOND:
            case LINK_OBD2_UNIT_LITRES_PER_HOUR:
                (void)snprintf(buffer, capacity, "%.2f %s", display, display_unit);
                return;
            default:
                break;
            }
        }
    }

    if (mblink_parameter_from_obd2(sample, 0U, &parameter) &&
        mblink_parameter_format_sample(&parameter, buffer, capacity)) {
        return;
    }

    unit = link_obd2_unit_name(sample->unit);
    if (unit == NULL || unit[0] == '\0' || sample->unit == LINK_OBD2_UNIT_NONE)
        (void)snprintf(buffer, capacity, "%.2f", sample->value);
    else
        (void)snprintf(buffer, capacity, "%.2f %s", sample->value, unit);
}'''
replace_braced("app/linux/main.c", "static void format_sample(", FORMAT_SAMPLE)

FORMAT_DISTANCE = r'''static void format_distance(
    double kilometres,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    double display = kilometres;
    const char *unit = "km";
    const LinkDistanceUnit preference = context != NULL
        ? (LinkDistanceUnit)context->distance_unit : LINK_DISTANCE_KM;
    if (!link_units_convert_distance(kilometres, preference, &display, &unit)) {
        display = kilometres;
        unit = "km";
    }
    (void)snprintf(buffer, capacity, "%.1f %s", display, unit);
}'''
replace_braced("app/linux/main.c", "static void format_distance(", FORMAT_DISTANCE)

FORMAT_FUEL_VOLUME = r'''static void format_fuel_volume(
    double litres,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    double display = litres;
    const char *unit = "L";
    const LinkFuelVolumeUnit preference = context != NULL
        ? (LinkFuelVolumeUnit)context->fuel_volume_unit
        : LINK_FUEL_VOLUME_LITRES;
    if (!link_units_convert_fuel_volume(litres, preference, &display, &unit)) {
        display = litres;
        unit = "L";
    }
    (void)snprintf(buffer, capacity, "%.2f %s", display, unit);
}'''
replace_braced("app/linux/main.c", "static void format_fuel_volume(", FORMAT_FUEL_VOLUME)

FORMAT_FUEL_RATE = r'''static void format_fuel_rate(
    double litres_per_hour,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    double display = litres_per_hour;
    const char *unit = "L/h";
    const LinkFuelRateUnit preference = context != NULL
        ? (LinkFuelRateUnit)context->fuel_rate_unit
        : LINK_FUEL_RATE_L_PER_HOUR;
    if (!link_units_convert_fuel_rate(
            litres_per_hour, preference, &display, &unit)) {
        display = litres_per_hour;
        unit = "L/h";
    }
    (void)snprintf(buffer, capacity, "%.2f %s", display, unit);
}'''
replace_braced("app/linux/main.c", "static void format_fuel_rate(", FORMAT_FUEL_RATE)

FORMAT_FUEL_ECONOMY = r'''static void format_fuel_economy(
    double litres_per_100km,
    const MblinkLinuxContext *context,
    char *buffer,
    size_t capacity)
{
    double display = litres_per_100km;
    const char *unit = "L/100 km";
    const LinkFuelEconomyUnit preference = context != NULL
        ? (LinkFuelEconomyUnit)context->fuel_economy_unit
        : LINK_FUEL_ECONOMY_L_PER_100KM;
    if (!link_units_convert_fuel_economy(
            litres_per_100km, preference, &display, &unit)) {
        (void)snprintf(buffer, capacity, "—");
        return;
    }
    (void)snprintf(buffer, capacity, "%.1f %s", display, unit);
}'''
replace_braced("app/linux/main.c", "static void format_fuel_economy(", FORMAT_FUEL_ECONOMY)

# ---------------------------------------------------------------------------
# Discover: Mercedes owns its map; LINK owns standards-defined target shapes.
# ---------------------------------------------------------------------------
discover = read("src/mercedes/discover_plan.c")
old_29 = r'''    {
        size_t normal_index = index - MBLINK_SWEEP_11_TARGET_COUNT;
        unsigned int diagnostic_target = (unsigned int)normal_index;
        if (diagnostic_target >= 0xf1U) ++diagnostic_target;
        target->tx_can_id =
            UINT32_C(0x18da00f1) | ((uint32_t)diagnostic_target << 8U);
        target->rx_can_id =
            UINT32_C(0x18daf100) | (uint32_t)diagnostic_target;
        target->extended_id = true;
    }
    return 1;
'''
new_29 = r'''    {
        const size_t normal_index = index - MBLINK_SWEEP_11_TARGET_COUNT;
        return link_discover_standard_uds29_target_at(
            normal_index, UINT32_C(500000), target);
    }
'''
if discover.count(old_29) != 1:
    raise SystemExit("discover_plan.c: generic 29-bit target block changed")
discover = discover.replace(old_29, new_29, 1)
old_obd = r'''    if (index < MBLINK_MOBILE_OBD_COUNT) {
        target->tx_can_id = UINT32_C(0x7e0) + (uint32_t)index;
        target->rx_can_id = UINT32_C(0x7e8) + (uint32_t)index;
        target->extended_id = false;
        return 1;
    }
'''
new_obd = r'''    if (index < MBLINK_MOBILE_OBD_COUNT) {
        return link_discover_standard_obd11_target_at(
            index, UINT32_C(500000), target);
    }
'''
if discover.count(old_obd) != 1:
    raise SystemExit("discover_plan.c: generic 11-bit OBD target block changed")
discover = discover.replace(old_obd, new_obd, 1)
write("src/mercedes/discover_plan.c", discover)

# ---------------------------------------------------------------------------
# Apple controller: use LINK's responder choice and profile capability merge.
# ---------------------------------------------------------------------------
replace_once(
    "platform/apple/MBLinkDiagnosticsController.m",
    '#import "../../src/link/platform/apple/LinkDiagnosticsController.h"\n',
    '#import "../../src/link/platform/apple/LinkDiagnosticsController.h"\n#import "link/diagnostic_request.h"\n')

PREFERRED = r'''static BOOL MBLinkStandardSnapshotPreferred(
    MBLinkStandardDataSnapshot *candidate,
    MBLinkStandardDataSnapshot *current)
{
    if (candidate == nil) return NO;
    if (current == nil) return YES;
    return link_diagnostic_response_route_preferred(
        candidate.responderCANIdentifier,
        candidate.isExtendedID,
        YES,
        current.responderCANIdentifier,
        current.isExtendedID,
        UINT32_C(0x7e8),
        false);
}'''
replace_braced(
    "platform/apple/MBLinkDiagnosticsController.m",
    "static BOOL MBLinkStandardSnapshotPreferred(", PREFERRED)

CACHED_PIDS = r'''- (NSArray<NSNumber *> *)cachedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID
{
    return LinkVehicleProfileCachedPIDs(
        _cachedVehicleProfile, responderCANIdentifier, extendedID);
}'''
replace_braced(
    "platform/apple/MBLinkDiagnosticsController.m",
    "- (NSArray<NSNumber *> *)cachedPIDsForResponderCANIdentifier:",
    CACHED_PIDS)

PERSIST_DISCOVERED = r'''- (void)persistDiscoveredCapabilities
{
    if (_shared.isSimulated || self.mercedesVINText.length == 0U) return;
    const LinkDiagnosticFlow *flow = [_shared diagnosticFlow];
    if (flow == NULL) return;
    if ([_vehicleProfileStore
            mergeStandardCapabilitiesFromDiagnosticFlow:flow
            forVIN:self.mercedesVINText]) {
        _cachedVehicleProfile = [_vehicleProfileStore
            profileForVIN:self.mercedesVINText];
    }
}'''
replace_braced(
    "platform/apple/MBLinkDiagnosticsController.m",
    "- (void)persistDiscoveredCapabilities\n", PERSIST_DISCOVERED)

PERSIST_EVENT = r'''- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event
{
    if (event == NULL || self.mercedesVINText.length == 0U) return;
    if ([_vehicleProfileStore
            mergeStandardCapabilitiesFromFlowEvent:event
            forVIN:self.mercedesVINText]) {
        _cachedVehicleProfile = [_vehicleProfileStore
            profileForVIN:self.mercedesVINText];
    }
}'''
replace_braced(
    "platform/apple/MBLinkDiagnosticsController.m",
    "- (void)persistCapabilitiesFromFlowEvent:\n", PERSIST_EVENT)

# ---------------------------------------------------------------------------
# Swift: generic diagnostic models and reusable tiles now come from LINK.
# ---------------------------------------------------------------------------
view_model = read("app/ios/MBLINK/ConnectionViewModel.swift")

def replace_swift_struct(text: str, marker: str, replacement: str) -> str:
    start = text.find(marker)
    if start < 0:
        raise SystemExit(f"ConnectionViewModel.swift: missing {marker}")
    opening = text.find("{", start)
    depth = 0
    in_string = False
    escape = False
    for index in range(opening, len(text)):
        ch = text[index]
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
        else:
            if ch == '"': in_string = True
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[:start] + replacement + text[index + 1:]
    raise SystemExit(f"ConnectionViewModel.swift: unterminated {marker}")

for marker, alias in [
    ("struct DiagnosticParameter: Identifiable", "typealias DiagnosticParameter = LinkDiagnosticParameter"),
    ("struct DiagnosticModule: Identifiable", "typealias DiagnosticModule = LinkDiagnosticModule"),
    ("struct PIDConfigurationItem: Identifiable", "typealias PIDConfigurationItem = LinkPIDConfigurationItem"),
    ("struct SavedVehicleProfileSummary: Identifiable", "typealias SavedVehicleProfileSummary = LinkSavedVehicleProfileSummary"),
    ("struct DiagnosticFault: Identifiable", "typealias DiagnosticFault = LinkDiagnosticFault"),
]:
    view_model = replace_swift_struct(view_model, marker, alias)
write("app/ios/MBLINK/ConnectionViewModel.swift", view_model)

app = read("app/ios/MBLINK/MBLINKApp.swift")

def replace_named_swift_block(text: str, marker: str, replacement: str) -> str:
    start = text.find(marker)
    if start < 0:
        raise SystemExit(f"MBLINKApp.swift: missing {marker}")
    if text.find(marker, start + 1) >= 0:
        raise SystemExit(f"MBLINKApp.swift: ambiguous {marker}")
    opening = text.find("{", start)
    depth = 0
    in_string = False
    escape = False
    for index in range(opening, len(text)):
        ch = text[index]
        if in_string:
            if escape: escape = False
            elif ch == "\\": escape = True
            elif ch == '"': in_string = False
        else:
            if ch == '"': in_string = True
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[:start] + replacement + text[index + 1:]
    raise SystemExit(f"MBLINKApp.swift: unterminated {marker}")

app = replace_named_swift_block(
    app, "private struct MBInfoRow: View",
    '''private struct MBInfoRow: View {\n    let label: String\n    let value: String\n    var monospaced = false\n\n    var body: some View {\n        LinkInfoRow(label: label, value: value, monospaced: monospaced)\n    }\n}''')
app = replace_named_swift_block(
    app, "private struct MBVehicleFact: Identifiable",
    "private typealias MBVehicleFact = LinkVehicleFact")
app = replace_named_swift_block(
    app, "private struct MBVehicleFactTile: View", "")
app = replace_named_swift_block(
    app, "private struct MBVehicleFactGrid: View",
    '''private struct MBVehicleFactGrid: View {\n    let facts: [MBVehicleFact]\n\n    var body: some View {\n        LinkVehicleFactGrid(facts: facts)\n    }\n}''')
app = replace_named_swift_block(
    app, "private struct MBMetricTile: View",
    '''private struct MBMetricTile: View {\n    let parameter: DiagnosticParameter\n\n    var body: some View {\n        LinkMetricTile(parameter: parameter)\n    }\n}''')
app = app.replace("private extension DiagnosticParameter {", "private extension LinkDiagnosticParameter {")
app = app.replace("private extension DiagnosticModule {", "private extension LinkDiagnosticModule {")
old_pid = '''    var brandPidText: String {\n        let value = String(parameterIdentifier, radix: 16, uppercase: true)\n        return "0x" + (value.count < 2 ? "0\\(value)" : value)\n    }\n\n    var brandSourceText: String {\n        protocolName.lowercased() == "obd2" ? "SAE OBD-II · \\(brandPidText)" :\n            "\\(protocolName.uppercased()) · \\(brandPidText)"\n    }'''
new_pid = '''    var brandPidText: String { pidText }\n\n    var brandSourceText: String { sourceText }'''
if old_pid not in app:
    raise SystemExit("MBLINKApp.swift: generic PID/source formatter block changed")
app = app.replace(old_pid, new_pid, 1)
write("app/ios/MBLINK/MBLINKApp.swift", app)

# ---------------------------------------------------------------------------
# Native Linux builder: consume LINK's parameterised implementation.
# ---------------------------------------------------------------------------
NATIVE_BUILDER = r'''#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(tr -d '[:space:]' < "$project_root/VERSION")"
template="$project_root/src/link/packaging/native-product-installer.sh.in"
output="${1:-$project_root/MBLINK-${version}-linux-native.run}"

[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || {
    echo "Invalid VERSION: $version" >&2
    exit 1
}
test -f "$template"
grep -qx '__LINK_NATIVE_PAYLOAD_BELOW__' "$template"
test -f "$project_root/src/link/VERSION"
test -f "$project_root/src/link/src/infiltratr-common/VERSION"

temporary="$(mktemp -d)"
cleanup() { rm -rf -- "$temporary"; }
trap cleanup EXIT
source_epoch="$(git -C "$project_root" log -1 --format=%ct 2>/dev/null || printf '0')"
payload="$temporary/source.tar.gz"
generated_header="$temporary/native-installer.sh"

python3 - "$template" "$generated_header" "$version" <<'PY'
from pathlib import Path
import sys
source = Path(sys.argv[1]).read_text(encoding="utf-8")
values = {
    "__LINK_NATIVE_PRODUCT_NAME__": "MBLINK",
    "__LINK_NATIVE_PRODUCT_SLUG__": "mblink",
    "__LINK_NATIVE_PACKAGE_NAME__": "mblink",
    "__LINK_NATIVE_VERSION__": sys.argv[3],
    "__LINK_NATIVE_CMAKE_ENABLE_OPTION__": "MBLINK_BUILD_LINUX_APP",
    "__LINK_NATIVE_CMAKE_PROFILE_OPTION__": "MBLINK_BUILD_PROFILE",
    "__LINK_NATIVE_CMAKE_PACKAGE_VERSION_OPTION__": "MBLINK_PACKAGE_VERSION",
    "__LINK_NATIVE_LEGACY_CLEANUP_PATHS__": ":".join([
        "/usr/local/bin/mblink-linux",
        "/usr/local/share/icons/hicolor/180x180/apps/mblink.png",
        "/usr/local/share/pixmaps/mblink.png",
        "/usr/local/share/applications/com.github.The-First-Infiltrator.MBLINK.desktop",
        "/usr/local/share/doc/mblink",
    ]),
}
for key, value in values.items():
    if key not in source:
        raise SystemExit(f"LINK native installer template missing {key}")
    source = source.replace(key, value)
if "__LINK_NATIVE_" in source:
    raise SystemExit("Unresolved LINK native installer placeholder")
Path(sys.argv[2]).write_text(source, encoding="utf-8")
PY

tar \
    --directory "$project_root" \
    --sort=name \
    --mtime="@$source_epoch" \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --pax-option=delete=atime,delete=ctime \
    --exclude-vcs \
    --exclude='./build' \
    --exclude='./build-*' \
    --exclude='./MBLINK-*.deb' \
    --exclude='./MBLINK-*.ipa' \
    --exclude='./MBLINK-*.run' \
    -cf - . | gzip -n -9 > "$payload"

mkdir -p -- "$(dirname "$output")"
cp "$generated_header" "$output"
cat "$payload" >> "$output"
chmod 0755 "$output"
test -x "$output"
"$output" --help >/dev/null
printf 'Created %s\n' "$output"
'''
write("scripts/build-native-installer.sh", NATIVE_BUILDER)
old_installer = ROOT / "packaging/native-installer.sh"
if old_installer.exists():
    old_installer.unlink()

print("MBLINK consumer migration source edits completed")
