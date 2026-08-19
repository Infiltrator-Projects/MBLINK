<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Diagnostic Parameters

MBLINK uses a protocol-neutral C parameter contract so the Live Data, Table, Dashboard and Graphs workspaces do not need separate application models for standard OBD-II and Mercedes UDS data.

`MblinkParameterKey` identifies a value by protocol, module and protocol identifier. `MblinkParameterDefinition` supplies stable persistence identity, display metadata, precision and optional display clamping. `MblinkParameterSample` carries availability, timestamp and scalar value without exposing platform objects.

The first descriptor set covers the eight standard OBD-II values already polled by MBLINK. `mblink_parameter_from_obd2()` converts a decoded `MblinkObd2Sample` transactionally into the shared model. Manufacturer-specific UDS definitions can use the same contract later without teaching SwiftUI or GTK protocol formulas.

Scalar rendering deliberately reuses `infiltratr_format_scalar()` from the pinned Infiltratr Common library rather than maintaining another precision, clamping and unavailable-value formatter in MBLINK.

This layer does not decode OBD-II or UDS and does not own polling. OBD-II formulas stay in `obd2.c`; UDS and Mercedes interpretation stay in their owning layers; the scheduler remains application diagnostic policy.
