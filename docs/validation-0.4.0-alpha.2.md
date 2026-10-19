# 0.4.0-alpha.2 — browser headings and dock refinements

Candidate awaiting manual acceptance, including the preceding 0.4.0-alpha.1 features.

## Changes

- Browser popup heading: `chatgpt.com — msedge.exe`, with the hostname first. Without a detected website, show only the browser executable. This applies with or without saved profiles. Other applications retain their existing names; Settings retains its own heading.
- Dock has no native scrollbars. Oversized selections use Previous/Next pages, retaining selected shortcut order, configured orientation and scale. Page selection is session state and resets when switching profiles.
- Dock settings separates behavior/appearance from shortcut selection and ordering. Enable dock and Start collapsed use native checkboxes with keyboard and accessibility semantics. Theme drawing preserves the native checked state.
- Explicit Apply, validation and unsaved-edit guards remain in use. No storage or collection format change.

## Acceptance

1. Open popup over Edge/Chrome with a recognized hostname. Check hostname first, process second; navigate to an unknown/internal page and check process-only fallback. Check no saved profile and long hostnames.
2. Configure horizontal/vertical docks at several scales with many shortcuts. Confirm no scrollbars, all commands accessible through pages, Previous/Next enabled states and correct order/execution on each page.
3. Collapse/expand, switch applications, change scale/orientation or monitor/DPI and reduce the selected shortcuts. Confirm page bounds and focus remain correct.
4. In Dock settings, toggle checkboxes with mouse and Space; verify checked state after Apply/reopen. Check Dark, Light, System and High Contrast, plus Tab navigation.
5. Configure enabled/disabled and collapsed/expanded combinations, edit fields, reorder selected shortcuts and check selection counts. Cancel discarding pending edits and confirm the draft remains.
6. Invalid scale/opacity, more than 64 selections or enabled-with-no-selection must provide feedback without saving invalid data.

## Validation evidence

Evidence will be recorded after compilation and DEMO capture. No automated tests are added or run. The existing screenshot helper's `--dock-overflow` option creates fictional 64-shortcut/200% data for visual capture, without execution assertions.

## Completed evidence
Release build with SkipTests succeeded (build/header-dock-settings-build.log). Independent inspection confirmed native checkbox semantics, pending/apply guards, counts, order and minimum layout. Review caught a Windows LONG/int mismatch in pagination dimensions; corrected before the successful build. Fresh isolated DEMO captures generated in Dark/Light/System; dark/light settings and normal dock inspected. Live browser-heading and interaction acceptance remains manual. Initial DEMO capture attempts stopped on lost foreground; those attempts were not used as evidence or images. No automated tests were added or run.

Overflow DEMO capture (64 shortcuts, 200% scale) completed and was inspected: Previous/Next controls are visible and no scrollbars are present. Interaction on later pages remains part of manual acceptance.

Library: [0.4.0-alpha.2 decisions](tgslib://0c4a74e9bf57545b).
