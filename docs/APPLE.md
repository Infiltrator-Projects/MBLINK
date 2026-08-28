<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone layer

The Apple application is a native presentation/platform edge over the shared LINK diagnostic engine plus MBLINK's Mercedes-specific extension.

## Ownership boundary

LINK owns CoreBluetooth transport coordination, ELM327 framing/parsing, standard OBD-II sequencing, VIN/DTC/live-data flow and the generic manufacturer-extension boundary. MBLINK's Apple compatibility transport compiles LINK's shared provider rather than maintaining an independent protocol stack. `MBLinkDiagnosticsController` owns the Mercedes-specific read-only probe and eight-target EOBD quick module scan and presents their evidence to SwiftUI.

The shared LINK query timeout includes the longer first cold `ATSP0` protocol-acquisition allowance, so MBLINK no longer carries a product-private timeout override.

## Current live behaviour

The live iPhone controller now performs the sequence used by the C207 work:

```text
ELM initialization
  → standard PID capability discovery
  → standard VIN
  → stored / pending / permanent standard DTC inventory
  → Mercedes read-only engine identification
  → eight-target Mercedes EOBD quick module scan
  → adapter restore
  → normal live-data polling
```

The broad forensic 11-bit/29-bit FULL SWEEP remains a Linux/desktop function rather than an iPhone default. MBLINK preserves Mercedes evidence captured before a manufacturer-scan interruption. LINK 0.14.25 attempts a bounded prompt-safe ELM resynchronisation after an interrupted manufacturer request and resumes the standard diagnostic flow when resynchronisation succeeds; only a failed resynchronisation still requires reconnect. It also treats the captured C207 `7F 0A 22` response as an unavailable optional permanent-DTC inventory instead of aborting before Mercedes discovery, and reuses the last ATI-validated iOS peripheral before falling back to a longer bounded cold scan.

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


## Evidence export continuity

Preparing a CSV is a snapshot operation, not a disconnect operation. The Apple controller copies the in-memory recorder bytes and MBLINK performs the atomic file write on a utility task, away from CoreBluetooth callbacks and LINK's 100 ms session tick. Live diagnostic polling is expected to continue while evidence is prepared.

## Fuel level

LINK 0.14.25 adds SAE PID `0x2F` Fuel Level Input. MBLINK enables it in the bounded first-run polling set and includes it on the dashboard when the vehicle advertises the PID. The standard value is a percentage of nominal tank capacity; a verified Mercedes/Delphi litres value remains preferred when its factory mapping is available.


## Recoverable live-request timeouts

The 0.7.79 C207/Vgate field capture showed that a standard live request can time out while CoreBluetooth remains physically connected. LINK 0.14.25 distinguishes those states. During a live timeout the Apple controller immediately drops diagnostic readiness, requests a fresh ELM prompt, abandons only the timed-out PID after successful resynchronisation, defers that PID by one interval and resumes the existing live scheduler. A successful sample clears the consecutive-timeout counter.

Up to three consecutive live timeouts are recovered this way. Persistent failure still stops diagnostics and asks for a reconnect. This also prevents the command-centre status indicator from remaining green after the diagnostic flow has actually failed.
