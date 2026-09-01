// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_signal_catalog.h"

#include <string.h>

static const MblinkMercedesBackendSignalDefinition backend_signals[] = {
    {
        "mercedes.backend.engine_state", "engineState", "Engine state",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.ignition_state", "ignitionstate", "Ignition state",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.filter_particle_loading", "filterParticleLoading", "Particulate filter loading",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.adblue_level", "tankLevelAdBlue", "AdBlue level",
        MBLINK_MERCEDES_SIGNAL_INT, "ratio",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.liquid_tank_level", "tanklevelpercent", "Liquid fuel tank level",
        MBLINK_MERCEDES_SIGNAL_INT, "ratio",
        "obd2.fuel.tank_level",
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.liquid_range", "rangeliquid", "Liquid fuel range",
        MBLINK_MERCEDES_SIGNAL_INT, "distance",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.liquid_consumption_reset", "liquidconsumptionreset", "Liquid consumption since reset",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "combustion-consumption",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.liquid_consumption_start", "liquidconsumptionstart", "Liquid consumption since start",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "combustion-consumption",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.odometer", "odo", "Odometer",
        MBLINK_MERCEDES_SIGNAL_INT, "distance",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.service_interval_days", "serviceintervaldays", "Service interval days",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.service_interval_distance", "serviceintervaldistance", "Service interval distance",
        MBLINK_MERCEDES_SIGNAL_INT, "distance",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.starter_battery_state", "starterBatteryState", "Starter battery state",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_pressure_front_left", "tirepressureFrontLeft", "Tyre pressure front left",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "pressure",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_pressure_front_right", "tirepressureFrontRight", "Tyre pressure front right",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "pressure",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_pressure_rear_left", "tirepressureRearLeft", "Tyre pressure rear left",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "pressure",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_pressure_rear_right", "tirepressureRearRight", "Tyre pressure rear right",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "pressure",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_sensor_available", "tireSensorAvailable", "Tyre sensor availability",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.tire_warning_lamp", "tirewarninglamp", "Tyre warning lamp",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.warning_coolant_low", "warningcoolantlevellow", "Coolant level low warning",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.warning_brake_fluid", "warningbrakefluid", "Brake fluid warning",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.warning_brake_lining", "warningbrakeliningwear", "Brake lining wear warning",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.warning_engine_light", "warningenginelight", "Engine warning light",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        true,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.park_brake_status", "parkbrakestatus", "Park brake status",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.engine_hood_status", "engineHoodStatus", "Engine hood status",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.average_speed_reset", "averageSpeedReset", "Average speed since reset",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "speed",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.average_speed_start", "averageSpeedStart", "Average speed since start",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "speed",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.driven_time_reset", "drivenTimeReset", "Driven time since reset",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.driven_time_start", "drivenTimeStart", "Driven time since start",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.auxheat_active", "auxheatActive", "Auxiliary heater active",
        MBLINK_MERCEDES_SIGNAL_BOOL, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.vehicle_lock_state", "vehicleLockState", "Vehicle lock state",
        MBLINK_MERCEDES_SIGNAL_INT, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.position_latitude", "positionLat", "Vehicle latitude",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    },
    {
        "mercedes.backend.position_longitude", "positionLong", "Vehicle longitude",
        MBLINK_MERCEDES_SIGNAL_DOUBLE, "none",
        NULL,
        "Mercedes connected-vehicle backend; exact model/year availability varies",
        false,
        "https://github.com/mercedes-benz/MBSDK-CarKit-iOS/blob/master/MBCarKit/MBCarKit/Models/ProtoMessageKeys.swift",
        "Mercedes MBSDK backend semantic only; no ECU, request, DID or scale is asserted by this entry."
    }
};

const char *mblink_mercedes_signal_value_type_name(
    MblinkMercedesSignalValueType value_type)
{
    switch (value_type) {
    case MBLINK_MERCEDES_SIGNAL_BOOL: return "bool";
    case MBLINK_MERCEDES_SIGNAL_INT: return "int";
    case MBLINK_MERCEDES_SIGNAL_DOUBLE: return "double";
    case MBLINK_MERCEDES_SIGNAL_STRING: return "string";
    case MBLINK_MERCEDES_SIGNAL_COMPOSITE: return "composite";
    }
    return "unknown";
}

size_t mblink_mercedes_backend_signal_count(void)
{
    return sizeof(backend_signals) / sizeof(backend_signals[0]);
}

const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_at(size_t index)
{
    return index < mblink_mercedes_backend_signal_count()
        ? &backend_signals[index] : NULL;
}

const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_find_key(const char *stable_key)
{
    size_t index;
    if (stable_key == NULL || stable_key[0] == '\0') return NULL;
    for (index = 0U; index < mblink_mercedes_backend_signal_count(); ++index) {
        if (strcmp(backend_signals[index].stable_key, stable_key) == 0)
            return &backend_signals[index];
    }
    return NULL;
}

const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_find_backend_key(const char *backend_key)
{
    size_t index;
    if (backend_key == NULL || backend_key[0] == '\0') return NULL;
    for (index = 0U; index < mblink_mercedes_backend_signal_count(); ++index) {
        if (strcmp(backend_signals[index].backend_key, backend_key) == 0)
            return &backend_signals[index];
    }
    return NULL;
}
