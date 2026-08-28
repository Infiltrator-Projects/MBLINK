<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Diagnostic Parameters

MBLINK uses a protocol-neutral C parameter contract so the Live Data, Table, Dashboard and Graphs workspaces do not need separate application models for standard OBD-II and Mercedes UDS data.

`MblinkParameterKey` identifies a value by protocol, module and protocol identifier. `MblinkParameterDefinition` supplies stable persistence identity, short/display names, unit suffix, precision and optional display clamping. `MblinkParameterSample` carries availability, timestamp and scalar value without exposing platform objects.

The shared descriptor set currently covers 27 standard OBD-II values used by the capability-gated live scheduler, including distinct throttle-valve and accelerator-pedal channels. `mblink_parameter_from_obd2()` converts a decoded `MblinkObd2Sample` transactionally into the shared model. Manufacturer-specific UDS definitions can use the same contract later without teaching SwiftUI or GTK protocol formulas.

The iPhone front end reads this descriptor catalog through the C bridge. Live Data, Table, Dashboard and Graphs iterate the catalog instead of maintaining a second Swift table of PID names, units and formatting rules. Recent values still come from the established OBD telemetry store during the migration; that compatibility path is temporary rather than a second parameter definition source.

## Parameter store

`MblinkParameterStore` is the protocol-neutral bounded state/history layer for the next migration step. It registers borrowed definitions by full `{protocol, module, identifier}` key, separately rejects duplicate stable keys, stores latest values and favourites by the full key, and retains a 1024-sample chronological ring while maintaining an independent saturating total-sample counter.

Definitions remain registered when sample history is cleared, and favourites are preserved. A sample must use the exact registered definition pointer, preventing a caller from silently changing metadata for an already-registered key. Store capacity is fixed at 256 definitions and performs no heap allocation.

Scalar rendering deliberately reuses `infiltratr_format_scalar()` from the pinned Infiltratr Common library rather than maintaining another precision, clamping and unavailable-value formatter in MBLINK. LINK keeps canonical measurement units in the sample/evidence layer and may choose a clearer presentation unit; notably standard fuel-rail pressure remains kPa internally but displays as MPa once it reaches 1000 kPa. Stable-key comparison and total-count saturation also reuse Common primitives.

This layer does not decode OBD-II or UDS and does not own polling. OBD-II formulas stay in `obd2.c`; UDS and Mercedes interpretation stay in their owning layers; the scheduler remains application diagnostic policy.


## Source and presentation policy

Language and measurement units are independent preferences. Selecting English (United States), for example, changes translated interface text but does not switch a Metric installation to US customary units. The unit profile is selected separately. Canonical telemetry and CSV evidence retain protocol-native values; conversions are presentation-only.

When both sources are available for the same physical concept, a vehicle-verified Mercedes/Delphi factory definition has presentation priority over SAE OBD-II. A manufacturer target that is only corroborated but not protocol-mapped remains visible as a target and is never polled with a guessed DID, byte layout or scaling.
