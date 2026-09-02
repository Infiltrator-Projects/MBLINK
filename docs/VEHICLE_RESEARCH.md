<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Vehicle Research and Discovery Architecture

This document defines the product boundary and workflow for learning unknown
vehicle/module data without depending on proprietary diagnostic databases.

The core rule is:

> **Desktop is the laboratory. iPhone is the diagnostic instrument.**

MBLINK must support Mercedes-Benz vehicles broadly without turning the normal phone
application into an exhaustive reverse-engineering scanner. The current C207/OM651
vehicle is one development fixture and evidence source, not the research-product boundary.

## Product boundary

### Linux and Windows: research / discovery

The Linux and Windows MBLINK Discover applications own the heavy vehicle
research workflow.

They are expected to support long-running, evidence-oriented work including:

- Tactrix OpenPort / J2534 access where available;
- ELM327/Vgate as a lower-bandwidth fallback where appropriate;
- passive CAN observation before active probing;
- 11-bit and 29-bit diagnostic-route discovery;
- standard OBD-II capability/VIN/DTC/reference acquisition;
- UDS and KWP module identification;
- bounded read-only manufacturer-data harvesting;
- repeated sampling of positive unknown identifiers;
- guided operator events such as idle, RPM changes, pedal movement, steering,
  gear selection and controlled driving states;
- correlation of unknown values with known SAE/reference channels;
- passive-CAN signal-change analysis;
- structured evidence export;
- offline analysis and candidate promotion.

The desktop tools are therefore the place where a previously unknown vehicle
is *learnt*.

### iPhone: normal diagnostics

The iPhone application is not the primary exhaustive discovery environment.

It should remain fast and predictable:

- connect and identify the VIN;
- load and validate a saved vehicle/module profile;
- perform normal standards-defined OBD-II diagnostics;
- read faults, readiness and freeze-frame data;
- display known Mercedes factory values;
- refresh already-known manufacturer identifiers;
- show gauges, tables and graphs;
- export diagnostic evidence.

A first connection may still perform the bounded module census needed to build
a usable VIN profile, but iPhone must not evolve into the primary brute-force
DID/KWP research scanner.

The iPhone **Factory data** action is therefore a diagnostic read/refresh
surface. Its long-term role is to read identifiers already known for that
vehicle/module or identifiers that have been promoted into the product
catalogue. Exhaustive unknown-identifier harvesting belongs on desktop.

## Why discovery is a separate workflow

There is no universal UDS/KWP request that returns every sensor together with
its name, unit, byte layout and formula.

Vehicle research has three separate problems:

```text
find modules
    ->
find readable values
    ->
determine what those values mean
```

A positive response proves that a value exists. It does not prove its physical
meaning or scaling.

Some useful vehicle signals may not be diagnostic DIDs/local identifiers at
all. They may be periodic broadcast CAN signals. Research therefore needs both
active diagnostic acquisition and passive bus observation.

## Research-session pipeline

A desktop Research Session should progress through explicit phases.

### 1. Passive network observation

Before transmitting discovery traffic, capture a bounded period of vehicle bus
activity.

Record, where the adapter exposes it:

- CAN bitrate;
- 11-bit / 29-bit identifier;
- timestamp;
- DLC / payload;
- message rate;
- changing-byte masks;
- bus/channel identity.

Passive traffic is evidence only. A CAN ID must not be labelled as a component
or signal solely because its bytes change.

### 2. Standards baseline

Collect the standards-defined information first.

This provides both useful diagnostics and trusted reference channels:

- supported SAE J1979 PIDs;
- VIN;
- OBD DTCs;
- readiness;
- freeze-frame where applicable;
- RPM;
- speed;
- MAP;
- temperatures;
- accelerator channels;
- rail pressure;
- battery voltage;
- other advertised Mode 01 values.

These reference channels become ground truth for later correlation.

### 3. Module census

Build a physical diagnostic map of responding control units.

Each responder record should retain:

```text
network / bus
request CAN ID
response CAN ID
11-bit / 29-bit
protocol
diagnostic-session behaviour
system / ECU identity where readable
part number
software number
hardware number
DTC-read result
raw request/response evidence
```

A valid positive *or valid negative* diagnostic response can prove that a
controller exists. Silence does not.

Mercedes-specific route definitions belong in MBLINK. Generic target walking,
transport, ISO-TP and result handling belong in LINK.

### 4. Read-only manufacturer-data harvest

Only after a diagnostic route has been established should Research attempt
manufacturer-data acquisition.

Primary read services are:

- UDS `ReadDataByIdentifier (0x22)`;
- KWP2000 `ReadDataByLocalIdentifier (0x21)`;
- documented/read-only protocol equivalents where later manufacturers require
  them.

Unknown positive values are retained exactly as returned.

Example:

```text
module: 0x7E0 -> 0x7E8
protocol: UDS
request: 22 20 37
response: 62 20 37 03 75
state: positive / unmapped
raw payload: 03 75
```

MBLINK must never discard a positive response because its meaning is unknown.

The first successful harvest should produce a module-scoped positive-identifier
set. Later reads target that set instead of repeating the whole discovery
range.

### 5. Guided signal experiment

Research should be able to record operator-labelled events while sampling
known and unknown values.

Useful events include:

- warm idle;
- fixed RPM hold;
- accelerator sweep;
- brake application;
- steering movement;
- headlights on/off;
- air-conditioning on/off;
- gear selection;
- steady-speed driving;
- controlled acceleration;
- coast/deceleration.

The operator event is a timestamped observation, not an automatic semantic
claim.

### 6. Correlation and candidate generation

Offline analysis compares unknown manufacturer values and passive-CAN fields
against trusted reference series and operator events.

Useful calculations include:

- paired-sample count;
- Pearson correlation;
- lag search;
- affine scale/offset fit;
- RMSE / normalized RMSE;
- monotonic similarity;
- range and rate-of-change comparison;
- discrete state-transition matching.

Examples:

- an unknown pressure-like value that tracks SAE MAP until the SAE 255 kPa
  ceiling and continues above it is a strong boost/manifold-pressure candidate;
- a value following SAE rail pressure after a fixed scale is a fuel-pressure
  candidate;
- a slow value that rises with engine warm-up is a temperature candidate;
- a discrete value that changes exactly with P/R/N/D is a transmission-state
  candidate.

Correlation supports a hypothesis. It does not by itself prove a DID meaning.

### 7. Passive-CAN signal learning

The same guided-event timeline should be usable against broadcast CAN traffic.

For every CAN ID/byte/bit field, Research can rank changes associated with an
operator event.

For example:

```text
brake applied
  -> rank fields that transition at the same timestamps

steering left/right
  -> rank signed/monotonic fields that track steering motion

gear P/R/N/D
  -> rank discrete fields whose state transitions match
```

Broadcast-signal candidates remain separate from diagnostic-DID candidates.

### 8. Vehicle Research Pack

Every Research Session should export one durable package containing enough raw
material for later analysis without returning to the vehicle.

The target contents are:

```text
vehicle/VIN fingerprint
adapter identity
bus inventory
module inventory
ECU identities
diagnostic routes
positive UDS/KWP identifiers
negative-response map
raw request/response transcript
SAE reference time series
manufacturer-value time series
passive CAN capture
operator event markers
correlation results
candidate scale/offset models
candidate semantic names
evidence/provenance metadata
```

The format must be deterministic and machine-readable. Large binary/passive
captures may be separate members inside the pack rather than embedded into a
single CSV.

## Evidence states

Research results use the existing MBLINK evidence boundary:

```text
corroborated-unmapped
source-backed-candidate
vehicle-verified
```

A newly discovered positive raw identifier is not automatically a
source-backed candidate.

A candidate can be strengthened by:

- repeated observations;
- correlation to a trusted reference;
- independent documentation;
- matching behaviour on multiple vehicles;
- exact request/response/formula evidence.

Only a reproducible, defensible mapping may become `vehicle-verified`.

## Adapter roles

### Tactrix OpenPort / J2534

OpenPort is the preferred desktop research adapter where supported because the
research workflow benefits from direct CAN/ISO-TP/J2534 access, higher
throughput, and passive traffic capture without depending on an ELM command
parser.

The OpenPort path should therefore become the primary implementation target for
desktop Research.

### ELM327 / Vgate

ELM327 remains useful for:

- normal user diagnostics;
- iPhone operation;
- compatibility testing;
- bounded diagnostic discovery;
- vehicles/adapters where OpenPort is unavailable.

It should not define the capability ceiling of the desktop research engine.

### Future adapters

LINK should expose the Research engine through reusable adapter/transport
interfaces so STM32 and later hardware can participate without duplicating
vehicle logic.

## Safety boundary

Research remains read-only by default.

The discovery/harvest allowlist must not include:

- SecurityAccess `0x27`;
- WriteDataByIdentifier `0x2E`;
- RoutineControl `0x31`;
- ECUReset `0x11`;
- ClearDiagnosticInformation `0x14`;
- InputOutputControlByIdentifier `0x2F`;
- coding;
- flashing/programming;
- persistent communication changes;
- persistent DTC-setting changes.

Transient diagnostic-session entry may be used only where required for
read-only identification/data acquisition and where the module/protocol path
has been reviewed.

The research tool must preserve negative responses, timeouts and unsupported
states instead of attempting increasingly invasive services.

## LINK versus MBLINK ownership

The reusable research machinery belongs in LINK:

- J2534/OpenPort transport;
- passive capture;
- ISO-TP;
- UDS/KWP request execution;
- target-plan iteration;
- raw-result models;
- sampling;
- event markers;
- correlation/statistics;
- Research Pack container/schema.

MBLINK owns Mercedes-specific knowledge:

- Mercedes route catalogues;
- ECU-family identities;
- Mercedes DID/KWP candidate ranges and known identifiers;
- Mercedes formulas/units;
- Mercedes source provenance;
- C207/OM651 evidence and regression fixtures.

This separation is required so the same Research engine can later underpin
JAGLINK and other manufacturer products.

## Immediate desktop implementation direction

The next desktop feature should evolve the existing MBLINK Discover product
rather than creating another repository or application.

The intended operator workflow is:

```text
Connect adapter
  -> Start Vehicle Research
  -> Passive Capture
  -> Standards Baseline
  -> Discover Modules
  -> Harvest Read-Only Data
  -> Guided Signal Test
  -> Analyse / Correlate
  -> Export Research Pack
```

Windows and Linux should expose the same research-state model and result
format, even if the platform adapter backends differ.

This is the primary path for teaching MBLINK about additional vehicles and
factory values. The normal iPhone application consumes the resulting verified
knowledge; it is not the laboratory that creates it.
