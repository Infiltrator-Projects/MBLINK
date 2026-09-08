<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Vehicle profiles and connection state machine

This document owns MBLINK's normal iPhone vehicle-selection, saved-profile and adapter-selection behaviour. It records the intended product contract separately from the lower-level diagnostic protocol implementation.

## Core invariant

**Saved vehicle state controls MBLINK while offline. Live VIN controls MBLINK once connected.**

An adapter identifier is only a connection convenience. It is never vehicle identity. Simulation is test data and must never replace or rewrite the current real vehicle profile.

## Canonical start-to-finish iPhone flow

This is the required normal workflow. It begins with the user's first action;
later diagnostic and screen-layout documents must not silently reorder it.

1. The user taps **Connect**.
2. MBLINK opens the connection chooser. It does not initialise an adapter
   before the user has selected a connection source.
3. The user chooses one exact nearby/saved adapter, explicit automatic
   discovery, or the clearly separated test adapter.
4. For a physical selection, LINK connects to and initialises only that
   selected adapter. For the test adapter, LINK starts the deterministic test
   data source instead of opening a physical radio connection.
5. MBLINK requests the live Mode 09 VIN. This is the only standard OBD
   inventory work performed at this point: supported-PID discovery, standard
   fault inventory and live PID reads have not started yet.
6. The valid live VIN becomes authoritative. MBLINK loads its existing profile
   or creates a new profile if that VIN has never been seen.
7. For an existing valid profile, MBLINK validates and refreshes only its saved
   Mercedes module routes. For a new or invalidated profile, MBLINK runs the
   bounded identity-first mobile module census once. This produces the module
   map before standard OBD PID capability discovery begins.
8. MBLINK restores the adapter to the standard OBD channel.
9. LINK completes the standards-defined OBD inventory: responder-attributed
   supported-PID bitmaps, stored/pending/permanent faults, readiness and
   supported freeze-frame context. Reading a supported-PID bitmap identifies
   capability; it does not read or begin polling every live PID.
10. MBLINK is now ready with one authoritative VIN and one identified module
    map. No manufacturer live-data record or arbitrary data identifier has
    been swept as part of reaching this state.
11. PID Setup presents one complete vehicle-wide Standard OBD/EOBD catalogue
    first, followed by one Mercedes catalogue section for each module discovered
    or restored from the VIN profile.
12. Normal live polling begins only for channels the user explicitly selected
    for this VIN. No live channel is silently enabled on a clean installation.

The simulated source must exercise the same application-level VIN, profile,
fault, module and presentation rules using deterministic test responses. It
must not change the remembered real vehicle or create a real adapter-to-VIN
association.

In short, the diagnostic order is **VIN -> module identification -> standard
OBD PID capability/fault inventory -> selected live polling**. The fact that
Mode 09 carries the VIN must never be used to move Mode 01 PID discovery ahead
of module identification.

### Required gates

| Gate | Work permitted after the gate | Work still forbidden |
| --- | --- | --- |
| User selected a source | Initialise that physical adapter, or start deterministic test data | Initialising an unselected adapter |
| Valid live VIN captured | Select or create the authoritative vehicle profile | Applying another VIN's module or PID choices |
| Module identification complete | Restore standard OBD and discover supported-PID bitmaps/fault context | Manufacturer actual-value sweeps or live polling |
| User selected live channels | Poll only the records needed by those selections | Automatically enabling a discovered module or PID |

### What automatic module identification is

The product action is **identify modules**. Its implementation is a bounded,
precompiled mobile census that asks plausible Mercedes physical routes whether
a controller is present. The current plan contains 57 targets: the compact
47-slot Mercedes gateway lattice, source-backed exceptions and the eight
legislated OBD physical slots. A dead route receives only the minimum read-only
presence probe. Identity metadata and fault memory are read only after a
responder has been proven.

The census answers **which modules are fitted and what they can be identified
as**. Standard OBD responders and bounded Mercedes route responses are evidence
for that one module map. The census is not a factory-data scan, a live-PID scan
or continuous polling.

The normal Connect flow must never automatically:

- run the workstation FULL forensic address sweep;
- scan every possible CAN address;
- sweep every possible UDS DID or KWP local identifier within each module;
- issue a manufacturer live-data request merely because a module was found;
- treat module discovery responses as the module's selectable data catalogue;
- invent names, units or scaling for unknown positive data;
- enable all standard or Mercedes live channels; or
- send coding, programming, reset, security-access, write or clear commands.

Broad manufacturer-data discovery is a separate, explicit operation. Heavy
unknown-vehicle research belongs to MBLINK Discover on Linux/Windows, not the
normal iPhone connection path.

In product requirements and user-facing text, call this step **module
identification**. Reserve **FULL sweep**, **factory-data discovery** and **PID
polling** for their distinct operations; none is a synonym for identifying the
fitted modules.

## Offline startup

1. Read the remembered current vehicle VIN from local storage.
2. If there is no remembered VIN, or the remembered VIN no longer has a valid saved profile, start with no vehicle loaded. Do not invent a VIN and do not select the newest unrelated saved vehicle. The UI should say that no vehicle is loaded and ask the user to connect or choose a saved vehicle.
3. If the remembered VIN has a saved profile, load that profile immediately without requiring a physical connection.
4. Reconstruct the locally derivable vehicle identity from the saved VIN and expose the saved controller inventory, ECU identity/part/software/hardware data, responder-specific PID capability map and saved PID selections offline.
5. Offline data must remain clearly labelled as saved/disconnected. Current live values or current fault state must not be implied merely because a saved vehicle is loaded.

The loaded vehicle remains current until a different real VIN is observed or the user deliberately loads another saved vehicle while offline.

## Connect action and adapter selection

Every normal iPhone **Connect** entry point opens the MBLINK connection chooser before starting a physical session. The chooser can offer:

- the adapter previously associated with the currently loaded vehicle;
- nearby CoreBluetooth devices, ordered primarily by signal strength;
- LINK's existing automatic adapter discovery; and
- a clearly separated simulated ELM327 test source.

Choosing a specific Bluetooth peripheral is strict: LINK is asked to connect to that exact CoreBluetooth identifier and must not silently substitute another nearby adapter. Automatic discovery remains available as an explicit alternative.

A selected adapter is associated with a vehicle only after a real 17-character live VIN has been captured. If the adapter turns out to be fitted to another car, the association belongs to the VIN actually read from that car. Simulation never writes the real vehicle-to-adapter association.

## Live VIN authority

After the adapter channel is established and initialised, MBLINK must obtain the live VIN as early as the shared diagnostic flow permits. Once a valid live VIN exists, it is authoritative:

1. **Live VIN matches the loaded profile** — keep the profile, validate/refresh its saved controller map and continue.
2. **Live VIN differs and a saved profile exists for it** — stop using the previously loaded vehicle, load the matching saved profile, make it current/last-used, validate/refresh it and continue.
3. **Live VIN differs and no saved profile exists** — create a new profile for the live VIN, make it current/last-used, run the bounded first-time Mercedes controller census, save the learned profile and continue.
4. **No vehicle was loaded before connection** — the same live-VIN branching applies: load an existing matching profile or create a new one.

There is no requirement for a mismatch confirmation prompt: a physical live VIN is stronger evidence than an offline selection.

## Disconnect behaviour

Disconnecting changes connection state; it does not unload the current vehicle. The last real vehicle profile remains selected and usable offline until another live VIN is observed or the user explicitly chooses another saved vehicle.

A simulation session may temporarily display its synthetic test vehicle while active, but ending simulation returns to the unchanged real saved-vehicle selection.

## Controller census and cached reconnects

A newly learned VIN may run the wider bounded Mercedes read-only mobile census once. Subsequent connections should prefer the saved controller map and validate only those expected routes. If the cached map fails validation or materially changes, discard the invalid map and rebuild it rather than silently trusting stale topology.

Within a fresh Mercedes module census, discovery is identity-first:

```text
route/session setup
  -> ECU self-identification
  -> identity metadata
  -> module DTC pass
```

UDS identity uses the relevant F18x/F19x identifiers, including F197/F187/F188/F191. KWP2000 identity tries Daimler `1A 87` first, followed by bounded read-only `1A 86` and `1A 89` fallbacks. Identity evidence may classify a controller only when the returned data supports that classification; a CAN address alone must not manufacture an ECU family.

SAE Mode 01 capability evidence and live replies retain their physical responder
attribution. That evidence may be shown inside an individual control-unit detail
screen, but it must not create duplicated responder-specific PID Setup
catalogues. PID Setup owns one complete vehicle-wide Standard OBD/EOBD catalogue;
Mercedes module sections use only the separately documented manufacturer
catalogue appropriate to that module.

## Required implementation contract

The implementation must satisfy all of these profile/adapter state rules:

- no remembered vehicle means no vehicle is auto-selected;
- a remembered valid profile loads offline, including VIN-derived identity and saved controller/PID configuration;
- the Modules view falls back to the saved profile inventory while disconnected;
- all Connect entry points use the adapter chooser;
- saved, nearby, automatic and simulated connection sources remain distinct;
- a specifically chosen BLE peripheral is strict rather than substitutable;
- live VIN replaces an incorrect offline selection and selects/creates the correct profile;
- adapter association is written only after a real live VIN exists;
- disconnect retains the current real vehicle profile;
- simulation does not change the saved real-vehicle selection; and
- fresh Mercedes module discovery is identity-first per controller.

The authoritative startup order is the complete sequence at the beginning of
this document. Its vehicle-first diagnostic portion keeps the live VIN
authoritative before profile-specific work while retaining the complete generic
OBD path. The later PID capability pass is written back to the already-selected
VIN profile so startup ordering does not lose responder attribution.

## Validation cases

The state machine should remain covered by regression tests for at least these cases:

1. no last-used profile while offline;
2. valid last-used profile while offline;
3. connect to the same VIN;
4. connect to a different VIN that already has a saved profile;
5. connect to a new VIN with no saved profile;
6. disconnect and retain the current vehicle offline;
7. choose a specific adapter and reject substitution by another peripheral;
8. associate the adapter only after real VIN confirmation;
9. run simulation without altering the real saved vehicle or adapter mapping;
10. initialise no adapter before the user chooses a connection source;
11. use saved-route validation for an existing VIN and the bounded mobile census
    only for a new/invalidated VIN;
12. never turn the normal Connect path into a factory-data or forensic sweep;
13. show the vehicle-wide Standard OBD catalogue before Mercedes module
    catalogues; and
14. begin polling only the selections belonging to the authoritative VIN; and
15. distinguish the initial Mode 09 VIN read, later supported-PID bitmap
    discovery and still-later live PID reads as three separate stages.
