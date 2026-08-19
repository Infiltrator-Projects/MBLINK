<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Diagnostic Parameters

MBLINK uses a protocol-neutral C parameter contract so the Live Data, Table, Dashboard and Graphs workspaces do not need separate application models for standard OBD-II and Mercedes UDS data.

`MblinkParameterKey` identifies a value by protocol, module and protocol identifier. `MblinkParameterDefinition` supplies stable persistence identity, short/display names, unit suffix, precision and optional display clamping. `MblinkParameterSample` carries availability, timestamp and scalar value without exposing platform objects.

The first descriptor set covers the eight standard OBD-II values already polled by MBLINK. `mblink_parameter_from_obd2()` converts a decoded `MblinkObd2Sample` transactionally into the shared model. Manufacturer-specific UDS definitions can use the same contract later without teaching SwiftUI or GTK protocol formulas.

The iPhone front end reads this descriptor catalog through the C bridge. Live Data, Table, Dashboard and Graphs iterate the catalog instead of maintaining a second Swift table of PID names, units and formatting rules. Recent values still come from the established OBD telemetry store during the migration; that compatibility path is temporary rather than a second parameter definition source.

## Parameter store

`MblinkParameterStore` is the protocol-neutral bounded state/history layer for the next migration step. It registers borrowed definitions by full `{protocol, module, identifier}` key, separately rejects duplicate stable keys, stores latest values and favourites by the full key, and retains a 1024-sample chronological ring while maintaining an independent saturating total-sample counter.

Definitions remain registered when sample history is cleared, and favourites are preserved. A sample must use the exact registered definition pointer, preventing a caller from silently changing metadata for an already-registered key. Store capacity is fixed at 256 definitions and performs no heap allocation.

Scalar rendering deliberately reuses `infiltratr_format_scalar()` from the pinned Infiltratr Common library rather than maintaining another precision, clamping and unavailable-value formatter in MBLINK. Stable-key comparison and total-count saturation also reuse Common primitives.

This layer does not decode OBD-II or UDS and does not own polling. OBD-II formulas stay in `obd2.c`; UDS and Mercedes interpretation stay in their owning layers; the scheduler remains application diagnostic policy.
