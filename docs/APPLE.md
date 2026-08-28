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

The broad forensic 11-bit/29-bit FULL SWEEP remains a Linux/desktop function rather than an iPhone default. MBLINK preserves Mercedes evidence captured before a manufacturer-scan interruption. LINK 0.14.21 attempts a bounded prompt-safe ELM resynchronisation after an interrupted manufacturer request and resumes the standard diagnostic flow when resynchronisation succeeds; only a failed resynchronisation still requires reconnect. It also treats the captured C207 `7F 0A 22` response as an unavailable optional permanent-DTC inventory instead of aborting before Mercedes discovery, and reuses the last ATI-validated iOS peripheral before falling back to a longer bounded cold scan.

The current implementation has been exercised against real Vgate/C207 traffic, including the C207 VIN/CRD3 response shapes, UDS negative responses and response-pending followed by a positive DTC response. Deterministic fixtures preserve those shapes without publishing the vehicle's real VIN.

## CoreBluetooth provider

The LINK-owned provider discovers services and characteristics dynamically, validates candidate write/notify pairs with an ELM-style `ATI` exchange, subscribes before application traffic, respects CoreBluetooth write limits/backpressure and bounds asynchronous recovery. No Vgate GATT UUID is hard-coded as a product assumption.

The iOS target declares the normal Bluetooth privacy description but does not claim background diagnostic recording.

## Validation boundary

CI builds Debug and Release simulator configurations and an unsigned physical-device IPA. Deterministic replay proves the diagnostic state machines; CI cannot substitute for a physical iPhone/Vgate/vehicle radio session. Hardware findings should continue to become regression fixtures in LINK or MBLINK according to ownership rather than platform-specific forks.
