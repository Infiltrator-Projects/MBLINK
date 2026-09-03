<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. MBLINK's manufacturer scope is Mercedes-Benz broadly; individual chassis, engine and ECU-family profiles are evidence-scoped subsets within it. The current C207/OM651/CRD3 work is one populated profile, not the product boundary. The manufacturer layer owns vehicle profiles, ECU endpoint provenance and Mercedes-specific evidence/definitions; it does not duplicate transport framing or UDS response validation. Endpoint profiles use the transport-neutral ISO-TP address contract rather than an adapter-specific configuration type.

## Shared Apple session architecture

The iPhone face uses LINK 0.14.25's shared Apple diagnostic-session engine. LINK owns CoreBluetooth lifecycle, ELM327 session driving, timers, standard VIN/PID/DTC flow, live telemetry, favourites/history, CSV recording, deterministic simulation, timeout handling and reconnect behaviour. MBLINK's Apple controller is now a product adapter: it retains Mercedes VIN interpretation, read-only Mercedes UDS identity/CRD3 probing, Mercedes module inventory and Mercedes-specific presentation, and supplies those operations through LINK's manufacturer-extension callbacks.

This keeps transport/session fixes in one implementation while preserving Mercedes-specific policy in MBLINK.

## Offline Mercedes VIN / FIN / Baumuster decoder

MBLINK contains a portable offline Mercedes VIN decoder. It follows Mercedes-Benz's own FIN decomposition for the long-running numeric Baumuster format: three-character WMI, six-digit vehicle Baumuster, steering code, assembly-plant code and six-digit production serial. When positions 4-9 are not numeric, the decoder preserves the VIN as an ISO VDS-style layout rather than falsely treating it as a Mercedes Baumuster.

The decoded facts are deliberately separated:

- WMI identifies the manufacturer/legal manufacturer context and WMI country.
- The six-digit Baumuster identifies series/body/engine configuration when a catalogue entry exists.
- Steering code 1/2 decodes left/right-hand drive on the Mercedes FIN layout.
- Plant code is decoded separately from the WMI; this is the vehicle assembly location, not merely the WMI country.
- The final six characters are retained as the production serial.
- Exact ECU hardware/software remains live UDS evidence and is never inferred solely from VIN.

The first populated Baumuster catalogue covers the C207 coupe family across diesel and petrol engines, including OM651, OM642, M271, M272, M273, M274, M276 and M278 variants. Unknown six-digit types still decode structurally and remain catalogue-unknown instead of being guessed. The catalogue is data-driven so additional Mercedes series can be added without changing the VIN parser or LINK.

## Vehicle-family identification before ECU-family probing

MBLINK does not treat a chassis or badge as an engine/ECU identity. C207 is a platform family: for example, public fitment data identifies C207 E 250 CGI type 207.347 with M271.860 petrol, while C207 diesel type codes such as 207.301-207.304 map to OM651-family applications, and later petrol types such as 207.334/207.336 use M274.920.

The normal Mercedes engine probe therefore starts from a generic read-only physical engine-ECU candidate, reads VIN and standardized identity first, and selects the narrowest supported vehicle/engine profile from evidence. CRD3-only fingerprint DIDs are gated behind that selection. Known petrol or unknown engine families proceed directly from standardized identity to read-only UDS fault inventory unless their returned ECU identity independently indicates CRD3.

C207 identification now comes from the shared offline Baumuster catalogue rather than a short hard-coded list. The catalogue carries exact model, engine code, engine family, fuel, body style, displacement and source provenance. Diagnostic selection consumes only the resulting family evidence: OM651 permits the CRD3 extension; petrol M271/M274 and other non-CRD3 families do not. Unknown Baumuster values remain structurally decoded but engine-unidentified until catalogue evidence is added.

A vehicle capture from one member of a family does not promote every member to vehicle-verified status. Family profiles are source-corroborated; exact vehicle verification remains attached to reproducible evidence rather than to a model badge.

## C207 E 250 CDI / OM651 / Delphi CRD3.x profile

The OM651 family profile contains source-corroborated endpoints; it is selected only after VIN/ECU evidence supports that family:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | source-corroborated |

The ECU-family side has direct fitment evidence: the public `autodiag2/database` W207 E 250 2200 CDI record identifies Delphi `CRD3.x` for the 148 kW and 150 kW configurations. Independent public Mercedes CRD3 diagnostic material also corroborates the conventional engine request/response pair. That grounds MBLINK's C207/OM651 work in the correct ECU family and justifies source-corroborated status while still keeping vehicle verification gated on a reproducible C207 capture.

Physical hardware is a validation gate, not a development gate. MBLINK continues to build the Mercedes protocol model, fixture replay, ECU-family decoding, fault knowledge and safe read-only requests offline. When the development adapter becomes available, the real C207 capture promotes only what it actually proves rather than beginning the implementation from scratch.

## Archived Mercedes me Whisper parameterisation

The archived Mercedes me Adapter application has now yielded its active Whisper
parameterisation rather than only native-library symbols. The encrypted
`.properties` payloads were reproducibly decrypted and the active
`MSA_VIN_cascade.properties` source provides manufacturer-owned DataID
metadata plus concrete device-provider and VIN-request definitions.

High-value factory DataIDs present in that source include engine oil
temperature, intake-manifold pressure in bar, engine fuel rate in L/h,
instantaneous and trip fuel-consumption values in l/100km, individual tyre
pressures, fuel-pressure values, a particle-filter value, engine torque and
AdBlue range. These names/units are now source-corroborated but remain
**unmapped** until the archived diagnostic-logic layer yields their exact
provider/request/result/formula binding.

The same configuration directly confirms the 500 kbit/s routes
`7E0 -> 7E8`, `4E0 -> 5FF`, `602 -> 480`, `607 -> 587` and
`612 -> 482`, together with exact VIN bootstrap requests including
`22 F1 A0 -> 62 F1 A0`, `1A 90 -> 5A 90`, `21 05 -> 61 05` and
standard OBD `09 02 -> 49 02`.

The complete recovered evidence, encryption/extraction details, embedded source
revisions and interpretation boundaries are preserved in
`MERCEDES_ME_WHISPER.md`. That document is the source of truth for this
archived-app evidence; this file continues to own how it affects MBLINK's
Mercedes diagnostic model.

## W212 / C207 gateway module map

MBLINK now separates the **expected platform roster** from the **observed vehicle map**.

Mercedes' Series 207/212 introduction and component documentation shows that the central gateway is integrated into the N10/1 front-SAM housing. The housing contains two microprocessors: the SAM processor and a separate central-gateway processor, each with its own CAN interface, and Mercedes states that the two processors can be diagnosed separately. Diagnostic CAN (CAN D) reaches that integrated gateway processor, which bridges diagnostic traffic to the drivetrain, chassis, interior and other vehicle networks.

MBLINK therefore models **CGW and SAMF as separate logical diagnostic controllers that share the N10/1 physical housing**. It does not assume that a proprietary "open N93 route" command is required before ordinary ECU diagnosis. The strongest public CAESAR/Monaco evidence instead shows ECU definitions carrying independently defined physical request and response CAN identifiers, with the integrated gateway providing the cross-network path.

### Source-backed priority routes

The following physical routes have independently evidenced endpoints and
protocols. They are attempted before the generic census, and then omitted from
the generic 11-bit pass so they are not probed twice. Rows labelled as Daimler
production bootstrap evidence come from the recovered MSA/Whisper VIN cascade;
that proves the route and probe, but does not by itself identify the ECU family.

| ECU family | Request | Response | Protocol | Evidence basis |
| --- | ---: | ---: | --- | --- |
| Unclassified production route | `0x602` | `0x480` | UDS, VIN `22 F1 A0` | Daimler production VIN bootstrap |
| Unclassified production route | `0x607` | `0x587` | UDS, VIN `22 F1 A0` | Daimler production VIN bootstrap |
| Unclassified production route | `0x4E0` | `0x5FF` | KWP2000, local ID `21 05` | Daimler production VIN bootstrap |
| EIS_212 / EIS_204 | `0x612` | `0x482` | UDS, VIN `22 F1 A0` | Daimler production VIN bootstrap plus public CAESAR trace |
| ABR2XT / ESP | `0x632` | `0x486` | UDS | Public CAESAR communication parameters |
| ORC_212 / SRS | `0x64A` | `0x489` | KWP2000 / KW2C3PE | Public Monaco trace |
| HU_204 / COMAND | `0x652` | `0x48A` | KWP2000 / KW2C3PE | Public HU_204 Monaco trace |
| EPS212 | `0x6B2` | `0x496` | UDS | Public CAESAR communication parameters |

IC_204, SAMF_212, SAMR_212 and HVAC_212 are independently corroborated as real 204/207/212 diagnostic families, and public material establishes UDS/HSCAN operation for several of them, but MBLINK still leaves their physical TX/RX pairs unbound until an equally defensible trace or CBF/CAESAR parameter source is found.

The catalogue in `mercedes_module_catalog.h` records those families, Mercedes component designations, coarse subsystem kind, expected-presence class and diagnostic identity aliases. It deliberately does **not** assign a CAN address from documentation alone.

Normal mobile/desktop discovery and forensic discovery now use separate
Mercedes-owned target plans.

For a new VIN, the iPhone and the normal Linux scan use the compact
**47-slot gateway census** reconstructed from the source-backed C207/W212
request/response pattern:

`TX = 0x602 + 8 * slot`, `RX = 0x480 + slot`, for slots `0..46`.

That covers request IDs `0x602..0x772` and exact response IDs
`0x480..0x4AE`. The independently evidenced `0x607 -> 0x587` and
`0x4E0 -> 0x5FF` routes sit outside that lattice and are appended explicitly,
as are the eight legislated OBD physical slots `0x7E0..0x7E7`. The resulting
normal census is 57 exact-filter targets rather than the 760-target forensic
address sweep. Dead targets receive only bounded read-only presence probes;
deeper reads run only after a responder is proved.

For UDS responders MBLINK retains the physical route and reads read-only `F197`
system name plus `F187` spare-part number, `F188` software number and `F191`
hardware number where supported. It classifies a module only when returned
identity text or a standards-defined functional route supports that
classification, and reads each responder's protocol-appropriate DTC memory.
KWP routes are retained without applying UDS identity semantics to them.

The explicit **DEEP RESCAN** on Linux retains the exhaustive 760-target
forensic plan (505 11-bit targets plus 255 29-bit normal-fixed targets): the broad 11-bit diagnostic range plus 29-bit normal-fixed
logical addressing. FULL may try DTC/VIN fallbacks after a missed TesterPresent
and, for otherwise unknown 11-bit routes, may temporarily use headered receive
learning to discover a valid non-`TX+8` response before immediately re-locking
the exact filter. DEEP RESCAN is an in-session manufacturer rescan: LINK lets
any in-flight live PID request finish, pauses live polling, runs the full
Mercedes extension on the existing Bluetooth/USB transport, restores the
normal ELM channel and resumes the existing live scheduler. It does not
disconnect/reconnect the adapter or repeat standard PID/VIN discovery.

The 2026-08-29 C207/Vgate Linux capture proved that a promiscuous 11-bit receive configuration (`ATCRA` plus `ATCF000/ATCM000`) admits normal vehicle broadcast traffic fast enough to swamp the ELM327 command parser. Production discovery therefore keeps an exact receive route active for each 11-bit probe instead of opening the whole CAN bus.

That capture also disproved a second assumption: a Mercedes 11-bit diagnostic response is **not always request+8**. Public CAESAR/Vediamo traces publish independent `CP_REQUEST_CANIDENTIFIER` and `CP_RESPONSE_CANIDENTIFIER` values and give concrete 204/212-family examples: EIS `0x612 -> 0x482`, ABR2XT `0x632 -> 0x486` and EPS212 `0x6B2 -> 0x496`. MBLINK now treats those published pairs as source-corroborated routes and never extrapolates unobserved pairs from them.

The normal 47-slot census never opens a promiscuous receive window: each lattice
slot already has an exact request/response pair. Unknown-response learning is
reserved for the explicit FULL forensic scan. For a source-corroborated UDS
route, MBLINK keeps the published receive ID and first performs the same
transient extended-session handshake (`10 03`) shown by the public
EIS_212/EIS_204 CAESAR and DTS traces. That session change is non-persistent and
is used only to make diagnostic read services available; it does not alter
coding or stored vehicle data. The primary recovered VIN probes are route
specific: `22 F1 A0` for `0x602`, `0x607` and `0x612`, and KWP `21 05` for
`0x4E0`. Other UDS targets retain the bounded `22 F190` fallback. A positive
**or valid negative** protocol response proves a responder exists. No reset,
security access, routine control, communication/DTC-setting change, DTC clear,
coding, data write or programming request is introduced by this route-aware
census.

A catalogue entry is not evidence that a module is fitted to a particular car. Optional equipment remains optional, petrol/diesel control-unit alternatives remain mutually dependent on the decoded vehicle configuration, and a discovered but unclassified responder remains unresolved. Conversely, an ECU becomes part of the live vehicle map only after it actually responds during the read-only scan.

### VIN-keyed saved vehicle profiles on iPhone

The iPhone keeps a small local vehicle profile keyed by the exact 17-character VIN. The first successful connection performs the bounded module discovery and stores only stable topology and identity facts: module request/response routes, addressing mode, classified family, system name, and returned part/software/hardware identifiers. Dynamic DTCs and live measurements are never treated as persistent profile facts.

On a later connection to the same VIN, MBLINK loads the known topology immediately instead of rediscovering the address space. It reinitialises the ELM channel, recreates the transient `10 03` diagnostic-session context for any source-backed nonstandard route that required it, validates each saved module with TesterPresent, rereads each module's DTC memory, and then restores the standard OBD channel for live data. If any expected saved route no longer responds, the cached topology is discarded and one fresh bounded discovery is performed during that connection before the replacement VIN profile is saved.

Profiles use a schema version so an incompatible future format is ignored rather than misread. They are local application state; they are not source evidence and do not promote a module definition's provenance status.

## Read-only ECU evidence probe

`MblinkMercedesEcuProbe` owns the complete bounded engine-evidence sequence:

1. configure the ELM transmit header and receive address;
2. enable CAN automatic formatting and automatic flow control;
3. send UDS TesterPresent with a positive response requested (`3E00`);
4. if TesterPresent succeeds, request standardized VIN DID `F190` (`22F190`);
5. perform a bounded read-only standardized ECU-identification sweep;
6. perform a second bounded read-only CRD3 fingerprint sweep using publicly documented identifiers from the open-source CaesarSuite `Simulated_CRD3` model;
7. issue read-only UDS `ReadDTCInformation` `19 02 FF` while the Mercedes physical ECU channel is still selected;
8. retain every raw ELM327 command/response in the session evidence transcript;
9. restore the normal ELM/OBD-II configuration before generic polling resumes.

The standardized identity sweep currently requests:

| DID | Standard identity purpose |
| --- | --- |
| `F18C` | ECU serial number |
| `F187` | Vehicle manufacturer spare-part number |
| `F188` | Vehicle manufacturer ECU software number |
| `F189` | Vehicle manufacturer ECU software version |
| `F191` | Vehicle manufacturer ECU hardware number |
| `F197` | System name / engine type |

The CRD3 fingerprint sweep requests these evidence-only identifiers:

| DID | CaesarSuite CRD3 label | MBLINK treatment |
| --- | --- | --- |
| `F100` | Session / variant | ECU-family evidence |
| `F154` | Supplier identifier | ECU-family evidence |
| `F196` | EROTAN | raw ECU-family evidence |
| `1001` | Full variant coding | raw ECU-family evidence |
| `1002` | Partial variant coding | raw ECU-family evidence |

The CRD3 identifiers are not treated as DPF, rail-pressure, injector, EGR or turbo parameters. Their purpose is to identify the responding engine-control family and preserve exact raw responses for later fixture promotion. MBLINK does not infer a live-data formula from a positive fingerprint response.

## Delphi CRD3.10 hardware profile research

A dedicated external-research pass on 2026-08-27 strengthened the hardware-family evidence without changing MBLINK's read-only boundary.

Multiple public ECU/firmware catalogues identify 2011 E 250 CDI 2.2-litre / 150 kW applications with Delphi CRD3.10-family control hardware. The strongest recurring identity set is Mercedes hardware `6519040001`, software `6519020001` and spare-part number `6519011801`. Separate C207/A207 fitment listings identify Delphi CRD3.10 units such as Mercedes `A6519003701 / A6519012101` and Delphi `28381049`, demonstrating that one model badge does not imply one immutable part number. Independent CRD3.10 tooling documentation identifies Infineon TriCore TC1797 as the processor used by this ECU family.

MBLINK therefore does **not** hard-code “2011 E250 = one ECU part number.” Instead, the portable probe already reads standardized identity DIDs `F187`, `F188`, `F191` and `F197`. The CRD3 layer now has a source-corroborated hardware profile catalogue and matches those returned values against known families:

- a matching CRD3 system-name prefix alone is a family-level match;
- one matching hardware/software/spare number is source-corroborated evidence;
- two or more returned typed numbers matching the same profile is a strong source match;
- absence of a source match is never converted into a guessed ECU variant.

The first catalogue entry represents the 2011 E250 150 kW CRD3.10 family with HW `6519040001`, SW `6519020001`, spare `6519011801` and source-corroborated TC1797 MCU. This is deliberately not marked vehicle-verified until the physical C207 returns matching identity values. Returned identifiers are displayed directly in the native application so the evidence can be compared with the catalogue instead of being hidden behind a model label.

This profile data remains in MBLINK because it is Mercedes/Delphi-specific; the generic UDS DID acquisition remains in LINK.

## OM651 / CDID3 family signature

The portable CRD3 decoder understands the payload shape published by the CaesarSuite CRD3 simulator for `F100` and `F154`. `F100` carries gateway mode, a big-endian 16-bit ECU variant and the active session; `F154` carries a one-byte supplier identifier.

Independent OM651/CDID3 diagnostic material corroborates the signature `02 21 31` with Delphi as the supplier. MBLINK therefore recognises gateway mode `02`, variant `2131`, supplier `64`/Delphi as an **OM651/CDID3-family signature**. The live probe decodes those fields instead of merely recording positive-response masks. That remains family evidence, not automatic C207 vehicle verification.

## Mercedes UDS fault reading

`uds_dtc.h` provides the portable read-only ISO 14229 `ReadDTCInformation` codec. The request is `19 02 FF`: report DTCs by status mask with every status bit requested. Positive `59 02` responses are decoded into 24-bit UDS DTC values plus their ISO 14229 status byte; negative responses retain their NRC.

This request is part of the same `MblinkMercedesEcuProbe` used by the native iPhone connection path. It executes after the CRD3 fingerprint and before MBLINK resets the ELM adapter back to generic OBD-II. The iPhone Faults screen therefore has a distinct Mercedes UDS section in addition to the standard stored/pending/permanent OBD-II sections.

No-response, negative-response and malformed Mercedes fault replies are classified explicitly. MBLINK must not turn any of those states into a misleading empty-fault result. It does not clear faults, alter DTC settings, enter an extended session, perform security access, write data or invoke routines.

`MblinkMercedesEngineScan` remains as a compatibility/high-level wrapper around the same portable probe. It no longer issues a duplicate second DTC request.

## Mercedes fault knowledge is mandatory

Retrieving a 24-bit UDS DTC and displaying a six-digit hexadecimal value plus `status 0xNN` is only the acquisition layer. It is **not** considered complete Mercedes fault support.

MBLINK must maintain an evidence-backed Mercedes diagnostic knowledge layer able to resolve, where trustworthy information exists:

- human-readable Mercedes-Benz DTC title/description;
- originating ECU/module;
- Mercedes subsystem/component category;
- applicable vehicle/engine/ECU family, including CRD3/CDID3 and OM651 where relevant;
- decoded ISO 14229 status semantics;
- manufacturer-specific subcode/status interpretation when documented;
- provenance/source status for the definition;
- links to related verified diagnostic measurements where useful to investigation.

The raw 24-bit DTC and status byte remain part of the record even after a description is resolved. Unknown Mercedes codes must be shown as unknown with raw evidence preserved; MBLINK must never invent a likely-sounding Mercedes description.

Generic UDS status interpretation belongs in LINK. Mercedes-specific DTC definitions and module/component meanings belong in MBLINK. The full normative requirement is in `FAULT_DIAGNOSTICS.md`.

MBLINK now carries two deliberately different Mercedes knowledge sets. The
exact KWP/UDS layer remains module-scoped and evidence-gated. Alongside it is a
299-row source-scoped five-character reference catalogue reconstructed from the
material supplied to the project: 78 BenzWorld body/chassis rows, 104 Mercedes
P1xxx reference rows, all 38 TaT teaser rows (including explicit subcodes and
duplicate meanings), and 79 supplied SprinterManual reference rows. The
reference layer retains duplicates rather than selecting an arbitrary meaning.
For example, `B1000` and `P2004` remain explicitly ambiguous until module or
subcode context resolves them. Standards-defined generic meanings in LINK take
precedence and are never overwritten by a Mercedes reference row.

On iPhone, a LINK-classified manufacturer-specific OBD code that has no shared
definition is passed through this Mercedes reference resolver. A unique
source-scoped meaning is shown with its area and source; an ambiguous result is
labelled as such rather than pretending that one of several supplied meanings
is authoritative.

The Faults workspace is expected to present Mercedes faults as diagnostic records, not just hexadecimal rows. A useful record includes at least the raw DTC, resolved title when known, module, status, subsystem/category and source/provenance. A successful clean scan, an unperformed scan and a failed scan must remain visibly distinct.

### Archived official-app fault vocabulary

The supplied archived Mercedes me Adapter `libdiaglogic.so` exposes
`diaglogic.api.Dtc`, `diaglogic.api.DtcCollection`, DTC flags named
`STATIC` and `SPORADIC`, and a `storedObdDtcs` data identifier. This
corroborates a structured DTC/state model in the original product.

It does not establish a raw KWP/UDS-status-bit mapping. MBLINK therefore does
not translate the captured ORC status `E0` into static/sporadic semantics.
That byte remains raw evidence until the mapping is independently recovered.

## Offline trace replay

The test suite contains a deterministic ELM trace-replay harness. A fixture is an ordered list of expected adapter commands and responses. The replay fails if command order changes, an unexpected request appears or a response no longer decodes through the portable layers.

The current synthetic Mercedes fixture drives the entire read-only sequence through channel configuration, TesterPresent, VIN, six standardized identity DIDs, five CRD3 fingerprint DIDs and UDS `19 02 FF`. It also verifies decoded CRD3 variant/supplier evidence and the OM651/CDID3 Delphi signature. This lets development and regression testing continue before the physical adapter arrives. Synthetic fixtures prove deterministic software behaviour only; they never count as vehicle verification.

When real evidence is available, the same harness is the promotion path: sanitize the captured exchange, commit it as a deterministic fixture, then promote only the endpoint/definitions that the capture actually proves.

Mercedes DTC knowledge should follow the same evidence discipline: definitions are test data with provenance, not UI strings scattered through SwiftUI.

## OM651 manufacturer signal catalogue

`mercedes_om651.h` defines stable identities for manufacturer-level values independently shown by OM651/CDID3 diagnostic material. Current priority groups are:

- DPF differential pressure, fill level, ash, multiple soot estimates, regeneration state and distance since regeneration;
- exhaust temperatures before the turbocharger, before the catalyst and at the DPF inlet;
- fuel rail pressure;
- boost pressure;
- EGR command/rate;
- cylinder 1–4 smooth-running corrections and injector correction factors.

Every entry is currently `corroborated-unmapped`: the capability is known to exist on OM651/CDID3 diagnostic systems, but MBLINK deliberately leaves its DID, byte layout, scaling and units unbound until defensible protocol evidence exists. This gives the program a concrete Mercedes feature model now without inventing attractive-looking numbers.

## Why proprietary DPF formulas are still unbound

Public Mercedes engine live-data mappings for OM651 are substantially weaker than standardized UDS identity material. Independent diagnostic pages show that CRD3 exposes the DPF, injector, rail, EGR and boost values MBLINK wants, while the W207 fitment database grounds the E 250 CDI in Delphi CRD3.x. Neither source, by itself, supplies enough raw request/response/scaling evidence to bind a proprietary live-data formula safely.

MBLINK therefore separates **fitment evidence**, **capability evidence**, **protocol mapping evidence** and **vehicle verification**. A DID/formula is only bound when its request, raw response layout and scaling are supportable. That is deliberately different from guessing a Mercedes DID and showing a plausible-looking value.

The same principle applies to Mercedes DTC descriptions: MBLINK should be comprehensive, but comprehensiveness must come from sourced definitions rather than fabricated interpretations.

## Secondary EOBD responder and TCM candidate

ISO 15765-4 defines the 11-bit legislated OBD physical slots and recommends
`0x7E0/0x7E8` for the engine control module and `0x7E1/0x7E9` for the
transmission control module. The development C207/OM651 has independently
returned traffic on both responder slots. MBLINK therefore treats a live
`0x7E9` response as evidence for a **transmission-control candidate** on the
`0x7E1 → 0x7E9` route.

This is functional classification, not guessed Mercedes identity. The exact
VGS/EGS family remains a candidate until returned Mercedes ECU identity names
it. Live Apple OBD polling enables `ATH1` after manufacturer discovery so
simultaneous `0x7E8` and `0x7E9` replies remain attributable in the raw
evidence stream rather than being collapsed into anonymous duplicate values.

## Genuine Mercedes me Adapter transport

Mercedes part A2138203202 / approval 10R-042695 is a genuine Mercedes me Adapter intended for compatible Series 207 vehicles. Mercedes setup material documents Bluetooth pairing under a local name in the form `MB-xxxx`.

A preserved official Mercedes me Adapter **4.7.61** Android build (`com.daimler.mbfa.android`) now gives MBLINK substantially stronger interoperability evidence. The analysed transport implementation is in the archived app's T-Systems/Daimler framework code, principally `classes3.dex` (local SHA-256 `83cd980cac55e517926469f165cdd83f55eddec7c45e1590c45b6686c5685ae0`). LINK keeps the full evidence ledger in `docs/MERCEDES-ME-ADAPTER-INTEROP.md`.

The official application's default adapter-name families are:

| Official-app role | Default Bluetooth-name pattern |
| --- | --- |
| BLE adapter | `^MB-[189].*` |
| first-generation adapter | `^MB-[2346].*` |
| second-generation adapter | `^MB-[57].*` |
| adapter for other apps | `^VAN-.*` |

A separate literal `^MB-[02-46].*` exists but its role is not yet strong enough to bind behaviour, so MBLINK records it without using it as a routing rule.

For Bluetooth Classic, the official `BluetoothObdAdapterDevice` uses the standard SPP UUID `00001101-0000-1000-8000-00805F9B34FB` and Android's insecure RFCOMM SPP socket path. Its static reference timing is a 44,000 ms connect timeout and a 6,000 ms minimum connection duration.

For BLE, the official `BleObdAdapterDevice` uses Nordic UART Service:

- service `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`;
- RX/write `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`;
- TX/notify `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`;
- CCCD `00002902-0000-1000-8000-00805F9B34FB`;
- requested Android MTU 512.

The app also carries Toshiba SPP-over-BLE identifiers `e079c6a0-aa8b-11e3-a903-0002a5d5c51b` (service) and `b38312c0-aa89-11e3-9cef-0002a5d5c51b` (characteristic). Those are retained as a known fallback candidate rather than assumed to apply to every generation.

MBLINK treats the adapter as a **native Mercedes telemetry transport**, not as an ELM327. LINK may discover and connect the evidence-backed RFCOMM/GATT channel and MBLINK may preserve native frames, but no ELM `ATI`, `ATZ` or guessed Mercedes me command is transmitted merely to establish identity.

The archived application also proves that the adapter lifecycle is more than an unauthenticated serial stream. It contains `setupObdAdapter`, `readObdAdapterData`, `deactivateSppServerMode`, Session Master Key classes/states and a backend SMK resource `POST tenants/{tenantId}/obdAdapters/{obdAdapterId}/v2/smk`. Those names are evidence of a commissioned/keyed session model; they do **not** yet reveal a safe local SMK algorithm or wire frame. MBLINK therefore keeps setup/read command bytes evidence-gated until the framing is independently recovered or captured from the physical adapter.

The first physical capture remains valuable, but it is no longer a blind transport-discovery exercise: LINK can now select the generation-specific Bluetooth path first and use the resulting byte stream to recover the remaining framing and vehicle-value decoder.

### Native Mercedes me engine findings from the archived app

A deeper clean-room pass over the archived 4.7.61 DEX files establishes that
the Bluetooth layer is only the transport edge. The official Java framework
loads a native `gdk` library for adapter state/session handling and a native
`diaglogic` library for vehicle diagnosis. The native GDK interface returns
explicit QoS, execution and failure ordinals and requests Session Master Keys
through a Java callback taking the adapter ID and passkey.

The Java/native bridge also proves the Bluetooth record contract: outbound GDK
commands must end in CR (`0x0D`); inbound CR closes a normal record; inbound
BEL (`0x07`) closes a special NACK record; the official receive accumulator
is 700 bytes and clears an unterminated record at position 698. MBLINK/LINK can
therefore frame native evidence deterministically without inventing the
remaining command vocabulary.

The embedded DiagLogic protobuf schema separates requested ECU/device identity
from the responding device address and defines typed measured values, DTC
collections, VIN/vehicle configuration and adapter software version. The
official framework also contains 120 exact diagnostic/live-data IDs, preserved
in LINK's `docs/MERCEDES-ME-DATA-IDS.md`. Important examples include
`relativeAcceleratorPedalPosition`, `throttlePosition`,
`engineReferenceThrottle`, `engineRpm`, `engineFuelRate`,
`fuelVolume`, `fuelLevelMin`, `fuelPressure`, `mileage`,
`tankRange`, `vehicleSpeed`, maintenance/service values, ECU fingerprint
identities and many trip-scoped lamp/brake/tire fault flags.

These identifiers are an implementation checklist, not guessed CAN/UDS
mappings. The native GDK/DiagLogic/Whisper shared objects have now also been
recovered and analysed. LINK preserves the detailed binary evidence in
`docs/MERCEDES-ME-NATIVE-BINARIES.md`.

That native pass proves that the genuine adapter stack contains raw CAN and
ISO-TP command classes, seed/key/passkey handling, a 32-byte Session Master Key
with two 16-byte random values, SHA-256-based session-key derivation, AES-256
and Base64 support, and a configuration-driven Whisper execution layer.

The most important architectural result for MBLINK is that Whisper exposes
configuration concepts for OBD/PDU transceive, raw CAN transmit, baud rate,
P2*/P3 timing, service/request/result IDs, PDU bytes, response matching,
CAN filtering, result extraction and formulas. This strongly indicates that
the actual Mercedes ECU addresses, requests and scaling rules live in APK
configuration assets rather than being inseparable from the genuine adapter.

Accordingly, proved Mercedes vehicle-side definitions should be transport
neutral. The same verified ECU/request/result/formula definition can be
executed through Mercedes me, Tactrix/OpenPort, STM32 CAN or a capable ELM
transport. Mercedes-me-specific Bluetooth/session/security remains in LINK's
genuine-adapter provider; Mercedes ECU knowledge remains in MBLINK.

The archived APK inventory subsequently recovered the production
`MSA_VIN_cascade` bootstrap. It proves the route-specific VIN requests now used
by discovery: UDS `22 F1 A0`, KWP `21 05`, the alternate KWP `1A 90`, and the
ordinary OBD `09 02` path. The implemented physical-route priorities use only
the exact primary mappings above; the alternate KWP and passive VIN candidates
remain evidence records rather than speculative extra traffic. The bootstrap
is not the complete runtime diagnostic configuration, so unknown Mercedes CAN
IDs, DIDs, byte layouts and formulas remain unbound.

### Native secure protocol now reproduced independently

The subsequent native-library pass went beyond architecture names and recovered
the local secure record format. LINK now independently implements the proved
wire codec without embedding the proprietary Android libraries or any fixed
application challenge key.

The session key is 32 bytes and is derived as:

`SHA-256(SMK || random_argument_1 || random_argument_2)`

where the Session Master Key is 32 bytes and each random argument is 16 bytes.
The authenticated call flow establishes that the arguments are ordered as the
device/adapter random followed by the application random.

The secure inner frame is a six-byte header followed by plaintext and bounded
length-valued padding:

- 16-bit big-endian plaintext length;
- 16-bit big-endian CRC-16/CCITT-XMODEM over the plaintext;
- two reserved zero bytes;
- plaintext;
- padding to the observed AES boundary, with each byte equal to the padding
  length.

The result is AES-256-ECB encrypted, Base64 encoded and wrapped as
`a<Base64(ciphertext)>CR`. The native implementation caps ciphertext at 512
bytes, which gives an effective maximum plaintext of 505 bytes.

LINK also now has pure, bounded builders for the native commands whose complete
syntax is proved: CAN open/close, adapter status, hardware information,
GetPasskey, GetSeed, SetKey, baud ordinal and X read/write. These functions are
**not** automatically transmitted. Raw-CAN and ISO-TP command payload layouts
remain disabled until their full builders are recovered.

### Shared native acquisition policy

The native DiagLogic engine contains acquisition/sanity policy that belongs
above any one adapter. LINK now exposes those exact reference values so MBLINK
can apply the same evidence to Vgate/ELM, Tactrix/OpenPort, STM32 and genuine
Mercedes-me transports.

Recovered defaults include 1500 ms live-data reads, 5000 ms availability
reads, a 10-second minimum ignition-read delay, 30-second to 5-minute mileage
read spacing, 12.2/13.2 V ignition-off thresholds, 60-second live-status age,
90-second run-cycle age, and the native fuel/mileage sanity thresholds. Where
a binary symbol does not itself prove the physical unit of a speed/distance
threshold, MBLINK preserves the number without inventing a unit.

DiagLogic also explicitly separates raw measurements from post-processed
vehicle state through components such as `UnplausibleFuelVolumeFilter`,
`MileageAdjustingVehicleStatusUpdater`,
`TripAverageSpeedVehicleStatusUpdater`,
`IrregularObdResponseVehicleStatusUpdater` and
`TripWarningLampVehicleStatusUpdater`. MBLINK should therefore retain raw
samples/provenance even when presenting filtered or derived values.

### Whisper response and DTC vocabulary

LINK now preserves the exact Whisper response-selection strategies
`SELECT_FIRST`, `SELECT_LOWEST_CANID_CACHED`, `SELECT_MAXIMUM` and
`MERGE_ELIMINATE_DUPLICATES`. This reinforces MBLINK's existing requirement
to retain ECU/source attribution for simultaneous OBD responders instead of
flattening them into an anonymous duplicate value.

Whisper also identifies DTC presentation families
`SAEDTC_KWP_DAI`, `SAEDTC_KWP_VW`, `SAEDTC_UDS_DAI`,
`SAEDTC_UDS_VW` and `SAEDTC_OBD`. These are evidence labels only until
their individual byte decoders are independently reconstructed.

The configuration engine contains ZIP decompression, revision/cache/blacklist
handling and active-configuration/VIN-mapping logic. The missing Mercedes
definition set may therefore be a compressed configuration bundle rather than
a simple directory of plaintext property files. That possibility must be
included when extracting the remaining APK assets.


## iPhone evidence path

The native iPhone workflow performs the full portable Mercedes sequence automatically after the standard OBD-II capability exchange: VIN, standard identity, CRD3 fingerprint, decoded CRD3 family evidence and Mercedes UDS fault memory. Vehicle and Modules expose the CRD3 result; Faults exposes the separate UDS records; Log exports the complete transcript. The adapter is then reset and ordinary OBD-II polling/fault services resume.

The Modules workspace now presents that census as structured, tappable control
units rather than a flat transcript. Each detail view owns the module's CAN
route, protocol, returned identity/part/software/hardware evidence, fault state
and any live standard PIDs returned by that exact response CAN identifier.
Modules without verified live mappings remain useful identity/fault cards and
show no invented measurements.

SwiftUI remains a view layer. Mercedes protocol state, CRD3 decoding, UDS fault parsing and Mercedes fault lookup/knowledge belong below the UI so the same evidence rules, definitions and replay tests can be used on other platforms.

## Per-module Mercedes manufacturer-data discovery

Live-data selection is not limited to legislated SAE Mode 01. A responding
Mercedes ECU may expose its own actual values through UDS
ReadDataByIdentifier (`0x22`) or KWP2000 ReadDataByLocalIdentifier (`0x21`),
even when it exposes no standard OBD-II PIDs at all.

MBLINK therefore keeps three layers distinct:

1. the control-unit census identifies the physical Mercedes ECU route;
2. a read-only manufacturer-data scan is run against that exact TX/RX route;
3. standard OBD-II Mode 01 values, when a module also returns them, are shown
   only as a secondary standards-based layer.

The manufacturer-data scanner currently performs a bounded discovery pass over
the source-backed Daimler actual-value neighbourhood `0x2000..0x20FF` for UDS
ECUs and local identifiers `0x01..0xFF` for KWP2000 ECUs. Only positive
responses are retained. Unknown positives remain attached to the originating
module as raw identifiers and raw response bytes; MBLINK does not invent a
name, unit or scaling formula.

Positive manufacturer identifiers are persisted against the VIN-keyed module profile. After one successful full discovery pass, later sessions can refresh only the exact UDS DIDs / KWP local identifiers that previously responded, avoiding a repeated full-range sweep. Historical Mercedes-me runtime/app-data is therefore useful optional evidence, not a dependency of manufacturer-data discovery.

The first exact numeric mapping remains CRD3 UDS DID `0x2007`, battery voltage,
decoded as a two-byte big-endian value multiplied by `0.0078125 V`. Other
positive manufacturer identifiers are deliberately shown as `RAW` until the
Whisper/CAESAR definition or a reproducible vehicle mapping proves their
meaning.

LINK 0.14.50 retains responder-specific OBD capability masks and also adds the product-neutral ability to pause an idle live scheduler,
enter a read-only manufacturer extension, perform physical-ECU reads, restore
the adapter and resume the existing standard live schedule without losing OBD
capability or fault state. MBLINK owns the Mercedes scan plan and all semantic
promotion.

This is the architecture required for ESP/ABR, ORC/SRS, EIS/EZS, EPS,
head-unit and other body/chassis modules: a module does not disappear from Live
Data merely because it has no SAE Mode 01 PIDs.


## Evidence and promotion

An endpoint, DID, DTC definition or scaling rule may move to a stronger evidence status only when its provenance is defensible. Vehicle-specific promotion requires a reproducible capture tied to the relevant vehicle/ECU/adapter conditions and regression fixture where the definition depends on observed protocol behaviour.

No unverified Mercedes manufacturer live-data formula is currently promoted into the C207/OM651 profile. The work can continue without hardware; hardware is required only for final promotion from corroborated/candidate protocol definitions to exact C207/OM651 vehicle-verified mappings.


## Factory-source precedence and expanded CRD3 target catalogue

MBLINK treats Mercedes/Delphi factory actual values as the preferred diagnostic source whenever the request, payload layout, scaling and meaning have been verified. Standard SAE OBD-II remains a trustworthy fallback and cross-check; it is not relabelled as factory data.

The OM651/CDID3 target catalogue now also records factory-observed ambient temperature, battery voltage, engine speed, coolant/oil/intake/fuel temperatures, barometric pressure, accelerator-pedal sensors 1 and 2, throttle valve, injection quantity, rail-pressure regulation state, mass air per cylinder, EGR valve and cooler-bypass valve, air-filter downstream pressure, boost-pressure control flap, driver torque request and fuel-tank level. These targets remain `corroborated-unmapped` until exact protocol evidence exists. Their presence in the catalogue therefore improves completeness without causing speculative requests on the vehicle bus.


## Fuel evidence on the C207 / OM651

The captured C207 supported-PID map advertises SAE PID `0x2F` Fuel Level Input, so MBLINK can read a real standard fuel-gauge percentage immediately. The same captured `0x40` capability block does not advertise SAE PID `0x5E` Engine Fuel Rate, so MBLINK must not pretend that an SAE fuel-flow value exists on this vehicle.

Public OM651 CDID3/Delphi diagnostic data confirms that the factory ECU exposes fuel tank level in litres and injection quantity in mg/stroke. Those are retained as Mercedes/Delphi targets until their exact request, payload and scaling are mapped. This is also the likely path to the vehicle's own instantaneous consumption display: factory fuel accounting/injection data combined with vehicle speed rather than SAE PID `0x5E`.

Unknown factory values may be identified by correlation, but only after a candidate request is source-backed. MBLINK can compare a candidate's time series with known values such as SAE speed, RPM, coolant, rail pressure, ambient temperature or control-module voltage. Matching shape, scale, offset, response timing and physical behaviour can promote a candidate from corroborated-unmapped to vehicle-verified; correlation alone never invents the request address.


## 0.7.79 C207 live-capture observations

A de-identified C207/Vgate iPhone evidence run validated several interpretation decisions directly against vehicle behaviour. SAE fuel-level PID `2F` reported roughly 42 percent; ambient PID `46` moved through 16–18 °C; accelerator-pedal D and E closely tracked each other; and absolute throttle-valve PID `11` followed a materially different trajectory. That is consistent with PID `11` being the intake throttle valve rather than accelerator-pedal demand.

A later 0.7.94 C207/Vgate capture recorded 13,728 decoded samples and repeated
the six-route mobile census twice. It proved simultaneous `0x7E8` and `0x7E9`
Mode 01 replies for PIDs `04`, `05`, `0C`, `0D`, `42` and `49`. It also repeated
the ORC `0x64A -> 0x489` KWP record `9B51 / E0`. MBLINK's module-scoped
Mercedes fault table now resolves `9B51` as **driver seat-belt buckle circuit:
short to positive or open circuit**. The meaning is source-corroborated; the
exact raw code and route are vehicle-verified. The raw `E0` status remains
visible because its bit-level meaning has not been independently verified. The
same capture exposed an incomplete multi-frame ESP fault response. These shapes
are regression evidence; `0x7E9` remains a transmission ECU candidate until
returned Mercedes identity confirms VGS/EGS.

The same run exposed the old default schedule as too aggressive for the ELM/BLE path, which is why LINK 0.14.25 lowers the default request cadences while retaining per-PID enable controls.


## 0.7.109 C207 manufacturer-data capture

A 0.7.109 iPhone/Vgate evidence export from the C207 records two consecutive
connections in one evidence stream. The first connection still hit the known
BLE first-connect timeout before the automatic retry recovered the same session;
the following reconnect reached the ELM channel immediately. This is regression
evidence that retry recovery works, but it is **not** evidence that the initial
connection defect is fixed.

The longer session captured 33,943 responder-attributed SAE Mode 01 samples.
The engine responder `0x7E8` supplied 24 distinct live PIDs and the secondary
`0x7E9` responder supplied nine. In particular, the previously added standard
values `1F`, `21`, `24`, `30`, `31`, `3E` and `4D` all produced
real samples, so those catalogue additions are now vehicle-verified on this
capture rather than merely standards-backed.

More importantly, the module-scoped read-only manufacturer-data scanner found
positive raw identifiers on three physical Mercedes routes:

- ESP/ABR `0x632 -> 0x486`, UDS: 18 positive DIDs:
  `2001, 2003, 2004, 2007, 2009, 200A, 200D, 200F, 2010, 2014, 2017, 2023, 2043, 2046, 2047, 2070, 20C0, 20DF`.
- ORC/SRS `0x64A -> 0x489`, KWP2000: 29 positive local IDs:
  `01, 02, 07, 0D, 0F, 11, 13, 18, 23, 24, 2B, 2D, 51, 52, 58, 59, 60, 61, 62, 63, 64, 65, 69, 70, 71, 72, 77, E0, E4`.
- Audio/head unit `0x652 -> 0x48A`, KWP2000: four positive local IDs:
  `01, 02, 05, 06`.

The capture also records complete raw payloads for these positives. Regression
tests preserve representative examples from each route, including ESP DID
`0x2001 -> 06 1A 06`, ORC local ID `0x58 -> 00 90 55 68 00`, and head-unit
local ID `0x01 -> 10 10 22 10`. These are vehicle-verified **existence,
routing and payload** facts only. No engineering meaning, unit or scaling is
assigned until an independent Mercedes/Whisper/CAESAR definition or a
reproducible mapping proves it.

The same capture repeated the already established module fault evidence:
ORC `9B51/E0` and the ESP fault fragment. The unclassified
`0x602 -> 0x480` response is preserved as the complete UDS PDU
`59 02 7B D1 81 00 50`. Under the standard `0x19/0x02` response envelope
this is status-availability mask `0x7B`, DTC `D18100`, status `0x50`;
`0x7B` is not part of the DTC number. Only the source-backed ORC meaning is
promoted; the ESP and `0x602` meanings remain raw until independently
defined.


## 0.7.153 C207 completed forensic capture

The 2026-09-03 C207/Vgate evidence export was produced by MBLINK 0.7.153 /
LINK 0.14.81. That version predates the current 47-slot mobile-census code, so
the capture does **not** validate the new scanner implementation. It remains
strong vehicle-side evidence because it records the actual ECU replies.

The completed pass independently observed seven diagnostic responders:
`0x602 -> 0x480`, `0x612 -> 0x482`, `0x632 -> 0x486`,
`0x64A -> 0x489`, `0x652 -> 0x48A`, `0x7E0 -> 0x7E8` and
`0x7E1 -> 0x7E9`. The first five proprietary responders all match the
request/response pairs used by the compact Mercedes census. No 29-bit responder
was observed during the completed forensic pass.

The two legislated OBD responders retained independently different vehicle
state. PID `30` reported 31 warm-ups from `0x7E8` and 56 from `0x7E9`;
PID `31` reported 1650 km and 2796 km respectively. These are regression
evidence that responder attribution must be retained and that a secondary
EOBD responder must never overwrite the engine's value merely because it
answered the same PID.

The same drive vehicle-verified SAE PID `2F` fuel-level replies spanning
approximately 96.47% to 100%. Accelerator-pedal channels `49`/`4A`
remained distinct from absolute throttle-valve PID `11`, preserving the
product distinction between driver demand and the ECU-controlled diesel intake
throttle.

One ESP `0x632 -> 0x486` indexed ELM response ended at the fragment
`59 02 39 47 5C 00`. After the availability mask this leaves only three
bytes, not a complete four-byte DTC/status record. Current regression tests
therefore require that fragment to be rejected as malformed rather than
inventing a status byte or promoting a guessed DTC.

A sanitised fixture under `tests/support/c207_20260903_evidence.h` retains
these vehicle-side facts without the VIN and replays them against current
decoders and module-scan behaviour.
