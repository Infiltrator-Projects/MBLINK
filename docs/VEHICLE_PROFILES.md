<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Vehicle profiles and connection state machine

This document owns MBLINK's normal iPhone vehicle-selection, saved-profile and adapter-selection behaviour. It records the intended product contract separately from the lower-level diagnostic protocol implementation.

## Core invariant

**Saved vehicle state controls MBLINK while offline. Live VIN controls MBLINK once connected.**

An adapter identifier is only a connection convenience. It is never vehicle identity. Simulation is test data and must never replace or rewrite the current real vehicle profile.

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

SAE Mode 01 capability and live-data presentation remain responder-scoped. A PID advertised or returned by one ECU must not appear as though it belongs to every module.

## Current implementation status

The released 0.7.167 implementation satisfies the profile/adapter state rules above:

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

### Vehicle-first startup order

The normal iPhone startup sequence now follows the vehicle-first contract:

```text
adapter / ELM initialisation
  -> live Mode 09 VIN
  -> select / create the authoritative vehicle profile
  -> cached Mercedes controller validation or identity-first first-VIN census
  -> adapter restore
  -> responder-scoped SAE PID capability discovery
  -> stored / pending / permanent DTC inventory
  -> readiness / freeze-frame context
  -> normal live polling
```

This keeps the live VIN authoritative before expensive diagnostic work while retaining the complete generic OBD path. The later PID capability pass is written back to the already-selected VIN profile so reordering startup does not lose responder-specific capability data.

## Validation cases

The state machine should remain covered by regression tests for at least these cases:

1. no last-used profile while offline;
2. valid last-used profile while offline;
3. connect to the same VIN;
4. connect to a different VIN that already has a saved profile;
5. connect to a new VIN with no saved profile;
6. disconnect and retain the current vehicle offline;
7. choose a specific adapter and reject substitution by another peripheral;
8. associate the adapter only after real VIN confirmation; and
9. run simulation without altering the real saved vehicle or adapter mapping.
