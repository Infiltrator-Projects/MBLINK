# Documentation

This directory is the canonical documentation entry point for MBLINK. The Infiltrator project family uses the same baseline document roles in every repository so readers can move between projects without relearning the structure.

## Canonical baseline

- [Architecture](ARCHITECTURE.md) — ownership, layers, dependencies and system boundaries.
- [Design](DESIGN.md) — first-principles goals, non-goals, trade-offs and failure philosophy.
- [Decisions](DECISIONS.md) — durable architectural decisions, alternatives and consequences.
- [Roadmap](ROADMAP.md) — current foundation, near-term priorities and longer-term direction.
- [Validation](VALIDATION.md) — automated, manual and environment-specific evidence boundaries.
- [Project README](../README.md) — product overview, capabilities, build/use entry point and engineering ethos.
- [Changelog](../CHANGELOG.md) — user-visible and contract-relevant change history.
- [Contributing](../CONTRIBUTING.md) — development, ownership and verification rules.
- [Security](../SECURITY.md) — vulnerability scope, reporting and response policy.

## Documentation authority

The baseline files have distinct responsibilities and should not compete as alternate sources of truth. Architecture describes where behaviour belongs; Design explains why; Roadmap describes direction; Validation records what evidence is required. Code and tests remain authoritative for executable behaviour, while immutable tags/releases identify historical source.

Specialist documents may go deeper into one subsystem, protocol, platform, research area or historical investigation. They should link back to the canonical baseline when a reader needs the wider project context.

## Specialist documentation

- docs/MERCEDES.md — specialist or historical detail retained alongside the canonical baseline.
- docs/MERCEDES_ME_WHISPER.md — specialist or historical detail retained alongside the canonical baseline.
- docs/MERCEDES-ME-DATA-IDS.md — specialist or historical detail retained alongside the canonical baseline.
- docs/DID_LAB.md — specialist or historical detail retained alongside the canonical baseline.
- docs/DISCOVER.md — specialist or historical detail retained alongside the canonical baseline.
- docs/FAULT_DIAGNOSTICS.md — specialist or historical detail retained alongside the canonical baseline.
- docs/VEHICLE_PROFILES.md — specialist or historical detail retained alongside the canonical baseline.
- docs/VEHICLE_RESEARCH.md — specialist or historical detail retained alongside the canonical baseline.
- docs/APPLE.md — specialist or historical detail retained alongside the canonical baseline.
- docs/LINUX-BLUETOOTH.md — specialist or historical detail retained alongside the canonical baseline.

## Maintenance rule

When a change moves an ownership boundary, support boundary, validation claim or major design decision, update the corresponding canonical document in the same change. Avoid copying the same status statement into several files; link to the authoritative document instead.
