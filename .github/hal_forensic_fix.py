#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
from pathlib import Path


def replace(path, old, new, count=1):
    p = Path(path)
    text = p.read_text()
    if old not in text:
        raise SystemExit(f"expected text not found in {path}: {old[:120]!r}")
    p.write_text(text.replace(old, new, count))


# Release identity and exact LINK source identity.
Path("VERSION").write_text("0.7.180\n")
replace(
    "include/mblink/version.h",
    '#define MBLINK_VERSION_STRING "0.7.179"',
    '#define MBLINK_VERSION_STRING "0.7.180"',
)
replace(
    "app/ios/MBLINK.xcodeproj/project.pbxproj",
    "f19eedcf5b26abde6107ee411d904f9015ef8ac1",
    "0ec3d2614c2aebd706d23655d993496acd7fc69f",
)

# Portable, shape-qualified canonical view of field-verified GS 21 30 responses.
header = Path("include/mblink/mercedes_transmission.h")
text = header.read_text()
anchor = """typedef struct MblinkMercedesTransmission2130 {
    bool oil_temperature_available;
    double oil_temperature_c;
    bool actual_gear_code_available;
    uint8_t actual_gear_code;
} MblinkMercedesTransmission2130;
"""
addition = anchor + """
/**
 * Canonical live values recoverable from a source/vehicle-corroborated
 * Mercedes GS KWP 21 30 response. The decoder qualifies the response by
 * layout/length and never treats diagnostic route 0x7E1 -> 0x7E9 as proof of
 * a particular EGS/VGS controller family.
 */
typedef struct MblinkMercedesTransmissionLive2130 {
    bool rich_layout;
    bool oil_temperature_available;
    double oil_temperature_c;
    bool actual_gear_available;
    uint8_t actual_gear_code;
    bool target_gear_available;
    uint8_t target_gear_code;
    bool selector_position_available;
    uint8_t selector_position_code;
    bool drive_program_available;
    uint8_t drive_program_code;
} MblinkMercedesTransmissionLive2130;
"""
if anchor not in text:
    raise SystemExit("transmission 2130 struct anchor missing")
text = text.replace(anchor, addition, 1)
decl = """bool mblink_mercedes_transmission_decode_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmission2130 *decoded);
"""
decl_add = decl + """
bool mblink_mercedes_transmission_decode_live_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmissionLive2130 *decoded);
"""
if decl not in text:
    raise SystemExit("transmission 2130 declaration anchor missing")
header.write_text(text.replace(decl, decl_add, 1))

source = Path("src/mercedes/transmission.c")
text = source.read_text()
marker = """bool mblink_mercedes_transmission_decode_egs51_gs218(
"""
implementation = r'''bool mblink_mercedes_transmission_decode_live_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmissionLive2130 *decoded)
{
    MblinkMercedesTransmissionLive2130 value;
    if (data == NULL || decoded == NULL) return false;
    memset(&value, 0, sizeof(value));

    /*
     * Two response shapes are corroborated for this local identifier. A
     * 24-byte DAS-compatible RLI carries the richer values; the shorter
     * public/custom-PID shape carries ATF temperature plus current gear. Do
     * not silently reinterpret an unrecognised long payload as the compact
     * layout: that would move offsets and manufacture believable values.
     */
    if (data_length >= 24U) {
        MblinkMercedesKwpRli30 rich;
        if (!mblink_mercedes_transmission_decode_kwp_rli30(
                data, data_length, &rich)) return false;
        if (rich.tcc_status > UINT8_C(6) ||
            rich.actual_gear_code > UINT8_C(14) ||
            rich.target_gear_code > UINT8_C(14)) return false;

        value.rich_layout = true;
        value.oil_temperature_available = true;
        value.oil_temperature_c = rich.atf_temperature_c;
        value.actual_gear_available = true;
        value.actual_gear_code = rich.actual_gear_code;
        value.target_gear_available = true;
        value.target_gear_code = rich.target_gear_code;
        value.selector_position_available = true;
        value.selector_position_code = rich.selector_position;
        value.drive_program_available = true;
        value.drive_program_code = rich.drive_program;
    } else {
        MblinkMercedesTransmission2130 compact;
        if (!mblink_mercedes_transmission_decode_2130(
                data, data_length, &compact)) return false;
        value.oil_temperature_available = compact.oil_temperature_available;
        value.oil_temperature_c = compact.oil_temperature_c;
        value.actual_gear_available = compact.actual_gear_code_available;
        value.actual_gear_code = compact.actual_gear_code;
    }

    *decoded = value;
    return value.oil_temperature_available || value.actual_gear_available;
}

'''
if marker not in text:
    raise SystemExit("transmission source insertion marker missing")
source.write_text(text.replace(marker, implementation + marker, 1))

# Permanent unit regression for compact/rich/malformed response selection.
tests = Path("tests/test_mercedes_transmission_core.inc")
text = tests.read_text()
test_anchor = """    CHECK(strcmp(mblink_mercedes_transmission_target_gear_name(14U), "Cancel") == 0);
    return 0;
}
"""
test_new = """    CHECK(strcmp(mblink_mercedes_transmission_target_gear_name(14U), "Cancel") == 0);

    MblinkMercedesTransmissionLive2130 live;
    CHECK(mblink_mercedes_transmission_decode_live_2130(
        data, sizeof(data), &live));
    CHECK(!live.rich_layout);
    CHECK(live.oil_temperature_available && live.oil_temperature_c == 50.0);
    CHECK(live.actual_gear_available && live.actual_gear_code == 6U);
    CHECK(!live.target_gear_available);

    uint8_t rich[24] = {0};
    rich[6] = UINT8_C(3);
    rich[7] = UINT8_C(4);
    rich[8] = UINT8_C(2);
    rich[10] = UINT8_C(0x54);
    rich[11] = UINT8_C(100);
    CHECK(mblink_mercedes_transmission_decode_live_2130(
        rich, sizeof(rich), &live));
    CHECK(live.rich_layout);
    CHECK(live.oil_temperature_c == 50.0);
    CHECK(live.actual_gear_code == 4U);
    CHECK(live.target_gear_code == 5U);
    CHECK(live.selector_position_code == 4U);
    CHECK(live.drive_program_code == 2U);

    rich[6] = UINT8_C(0xff);
    CHECK(!mblink_mercedes_transmission_decode_live_2130(
        rich, sizeof(rich), &live));
    return 0;
}
"""
if test_anchor not in text:
    raise SystemExit("transmission unit-test anchor missing")
tests.write_text(text.replace(test_anchor, test_new, 1))

# Objective-C owns the platform bridge; Swift receives decoded values only.
objc_h = Path("platform/apple/MBLinkDiagnosticsController.h")
text = objc_h.read_text()
class_anchor = """@end

@interface MBLinkMercedesModuleSnapshot : NSObject
"""
class_new = """@end

@interface MBLinkTransmissionLiveValueSnapshot : NSObject
@property(nonatomic, copy, readonly) NSString *identifier;
@property(nonatomic, readonly) uint16_t localIdentifier;
@property(nonatomic, copy, readonly) NSString *shortName;
@property(nonatomic, copy, readonly) NSString *title;
@property(nonatomic, copy, readonly) NSString *suffix;
@property(nonatomic, copy, readonly) NSString *formattedValue;
@property(nonatomic, readonly, getter=isNumericValueAvailable) BOOL numericValueAvailable;
@property(nonatomic, readonly) double numericValue;
@property(nonatomic, copy, readonly) NSString *rawHex;
@property(nonatomic, readonly, getter=isPollingEnabled) BOOL pollingEnabled;
@property(nonatomic, copy, readonly) NSString *qualityNote;
@end

@interface MBLinkMercedesModuleSnapshot : NSObject
"""
if class_anchor not in text:
    raise SystemExit("Objective-C snapshot class anchor missing")
text = text.replace(class_anchor, class_new, 1)
method_anchor = """- (NSArray<MBLinkMercedesDataSnapshot *> *)
    manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier;
"""
method_new = method_anchor + """
/**
 * Presentation-ready live GS 21 30 values decoded only by the portable
 * Mercedes transmission layer. Swift must not reinterpret raw KWP bytes.
 */
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots;
"""
if method_anchor not in text:
    raise SystemExit("Objective-C transmission method anchor missing")
objc_h.write_text(text.replace(method_anchor, method_new, 1))

objc_m = Path("platform/apple/MBLinkDiagnosticsController.m")
text = objc_m.read_text()
raw_anchor = '@property(nonatomic, copy, readwrite) NSString *rawHex;\n@property(nonatomic, readwrite, getter=isMapped) BOOL mapped;'
raw_new = '@property(nonatomic, copy, readwrite) NSString *rawHex;\n@property(nonatomic, copy, readwrite) NSData *rawData;\n@property(nonatomic, readwrite, getter=isMapped) BOOL mapped;'
if raw_anchor not in text:
    raise SystemExit("Mercedes data raw property anchor missing")
text = text.replace(raw_anchor, raw_new, 1)
module_anchor = """@implementation MBLinkMercedesDataSnapshot
@end

@interface MBLinkMercedesModuleSnapshot ()
"""
live_private = """@implementation MBLinkMercedesDataSnapshot
@end

@interface MBLinkTransmissionLiveValueSnapshot ()
@property(nonatomic, copy, readwrite) NSString *identifier;
@property(nonatomic, readwrite) uint16_t localIdentifier;
@property(nonatomic, copy, readwrite) NSString *shortName;
@property(nonatomic, copy, readwrite) NSString *title;
@property(nonatomic, copy, readwrite) NSString *suffix;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, readwrite, getter=isNumericValueAvailable) BOOL numericValueAvailable;
@property(nonatomic, readwrite) double numericValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@property(nonatomic, readwrite, getter=isPollingEnabled) BOOL pollingEnabled;
@property(nonatomic, copy, readwrite) NSString *qualityNote;
@end
@implementation MBLinkTransmissionLiveValueSnapshot
@end

@interface MBLinkMercedesModuleSnapshot ()
"""
if module_anchor not in text:
    raise SystemExit("Objective-C live snapshot insertion anchor missing")
text = text.replace(module_anchor, live_private, 1)
publish_anchor = """        snapshot.codeText = MBLinkStringFromCString(code);
        snapshot.rawHex = MBLinkStringFromCString(raw);

        double numeric = 0.0;
"""
publish_new = """        snapshot.codeText = MBLinkStringFromCString(code);
        snapshot.rawHex = MBLinkStringFromCString(raw);
        snapshot.rawData = [NSData dataWithBytes:record->data
                                         length:record->data_length];

        double numeric = 0.0;
"""
if publish_anchor not in text:
    raise SystemExit("manufacturer data publication anchor missing")
text = text.replace(publish_anchor, publish_new, 1)
method_marker = """- (BOOL)manufacturerLivePollingSupportedForModuleIdentifier:
    (NSString *)identifier
"""
live_method = r'''- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots
{
    const MblinkMercedesModuleScanEntry *module = NULL;
    NSString *moduleIdentifier = nil;
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *candidate =
            mblink_mercedes_module_scan_module_at(&_mercedesModuleScan, index);
        if (candidate == NULL || candidate->extended_id ||
            candidate->tx_can_id != UINT32_C(0x7e1) ||
            candidate->rx_can_id != UINT32_C(0x7e9) ||
            mblink_mercedes_module_scan_entry_protocol(candidate) !=
                MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) continue;
        module = candidate;
        moduleIdentifier = MBLinkMercedesModuleIdentifier(candidate);
        break;
    }
    if (module == NULL || moduleIdentifier.length == 0U) return @[];

    MBLinkMercedesDataSnapshot *rli30 = nil;
    for (MBLinkMercedesDataSnapshot *candidate in
         _manufacturerDataByModule[moduleIdentifier] ?: @[]) {
        if (candidate.service == UINT8_C(0x21) &&
            candidate.identifier == UINT16_C(0x30) &&
            candidate.rawData.length != 0U) {
            rli30 = candidate;
            break;
        }
    }
    if (rli30 == nil) return @[];

    MblinkMercedesTransmissionLive2130 decoded;
    if (!mblink_mercedes_transmission_decode_live_2130(
            (const uint8_t *)rli30.rawData.bytes,
            rli30.rawData.length, &decoded)) return @[];

    NSMutableArray<MBLinkTransmissionLiveValueSnapshot *> *values =
        [[NSMutableArray alloc] init];
    NSString *quality = decoded.rich_layout
        ? @"Mercedes GS 21 30 rich response · portable MBLINK decoder · response shape/evidence qualified; ECU family not inferred from route"
        : @"Mercedes GS 21 30 compact response · portable MBLINK decoder · response shape/evidence qualified; ECU family not inferred from route";

    if (decoded.oil_temperature_available) {
        const double display =
            [_shared displayTemperatureCelsius:decoded.oil_temperature_c];
        NSString *unit = _shared.displayTemperatureUnit;
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.oil_temperature";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"ATF";
        value.title = @"Transmission oil temperature";
        value.suffix = [@" " stringByAppendingString:unit];
        value.formattedValue = [NSString stringWithFormat:@"%.6g %@", display, unit];
        value.numericValueAvailable = YES;
        value.numericValue = display;
        value.rawHex = rli30.rawHex;
        value.pollingEnabled = YES;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.actual_gear_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.actual_gear";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"GEAR";
        value.title = @"Current gear";
        value.suffix = @"";
        value.formattedValue = MBLinkStringFromCString(
            mblink_mercedes_transmission_actual_gear_name(
                decoded.actual_gear_code));
        value.numericValueAvailable = NO;
        value.rawHex = rli30.rawHex;
        value.pollingEnabled = YES;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.target_gear_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.target_gear";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"TARGET";
        value.title = @"Target gear";
        value.suffix = @"";
        value.formattedValue = MBLinkStringFromCString(
            mblink_mercedes_transmission_target_gear_name(
                decoded.target_gear_code));
        value.numericValueAvailable = NO;
        value.rawHex = rli30.rawHex;
        value.pollingEnabled = YES;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.selector_position_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.selector_position";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"SELECT";
        value.title = @"Selector position code";
        value.suffix = @" raw";
        value.formattedValue = [NSString stringWithFormat:@"%u raw",
            (unsigned int)decoded.selector_position_code];
        value.numericValueAvailable = YES;
        value.numericValue = (double)decoded.selector_position_code;
        value.rawHex = rli30.rawHex;
        value.pollingEnabled = YES;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.drive_program_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.drive_program";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"PROGRAM";
        value.title = @"Transmission drive program code";
        value.suffix = @" raw";
        value.formattedValue = [NSString stringWithFormat:@"%u raw",
            (unsigned int)decoded.drive_program_code];
        value.numericValueAvailable = YES;
        value.numericValue = (double)decoded.drive_program_code;
        value.rawHex = rli30.rawHex;
        value.pollingEnabled = YES;
        value.qualityNote = quality;
        [values addObject:value];
    }

    return [values copy];
}

'''
if method_marker not in text:
    raise SystemExit("manufacturer polling method marker missing")
objc_m.write_text(text.replace(method_marker, live_method + method_marker, 1))

# Swift becomes presentation-only for transmission values.
swift = Path("app/ios/MBLINK/ConnectionViewModel.swift")
text = swift.read_text()
start = text.find("    private func manufacturerBytes(_ rawHex: String) -> [UInt8] {")
end_marker = "    /*\n     * The top-level Table/Graphs surfaces must never use an aggregate\n"
end = text.find(end_marker, start)
if start < 0 or end < 0:
    raise SystemExit("Swift transmission decoder block markers missing")
replacement = r'''    private func transmissionDiagnosticParameters() -> [DiagnosticParameter] {
        guard let module = diagnosticModules.first(where: {
            !$0.extendedID &&
                $0.requestCANIdentifier == 0x7E1 &&
                $0.responseCANIdentifier == 0x7E9
        }) else { return [] }

        let source = "\(module.name) · \(module.addressText)"
        return controller.transmissionLiveValueSnapshots().map { snapshot in
            let numeric = snapshot.isNumericValueAvailable
                ? snapshot.numericValue : nil
            return DiagnosticParameter(
                id: snapshot.identifier,
                protocolName: "kwp2000",
                moduleIdentifier: 0x7E1,
                parameterIdentifier: UInt32(0x2100) |
                    UInt32(snapshot.localIdentifier),
                shortName: snapshot.shortName,
                title: snapshot.title,
                suffix: snapshot.suffix,
                formattedValue: snapshot.formattedValue,
                value: numeric,
                structuredValue: numeric == nil ? snapshot.formattedValue : nil,
                rawHex: snapshot.rawHex,
                vehicleSupported: true,
                favourite: false,
                pollingEnabled: snapshot.isPollingEnabled,
                history: numeric.map {
                    manufacturerHistory(
                        id: snapshot.identifier,
                        value: $0,
                        rawHex: snapshot.rawHex)
                } ?? [],
                sourceLabel: source,
                qualityNote: snapshot.qualityNote)
        }
    }

'''
text = text[:start] + replacement + text[end:]

bad_indent = """        var responderPIDs = [String: Set<UInt8>]()
for responder in LinkVehicleProfileStandardResponders(profile) {
    let rx = responder.responderCANIdentifier
    let extended = responder.isExtendedID
    let key = String(format: "%@:%08X", extended ? "29" : "11", rx)
    let pids = responder.pids.compactMap { UInt8(exactly: $0.uintValue) }
    responderPIDs[key, default: []].formUnion(pids)
}
"""
good_indent = """        var responderPIDs = [String: Set<UInt8>]()
        for responder in LinkVehicleProfileStandardResponders(profile) {
            let rx = responder.responderCANIdentifier
            let extended = responder.isExtendedID
            let key = String(format: "%@:%08X", extended ? "29" : "11", rx)
            let pids = responder.pids.compactMap { UInt8(exactly: $0.uintValue) }
            responderPIDs[key, default: []].formUnion(pids)
        }
"""
if bad_indent not in text:
    raise SystemExit("saved responder indentation anchor missing")
text = text.replace(bad_indent, good_indent, 1)

# DEBUG-only end-to-end Apple regression hook. Release builds contain none of it.
init_anchor = """        mercedesNativeDataIdentities = loadMercedesNativeDataIdentities()
        refresh()
    }
"""
init_new = """        mercedesNativeDataIdentities = loadMercedesNativeDataIdentities()
        refresh()
#if DEBUG
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1" {
            isSimulationActive = true
            controller.startSimulated()
        }
#endif
    }
"""
if init_anchor not in text:
    raise SystemExit("Swift init CI hook anchor missing")
text = text.replace(init_anchor, init_new, 1)
ready_anchor = """        isActive = controller.isActive
        isReady = controller.isReady
        if controller.isActive, !isSimulationActive,
"""
ready_new = '''        isActive = controller.isActive
        isReady = controller.isReady
#if DEBUG
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1",
           isSimulationActive, isReady,
           let liveVIN = controller.mercedesVINText, liveVIN.count == 17 {
            let marker = "ready=true\\nvin=\\(liveVIN)\\nstatus=\\(controller.statusText)\\nprofile=\\(controller.vehicleProfileStatusText)\\n"
            if let directory = FileManager.default.urls(
                    for: .documentDirectory, in: .userDomainMask).first {
                try? marker.write(
                    to: directory.appendingPathComponent(
                        "mblink-ci-simulated-flow.ok"),
                    atomically: true, encoding: .utf8)
            }
        }
#endif
        if controller.isActive, !isSimulationActive,
'''
if ready_anchor not in text:
    raise SystemExit("Swift ready CI marker anchor missing")
text = text.replace(ready_anchor, ready_new, 1)
swift.write_text(text)

# CI permanently proves architectural boundary and actual simulated flow.
ci = Path(".github/workflows/ci.yml")
text = ci.read_text()
grep_anchor = """          grep -Fq 'vehicleProfileStatusText' app/ios/MBLINK/ConnectionViewModel.swift
"""
grep_new = grep_anchor + """          grep -Fq 'transmissionLiveValueSnapshots' app/ios/MBLINK/ConnectionViewModel.swift
          ! grep -Fq 'private func egs52GearText' app/ios/MBLINK/ConnectionViewModel.swift
          ! grep -Fq 'private func manufacturerBytes' app/ios/MBLINK/ConnectionViewModel.swift
          grep -Fq 'mblink_mercedes_transmission_decode_live_2130' src/mercedes/transmission.c
"""
if grep_anchor not in text:
    raise SystemExit("CI architecture grep anchor missing")
text = text.replace(grep_anchor, grep_new, 1)
launch_old = '''          app="$derived/Build/Products/Debug-iphonesimulator/MBLINK.app"
          test -d "$app"
          xcrun simctl install "$udid" "$app"
          launch_output="$(xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
          printf '%s\\n' "$launch_output"
          grep -Fq 'com.github.The-First-Infiltrator.MBLINK' <<<"$launch_output"
          xcrun simctl terminate "$udid" com.github.The-First-Infiltrator.MBLINK
'''
launch_new = '''          app="$derived/Build/Products/Debug-iphonesimulator/MBLINK.app"
          test -d "$app"
          xcrun simctl install "$udid" "$app"
          launch_output="$(SIMCTL_CHILD_MBLINK_CI_SIMULATED_FLOW=1 xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
          printf '%s\\n' "$launch_output"
          grep -Fq 'com.github.The-First-Infiltrator.MBLINK' <<<"$launch_output"

          data_container="$(xcrun simctl get_app_container "$udid" com.github.The-First-Infiltrator.MBLINK data)"
          marker="$data_container/Documents/mblink-ci-simulated-flow.ok"
          completed=false
          for attempt in $(seq 1 180); do
            if [[ -s "$marker" ]] && grep -Fq 'ready=true' "$marker" && grep -Eq '^vin=.{17}$' "$marker"; then
              cat "$marker"
              completed=true
              break
            fi
            sleep 1
          done
          if [[ "$completed" != true ]]; then
            echo "Simulated LINK -> MBLINK diagnostic flow did not reach ready state." >&2
            [[ -f "$marker" ]] && cat "$marker" >&2 || true
            exit 1
          fi
          xcrun simctl terminate "$udid" com.github.The-First-Infiltrator.MBLINK
'''
if launch_old not in text:
    raise SystemExit("CI simulator launch anchor missing")
text = text.replace(launch_old, launch_new, 1)
apt_old = '''          echo "::error::GitHub release was published, but the central APT repository did not advertise $APP_ID $VERSION within 12 minutes."
          exit 1'''
apt_new = '''          echo "::warning::GitHub release is published and verified, but the independently scheduled central APT catalogue has not advertised $APP_ID $VERSION yet. The APT repository continues its five-minute safety refresh independently."
          exit 0'''
if apt_old not in text:
    raise SystemExit("APT release-tail anchor missing")
ci.write_text(text.replace(apt_old, apt_new, 1))

# Documentation: remove brittle current-version prose and correct Discover baseline.
readme = Path("README.md")
lines = readme.read_text().splitlines(True)
lines = [line for line in lines if not line.startswith("**Latest UI refresh:**")]
readme.write_text("".join(lines))

arch = Path("docs/ARCHITECTURE.md")
text = arch.read_text()
old = "The current Windows Discover target provides passive CAN capture and bounded OBD inventory. Future manufacturer-aware module discovery should extend this target rather than introducing a parallel `MBLINK Reader` codebase."
new = "The current Windows Discover target provides passive CAN capture, bounded OBD inventory and the explicit Mercedes FULL SWEEP built on LINK's shared deep read-only discovery-plan interface. Further manufacturer-aware research extends this target rather than introducing a parallel `MBLINK Reader` codebase."
if old not in text:
    raise SystemExit("architecture Windows Discover stale-text anchor missing")
arch.write_text(text.replace(old, new, 1))
