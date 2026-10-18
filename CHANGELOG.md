# Changelog

## 0.1.0-alpha.2 — 2026-10-18

Raycast-inspired popup and shortcut library; persistent System/Light/Dark themes; searchable categorized rows, keyboard navigation and keycaps. Clicking a row only selects; the explicit Execute button or Enter from search runs the shortcut. Popup search owns focus while retaining the external target, with focus restoration and stale-context guards. Schema v1 loads with System appearance; v2 persists the selected theme. Added theme/search/migration tests and selection-versus-execution integration coverage.

Local sample: 13.6 MiB working set, 2.68 MiB private commit, 0 ms CPU over 1 second before opening the UI. Physical-hotkey, live OS-theme changes, High Contrast and the full DPI/OS matrix remain manual acceptance items. Candidate not published as a GitHub Release.

## 0.1.0-alpha.1 — 2026-10-18

First Windows x64 candidate: native tray app, configurable popup toggle, foreground tracking, categorized keycaps, click-to-run and local shortcut/profile management. UTF-8 schema v1, safe file replacement, previous-save backup and corruption rejection. CMake/MSVC build, core tests, controlled desktop integration checks and CI build artifacts.

Known limits: one chord per entry; filename-based app matching; Windows privilege/focus restrictions; manual physical-hotkey and multi-monitor/DPI verification remains. Workflows, dock and community are deferred. This candidate has not been published as a GitHub Release.
