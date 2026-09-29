# MBLINK PID and module architecture

This document is the canonical product contract for how MBLINK discovers modules, presents live-data choices, and schedules polling. It exists to prevent the UI and diagnostic implementation from drifting away from the intended design.

The complete user and connection sequence is owned by
[`VEHICLE_PROFILES.md`](VEHICLE_PROFILES.md). This document begins after the
authoritative VIN/profile and module map exist. In particular, standard PID
capability discovery occurs after module identification, and PID Setup screen
order must never be mistaken for adapter-connection order.

## Product intent

MBLINK must separate three different jobs that were previously conflated:

1. Discover which physical control units are present on the vehicle.
2. Identify/classify each discovered control unit and map it to the documented diagnostic knowledge compiled into MBLINK.
3. Let the user explicitly choose which live-data channels to poll.

Discovery decides which module sections exist. The compiled diagnostic catalogue decides which documented channels belong in those sections. Polling is driven only by the user's explicit selections.

A discovery scan must not be used as the catalogue itself.

The automatic first-VIN mobile module census is a bounded presence,
identification and fault-inventory pass. Standard supported-PID bitmap reads
record capability without polling every live value. Neither operation sweeps
manufacturer data identifiers, and opening PID Setup does not start either
operation.

## PID Setup screen

The iPhone PID Setup experience is one clear hierarchy, not a chain of generic nested tiles.

The screen order is:

1. Standard OBD / EOBD
2. One section for every discovered or VIN-profiled Mercedes module

### Standard OBD / EOBD

The first section exposes the complete supported SAE J1979 live-data catalogue compiled into LINK/MBLINK. It is not split into confusing responder-specific copies in the user interface.

Capability evidence can still be shown as metadata, but the catalogue is the catalogue. A PID is not removed from the configuration UI merely because it has not yet produced a sample in the current session.

Generic OBD definitions belong in LINK so all LINK-family products benefit from the same standards implementation.

### Mercedes module sections

After the generic OBD section, MBLINK shows each Mercedes module actually discovered on the connected vehicle, or restored from that vehicle's saved VIN profile while offline.

Each module section is populated from the narrowest documented knowledge justified by that module's evidence:

- physical TX/RX route
- diagnostic protocol
- Mercedes module definition/kind
- controller-family classification
- ECU identity
- hardware/software/part-number evidence
- exact-route positive evidence already proven on that vehicle

The UI must not populate a Mercedes module merely with SAE Mode 01 PIDs advertised by the same CAN responder. Mercedes manufacturer data is a separate diagnostic namespace and must be presented from the Mercedes catalogue.

Unknown or unresolved modules may still appear in the module list. They must not be assigned invented semantics. PID Setup does not use route guesses or vehicle captures to manufacture a catalogue: a manufacturer definition appears only when an online source documents it for the identified ECU/controller family.

## ECU definition packs

Once a controller family is resolved, callers must consume one
`MblinkMercedesEcuPack` view rather than independently joining route,
module, controller-family and PID tables in the UI.

The pack is the canonical controller-scoped diagnostic view. It carries:

- the stable controller-family key and human-facing ECU/module names
- component designation and vehicle network
- physical request/response CAN identifiers and identifier width
- the resolved UDS or KWP2000 protocol
- documented session, TesterPresent and quit commands when available
- module/controller aliases used for identification
- the controller-owned data catalogue
- each data item's service, identifier, real name, live/read-only
  classification, evidence status, provenance and decoded field metadata

A pack may also contain online-documented identity, configuration, coding,
session and DTC records. Those are diagnostic records, not PIDs, and therefore
do not appear in PID Setup. Vehicle-verified raw identifiers whose semantics are
still unknown remain explicitly `raw-observed` and also stay out of PID Setup.
Captures can corroborate an online definition; they never create a PID.

Factory Readings and PID Setup consume the same exact-ECU pack for different
purposes. Factory Readings may use the pack's documented diagnostic commands.
PID Setup contains all and only the actual-value/PID definitions that online
documentation assigns to the identified ECU/controller family.

The iPhone catalogue is documentation-driven in the same way as the Standard
OBD catalogue. A documented PID remains visible whether or not this particular
vehicle has answered it yet. Vehicle response, NO DATA, previous samples and
runtime probing never add, remove or hide catalogue entries.

The diagnostic protocol follows the same ownership rule. A physical route is
only discovery evidence because Mercedes reused request/response CAN IDs across
controller generations; some exact routes are documented with both UDS and
KWP2000 ECUs. Discovery may therefore make only the bounded protocol attempts
supported by the route catalogue. Once ECU identity resolves a controller
family with an authoritative pack, the pack's protocol and session metadata
override stale route/profile state. MBLINK must not silently fall back to a
different protocol after that point. A real response that conflicts with the
resolved pack is an identity/evidence conflict and requires re-identification,
not protocol guessing.

## Transmission example

For the Mercedes gearbox-control route 0x7E1 -> 0x7E9, MBLINK already contains source-backed KWP2000 transmission knowledge.

Where the evidence supports the canonical 0x21 0x30 actual-values record, the Transmission / GS section should expose useful user-facing channels such as:

- Transmission oil temperature
- Current gear
- Target gear
- Selector position
- Drive program

These are separate selectable signals in the UI even when they are decoded from the same underlying request/response record.

Selecting several signals that share one diagnostic record must not cause duplicate wire traffic. The scheduler should issue one 0x21 0x30 read and fan the decoded result out to all selected signals.

The same principle applies to other grouped records across Mercedes controllers.

## Selection policy

Every selectable live-data channel starts OFF by default.

This applies to both SAE and Mercedes manufacturer-specific channels.

MBLINK must not silently enable manufacturer polling merely because a module or a documented live record exists.

Existing explicit user selections may be preserved during migrations, but untouched historical automatic defaults must not be recreated.

The user may explicitly enable or disable individual channels. PID Setup is the
only control surface permitted to change that state. There are no starter,
dashboard, graph, table, Factory Readings, module-wide, or other shortcuts that
can turn live channels on or off. A bulk reset inside PID Setup may turn all
channels off, but nothing is selected automatically.

Factory Readings is manual diagnostics only. Reading, refreshing or rescanning a
module there must never change live-PID selections or create recurring polling.

Selections are stored per VIN and, where relevant, per controller/module. Reconnecting to the same VIN restores the user's own choices. Loading a saved VIN profile offline must expose the same configuration without pretending a live vehicle is attached.

Standard OBD choices use one VIN-scoped selection because the scheduler issues
one functional Mode 01 request and retains responder-specific replies. Mercedes
choices remain VIN-and-module scoped. Correctly spelled stable identifiers are
persistent API: a migration may recognise an older misspelling, but newly
written catalogue and selection records must use the canonical identifier.

## Polling and scheduler behaviour

PID Setup is the single control for both polling and display membership. The
main-screen PID Setup tile is the only navigation entry point to that control;
Vehicle, Modules, Live Data, Dashboard, Table and Graphs must never link back
into PID Setup. Manufacturer PID rows come only from the online source-backed actual-value/PID
catalogue for the specifically identified ECU/controller family. Every
documented PID is selectable on iPhone. Whether the connected vehicle responds
to it is runtime state, not catalogue membership. Route-wide unions,
response-driven candidate discovery and brute-force PID scans are not allowed
to manufacture iPhone PID definitions.
An enabled measurement appears in Dashboard, Graphs and Table;
disabling it
removes it from all three even when old samples remain. There is no separate
dashboard selection or favourite requirement. All enabled channels appear,
without a four-graph limit. Channels awaiting data remain visible with their
status. Text/raw channels show their value and an explanation when no numeric
graph can be drawn. This applies equally to SAE and Mercedes channels.

Only explicitly selected channels are polled.

Finding or identifying a module is not an implicit selection. It must not issue
a manufacturer live-data request, including a one-off `21 xx` or `22 xxxx`
actual-value read, merely because that route responded during the census.

The diagnostic scheduler must remain single-owner/serialized through LINK so generic OBD and manufacturer-specific work never compete for the adapter wire.

The scheduler should de-duplicate requests by underlying diagnostic record. Multiple selected signals decoded from one response must share one scheduled request.

A PID setup screen must never trigger a broad brute-force scan simply because the user opened it. Broad or bounded discovery is a separate explicit diagnostic operation.

## Discovery versus catalogue

A successful discovery pass answers: "What modules are here?"

Identification answers: "What controller/family is this, and what documented knowledge can MBLINK safely associate with it?"

The catalogue answers: "Which actual-value/PID definitions do the online sources document for this exact identified module?"

Selection answers: "Which documented PIDs did the user ask MBLINK to read?"

Runtime answers: "Which selected documented PIDs did this particular ECU answer, and what values did it return?"

Those four questions must remain separate in code and UI.

## Evidence and safety

MBLINK is read-first and deny-by-default for write/clear operations.

No manufacturer-specific signal may receive a friendly name, engineering unit,
or PID-catalogue membership based only on a CAN address or a positive vehicle
response. Unknown positive identifiers remain raw evidence until an online
source supplies the applicable meaning/scaling for the identified ECU.

Exact-route vehicle-positive evidence may justify retaining/re-reading a safe
raw observation for diagnostics, but it never promotes that observation into
PID Setup. Online documentation is the source of catalogue membership; captures
are corroboration.

## Acceptance criteria

A release satisfies this design only when all of the following are true:

- PID Setup shows the full generic SAE catalogue first.
- Discovered/saved Mercedes modules appear below it as separate sections.
- A module's Mercedes choices come from its resolved `MblinkMercedesEcuPack`, not merely its advertised SAE PIDs.
- The ECU pack carries the friendly name, controller key, physical lookup route, protocol/session metadata and named data items as one controller-scoped view.
- Route addresses alone never force a controller-family name or permanent protocol when the global catalogue contains multiple generations on that route.
- After controller identity resolves an authoritative pack, that pack dictates UDS/KWP2000 and session behaviour; conflicting live evidence triggers re-identification rather than silent protocol fallback.
- The known transmission module exposes the supported transmission live channels described above.
- All live-data toggles are OFF on a clean first run.
- Completing module discovery causes no manufacturer live-data request.
- Enabling multiple signals from one record produces one underlying request, not duplicate requests.
- Standard selections persist by VIN, Mercedes selections persist by
  VIN/module, and both can be edited from a saved offline vehicle profile.
- Unknown modules are not assigned invented PID meanings.
- Opening PID Setup does not launch a brute-force scan.
- PID Setup is the only UI that can enable or disable live channels.
- The main-screen PID Setup tile is the only navigation entry point to PID Setup.
- Manufacturer PID Setup contains all and only online-documented actual-value/PID definitions for the identified ECU/controller family.
- Every documented manufacturer PID is visible and selectable on iPhone even before the connected ECU has answered it.
- Identity, coding, configuration, session and DTC records never masquerade as PIDs.
- Vehicle-capture-only identifiers never become PID catalogue entries.
- A NO DATA or unsupported response changes runtime status only; it never removes a documented PID from PID Setup.
- iPhone PID Setup has no separate pollable/read-only/source-completeness gate.
- Linux/Windows deep-scanner policy may classify safe/manual/repeatable probing separately, but that scanner state never changes shared PID documentation or iPhone catalogue membership.
- Live polling reads only the documented PIDs the user selected; it never probes candidate identifiers to create new PIDs.
- Factory Readings re-reads documented commands for the identified ECU and never scans a PID range.
- No route-wide fallback catalogue is exposed when ECU identity is unresolved.
- Factory Readings read/refresh/rescan operations never alter live selections.
- No starter-reading or module-wide polling enable shortcut exists.
- LINK remains the sole scheduler/transport owner for shared generic and manufacturer jobs.

If implementation code, UI text, tests, or CI guards conflict with this document, the implementation should be treated as the regression unless this document is deliberately revised first.
