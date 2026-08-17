<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Dependencies and Shared-Code Policy

MBLINK deliberately keeps its dependency surface small.

## Infiltratr Common

The primary internal dependency is **Infiltratr Common**, the canonical shared C foundation in `The-First-Infiltrator/Infiltrator-Libraries`.

Pinned baseline:

| Field | Value |
| --- | --- |
| Component | Infiltratr Common |
| Version | `1.5.0` |
| Tag | `v1.5.0` |
| Commit | `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb` |
| MBLINK path | `src/infiltratr-common` |
| Integration | Git submodule |

CMake verifies `VERSION` and, when Git metadata is available, the exact commit.

## Initial Common footprint

MBLINK initially compiles only the portable Common sources:

- `src/core.c`
- `src/format.c`

The POSIX provider is not part of the default iOS/portable MBLINK core. Platform-provider code is selected only where its API contract is appropriate to the target.

## Reuse before invention

Before adding a MBLINK helper, check whether Infiltratr Common already supplies the required contract.

Common currently provides useful foundations for:

- bounded string copies and trimming;
- deterministic string comparisons;
- strict integer/decimal parsing and range checks;
- checked and saturating unsigned arithmetic;
- percentage/rate calculations;
- finite-value clamping;
- generic scalar/quantity formatting;
- Celsius and percentage formatting;
- stable project metadata;
- portable compiler annotations.

MBLINK should call these APIs directly where their semantics match the vehicle-diagnostics requirement.

## What stays in MBLINK

The following are application/domain code and should not be pushed into Common simply for convenience:

- ELM327 command parsing;
- OBD-II service/PID definitions;
- DTC interpretation;
- ISO-TP;
- UDS;
- vehicle ECU addressing;
- Mercedes-Benz data identifiers/decoders;
- diagnostic polling/session policy;
- adapter profile behaviour;
- vehicle-session logging semantics.

## Promoting code to Common

A MBLINK helper becomes a candidate for Infiltratr Common only when it satisfies the shared library's existing real-consumer policy, for example when another maintained consumer needs substantially the same capability or MBLINK can immediately replace duplicated application-private implementations with a canonical shared operation.

Common is a reusable foundation, not a dumping ground for speculative utility functions.

## Apple frameworks

The iPhone front end necessarily depends on Apple SDK frameworks. These are isolated from `libmblink` behind the platform boundary.

- CoreBluetooth: Apple BLE provider only.
- SwiftUI: iPhone presentation layer only.

Neither framework may become a dependency of the portable C diagnostic library.

## Dependency updates

An Infiltratr Common update should be a deliberate change with:

1. version and commit pin updated together;
2. Common release notes reviewed;
3. MBLINK portable build and tests passing;
4. any new Common API usage documented where it materially changes the architecture;
5. no application-local fork of Common sources.
