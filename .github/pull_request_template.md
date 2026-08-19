## Summary

Describe what this pull request changes and why.

## Scope

List the user-visible, protocol, API, build, or documentation changes. Keep unrelated work out of the same pull request.

## Validation

Describe the tests performed and include the exact commands, CI jobs, hardware, adapter, or vehicle evidence where relevant.

## Architecture and safety checks

- [ ] Portable diagnostic behaviour remains in C unless there is a documented platform-only reason.
- [ ] UDS consumes complete ISO-TP PDUs and does not duplicate transport segmentation.
- [ ] Platform code does not duplicate ELM327, OBD-II, ISO-TP, UDS, scheduler, or telemetry logic owned by the C core.
- [ ] Manufacturer-specific definitions include provenance and are not represented as vehicle-verified without real evidence.
- [ ] Infiltratr Common was not modified by this pull request.
- [ ] New or changed public behaviour has deterministic tests where practical.
- [ ] No credentials, VINs, registration details, signing material, or other private data are included.

## Release impact

State whether this changes `VERSION`, Xcode version metadata, release artefacts, or compatibility. If none, write `None`.
