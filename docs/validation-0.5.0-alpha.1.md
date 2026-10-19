# 0.5.0-alpha.1 — isolated build and local distribution

Local candidate; not accepted or published. Includes unaccepted 0.4 features.

Changes: optional BuildDirectory, clean-source x64 Release packaging, explicit
payload allowlist, metadata and SHA-256; support matrix and recovery/measurement
instructions. Native runtime behavior and schema are unchanged from 0.4 alpha.2.

Release build with `./scripts/build.ps1 -SkipTests -BuildDirectory build/local-distribution`
completed successfully on 2026-10-19; log: build/local-distribution-build.log.
The existing live process (PID 42112, build/Release/QuickCheat.exe) was left running.
No automated tests were added or run. Existing test/helper targets were compiled
by the unchanged CMake build graph, without execution.
Package inspection evidence is recorded separately in the version's Library page.
The build emitted 25 warnings: 24 in MD4C and one existing alignment warning in
website_editor.cpp. PE inspection confirmed x64 (8664), version 0.5.0-alpha.1.

Manual acceptance:
1. Verify ZIP hash, extract in a new directory on a second Windows x64 machine.
2. Inspect licenses, source commit and version; confirm no personal data.
3. Exit the real instance normally, preserving drafts. Launch with isolated data.
4. Run cumulative 0.4 alpha.1/alpha.2 checks, recording browser/OS/DPI/layout.
5. Back up/restore isolated data with attachments and verify migration/rollback.
6. Record the controlled resource baseline in support-matrix.md before budgeting.

Limits: no installer, signature, updater, byte-reproducibility claim or new
minimum-OS guarantee. Browser-provider faults and full 0.5 robustness remain open.

Library: [0.5.0-alpha.1 evidence](tgslib://72d6ce468fdd768b).
