<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Security Policy

## Supported versions

Security fixes are applied to the current `main` branch and, where appropriate, the latest published release. Older pre-alpha releases should not be assumed to receive security backports.

## Reporting a vulnerability

Please do not open a public issue for a vulnerability that could expose users, vehicles, credentials, private data, build infrastructure or signing material to harm.

If GitHub private vulnerability reporting is available for this repository, use the repository's **Security** reporting flow. Otherwise, report the issue privately to `infiltratr@yandex.com` with the subject `MBLINK security report`.

Include enough information to reproduce and assess the problem where possible: affected version or commit, platform, adapter/vehicle context if relevant, impact, reproduction steps, logs or proof-of-concept material, and any suggested mitigation.

## Handling

Reports will be assessed before public disclosure. Please allow maintainers a reasonable opportunity to investigate and prepare a fix. Reporters are asked not to access data or systems beyond what is necessary to demonstrate the issue and not to perform testing that could endanger people, vehicles or third-party systems.

## Automotive scope

MBLINK is pre-alpha diagnostic software. A successful build or simulated protocol exchange does not prove safe behaviour on a real vehicle. Manufacturer-specific definitions remain unverified until their provenance and real-vehicle behaviour have been established and recorded by the project.
