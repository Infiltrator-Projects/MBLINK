# Validation

## Evidence model

Vehicle diagnostics needs several evidence layers: pure protocol/unit tests, captured-traffic replay, platform/adapter integration and physical vehicle validation. These layers complement one another but are not interchangeable.

## Automated gates

- .github/workflows/ci.yml
- .github/workflows/release-policy.yml
- .github/workflows/self-hosted-smoke.yml
- .github/workflows/apt-publication.yml

tests/ covers Mercedes module discovery, C207 captured/replayed flows, engine/data scans, DIDs, Mercedes me data, LINK facade behaviour, ELM327, ISO-TP, UDS, diagnostics and telemetry-related contracts.

## Physical/manual evidence

Physical Bluetooth/J2534 adapters, real Mercedes modules and manufacturer-session behaviour require vehicle/hardware evidence. Captured replays are valuable regression evidence but are not a substitute for every physical path.

A replay proves deterministic handling of that capture. It does not prove every adapter, ECU software version or vehicle topology. A simulator build proves source/platform integration, not physical Bluetooth/USB behaviour.

## Safety validation

Regression coverage must ensure that adding a codec, DID, module or transport cannot silently broaden transmit permissions. Failed/not-scanned/scanning/clean states remain semantically distinct.

## Release criterion

The exact source/dependency tree intended for release must pass the required CI gates. Release notes and documentation must reflect the evidence actually held for that revision.
