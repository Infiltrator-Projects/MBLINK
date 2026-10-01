# Changelog

## 0.7.274 — 2026-10-01

- Remove the obsolete C207-scoped Mercedes module-catalogue compatibility API and its four forwarding implementations.
- The repository already uses the Mercedes-wide module-catalogue API; this is real dead compatibility-code removal, not a helper extraction or behaviour change.

## 0.7.273 — 2026-10-01

- Extract the empty-state factory-data panel from the large module factory-data section into a focused SwiftUI helper.
- Preserve the same scan action, disabled state and RAW/unknown-value explanation; no diagnostic discovery, polling, or decoding behaviour changes.

## 0.7.272 — 2026-10-01

- Extract the startup OBD readiness/status grid from the large Modules card builder into a focused SwiftUI helper.
- Preserve the same MIL indicator, readiness colouring, labels, accessibility grouping and read-once presentation; no diagnostic or polling behaviour changes.

## 0.7.271 — 2026-10-01

- Extract Bluetooth transport-boundary alert handling from the central iPhone refresh callback into a focused helper.
- Preserve the exact alert conditions and duplicate-suppression behaviour; no connection, polling, decoding, or vehicle-profile behaviour changes.

## 0.7.270 — 2026-10-01

- Extract manufacturer-history session reset from the central iPhone refresh callback into a focused helper.
- Preserve the existing rule that graph history is cleared across disconnects and vehicle/session changes; no polling, decoding, or presentation behaviour changes.

## 0.7.269 — 2026-10-01

- Extract live/saved/no-vehicle profile presentation from the central iPhone refresh callback into one focused helper.
- Preserve the same VIN, identity, probe, CRD3, UDS-fault and saved-profile display states; this is a housekeeping-only refactor with no polling or diagnostic ownership changes.

## 0.7.268 — 2026-10-01

- Move the simulated-flow regression marker construction out of the production refresh path into its CI-only helper, leaving runtime behaviour unchanged while making the main state-refresh method substantially easier to audit.
- Keep the exact same simulator evidence fields and release checks; this is a housekeeping-only iPhone refactor.

## 0.7.267 — 2026-10-01

- Finish the interrupted iPhone housekeeping pass: remove the redundant manufacturer PID catalogue cache and resolve offline catalogues directly from the saved controller profile/ECU pack.
- Move legacy standard-PID selection migration out of ConnectionViewModel, preserve explicit per-controller choices including deliberate all-off selections, and stop recreating the retired vehicle-wide standard selection bucket during migration.
- Pin LINK 0.15.89 at 70ee5c587cf8912817729ff6d367499459b30380: MBLINK now owns its controller/VIN-scoped polling policy directly, and its replacement table/dashboard presentation no longer makes LINK build generic lists that are immediately discarded.

## 0.7.266 — 2026-10-01

- Fix simulation stopping with obd2-error when a selected documented reading has no simulated sample, such as fuel level 01 2F. LINK 0.15.88 returns NO DATA for those valid requests so the session keeps polling.
- Keep the fuel-level catalogue entry and real-car percentage decoder intact.
- Require iOS CI to select fuel level alongside RPM/speed and observe its NO DATA reply before accepting a successful simulated session. LINK additionally exercises every documented live PID through two minutes of simulated polling.
- Require repeated responder-attributed RPM/speed replies and saved profile evidence before the CI cold restart; startup readiness alone must not satisfy the live-polling check.

## 0.7.265 — 2026-10-01

- Show decoded rear axle ratio, tyre circumference and engine inertia in the Vehicle profile under Vehicle configuration, using the current VIN’s saved startup readings.
- Remove those vehicle facts from the module card and module detail while retaining transmission-specific coding, integrity and raw evidence there. Acquisition remains startup-only, with no extra polling or PID toggles.

## 0.7.264 — 2026-09-30

- Pin LINK 0.15.87 at c397e6e423d0ef0e44363c38971111a35c67f32c: fix headered CAN fault decoding, handle populated/multi-frame DTC lists and continue into readiness and recurring polling.
- Offer documented Mode 01 readings on standard OBD controllers independently of individual capability bits, and let explicit selections create the corresponding polling request. Keep readiness, controller identity and ECU-pack startup data read once and excluded from live toggles.
- Mark retained manufacturer live readings as stale after a failed requested refresh; preserve raw evidence and startup module data, remove stale numeric values from gauges/graphs, and restore fresh status after a successful reply.
- Regress snapshot freshness/recovery and selecting an unadvertised documented PID alongside an advertised one without exposing SAE channels on body ECU routes.

## 0.7.263 — 2026-09-30

- Promote EGS53 21 B1 bytes 37-38 from tentative to corroborated tyre circumference after cross-vehicle coding comparisons; the development C207 decodes to 1960 mm.
- Decode byte 39 as the corroborated engine-inertia calibration; the captured C207 value 0x27 is 39 Nm.
- Keep byte 40 raw and explicitly unknown rather than hiding it inside the previous bytes-39-40 unknown range. The Modules screen now shows tyre circumference and engine inertia as decoded startup facts.

## 0.7.262 — 2026-09-30

- Read EGS53 KWP local identifier 0xB1 exactly once during connection startup after the controller family is positively identified; it is never admitted to recurring PID polling.
- Decode the captured EGS53 variant-coding record into KXCY variant code, C/S + Manual + A program flags, paddle-shift coding bit, 2.470 final-drive ratio, best-current tyre-circumference candidate, and CRC-16/ARC integrity while explicitly retaining confidence labels for inferred mappings.
- Show the decoded 21 B1 data directly on the EGS53 Modules card and module detail. Undecoded byte ranges and the complete raw payload remain visible for continued reverse engineering; trailing four bytes are labelled as a possible coding fingerprint rather than asserted as settled fact.
- Add a regression using the exact 46-byte field capture and keep 0xB1 classified as non-live/static configuration data.

## 0.7.261 — 2026-09-30

- Treat Mode 01 PID 01 as one-shot startup module metadata instead of a recurring live PID: the connection flow reads it once, retains each physical ECU's reply and never schedules it continuously.
- Show every returned PID-01 field directly on that controller's Modules card: MIL with green/red status, confirmed emissions DTC count, ignition-layout bit and every readiness-monitor state including Ready, Not ready, Not supported and Not applicable.
- Remove PID 01/readiness fields from PID Setup, Dashboard, Table and Graph live-channel membership; prune historical saved selections so upgrades cannot keep polling them in the background.
- Pin LINK 0.15.86 at cab1e7eb7463293dcbf970d44955040f62c2bd2e, which preserves responder identity for the single startup readiness transaction.

## 0.7.260 — 2026-09-30

- Add exact per-controller MIL status lights to the Modules screen: green when that ECU reports MIL not requested, red when it reports MIL requested, and no light when that controller does not return a usable Mode 01 PID 01 MIL value.
- Issue one temporary hidden MIL field probe independently of PID Setup display choices, retain LINK's responder-attributed 01 01 replies, then remove the hidden probe after the first valid responder sample so normal user-selected polling remains authoritative.

## 0.7.259 — 2026-09-30

- Correct the EGS53 RLI 0x30 drive-program decoder from the older two-state fallback to the four states directly corroborated on Siemens EGS53 A0034464310 (HW 06.48 / SW 18.29.00): 0 Sport, 1 Comfort, 2 Adaptive and 3 Manual.
- Add regression coverage for all four EGS53 drive-program codes and keep out-of-range values unmapped rather than manufacturing a label.

## 0.7.258 — 2026-09-30

- Make PID Setup controller-first: each discovered/saved physical controller now owns one combined catalogue containing that exact responder's advertised OBD-II channels alongside the exact Mercedes KWP2000/UDS controller-pack reads.
- Remove the synthetic vehicle-wide OBD section. OBD-II is now shown as a diagnostic interface of the responding controller, with every row clearly labelled OBD-II, KWP2000, UDS or Mercedes as appropriate.
- Keep equivalent-looking values from different interfaces as separate logical channels. An OBD-II vehicle-speed value and a Mercedes controller-specific vehicle-speed value may sit next to one another and can be enabled independently or together.
- Scope standard selections by VIN and controller while keeping physical Mode 01 scheduling de-duplicated: selections on several responders that require the same standard PID still produce one functional source request, and disabling one controller's copy cannot stop a source still required by another.
- Pin released LINK 0.15.84 at 4a022eb5cb680191a8a64b34622f67966ca7776a and use exact-responder structured samples for Mode 01 PID 0x01, so each controller presents only its own selected readiness fields even when several ECUs answer the same functional request.
- Preserve responder identity in standard histories/snapshots and give controller/interface/value combinations distinct stable presentation identities so data from two ECUs can no longer collapse into one displayed channel.
- Migrate the previous vehicle-wide standard selection conservatively to one deterministic primary advertised responder, preferring 0x7E8 when present, rather than unexpectedly enabling duplicate OBD values on every supporting ECU; existing older per-controller selections are retained.
- Keep saved VIN profiles controller-centric offline by reattaching cached responder capability maps to their saved Mercedes controllers and retaining unresolved saved OBD responders without inventing Mercedes identity.

## 0.7.257 — 2026-09-30

- Pin the completed grouped-field architecture to released LINK 0.15.83 at 3de497edaedda8459cc0cd19e7bcfed05dd1b325, keeping standard and Mercedes logical selections on one shared physical request per source record.
- Make field-selection changes wire-stable: enabling or disabling another logical value on an already-active source changes only the extraction mask and no longer restarts the underlying source poll.
- Preserve the final-field rule: the source request starts when the first constituent value is enabled and stops only when the last direct/logical selection is disabled.
- Retain 0.7.256 selective decoding for Mode 01 PID 0x01 and Mercedes EGS53 0x21/0x30, so unselected constituent values are not interpreted merely because their bytes arrived in the shared response.

## 0.7.256 — 2026-09-30

- Separate user-facing logical values from physical diagnostic source requests so several enabled values carried by one OBD/KWP record share one wire poll rather than becoming duplicate requests.
- Split Mode 01 PID 0x01 into independently selectable MIL, confirmed emissions DTC count, ignition-layout and readiness-monitor values while retaining one physical `01 01` request whenever any constituent field is enabled.
- Pin LINK 0.15.82 and pass the PID 0x01 logical selection as a per-source field mask; only selected fields are decoded and promoted to telemetry, history and display values even though the standards-defined response bytes arrive together.
- Preserve existing saved PID-01 choices by expanding the old whole-PID selection into the new constituent logical values instead of losing prior user intent.
- Apply the same grouped-read rule to Mercedes EGS53 `21 30`: selected transmission values are collapsed to one KWP request and the decoder receives a field mask so unselected gear, torque, pressure, speed and state fields are not interpreted.
- Keep PID Setup, module views, Dashboard, Graphs and Table on the same logical selection set so an OFF constituent field cannot reappear from cached data.
- Add portable, Apple and architecture regressions proving one or many logical selections still schedule exactly one source request and that selective extraction preserves the captured PID 0x01 and EGS53 behaviours.

## 0.7.255 — 2026-09-30

- Populate PID Setup from both exact controller-data sources and exact Foxwell/Xentry read profiles: every safe source-backed readable item for the identified controller is selectable during the current catalogue-completion phase, without filtering slow-changing, static-looking, identification or configuration values.
- Import and pin exact W204 Vediamo CBF Data services for CGW_204, EIS_204, IC_204, SCCM_204, ORC_204, HU_204 and FSCM212, alongside the existing ABR2XT, CGW_212 and MPC212 sources, preserving each diagnostic service with its identifier.
- Separate ORC_204 from ORC_212 on the reused restraint route so controller identity selects the correct catalogue rather than route coincidence.
- Expand the exact CRD3 catalogue with independently documented CRD3.CBF reads 0x1001, 0x1002, 0xF804, 0xF806 and 0x2007 while retaining its exact global documented profile reads.
- Preserve EGS53 as its own family: expose its documented KWP profile, family-owned read identifiers and individually selectable 0x21/0x30 actual-value signals without borrowing EGS52 or VGS/NAG2 records.
- Add whole-C207 completeness regressions across all 13 captured controller routes, proving every exact-profile safe read and every source-corroborated controller-data entry reaches PID Setup while capture-only unknown observations remain unadvertised.
- Remove temporary source-discovery probes after confirming the remaining RBTM/CRD3/EGS53 CBF binaries are not present in the pinned W204 archive; no neighbouring-controller data is substituted for unavailable exact definitions.

## 0.7.254 — 2026-09-30

- Complete the current Mercedes controller-catalogue population pass by exposing every safe source-backed read from the exact identified ECU profile in PID Setup, without filtering slow-changing, static-looking, identification or configuration reads during this completeness phase.
- Use the richest exact source profile available for the captured C207 controller families, add CRD3 to exact-profile resolution, and regress all 13 captured controller routes so every documented safe read reaches the selectable ECU pack.
- Retain semantic kinds for later UI grouping while keeping capture-only unknown identifiers explicitly raw and unadvertised; vehicle response still never creates or removes catalogue membership.
- Keep the generated pinned-CBF Data catalogues attached alongside the global 1,319-profile / 4,355-read Mercedes source catalogue, with exact-controller and protocol matching remaining authoritative.
- Preserve diagnostic service together with identifier throughout iPhone manufacturer polling, so mixed KWP2000 `1A xx` and `21 xx` selections cannot collapse into the wrong wire command; selected commands remain de-duplicated before scheduling.
- Add catalogue-completeness, exact-family and service-aware polling regressions across portable C11, sanitizers, GTK, Windows and the iOS simulator flow.

## 0.7.253 — 2026-09-29

- Make the iPhone Mercedes PID catalogue purely documentation-driven: exact-ECU online actual-value definitions determine catalogue membership, while vehicle response, NO DATA, capture-only observations, and desktop scanner repeatability never add, remove or hide a PID.
- Remove the temporary pollable/live-only/partial-catalogue gates, keep identity/configuration/session/DTC records out of PID Setup, and retain capture-only identifiers as raw diagnostic evidence rather than invented live values.
- Import source-backed Vediamo CBF actual-value services for CGW_212, MPC212 and FSCM212 so documented gateway, camera and fuel-pump data is visible even before a particular vehicle answers it.
- Complete EGS53 RLI 0x30 live presentation for source-validated speeds, pressure and signed 16-bit engine/converter torque, eliminating the 0xFFxx-to-65k unsigned wrap while preserving raw evidence.
- Restore explicit UDS/KWP diagnostic-session teardown after Mercedes identification and manufacturer reads, including HU_204/COMAND and EGS paths, while avoiding redundant identity probes on resolved cached reconnects.
- Align cold-launch/simulator acceptance with the documentation-only catalogue contract: a resolved ESP or ORC is valid with zero selectable PIDs when no online actual-value definition exists, while saved vehicle/module state and user selections must still restore correctly.

## 0.7.252 — 2026-09-29

- Complete the remaining source-backed EGS51/EGS52 pedal semantics: the documented 0..250 pedal scale is now presented as 0..100%, while out-of-range values are treated as unavailable instead of plausible percentages.
- Add family-local EGS52 derived TCC request semantics with the upstream open-over-slip precedence available to stateful callers, while keeping the individual open/slip request bits visible in passive frame output.
- Surface EGS52 ESP and cruise torque intervention states alongside their already-decoded torque demands in Nm.
- Normalize the EGS53 TCC request enum into None/Open/Slipping/Unavailable while retaining the original source enum.
- Add regression coverage for pedal scaling/range validation, EGS52 TCC precedence, ESP/cruise derived states and EGS53 TCC normalization.

## 0.7.251 — 2026-09-29

- Make the resolved Mercedes ECU definition pack authoritative for UDS/KWP2000 communication once controller identity is known; stale or route-level protocol guesses can no longer override a positively resolved family.
- Treat physical routes as discovery hints rather than controller identity. Reused addresses may carry different ECU generations/protocols, so route-only family guesses are rejected when their documented protocol conflicts with the responding ECU.
- For unresolved routes, try only documented alternate protocol variants and only during bounded discovery; once a pack resolves, all later reads, DTC work, PID Setup and Factory Readings use the pack's protocol.
- Add regression coverage for IC_204 protocol authority, reused 0x60A -> 0x481 UDS/KWP variants, cached-profile correction and protocol-safe discovery, while removing redundant route-profile counting/shortcut code.

## 0.7.250 — 2026-09-28

- Introduce a unified Mercedes ECU definition pack that presents controller key, friendly ECU/module names, component/network, physical TX/RX lookup, resolved protocol/session metadata, aliases, named data items, evidence status and field metadata through one API.
- Move the Apple manufacturer PID catalogue and Factory Readings onto the same ECU pack so the UI and read engine no longer reconstruct controller capability independently from several tables.
- Preserve raw vehicle-positive identifiers inside the pack as explicitly unadvertised raw observations, while keeping source-backed live values and documented reads distinct.
- Add pack-level regression coverage for IC_204, EGS53 grouped 21 30 signals and the ESP raw-observation safety gate, plus architecture documentation and CI invariants.

## 0.7.249 — 2026-09-28

- Correct IC_204 at 0x60A -> 0x481 to its source-backed HSCAN_UDS_500 / UDS diagnostic protocol instead of allowing the route to fall back to KWP2000.
- Attach the existing nine documented IC_204 UDS reads to the instrument-cluster catalogue, while keeping them as documented factory reads rather than inventing live PID semantics.
- Override stale cached KWP2000 protocol state for IC_204, preserve read-only discovery without an invented VIN DID, and add regression coverage for protocol selection, catalogue attachment and mixed-protocol fallback on still-unknown routes.

## 0.7.248 — 2026-09-28

- Make the CI diagnostics reporter recover automatically when its previous tracking issue has been deleted instead of failing an otherwise-green release.
- On failures, reuse an existing diagnostics issue when available or create a fresh one; on success, close any open matching diagnostics issue and succeed cleanly when none exists.
- Carry forward the 0.7.247 ECU catalogue hardening, stale-cache quarantine and cold-launch validation unchanged.

## 0.7.247 — 2026-09-28

- Publish the completed documented ECU PID catalogue wiring and corrected SCCM_212 family aliasing from the post-0.7.246 mainline work.
- Keep response-only Mercedes identifiers out of selectable live PID setup unless their semantics are source-corroborated.
- Correct the iOS simulated-flow release gate so the deliberately empty ESP selectable catalogue is required to remain empty instead of being misreported as a failed live-ready state.
- Treat a resolved ECU's current source-backed definitions as authoritative even when the selectable live set is empty, and move manufacturer catalogue caching to a fresh namespace so old response-only entries cannot reappear after relaunch.
- Prune saved manufacturer selections that no longer exist in the authoritative catalogue and make the cold-launch regression measure the same selectable catalogue shown by PID Setup.

## 0.7.246 — 2026-09-28

- Apply the 39-minute C207 road-capture findings to live manufacturer polling: UDS background refresh is now deny-by-default and enabled only for explicitly qualified runtime-changing routes/identifiers.
- Keep the static 0x602/0x480 F1xx identity/configuration records out of the live scheduler, while retaining them for startup/manual inspection.
- Demote ABR2XT DIDs 0x2003, 0x2009 and 0x20C0 to manual-only after the road capture showed they were overwhelmingly state-gated/NO DATA; retain 0x2001, 0x2004, 0x2007 and 0x200D as the qualified runtime set.
- Stop scheduled 11-bit Mercedes live reads from replaying the full ELM initializer/ATZ on every cycle; restore the functional CAN header/filter and return directly to LINK's live scheduler instead.
- Detect late positive responses carrying the wrong UDS/KWP identifier, never associate them with the current request, and retry the current identifier before advancing.

## 0.7.245 — 2026-09-28

- Route every Apple module-discovery entry through one per-connection startup gate, including the retained legacy engine-probe completion path.
- Prevent any future reuse of that legacy path from bypassing the 0.7.244 startup-only ECU-census invariant.

## 0.7.244 — 2026-09-28

- Make Mercedes module census and saved-profile validation strictly startup-only: one module-identification pass per connection before live polling begins.
- Remove the 250 ms high-priority live-scheduler module-followup job that could resume an interrupted startup census after the shared flow had already entered live mode.
- Remove live-sample-triggered late transmission identification so ordinary PID samples can never re-arm ECU discovery.
- Preserve partial startup module/fault evidence after an interruption and defer another topology validation until the next connection instead of polling the vehicle's ECU map again in live mode.
- Keep recurring Mercedes scheduler work limited to explicitly selected manufacturer live-data identifiers; module discovery itself is never a recurring job.

## 0.7.243 — 2026-09-28

- Separate historical evidence that a Mercedes ECU has accepted UDS `10 03` from MBLINK's permission to enter that session automatically.
- Permanently block unattended extended-session entry on the C207 ABR/ESP `0x632 → 0x486` route during first-VIN discovery, cached module/fault refresh and manufacturer live-data refresh; ESP reads now remain in the default diagnostic session.
- Add C207 safety regressions that preserve the captured positive-session evidence while requiring the automatic ESP wire sequence to proceed from route setup directly to TesterPresent, with no intervening `10 03`.

## 0.7.242 — 2026-09-28

- Decode the vehicle-proven Siemens EGS53 KWP RLI 0x30 path using the exact identified controller family instead of losing the EGS53 classification in the iOS live-value layer.
- Present proven EGS53 semantics from the captured 0x21/0x30 record, including TCC state, selector position, drive program, recognised gear, actual/target gear and ATF temperature, while leaving unproven pressure/speed/torque conversions conservative.
- Import source-backed EGS51, EGS52 and EGS53 CAN definitions as three strictly separate lookups: 11 frames/146 signals, 121 frames/528 signals and 95 frames/602 signals respectively, preserving family-specific enums, scaling, duplicate frame candidates and composite masks.
- Correct the legacy passive EGS CAN decoders to consume Mercedes CAN bytes in documented wire order rather than the upstream generator's internal little-endian union order.
- Fix the transmission regression-test helper contract that made Linux C11 and ASan/UBSan builds fail under -Werror after the wire-order tests were added.

## 0.7.241 — 2026-09-28

- Make the 1,319-profile global Mercedes ECU catalogue route-first as well as family-aware: every discovered exact TX/RX route can now supply its documented safe read-only command set even before ECU identity resolves one controller generation.
- De-duplicate documented 0x22/0x21/0x1A reads across controller generations sharing a diagnostic route and filter them by the detected protocol; positive replies become vehicle evidence while route coincidence remains only candidate evidence.
- Wire the route-wide catalogue into iOS Factory Data discovery and definition presentation, closing the gap where the database existed but only hand-mapped controller-family aliases could activate it.
- Add regressions for the global route unions, including 0x602/0x480, 0x7E0/0x7E8 and 0x7E1/0x7E9.

## 0.7.240 — 2026-09-28

- Add a global source-backed Mercedes ECU knowledge layer: 1,319 documented controller profiles, 1,107 explicit CAN routes and 4,355 simple read-only 0x22/0x21/0x1A references normalized from public Foxwell/Xentry-derived metadata.
- Preserve route ambiguity: shared CAN routes can enumerate several controller generations; ECU identity/part-number evidence selects the exact family before family-specific documentation is applied.
- Add GPL-3.0 detailed field-schema provenance from laravelcompany/ecudocs.com and portable byte/bit-location metadata for common Daimler UDS and KWP identity records.
- Extend manufacturer-data probing to explicit service+identifier command lists, including KWP 0x1A, while rejecting security, routine, IO-control and write/programming services before they reach the adapter.
- Feed exact classified controller families into the global documented profile in the iOS module-data view and probe plan. The captured EGS53 therefore selects the public EGS53 command set rather than inheriting generic 7E1/7E9 assumptions.
- Add regression coverage for all 13 captured C207 responder routes, route ambiguity, EGS53 family selection, SCCM field locations and mixed KWP 0x1A/0x21 reads.

## 0.7.239 — 2026-09-28

- Keep a fixed control-unit identity panel for every discovered ECU: CAN route, protocol, designation, network, ECU identity, part number, software version and hardware version are always visible.
- Show `N/A` when an ECU did not report an identity field instead of hiding that row, so missing evidence is explicit and module detail layouts remain directly comparable.
- Preserve every decoded value when the ECU does report it; the captured EGS53 therefore exposes its KWP identity, Mercedes part number, software version and hardware version together in the module detail view.

## 0.7.238 — 2026-09-28

- Replace the C207 capture's presentation-only "Likely" ECU hints with structured route identities that feed the real module catalogue, module kind, controller family, component designation and saved VIN profile.
- Resolve the captured 0x60A/0x481 route as IC_204 instrument cluster, 0x622/0x484 as SCCM/SCM steering-column controller, 0x6A2/0x494 as MFK A40/11 multifunction camera, 0x6BA/0x497 and 0x6C2/0x498 as RBTMFL/RBTMFR PRE-SAFE tensioners, and 0x6FA/0x49F as N118/FSCU fuel-pump control. Preserve 0x602/0x480 as a candidate central-gateway match because Mercedes reuses 0x602 on other families.
- Promote source-backed exact routes already in the scanner to their useful family names: EIS/EZS_212, ABR2XT, ORC_212 and HU_204. Keep ECU-returned identity/part-number evidence authoritative, including the EGS53 A0034464310 classifier from 0.7.237.
- Correct the model-207 ORC component designation to N2/10 and add structured A40/11 and N118 module/controller definitions.
- Register OSUSecLab/CANHunter as a standing Mercedes semantic research source in the machine-readable source inventory and project research documentation; route matches retain provenance and model-family ambiguity rules.
- Rehydrate older saved VIN profiles through the new route classifier so existing profiles gain the improved labels without requiring a fresh vehicle census.

## 0.7.237 — 2026-09-28

- Promote a generic 0x7E1/0x7E9 Mercedes transmission controller to the exact EGS53 family when its captured Daimler KWP identification supplies corporate part number `0034464310` / `A 003 446 43 10`.
- Feed parsed ECU spare-part numbers into controller-family classification after textual identity, software and hardware evidence, so numeric KWP identities no longer hide a source-corroborated family.
- Add a regression using the exact `1A87` payload captured from the C207 session and require the UI-facing module name to become `EGS53 transmission ECU`.

## 0.7.236 — 2026-09-28

- Advance the exact shared LINK dependency to 0.15.78 at `090981200402518465beb5ae714c5a1ccdc601b6`, bringing automatic ELM327 resynchronisation/retry to standard diagnostic phases and adding timeout/power-state lifecycle evidence for the iOS failure captured by MBLINK 0.7.203.
- Keep every responding controller visible during connection progress by showing its current MBLINK name and exact request/response address instead of reporting only a retained-module count.
- Keep source-backed Mercedes route identities authoritative, including EIS/EZS, ABR2XT/ESP, ORC_212, HU_204, engine and GS/VGS/EGS mappings.
- Add presentation-only online candidates for the observed 0x622/0x484 steering-column, 0x6A2/0x494 multifunction-camera, 0x6BA/0x497 and 0x6C2/0x498 reversible-tensioner, and 0x6FA/0x49F fuel-pump routes. These remain explicitly labelled "Likely" until C207-specific identity evidence confirms them; conflicting 0x60A evidence remains unresolved.

## 0.7.235 — 2026-09-27

- Recover Mercedes engine and module discovery when LINK refuses to queue a diagnostic command: stop the pending scan, preserve captured responses and restore normal diagnostics instead of leaving the app stuck scanning.
- Show the actual Mercedes module discovery status on empty Live Data and Factory Readings screens so failures and delays are visible.
- Serialize queued factory reads with recurring Mercedes live jobs, including the period before a manual read acquires the diagnostic channel.

## 0.7.234 — 2026-09-27

- Make the iOS start flow clearer: show supported starter OBD readings, connection state and available live data without implying the Mercedes research catalogue is a working ECU integration.
- Add a Factory Readings screen driven by actual captured module responses, with module data reads and links to full response evidence. Keep unverified Mercedes identifiers in the separate Factory Reference catalogue.
- Update the LINK Bluetooth picker with explicit scan and connection states and separate BLE from Classic adapter behavior. Pin the shared LINK source at `eb360c6`.
- Qualify the portable diagnostic core, iOS simulator build and simulated diagnostic launch before release. Real vehicle access, protected Mercedes export decoding and physical adapter behavior require separate validation.

## 0.7.233 — 2026-09-24

- Advance the exact shared LINK dependency from 0.15.62 to released LINK 0.15.69 at `ca30258e0684b77689128b8a538891c79c2f876c`.
- Consume the latest shared Linux publisher-alignment work, including 12 px ordinary cards, without changing Mercedes diagnostic, transport or safety policy.
- Keep LINK as the sole Common authority and preserve MBLINK's existing dependencies and platform architecture.


## 0.7.232 — 2026-09-23

- Advance the exact shared LINK dependency from 0.15.61 to released LINK 0.15.62 at `0f2cc710fb8f211013cd5f55df390fe0911886da`.
- Retain LINK's exact nested Infiltratr Common 1.19.24 pin and consume the guarded STM32 UDS RX fallback added for LINK #47 without weakening MBLINK diagnostic or safety policy.
- Synchronise both iOS LINK provenance definitions and every product version surface with the new gitlink.


## 0.7.231 — 2026-09-23

- Advance the exact shared LINK dependency from 0.15.60 to 0.15.61 at `2e212f67e5a664330d432a6798350202be54eb10`.
- Consume LINK's exact nested Infiltratr Common 1.19.24 pin at `748e089ae175329471d4cf375522c44081371bd5`, replacing the now-stale Common 1.19.23 dependency chain without adding a second product-level Common authority.
- Synchronise both iOS LINK provenance definitions and the product version metadata with the new gitlink; no MBLINK diagnostic, safety or transport policy is weakened by this dependency-only release.

## 0.7.230 — 2026-09-22

- Advance the exact shared LINK dependency to 0.15.60 at `a42d380c27f1bf1612216b2345abadb72e3ba36a`, retaining the Common 1.19.23 dependency chain and synchronising both iOS LINK provenance definitions to the same gitlink.
- Consume LINK's completed STM32F103 workbook Freeze Frame Snapshot Record support: exact Sheet 7 record 0x01 framing for DF00/DF01/DF02/DF03/DF04/DD00, target-owned freeze-frame values, strict missing-record rejection and preserved one-page STM32F103 journal footprint.
- Qualify the complete MBLINK OTA bootloader control flow with a product-level backend regression covering fail-closed arming, ProgrammingSession, SecurityAccess, DTC/communication quiesce, inactive-slot download, TransferData sequencing, TransferExit, integrity/authenticity, secure-boot staging, anti-rollback and post-boot commit.
- Keep MBLINK #56 open for the real STM32F767 target backend: no board flash map, protected monotonic store, production signing/HSM material or secure-boot handoff is fabricated where the target evidence has not been supplied.

## 0.7.229 — 2026-09-22

- Correct the iOS LINK source-revision metadata to the actual 0.15.55 gitlink `0caaf4026dcf807397649cd27a38f25ec3be2934`, resolving the preflight failure that made 0.7.228 red before compilation.
- Carry forward the completed typed 0x29 Authentication facade/tests, issue #37 clear/session regression, exact STM32C092 issue-27 Cube-main qualification, tester/server naming repair and 35-minute asynchronous APT verification window.
- Requalify the complete MBLINK stack against the current LINK/Common chain before publication.

## 0.7.228 — 2026-09-22

- Advance MBLINK to LINK 0.15.55 at `0caaf4026dcf807397649cd27a38f25ec3be2934`, retaining the Common 1.19.23 pin and corrected dependency metadata.

## 0.7.227 — 2026-09-22

- Advance MBLINK to the canonical LINK 0.15.54 release at `38826aa2dca78072343a201553460e61cc2f726f`, which pins Infiltratr Common 1.19.23.
- Preserve the qualified Mercedes-specific diagnostics surface while consuming the newest shared LINK/Common implementation and provenance.

## 0.7.226 — 2026-09-22

- Replace the red LINK 0.15.52 dependency from MBLINK 0.7.225 with the exact corrected LINK 0.15.53 release commit `0c1d6b5367b7bb05596ddd46b23151ccfb9a19b2`.
- Carry forward the completed typed 0x29 Authentication facade/tests, DTC variable-record facade, fail-closed OTA core and 35-minute asynchronous APT verification repair.
- Qualify the corrected LINK DTC lifecycle engine, issue #36 tester/server naming, issue #37 clear/session semantics and exact STM32C092 issue-27 Cube-main compile through MBLINK's own cross-platform matrix.

## 0.7.225 — 2026-09-22

- Advance the exact LINK dependency to 0.15.52 so MBLINK qualifies the final STM32 tester/server clarification, issue #37 clear-DTC regression, exact issue-27 Cube integration check and issue #38 DTC lifecycle implementation together.
- Retain 0.7.224's typed 0x29 Authentication facade/tests and the widened central APT publication verification window; no product-local duplicate of LINK's DTC lifecycle is introduced.
- Inherit LINK's portable FunctionalGroupIdentifier/FDC/confirmation/aging reference engine for ECU-side examples while keeping MBLINK's diagnostic-client boundaries unchanged.

## 0.7.224 — 2026-09-22

- Advance the exact LINK dependency to 0.15.51, including the STM32 tester/server role clarification, issue #37 DTC-clear regression coverage and exact issue-27 Cube-main CI qualification.
- Complete MBLINK issue #63's generic product facade by re-exporting LINK's typed ISO 14229-1:2020 Authentication codec for all nine 0x29 tasks and adding MBLINK-level builder/decoder regression coverage.
- Fix the publication-infrastructure regression identified against issue #24: central APT refresh is asynchronous, so verification now permits 35 minutes instead of racing the repository/Pages propagation boundary at 20 minutes.
- Keep Authentication trust, certificates, private keys and Mercedes/OEM policy evidence-gated; no security bypass or unproven manufacturer algorithm is introduced.

## 0.7.223 — 2026-09-22

- Advance the exact LINK dependency to 0.15.50, which in turn pins Infiltratr Common 1.19.22, keeping the vehicle stack on one tested shared dependency chain.
- Synchronise the iOS embedded LINK provenance with the exact `src/link` gitlink so release builds report the code they actually contain.

## 0.7.222 — 2026-09-22

- Expose LINK 0.15.49's fail-closed UDS OTA/bootloader core directly through an MBLINK facade for issue #56 instead of leaving the capability hidden in the submodule.
- Add a product-level regression proving the default configuration cannot arm programming and requires security, quiesce, authenticity and secure-boot validation.
- Document the STM32F767 integration boundary: existing LINK bxCAN transport plus target-owned inactive-slot flash, integrity/authenticity/HSM, secure-boot and protected anti-rollback storage callbacks.

## 0.7.221 — 2026-09-22

- Complete MBLINK issue #59's generic DTC snapshot/stored/extended-data surface by advancing the exact LINK dependency to 0.15.49.
- Re-export LINK's typed variable-record APIs: DID-length resolver, DID values, snapshot/stored-data record views and extended-data record views.
- Add product-level regression coverage that decodes snapshot and stored-data DID/value sequences, extended-data records and the unknown-DID rejection path rather than leaving the newer LINK capability untested behind the facade.
- Inherit LINK's fail-closed UDS OTA/bootloader core for issue #56 while keeping all active programming disabled unless an explicit target backend is provided and armed.

## 0.7.220 — 2026-09-21

- Complete issue #29's current evidence tranche without inventing Mercedes meanings: all supplied C207 fault evidence remains regression represented and D18100/50 stays explicitly unknown.
- Add a typed module-scoped 24-bit UDS DTC definition contract parallel to the existing KWP contract so future proven raw UDS meanings have a safe destination.
- Add eight primary-documented CDID3/OM651 records from Mercedes-Benz/Daimler XENTRY bulletins, preserving CRD3/CRD3NFZ controller and vehicle-family applicability rather than promoting Sprinter, W212 hybrid or 117/176/246 meanings into C207.
- Keep the existing 299-row supplied reference corpus unchanged and separate from automatic wire-level resolution.
- Advance the shared LINK dependency to 0.15.42 with generic AES-CMAC and algorithm-neutral SecurityAccess support; no Mercedes seed/key algorithm is inferred or enabled.

## 0.7.219 — 2026-09-21

- Complete CI diagnostics issue #24 by restoring one canonical 0.7.219 version across VERSION, C header metadata and all iOS MARKETING_VERSION configurations.
- Pin LINK 0.15.41 exactly and record the matching LINK source revision in both iOS core configurations.
- Consume LINK issue #25's product-neutral UDS server policy metadata and contextual physical/functional dispatcher without adding any Mercedes-specific policy guesses.
- Requalify the complete Linux, Windows, sanitizer, Apple/iOS and publication pipeline from a clean preflight state.

## 0.7.218 — 2026-09-21

- Advance to LINK 0.15.40 so MBLINK's Linux, Windows where applicable, and iOS About surfaces share the completed suite-wide About contract.
- Retain the previously separated build identity and product-specific credits while inheriting the corrected shared Windows and standard SwiftUI metadata handling.


## 0.7.217 — 2026-09-21

- Standardise Linux and iOS About presentation on the suite-wide System Monitor contract through LINK 0.15.38.
- Separate the canonical build label from descriptive text so About exposes the same product/version/description/build hierarchy as the rest of the suite.
- Remove product-tagline duplication from About while retaining MBLINK branding in the application shell.


This file records user-visible, compatibility, diagnostic-knowledge and validation changes for MBLINK.

## Unreleased

- No unreleased changes.

## 0.7.216 — 2026-09-21

- Exposed LINK's forensic 27-service UDS catalogue and complete requested ReadDTCInformation report catalogue through MBLINK without duplicating generic protocol ownership.
- Advanced the exact LINK dependency to 0.15.38, including request-aware ReadDTCInformation response validation for MemorySelection, record-number, functional-group and applicable DTC echoes.
- Re-exported the request-aware ReadDTC decoder through the MBLINK compatibility facade and retained typed fixed-record views while leaving OEM-sized snapshot and extended-data tails raw.
- Preserved deny-by-default vehicle policy: ClearDiagnosticInformation 0x14 remains state-changing and unavailable to automatic discovery; Authentication 0x29 remains security-gated and has no inferred Mercedes procedure.
- Added forensic issue documentation for the supplied STM32 server deviations that caused misleading 0x19 differential results.

## 0.7.215 — 2026-09-21

- Completed a direct Common 1.19.20 forensic reuse pass in MBLINK without changing Common and without changing MBLINK's LINK dependency.
- Replaced the embedded STM32 console's private ASCII lowercase helper with Common's deterministic ASCII case conversion while preserving the console's deliberately narrow space/tab trimming grammar.
- Replaced manual guarded unsigned timestamp subtraction in Mercedes signal-correlation lag handling with Common's checked uint64 subtraction contract.
- Deliberately retained Mercedes-specific printable-text validation, variable-width decoding, RGB presentation extraction and other local code where Common does not provide an exact semantic improvement.

## 0.7.213 — 2026-09-21

- Advanced the exact shared engine to LINK 0.15.34 while retaining Infiltratr Common 1.19.20 unchanged.
- Reused Common's canonical monotonic clock in the native Linux MBLINK face instead of retaining a second GLib time wrapper.
- Removed the remaining locale-sensitive ctype use from the C207 replay command normaliser and DID-lab byte parser in favour of Common's deterministic ASCII contracts.
- Removed an obsolete ctype include from Mercedes data-scan code; Mercedes diagnostic semantics and evidence remain MBLINK-owned.
- No code was added to Common.

## 0.7.212 — 2026-09-21

- Advanced the exact shared engine to LINK 0.15.33 and therefore Infiltratr Common 1.19.20.
- Removed Mercedes-local ASCII case-folding/classification helpers where Common now provides the exact locale-independent contract for VIN, ECU identity, CRD3 family and DTC-reference text.
- Moved Linux preference path resolution and directory creation onto Common's XDG/POSIX helpers while retaining the MBLINK-owned INI schema.
- Publishes Linux preference bytes through Common's durable atomic-file writer instead of GTK/GLib filesystem policy; existing file permissions are preserved.
- Kept Mercedes diagnostic semantics, manufacturer evidence and MBLINK presentation policy local; no code was added to Common.

## 0.7.211 — 2026-09-20

- Advanced the exact shared engine to LINK 0.15.32, retaining the Vehicle Research implementation and Infiltratr Common 1.19.10.
- Published the repository-wide 2000-2026 copyright normalization already present on main.
- No diagnostic protocol, transport, safety, Mercedes evidence, polling or user-facing research behaviour changed from 0.7.210.

## 0.7.210 — 2026-09-20

- Advanced the exact shared automotive engine to LINK 0.15.31 while retaining Infiltratr Common 1.19.10.
- Pulled LINK's portable Vehicle Research session state and typed research evidence timeline into MBLINK without duplicating the generic implementation.
- Upgraded MBLINK Discover on Windows with shared passive-capture, standards-inventory and Mercedes FULL SWEEP phase tracking, operator event markers and Vehicle Research JSONL export summaries.
- Preserved the existing Mercedes-owned sweep plan and deny-by-default write/control boundary; the research workflow remains read-oriented.

## 0.7.209 — 2026-09-20

- Advanced the exact shared automotive engine to LINK 0.15.30 and therefore Infiltratr Common 1.19.10.
- Replaced the duplicated Linux Night palette, shared typography names/weights and matching design metrics in MBLINK's GTK face with Common 1.19.10's canonical native design contract while preserving the exact visible Night appearance.
- Kept only genuinely MBLINK-specific cockpit/trace gradients and Mercedes presentation details local; Common now supplies the product-neutral titlebar, connection, heading, summary, kicker, detail, note, status, accent and state-border roles.
- Reused Common's canonical MB Corpo CMake provenance for archive/file names and hashes instead of maintaining a second CMake copy of that metadata.
- Preserved the distinct iPhone and Windows product presentation contracts rather than mechanically replacing non-equivalent styling.

## 0.7.208 — 2026-09-19

- Advanced the exact shared engine to LINK 0.15.29 while retaining Infiltratr Common 1.19.8.
- Completed the MBLINK-side forensic reuse pass: Linux replay parsing, canonical array sizing, bounded string copies, checked timestamp arithmetic, Mercedes UDS endian access and embedded array sizing now use the corresponding Common contracts where those contracts exactly match the existing behaviour.
- Switched the Apple standard OBD-II scalar text path to LINK's shared deterministic formatter so iPhone and Linux no longer reconstruct the same unit/precision policy independently.
- Left Mercedes-specific variable-width decoding, CAN identifier composition, protocol state and evidence logic in MBLINK where Common/LINK do not provide a semantically equivalent contract.

## 0.7.207 — 2026-09-19

- Upgraded the exact shared engine dependency to LINK 0.15.28 and Infiltratr Common 1.19.8.
- Replaced Linux product-local OBD-II value, decoded-PID and fuel-economy formatting with LINK's shared preference-aware presentation contracts.
- Reused Common's strict ranged integer parser, checked dynamic-array growth helper and canonical array-length primitive in MBLINK-owned code.
- Moved the Mercedes identity-first module scanner out of multi-thousand-line public inline headers into a normal MBLINK translation unit while retaining the existing API and state-machine behaviour.
- Kept the portable diagnostic core C11-first; no C++ runtime or duplicate OO layer was introduced.


## 0.7.206 — 2026-09-19

- Replaced the Linux Dashboard navigation artwork with a high-contrast four-panel glyph that remains legible at sidebar size.

## 0.7.205 — 2026-09-19

- Fixed DID-lab unsigned parsing so negative timestamps and tolerances are rejected, and made CSV floating-point parsing use Common's strict finite parser.
- Removed signed overflow from extreme signal-correlation lag stepping and added boundary regression coverage.
- Reused Common's alignment-safe endian readers in Mercedes transmission, DiagLogic and data-scan decoding.
- Upgraded the exact shared engine dependency to LINK 0.15.25, which in turn pins Infiltratr Common 1.19.3.
- Linux live-data profiles now start with every real-vehicle channel OFF and store standard selections per VIN rather than globally.
- Linux Dashboard, Graphs and Table now follow the same explicit selected-channel model; selected graph channels remain visible while awaiting their first sample.
- Removed the old fixed eight-graph product limit and use LINK's runtime-selected graph configuration.
- Reused LINK's shared ELM327 CAN address formatting in Mercedes scanners instead of maintaining parallel ATSH/ATCRA formatters.
- Canonical documentation baseline aligned with the Infiltrator project family.

## Policy

Record new supported diagnostic behaviour, changed safety/permission boundaries, corrected definitions, platform changes, dependency revisions that alter behaviour and material fixes. Raw research activity belongs in the relevant evidence document until it changes supported product behaviour.

## Historical identity

Git tags and GitHub Releases remain authoritative for exact historical source and dependency identity.
