# Decisions

This file records durable MBLINK architectural choices.

## ADR-001 — MBLINK is a manufacturer face, not a second diagnostics engine

**Decision.** Generic automotive mechanics remain in LINK; MBLINK owns Mercedes-specific knowledge and behaviour.

**Why.** Duplicating transports, standards or application flow would create divergent safety and protocol implementations.

**Consequence.** Product code should become thinner as LINK gains genuinely reusable capability.

## ADR-002 — The physical C207 is evidence, not scope

**Decision.** The C207 E 250 CDI / OM651 development vehicle provides concrete evidence but does not define MBLINK's product boundary.

**Why.** A diagnostic product for Mercedes-Benz must distinguish evidence gathered from one vehicle from generic Mercedes coverage.

**Consequence.** Vehicle/profile provenance is preserved and unverified applicability is not presented as universal.

## ADR-003 — Manufacturer knowledge is evidence-gated

**Decision.** Unknown DIDs, endpoints, module identities and manufacturer semantics remain raw/unknown until evidence justifies interpretation.

**Why.** Incorrect diagnostic interpretation can mislead troubleshooting and, for transmitted requests, create safety risk.

**Consequence.** Captures and public references are retained close to the definitions they support.

## ADR-004 — Decode capability and transmit authority are independent

**Decision.** A codec or definition can exist without permission to send the corresponding request.

**Why.** Read-only discovery and protocol completeness must not silently broaden vehicle actions.

**Consequence.** Safety allowlists are tested independently from codec coverage.

## ADR-005 — One exact dependency chain defines the build

**Decision.** MBLINK pins LINK; LINK pins Common. MBLINK does not carry an independent Common choice.

**Why.** One dependency chain prevents incompatible shared foundations and makes release identity reproducible.

**Consequence.** Release notes derive dependency versions from the checked-out gitlinks rather than parallel hard-coded claims.