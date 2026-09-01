<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone layer

The Apple application is a native presentation/platform edge over the shared LINK diagnostic engine plus MBLINK's Mercedes-specific extension.

## Ownership boundary

LINK owns CoreBluetooth transport coordination, ELM327 framing/parsing, standard OBD-II sequencing, VIN/DTC/live-data flow and the generic manufacturer-extension boundary. MBLINK's Apple compatibility transport compiles LINK's shared provider rather than maintaining an independent protocol stack. `MBLinkDiagnosticsController` owns the Mercedes-specific read-only probe and the VIN-keyed module-learning pass and presents their evidence to SwiftUI.

The shared LINK query timeout includes the longer first cold `ATSP0` protocol-acquisition allowance, so MBLINK no longer carries a product-private timeout override.

## Current live behaviour

The live iPhone controller now performs the sequence used by the C207 work:

```text
ELM initialization
  → standard PID capability discovery
  → standard VIN
  → stored / pending / permanent standard DTC inventory
  → Mercedes read-only engine identification
  → first-VIN Mercedes mobile census (complete 11-bit/29-bit read-only target plan)
  → adapter restore
  → normal live-data polling
```

On a new VIN, iPhone now walks the complete Mercedes-owned 0x600–0x7F7 plus 29-bit logical target plan once, but uses only a single read-only TesterPresent probe on dead addresses. Deeper DTC and identity reads run only after a responder is proven. Later connections validate and refresh only the saved module routes. Linux/desktop FULL keeps the slower fallback probes for unusually quiet ECUs. MBLINK preserves Mercedes evidence captured before a manufacturer-scan interruption. LINK 0.14.25 attempts a bounded prompt-safe ELM resynchronisation after an interrupted manufacturer request and resumes the standard diagnostic flow when resynchronisation succeeds; only a failed resynchronisation still requires reconnect. It also treats the captured C207 `7F 0A 22` response as an unavailable optional permanent-DTC inventory instead of aborting before Mercedes discovery, and reuses the last ATI-validated iOS peripheral before falling back to a longer bounded cold scan.

The current implementation has been exercised against real Vgate/C207 traffic, including the C207 VIN/CRD3 response shapes, UDS negative responses and response-pending followed by a positive DTC response. Deterministic fixtures preserve those shapes without publishing the vehicle's real VIN.

The Vehicle screen decodes the captured VIN into a structured identity model and presents separate vehicle, powertrain and build cards. Raw controller strings and protocol delimiters remain evidence data; they are not used as the customer-facing VIN layout. The displayed facts include the catalogue-backed model, chassis, body style, Baumuster, production period, engine, displacement, rated output, fuel type, assembly plant, steering configuration and production serial when those fields are available.

## CoreBluetooth provider

The LINK-owned provider discovers services and characteristics dynamically, validates candidate write/notify pairs with an ELM-style `ATI` exchange, subscribes before application traffic, respects CoreBluetooth write limits/backpressure and bounds asynchronous recovery. No Vgate GATT UUID is hard-coded as a product assumption.

The iOS target declares the normal Bluetooth privacy description but does not claim background diagnostic recording.

## Validation boundary

CI builds Debug and Release simulator configurations and an unsigned physical-device IPA. Deterministic replay proves the diagnostic state machines; CI cannot substitute for a physical iPhone/Vgate/vehicle radio session. Hardware findings should continue to become regression fixtures in LINK or MBLINK according to ownership rather than platform-specific forks.


## Polling and units

Live-data rows expose a real per-PID Poll switch. The preference is persisted by stable parameter key and applied to LINK's scheduler before the first live request after reconnect. The first-run core set is intentionally small so capability discovery can remain comprehensive without continuously loading the BLE/ELM channel with every available measurement.

Interface language and unit profile are separate settings. Metric remains the default regardless of selected English variant. US customary converts temperatures, speed, pressure and volumetric fuel rate only for presentation; diagnostic evidence remains canonical.


## Canonical control-unit screen

iPhone now has one canonical detail screen for each responding control unit.
Modules, Live Data and Faults all navigate to that same ECU surface rather than
maintaining separate module-detail experiences.

The screen order is intentionally consistent:

1. control-unit identity and physical CAN route;
2. **Factory data** — Mercedes UDS/KWP manufacturer values with a read-only
   `Scan factory data` action before discovery and a targeted
   `Refresh N factory values` action after positive identifiers are known;
3. standard SAE Mode 01 live data from that exact responder, including Poll and
   favourite controls;
4. fault memory;
5. captured ECU evidence/technical details.

A responding ECU with no live values no longer presents a dead-end explanation:
the same screen offers factory-data discovery directly. Unknown positive
manufacturer identifiers remain RAW until independently mapped, and repeat
refreshes use the VIN-persisted positive identifier set rather than repeating
the full bounded discovery range.

## Evidence export continuity

Preparing a CSV is a snapshot operation, not a disconnect operation. The Apple controller copies the in-memory recorder bytes and MBLINK performs the atomic file write on a utility task, away from CoreBluetooth callbacks and LINK's 100 ms session tick. Live diagnostic polling is expected to continue while evidence is prepared.

## Fuel level

LINK 0.14.25 adds SAE PID `0x2F` Fuel Level Input. MBLINK enables it in the bounded first-run polling set and includes it on the dashboard when the vehicle advertises the PID. The standard value is a percentage of nominal tank capacity; a verified Mercedes/Delphi litres value remains preferred when its factory mapping is available.


## Recoverable live-request timeouts

The 0.7.79 C207/Vgate field capture showed that a standard live request can time out while CoreBluetooth remains physically connected. LINK 0.14.25 distinguishes those states. During a live timeout the Apple controller immediately drops diagnostic readiness, requests a fresh ELM prompt, abandons only the timed-out PID after successful resynchronisation, defers that PID by one interval and resumes the existing live scheduler. A successful sample clears the consecutive-timeout counter.

Up to three consecutive live timeouts are recovered this way. Persistent failure still stops diagnostics and asks for a reconnect. This also prevents the command-centre status indicator from remaining green after the diagnostic flow has actually failed.


## Remembered opt-in polling

From MBLINK 0.7.81, live-data polling is explicit opt-in. A new installation starts with every selectable PID disabled, so capability discovery does not imply continuous live traffic. Each Poll switch is saved by stable parameter key and restored on the next launch and reconnect.

The 0.7.80 automatic nine-PID starter set is migrated carefully: an untouched legacy default becomes an empty v2 selection, while a legacy set that differs from that built-in default is treated as a user choice and preserved.

## VIN-keyed mobile census

Earlier iPhone builds intentionally stopped Mercedes module discovery at the eight legislated EOBD physical endpoints. That made the engine and secondary powertrain responder visible but structurally prevented gateway-routed body, restraint, interior and multimedia modules from being learned on the phone.

Vehicle-profile schema 2 fixes that design limitation. A VIN without a schema-2 profile runs the read-only mobile census once across the same complete 11-bit/29-bit target plan used by forensic discovery. Dead addresses receive only TesterPresent; proven responders then receive DTC and identity reads. Responding routes, identities, part/software/hardware identifiers and fault evidence are persisted in the VIN profile. Existing schema-1 quick-scan profiles are deliberately invalidated so they are rebuilt with the wider topology rather than preserving an incomplete two-module map.

The catalogue includes source-corroborated C207/W212 families for the instrument cluster, Audio 20/COMAND head unit and controller/display, ORC/SRS, left/right PRE-SAFE reversible belt tensioners, driver/passenger seat controllers, SAMs, EIS/EZS, steering-column module, climate control and other established module families. The catalogue classifies returned identities; it does not invent diagnostic addresses.
