# Design

## First-principles position

MBLINK treats standards, captures, public documentation and mature diagnostic tools as evidence. Product behaviour is implemented from explicit contracts rather than by copying another tool or assuming undocumented manufacturer behaviour.

## Goals

- keep manufacturer knowledge evidence-backed
- preserve deny-by-default diagnostic safety
- avoid private copies of generic LINK behaviour
- make raw evidence and interpreted knowledge distinguishable

## Non-goals

The current C207/OM651 development vehicle is an evidence source, not the product boundary. Unknown Mercedes values are not guessed merely to make the UI look complete.

## Safety model

Read-only and write-capable actions are intentionally distinct. Capability discovery, protocol support and operator permission are not interchangeable. Unknown or failed scan states remain different from a clean result.

## Dependency policy

Generic automotive behaviour belongs in LINK; broadly reusable non-automotive mechanics belong in Common; manufacturer-specific behaviour belongs in the product face. Exact dependency revisions are pinned so later upstream changes cannot silently redefine a reviewed product.

## Evidence rule

A human-readable interpretation must be traceable to a standard, capture, verified public source or reproducible vehicle observation. Where evidence is incomplete, preserve raw values and uncertainty rather than inventing a label.

## Change quality

Newness is not a reason to replace a proven path. A change should improve fidelity, safety, coverage, performance or maintainability and include regression evidence for the contract it changes.
