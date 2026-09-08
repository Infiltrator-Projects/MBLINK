<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone layer

The Apple application is a native presentation/platform edge over the shared LINK diagnostic engine plus MBLINK's Mercedes-specific extension.

The authoritative user-visible connection and vehicle-profile sequence is
defined in [VEHICLE_PROFILES.md](VEHICLE_PROFILES.md). The implementation notes
here must not reorder that product contract.


## Product role: diagnostic instrument, not research laboratory

The iPhone application is intentionally not the primary exhaustive vehicle-research tool.

Linux and Windows MBLINK Discover own the heavy research workflow: passive CAN observation, OpenPort/J2534 acquisition, wide module census, bounded unknown UDS/KWP harvesting, guided operator experiments, correlation and Vehicle Research Pack export. That desktop workflow is defined in [VEHICLE_RESEARCH.md](VEHICLE_RESEARCH.md).

iPhone remains the normal diagnostic instrument: fast VIN/profile recognition, saved-route validation, standards-defined OBD diagnostics, faults/readiness/freeze-frame, known factory values, targeted refreshes, gauges/graphs and evidence export.

A first VIN may still perform the bounded module census required to establish a usable profile, but iPhone must not become the primary brute-force DID/KWP discovery environment. Its **Factory data** surface should converge on reading known or already-learned identifiers rather than carrying the full desktop research burden.

## Ownership boundary

LINK owns CoreBluetooth transport coordination, ELM327 framing/parsing, standard OBD-II sequencing, VIN/DTC/live-data flow and the generic manufacturer-extension boundary. MBLINK's Apple compatibility transport compiles LINK's shared provider rather than maintaining an independent protocol stack. `MBLinkDiagnosticsController` owns the Mercedes-specific read-only probe and the VIN-keyed module-learning pass and presents their evidence to SwiftUI.

The shared LINK query timeout includes the longer first cold `ATSP0` protocol-acquisition allowance, so MBLINK no longer carries a product-private timeout override.

## Required live behaviour

The live iPhone controller must perform this sequence:

```text
ELM initialization
  → standard VIN as the first vehicle request
  → select / create the authoritative VIN profile
  → validate saved Mercedes controller routes, or run the first-VIN identity-first census
  → adapter restore
  → standard PID capability discovery
  → stored / pending / permanent standard DTC inventory
  → readiness / freeze-frame context
  → normal live-data polling
```

The VIN/profile decision and module identification are deliberately ahead of
the broader standard OBD inventory. Immediately after adapter initialisation,
LINK reads Mode 09 VIN only. MBLINK then selects the authoritative VIN profile
and validates or learns its module map. Mode 01 supported-PID discovery and the
remaining standard OBD fault/readiness work begin only after that module stage
and adapter restoration. Their responder-specific capability evidence is then
persisted back into the same VIN profile.

On a new VIN, iPhone walks the bounded 57-target mobile plan once: the compact
47-slot Mercedes gateway lattice, source-backed exceptions and the eight
legislated OBD physical slots. Dead addresses receive only a minimal read-only
presence probe; deeper DTC and identity reads run only after a responder is
proven. It does not run the wider workstation FULL address sweep and does not
sweep manufacturer data identifiers during Connect. Later connections validate
and refresh only the saved module routes. Linux/desktop FULL keeps the broader
forensic workflow. MBLINK preserves Mercedes evidence captured before a
manufacturer-scan interruption. LINK attempts a bounded prompt-safe ELM
resynchronisation after an interrupted manufacturer request and resumes the
standard diagnostic flow when resynchronisation succeeds; only a failed
resynchronisation still requires reconnect. It also treats the captured C207
`7F 0A 22` response as an unavailable optional permanent-DTC inventory instead
of aborting before Mercedes discovery, and reuses the last ATI-validated iOS
peripheral before falling back to a longer bounded cold scan.

The current implementation has been exercised against real Vgate/C207 traffic, including the C207 VIN/CRD3 response shapes, UDS negative responses and response-pending followed by a positive DTC response. Deterministic fixtures preserve those shapes without publishing the vehicle's real VIN.

The Vehicle screen decodes the captured VIN into a structured identity model and presents separate vehicle, powertrain and build cards. Raw controller strings and protocol delimiters remain evidence data; they are not used as the customer-facing VIN layout. The displayed facts include the catalogue-backed model, chassis, body style, Baumuster, production period, engine, displacement, rated output, fuel type, assembly plant, steering configuration and production serial when those fields are available.

## CoreBluetooth provider

The LINK-owned provider discovers services and characteristics dynamically, validates candidate write/notify pairs with an ELM-style `ATI` exchange, subscribes before application traffic, respects CoreBluetooth write limits/backpressure and bounds asynchronous recovery. No Vgate GATT UUID is hard-coded as a product assumption.

The iOS target declares the normal Bluetooth privacy description but does not claim background diagnostic recording.

## Validation boundary

CI builds Debug and Release simulator configurations and an unsigned physical-device IPA. Deterministic replay proves the diagnostic state machines; CI cannot substitute for a physical iPhone/Vgate/vehicle radio session. Hardware findings should continue to become regression fixtures in LINK or MBLINK according to ownership rather than platform-specific forks.


## Polling and units

Live-data rows expose a real per-channel Poll switch. Standard choices are
persisted by stable parameter key and VIN; Mercedes choices are persisted by
VIN, module and stable parameter key. A clean installation starts with every
selectable channel OFF. Capability discovery remains comprehensive without
silently converting discovered capability into continuous BLE/ELM traffic.

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
3. responder-attributed standard SAE Mode 01 evidence or samples, when useful,
   without creating another standard PID configuration list;
4. fault memory;
5. captured ECU evidence/technical details.

PID choices are not configured here. PID Setup owns one complete vehicle-wide
Standard OBD/EOBD catalogue first, then the documented Mercedes catalogue for
each identified module. All of those choices begin OFF and become wire requests
only after explicit user selection.

A responding ECU with no live values no longer presents a dead-end explanation:
the same screen offers factory-data discovery directly. Unknown positive
manufacturer identifiers remain RAW until independently mapped, and repeat
refreshes use the VIN-persisted positive identifier set rather than repeating
the full bounded discovery range.

## Evidence export continuity

Preparing a CSV is a snapshot operation, not a disconnect operation. The Apple controller copies the in-memory recorder bytes and MBLINK performs the atomic file write on a utility task, away from CoreBluetooth callbacks and LINK's 100 ms session tick. Live diagnostic polling is expected to continue while evidence is prepared.

## Fuel level

SAE PID `0x2F` Fuel Level Input is available in the complete Standard OBD
catalogue. MBLINK polls it only when the user selects it for the current VIN and
may then include it on the dashboard. The standard value is a percentage of
nominal tank capacity; a verified Mercedes/Delphi litres value remains preferred
when its factory mapping is available.


## Recoverable live-request timeouts

The 0.7.79 C207/Vgate field capture showed that a standard live request can time out while CoreBluetooth remains physically connected. LINK 0.14.25 distinguishes those states. During a live timeout the Apple controller immediately drops diagnostic readiness, requests a fresh ELM prompt, abandons only the timed-out PID after successful resynchronisation, defers that PID by one interval and resumes the existing live scheduler. A successful sample clears the consecutive-timeout counter.

Up to three consecutive live timeouts are recovered this way. Persistent failure still stops diagnostics and asks for a reconnect. This also prevents the command-centre status indicator from remaining green after the diagnostic flow has actually failed.


## Remembered opt-in polling

From MBLINK 0.7.81, live-data polling is explicit opt-in. A new installation starts with every selectable PID disabled, so capability discovery does not imply continuous live traffic. Each Poll switch is saved by stable parameter key and restored on the next launch and reconnect.

The 0.7.80 automatic nine-PID starter set is migrated carefully: an untouched legacy default becomes an empty v2 selection, while a legacy set that differs from that built-in default is treated as a user choice and preserved.

## VIN-keyed mobile census

Earlier iPhone builds intentionally stopped Mercedes module discovery at the eight legislated EOBD physical endpoints. That made the engine and secondary powertrain responder visible but structurally prevented gateway-routed body, restraint, interior and multimedia modules from being learned on the phone.

A VIN without a current compatible profile runs the bounded 57-target mobile
census once. That plan is deliberately different from the 760-target forensic
plan. Dead routes receive only the minimum read-only presence probe; proven
responders may then receive bounded identity and fault-memory reads. Responding
routes and stable identity facts are persisted in the VIN profile. An
incompatible older profile is invalidated and rebuilt instead of preserving an
incomplete topology.

The catalogue includes source-corroborated C207/W212 families for the instrument cluster, Audio 20/COMAND head unit and controller/display, ORC/SRS, left/right PRE-SAFE reversible belt tensioners, driver/passenger seat controllers, SAMs, EIS/EZS, steering-column module, climate control and other established module families. The catalogue classifies returned identities; it does not invent diagnostic addresses.
