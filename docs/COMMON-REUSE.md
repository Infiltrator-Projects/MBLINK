<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Infiltratr Common reuse in MBLINK

The dependency hierarchy is `Infiltratr Common -> LINK -> MBLINK`. MBLINK does not carry a second top-level Common submodule or independently choose a Common revision. It pins LINK at `src/link`, and LINK owns the exact Common revision used by the whole product build.

The committed gitlinks are authoritative. MBLINK CMake validates that the checked-out LINK commit matches the superproject gitlink when repository metadata is available, while CI verifies the complete recursive submodule state. LINK performs the corresponding Common validation beneath it. No separate MBLINK CI/CMake constant is allowed to become a competing source of truth for dependency commits.

Common owns broadly reusable portable facilities that make sense outside vehicle diagnostics. LINK owns shared automotive behaviour: transport/session machinery, ELM327, Classical CAN/CAN-FD ISO-TP, standard OBD-II, generic DTC knowledge, the product-neutral UDS catalogue/codecs, parameter/store/scheduler/telemetry runtime, diagnostic-flow orchestration, safety/evidence and the shared Windows Discover scanner. MBLINK owns only its Mercedes identity, presentation assets, definitions and genuinely Mercedes-specific diagnostic behaviour.

CMake adds LINK as MBLINK's single implementation dependency and links `LINK::Core`; LINK links `InfiltratrCommon::Portable`. The native iPhone project reaches Common through LINK's nested checkout and links the `InfiltratrCommonPortable` product rather than maintaining another Common source copy or pin. Product bridge files compile the exact pinned LINK sources needed by the native iOS target and CI verifies the shared UDS service implementation is included.

Current Common reuse includes project metadata, bounded string helpers, deterministic string comparison, strict parsing, checked/saturating arithmetic, array sizing, periodic-deadline advancement and presentation-safe scalar formatting. Generic functionality should move downward only when its contract is useful beyond vehicle diagnostics; shared automotive functionality belongs in LINK rather than being duplicated in either product repository.

The POSIX provider is not part of the portable/iPhone footprint. Linux may consume it only when a real provider requirement matches its existing contract.

MBLINK does not modify the nested Common checkout. Common changes are made and released in the Common repository, LINK deliberately advances its exact Common gitlink, and MBLINK then deliberately advances its exact LINK gitlink.
