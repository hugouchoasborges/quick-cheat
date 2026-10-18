# Changelog

## 0.3.0-alpha.1 — 2026-10-18

Candidate: portable per-application collections. Import/export actions in Shortcuts and Workflows transfer saved shortcuts, workflow Markdown and referenced PNG images in a versioned JSON file. Import previews application identity and counts; Add skips exact duplicates, while Replace changes only the matching application after confirmation. Global settings and other applications are preserved.

Parsing and asset sizes are bounded. Unsafe paths, missing or invalid images and conflicting existing attachments abort import; newly created images roll back on failure. Export uses a flushed temporary file and atomic replacement. Native Windows JSON/Base64 APIs add no third-party dependency or idle work. Local store schema remains v3. Release compilation, independent code review and isolated demo captures/export completed; no tests added or run. Manual acceptance pending.


## 0.2.0-alpha.8 — 2026-10-18

Public prerelease, manually accepted by Hugo on 2026-10-18. Includes the local workflow, Markdown/image, PIN, collapsible-category and tabbed-settings improvements from the earlier 0.2 candidates below. README and screenshots refreshed using isolated demo data. Windows x64 ZIP includes the executable, documentation and third-party license notices, with SHA-256 checksums.

Selecting a workflow parent displays every step in order in the existing Markdown reader, with the workflow title, numbered step headings and horizontal separators. Selecting a child still displays only that step. Each step is parsed independently so unfinished Markdown cannot affect later sections. The reader accommodates the maximum stored workflow text, retains absolute link offsets and preserves its eight-image preview budget with a message directing users to individual steps for further images. Existing v3 data needs no migration. Release compilation, independent inspection and isolated dark/light captures completed; no tests added or run. Manual acceptance completed; published as a prerelease.

## 0.2.0-alpha.7 — 2026-10-18

Settings reorganized into General, Shortcuts and Workflows. Global appearance, activation and data-folder access have their own page; application selection is shared by the two content tabs. Workflow editing is embedded in the settings window with columns for guides, ordered steps and Markdown text. Popup management actions open the relevant tab; tray Settings opens General. Native themed navigation, clearer spacing and focused forms replace the mixed layout.

Pending edits are confirmed before tab/application changes and closing. Discarding a workflow reloads the saved draft, including when Browse is subsequently canceled. The parent handles dialog keyboard navigation across the embedded editor. Markdown source converts LF/CRLF for native multiline editing while retaining the 4096-character stored limit. Data schema remains v3. Release compilation, independent review and isolated theme captures completed; no tests added or run. Candidate awaiting manual acceptance.

## 0.2.0-alpha.6 — 2026-10-18

Shortcut cards are grouped beneath themed category headers with a divider and expand/collapse chevron. Click a header to toggle it. Expansion preferences are saved per executable and category in ui-state.ini, alongside shortcuts.qc; new categories default to expanded. Search temporarily reveals matching categories without changing saved preferences. Hidden shortcuts cannot be selected or executed; search navigation skips collapsed groups. Native group keyboard access is retained. Cards no longer repeat their category label. Schema v3 and workflow data are unchanged. Release build and isolated dark/light captures completed; manual acceptance pending, no tests added or run.

## 0.2.0-alpha.5 — 2026-10-18

Larger, centered active-application title in the popup header, with search on its own line. Native DPI-scaled pin and red close icons replace button text; pin state keeps its accent highlight and accessible names/tooltips remain available. High Contrast uses system colors. Close remains visible only while pinned. Shortcut and workflow layouts accommodate the taller header. Data format and popup behavior are unchanged. Release compilation and isolated dark/light captures completed; no tests added or run. Candidate awaiting acceptance.

## 0.2.0-alpha.4 — 2026-10-18

Workflow steps support rendered basic Markdown and local images. Native Rich Edit displays headings, emphasis, lists, checklists, quotes, code, links and attached images in the popup. The editor adds Attach image... for PNG/JPEG/BMP/GIF, storing normalized PNG copies in the local attachments folder. No browser runtime, remote-image downloads or HTML execution. Text storage remains schema v3; full backups must include attachments.

MD4C v0.6.0 is pinned and distributed under MIT. Image import/rendering has size and count limits; unused attachments are retained. Isolated dark/light/editor captures and Release compilation completed; no tests added or run. Three local ChatGPT guides updated with Markdown and three original sample diagrams, preserving nine shortcuts and a pre-edit backup. Candidate awaiting manual acceptance.

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
