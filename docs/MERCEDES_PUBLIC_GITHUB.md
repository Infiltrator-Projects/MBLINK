# Mercedes-Benz public GitHub research

MBLINK is a Mercedes-Benz-wide diagnostic product. The current C207/OM651 car is a development and evidence fixture, not a boundary on what upstream Mercedes-Benz material is collected or modelled.

The machine-readable inventory is `data/mercedes/public-github-sources.json`.

The source classes are intentionally different:

- **integrate-tooling**: useful upstream code can directly support an offline tool or build-time path under its licence. `odxtools` is the main example.
- **conformance-reference**: an independent implementation is used to verify LINK behaviour rather than replacing the portable implementation.
- **semantic-evidence**: manufacturer-authored API names, types, units and states become research vocabulary, not invented DIDs.
- **architecture-reference**: useful design material informs generic LINK interfaces without importing manufacturer-specific assumptions.
- **reference-only**: code or UI may be informative but is not copied into the product, particularly where the licence or technology stack makes direct integration undesirable.

The current high-value upstream set covers `odxtools`, `odex.viewer`, `socketcan-isotp`, the iOS and Android Mercedes Mobile SDK family, Vehicle Information Service, DLT and the newer car-integrated service-mesh work.

Model applicability is evidence-driven. A Mercedes backend property such as `filterParticleLoading` proves that Mercedes used that semantic in its connected-vehicle model; it does not prove that every chassis exposes it, nor that a specific ECU/DID supplies it. Conversely, a C207 capture proves what was seen on that test vehicle and can support a vehicle-family mapping without shrinking the rest of MBLINK to C207.

## Standing external Mercedes route source

**OSUSecLab/CANHunter** — https://github.com/OSUSecLab/CANHunter — is a standing MBLINK research source, not a one-off lookup. Its `Data/CAN_Bus_Commands/Mercedes.json` corpus contains a large set of Mercedes CAN request-ID to ECU-semantic mappings recovered from companion applications and is particularly useful when a live capture proves an otherwise unnamed responder.

CANHunter evidence must retain provenance. Mercedes reuses some diagnostic IDs across model families, so an address-only match is a candidate unless model-specific Mercedes/Vediamo/Xentry/service evidence removes the ambiguity. Exact ECU-returned identity and part-number evidence always outrank a generic CANHunter semantic.

## Global documented ECU knowledge layer

MBLINK no longer treats one captured vehicle as the Mercedes database. The portable core carries a normalized global controller catalogue built from public Mercedes diagnostic evidence: 1,319 named controller profiles, 1,107 profiles with explicit CAN request/response routes, and 4,355 simple read-only 0x22/0x21/0x1A references. A physical route may map to several controller generations, so route coincidence is candidate evidence until ECU identity, part number, model-specific evidence or another stronger discriminator selects the family.

The route/read layer is normalized from the public Foxwell/Xentry-derived Mercedes SysInfors/SysEnterCmd metadata in panda-zhao/panda-zhao.github.io. Security access, routines, IO control, coding, writes and programming commands are excluded from the automatic read catalogue.

Detailed response-field semantics come from source-backed controller definitions. laravelcompany/ecudocs.com is GPL-3.0 and publishes ECU JSON definitions with request/reply templates and byte/bit field positions, scaling and enumerations. MBLINK embeds documented Daimler identity/metadata layouts and exposes their response positions through the portable C API. Controller-specific actual-value layouts are added only when a matching definition exists; otherwise the payload remains RAW instead of being guessed.

The live ECU remains the strongest evidence of what one installed software revision actually supports. Public profiles define safe candidate reads; positive replies become vehicle evidence and unsupported/negative replies are not promoted into facts.


### Route-first use of the global catalogue

A discovered ECU no longer needs a hand-written MBLINK family alias before the
global documented catalogue becomes useful. If exact ECU identity selects a
documented controller profile, MBLINK uses that narrow profile. Otherwise it
takes the de-duplicated union of safe read-only 0x22/0x21/0x1A commands from
all documented controller generations published on the exact TX/RX route and
detected protocol. Positive replies become vehicle evidence; route coincidence
alone never upgrades a controller generation to fact.

This applies to the whole generated catalogue (currently 1,319 profiles, 1,107
with explicit routes and 4,355 source read references), not only the C207
development vehicle.
