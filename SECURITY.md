<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Security

## Supported source

Security fixes target current `main` and, where appropriate, the latest release.

## Reporting

Do not publish a vulnerability that could expose users, vehicles, credentials, private data, build infrastructure, adapter secrets or signing material in a public issue.

Use GitHub private vulnerability reporting when available. Otherwise contact `infiltratr@yandex.com` with the subject `MBLINK security report`.

Include the exact revision and recursive dependency identity, platform, adapter/vehicle context, impact, reproduction and sanitised evidence.

## Automotive security boundary

A successful build, unit test, simulator run or captured replay does not establish safe behaviour on every real vehicle. Particular care applies to transmit allowlists, security/session handling, diagnostic routing, malformed adapter input and any change capable of broadening a read-only path into a write/control path.

## Response

Treat security defects as correctness defects. Reproduce at the narrowest safe layer, add regression coverage where practical, repair the underlying safety contract and validate physical hardware when the defect crosses that boundary.

Do not perform testing that could endanger people, vehicles or third-party systems.

## Disclosure

Public disclosure should follow a fix or clear mitigation and identify the affected and corrected release/source identities.