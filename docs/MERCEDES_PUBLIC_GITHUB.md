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
