<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Infiltratr Common reuse in MBLINK

The dependency hierarchy is `Infiltratr Common -> LINK -> MBLINK`. MBLINK does not carry a second top-level Common submodule or independently choose a Common revision. It pins LINK, and LINK owns the exact Common revision used by the whole product build.

MBLINK 0.7.25 pins LINK 0.9.1 at commit `08ba2b4148836df1c9d584456f280ec131ae6778`. LINK 0.9.1 in turn pins Infiltratr Common 1.11.0 at exact release/main commit `6c1a6c239e51dcf7946b6303a9bad639e8455a17`. MBLINK CI validates the LINK version and gitlink and also validates the nested Common version, so dependency drift fails explicitly.

Common owns broadly reusable portable facilities that make sense outside vehicle diagnostics. LINK owns shared automotive behaviour: transport/session machinery, ELM327, ISO-TP, standard OBD-II, UDS, parameter/store/scheduler/telemetry runtime, diagnostic-flow orchestration, safety/evidence and the shared Windows Discover scanner. MBLINK owns only its Mercedes identity, presentation assets, definitions and genuinely Mercedes-specific diagnostic behaviour.

CMake adds LINK as MBLINK's single implementation dependency and links `LINK::Core`; LINK links `InfiltratrCommon::Portable`. The native iPhone project reaches Common through LINK's nested checkout and links the `InfiltratrCommonPortable` product rather than maintaining another Common source copy or pin.

Current Common reuse includes project metadata, bounded string helpers, deterministic string comparison, strict parsing, checked/saturating arithmetic, array sizing, periodic-deadline advancement and presentation-safe scalar formatting. Generic functionality should move downward only when its contract is useful beyond vehicle diagnostics; shared automotive functionality belongs in LINK rather than being duplicated in either product repository.

The POSIX provider is not part of the portable/iPhone footprint. Linux may consume it only when a real provider requirement matches its existing contract.

MBLINK does not modify the nested Common checkout. Common changes are made and released in the Common repository, LINK deliberately advances its exact Common gitlink, and MBLINK then deliberately advances its exact LINK gitlink.
