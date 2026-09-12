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

Unknown or unresolved modules may still appear in the module list. They must not be assigned invented semantics. Only source-backed or exact-route-proven read-only channels may be offered until better identification exists.

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

The user may explicitly enable or disable individual channels. A manual "Select all" action is acceptable, but nothing is selected automatically.

Selections are stored per VIN and, where relevant, per controller/module. Reconnecting to the same VIN restores the user's own choices. Loading a saved VIN profile offline must expose the same configuration without pretending a live vehicle is attached.

Standard OBD choices use one VIN-scoped selection because the scheduler issues
one functional Mode 01 request and retains responder-specific replies. Mercedes
choices remain VIN-and-module scoped. Correctly spelled stable identifiers are
persistent API: a migration may recognise an older misspelling, but newly
written catalogue and selection records must use the canonical identifier.

## Polling and scheduler behaviour

PID Setup is the single control for both polling and display membership.
An enabled measurement appears in Dashboard, Graphs and Table; disabling it
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

The catalogue answers: "What live-data channels can MBLINK offer for this identified module?"

Polling answers: "Which of those channels did the user actually ask MBLINK to read continuously?"

Those four questions must remain separate in code and UI.

## Evidence and safety

MBLINK is read-first and deny-by-default for write/clear operations.

No manufacturer-specific signal may receive a friendly name or engineering unit based only on a guess from a CAN address. Unknown positive identifiers remain raw until their meaning/scaling is supported by source or vehicle evidence.

Exact-route vehicle-positive evidence may justify re-reading a safe read-only identifier even when controller-family identification is incomplete, but the UI must describe the evidence honestly.

## Acceptance criteria

A release satisfies this design only when all of the following are true:

- PID Setup shows the full generic SAE catalogue first.
- Discovered/saved Mercedes modules appear below it as separate sections.
- A module's Mercedes choices come from documented controller/family knowledge, not merely its advertised SAE PIDs.
- The known transmission module exposes the supported transmission live channels described above.
- All live-data toggles are OFF on a clean first run.
- Completing module discovery causes no manufacturer live-data request.
- Enabling multiple signals from one record produces one underlying request, not duplicate requests.
- Standard selections persist by VIN, Mercedes selections persist by
  VIN/module, and both can be edited from a saved offline vehicle profile.
- Unknown modules are not assigned invented PID meanings.
- Opening PID Setup does not launch a brute-force scan.
- LINK remains the sole scheduler/transport owner for shared generic and manufacturer jobs.

If implementation code, UI text, tests, or CI guards conflict with this document, the implementation should be treated as the regression unless this document is deliberately revised first.
