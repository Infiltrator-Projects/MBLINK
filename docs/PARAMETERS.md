<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Diagnostic Parameters

MBLINK uses a protocol-neutral C parameter contract so the Live Data, Table, Dashboard and Graphs workspaces do not need separate application models for standard OBD-II and Mercedes UDS data.

`MblinkParameterKey` identifies a value by protocol, module and protocol identifier. `MblinkParameterDefinition` supplies stable persistence identity, short/display names, unit suffix, precision and optional display clamping. `MblinkParameterSample` carries availability, timestamp and scalar value without exposing platform objects.

The shared descriptor set currently covers 27 standard OBD-II values used by the capability-gated live scheduler, including distinct throttle-valve and accelerator-pedal channels. `mblink_parameter_from_obd2()` converts a decoded `MblinkObd2Sample` transactionally into the shared model. Manufacturer-specific UDS definitions can use the same contract later without teaching SwiftUI or GTK protocol formulas.

The iPhone front end reads this descriptor catalog through the C bridge. Live Data, Table, Dashboard and Graphs iterate the catalog instead of maintaining a second Swift table of PID names, units and formatting rules. Recent values still come from the established OBD telemetry store during the migration; that compatibility path is temporary rather than a second parameter definition source.

MBLINK 0.7.95 also consumes LINK's responder-attributed Mode 01 history. When a
functional request receives replies from more than one ECU, the ordinary live
screens retain their compatible preferred value while the Modules workspace
keeps every decoded value under its exact response CAN identifier. This is
important on the development C207: both `0x7E8` and `0x7E9` return RPM, speed,
load, coolant temperature, control-module voltage and accelerator-pedal D, and
their values are similar but not identical.

## Parameter store

`MblinkParameterStore` is the protocol-neutral bounded state/history layer for the next migration step. It registers borrowed definitions by full `{protocol, module, identifier}` key, separately rejects duplicate stable keys, stores latest values and favourites by the full key, and retains a 1024-sample chronological ring while maintaining an independent saturating total-sample counter.

Definitions remain registered when sample history is cleared, and favourites are preserved. A sample must use the exact registered definition pointer, preventing a caller from silently changing metadata for an already-registered key. Store capacity is fixed at 256 definitions and performs no heap allocation.

Scalar rendering deliberately reuses `infiltratr_format_scalar()` from the pinned Infiltratr Common library rather than maintaining another precision, clamping and unavailable-value formatter in MBLINK. LINK keeps canonical measurement units in the sample/evidence layer and may choose a clearer presentation unit; notably standard fuel-rail pressure remains kPa internally but displays as MPa once it reaches 1000 kPa. Stable-key comparison and total-count saturation also reuse Common primitives.

This layer does not decode OBD-II or UDS and does not own polling. OBD-II formulas stay in `obd2.c`; UDS and Mercedes interpretation stay in their owning layers; the scheduler remains application diagnostic policy.


## Source and presentation policy

Language and measurement units are independent preferences. Selecting English (United States), for example, changes translated interface text but does not switch a Metric installation to US customary units. The unit profile is selected separately. Canonical telemetry and CSV evidence retain protocol-native values; conversions are presentation-only.

When both sources are available for the same physical concept, a vehicle-verified Mercedes/Delphi factory definition has presentation priority over SAE OBD-II. A manufacturer target that is only corroborated but not protocol-mapped remains visible as a target and is never polled with a guessed DID, byte layout or scaling.

Responder attribution is not manufacturer-DID promotion. A value shown under
the `0x7E9` transmission candidate is explicitly labelled as a standard SAE
Mode 01 reply from that responder; it does not imply that a Mercedes VGS/EGS
factory DID or exact module identity has been proved.

The iPhone VIN profile now persists the observed PID set for every responder.
A module that has supplied live Mode 01 data therefore remains visible after a
later Mercedes UDS probe returns `NO DATA`, and its page can immediately show
which standard values that ECU has proved it supplies. Dashboard values are
explicitly sourced from the `0x7E8` engine responder when it is available;
module pages use their own response address and never substitute another ECU's
sample. State-dependent values such as the captured `0x2F` fuel-level report
are displayed raw with a quality note rather than silently corrected.
