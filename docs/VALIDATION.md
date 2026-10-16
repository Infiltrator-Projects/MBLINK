# Validation

## Evidence model

Vehicle diagnostics needs several evidence layers: pure protocol/unit tests, captured-traffic replay, platform/adapter integration and physical vehicle validation. These layers complement one another but are not interchangeable.

## Automated gates

- .github/workflows/ci.yml
- .github/workflows/release.yml
- .github/workflows/apt-publication.yml

tests/ covers Mercedes module discovery, C207 captured/replayed flows, engine/data scans, DIDs, Mercedes me data, LINK facade behaviour, ELM327, ISO-TP, UDS, diagnostics and telemetry-related contracts.

## Physical/manual evidence

Physical Bluetooth/J2534 adapters, real Mercedes modules and manufacturer-session behaviour require vehicle/hardware evidence. Captured replays are valuable regression evidence but are not a substitute for every physical path.

A replay proves deterministic handling of that capture. It does not prove every adapter, ECU software version or vehicle topology. A simulator build proves source/platform integration, not physical Bluetooth/USB behaviour.

## Safety validation

Regression coverage must ensure that adding a codec, DID, module or transport cannot silently broaden transmit permissions. Failed/not-scanned/scanning/clean states remain semantically distinct.

## Release criterion

The exact source/dependency tree intended for release must pass the required CI gates. Release notes and documentation must reflect the evidence actually held for that revision.

## Qualified publication

`VERSION` is the release-intent authority. CI is the sole builder and qualifier of the source archive, generic Debian package, native source installer, Windows executable and unsigned iPhone package. Each artifact bundle records its exact source commit and SHA-256 hashes. Publication consumes only the successful push-to-`main` run, compares draft and public bytes, and never recompiles. Public tags and bytes remain immutable. The lifecycle regression checks execute even with `NDEBUG` defined.

Historical experiment commits remain reachable through one immutable archive tag because `main` requires linear history. Current typed diagnostic state, explicit connection lifetime, bounded ECU semantics and qualified artifact ownership supersede the older duplicate or one-shot mechanisms. Branch retirement requires ancestry in `main` or that reviewed archive and an exact-head deletion lease, so a concurrent new commit is preserved.
