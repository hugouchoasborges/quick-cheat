# Changelog

## 0.2.0-alpha.3 — 2026-10-18

Top-right PIN, off by default: focus loss hides the popup while the tray app keeps running. PIN on keeps it visible across focus loss, execution and activation hotkey; Escape or the adjacent Close button hides it. PIN state lasts for the session. Management windows stay usable above a pinned popup.

Responsive native shortcut tiles replace full-width rows. Each card contains its category, description and keycaps, with native selection and keyboard support, deliberate double-click and a full-text reader for long content. Search Up/Down remains sequential. Double-click checks the original target and item. Initial layout and resize/DPI changes invalidate cached tile positions. Storage remains v3; candidate awaiting acceptance.

## 0.2.0-alpha.2 — 2026-10-18

Retain the last valid foreground application while interacting with Quick Cheat or transient tray/menu windows; failed window identification no longer clears the context. Popup selection is cleared only when a valid new target is identified.

Larger monitor-constrained popup, native edge resizing, header dragging, Expand/Restore and session-size retention. Responsive workflow layout with a larger step reader; abbreviated tree labels avoid horizontal scrolling. Shortcut descriptions wrap with measured row heights; a full-text reader handles the native 255-pixel row limit. Data schema remains v3. Candidate awaiting manual acceptance.

## 0.2.0-alpha.1 — 2026-10-18

Local per-application workflows: title and ordered text steps; dedicated native editor with add/edit/delete/reorder and unsaved-change prompts. Applications with workflows show Shortcuts/Workflows tabs in the popup, with expandable guides, title search and a full-text step reader. Workflows do not execute commands. Shared System/Light/Dark themes and DPI-aware layout.

Storage schema v3 reads v1/v2 and preserves a one-time shortcuts.qc.pre-v3.bak before migration, alongside the normal previous-save backup. Candidate awaiting manual acceptance; no public release yet.

## 0.1.0-alpha.4 — 2026-10-18

Double-click a shortcut row to execute it. Single-click selection, Execute and Enter remain available. Local data format is unchanged.

## 0.1.0-alpha.3 — 2026-10-18

Added an original lavender keyboard-key/Q icon as embedded Windows resources for the executable, tray and window classes. Ten icon sizes from 16 to 256px, with reproducible SVG/PNG/ICO assets. Added a desktop-shortcut script pointing at the current development build and verified all embedded icon sizes. No shortcut execution or theme behavior changes.

## 0.1.0-alpha.2 — 2026-10-18

Raycast-inspired popup and shortcut library; persistent System/Light/Dark themes; searchable categorized rows, keyboard navigation and keycaps. Clicking a row only selects; the explicit Execute button or Enter from search runs the shortcut. Popup search owns focus while retaining the external target, with focus restoration and stale-context guards. Schema v1 loads with System appearance; v2 persists the selected theme. Added theme/search/migration tests and selection-versus-execution integration coverage.

Local sample: 13.6 MiB working set, 2.68 MiB private commit, 0 ms CPU over 1 second before opening the UI. Physical-hotkey, live OS-theme changes, High Contrast and the full DPI/OS matrix remain manual acceptance items. Candidate not published as a GitHub Release.

## 0.1.0-alpha.1 — 2026-10-18

First Windows x64 candidate: native tray app, configurable popup toggle, foreground tracking, categorized keycaps, click-to-run and local shortcut/profile management. UTF-8 schema v1, safe file replacement, previous-save backup and corruption rejection. CMake/MSVC build, core tests, controlled desktop integration checks and CI build artifacts.

Known limits: one chord per entry; filename-based app matching; Windows privilege/focus restrictions; manual physical-hotkey and multi-monitor/DPI verification remains. Workflows, dock and community are deferred. This candidate has not been published as a GitHub Release.
