<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Discover

MBLINK Discover is the MBLINK-branded face of the shared LINK Discover subsystem.

MBLINK does not own a separate discovery engine or Windows OpenPort/J2534 scanner implementation. Shared discovery behaviour, safety classification, evidence writing and the Windows scanner source live in LINK.

The MBLINK compatibility API is exposed through `include/mblink/discover.h`, which aliases the product-prefixed names to the shared `link_*` API without duplicating implementation.

On Windows, `mblink-discover.exe` is built by LINK's `link_add_windows_discover` constructor from the same `platform/windows/link-discover.c` used for JAGLINK. The MBLINK face supplies only the product identity (`MBLINK`, `mblink`, and the MBLINK window class).

The behavioural contract and safety model are documented in LINK's `docs/DISCOVER.md`. Any shared scanner or Discover improvement belongs in LINK, not in this repository.
