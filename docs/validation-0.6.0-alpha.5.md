# 0.6.0-alpha.5 — compact Dock

User requested minimum-size buttons, 5 px padding on every side, and a compact
shell. Measurements use logical pixels at 100% DPI/Dock scale and preserve scaling.
Public release remains v0.5.0-alpha.3; this is a local candidate.

## Changes

Measure actual labels with the existing Segoe UI font, then add 10 px total to
width/font height. Remove fixed 180×38 dimensions. Margins/gaps 2 px, grip 12 px;
collapse and page controls also fit their visible glyphs. Vertical first row shares
collapse and a shortcut where possible. Variable-width row/column packing keeps
order and monitor-bounded pages, without scrollbars. No schema/settings rewrite.

Compact drawing uses the same padding inside the button, literal ampersands and
a smaller corner radius. Valid title/action labels fit the expanded draw buffer.
At monitor limits labels ellipsize inside the padded text rectangle. Previous/Next
retain native caption/accessibility names and tooltips while displaying arrows.
Existing execution, double-click, dragging, labels, defaults and opacity remain.

## Evidence and limits

scripts/build.ps1 -SkipTests succeeded in the existing build/Release path;
resource FileVersion/ProductVersion 0.6.0-alpha.5. Log compact-dock-final-build.log
under build/. No warnings in the last incremental build; earlier C4324 not fixed.
No automated tests added/run. Existing preview helper extended only for DEMO docs.
Nine fresh PNGs: docs/images/v0.6.0-alpha.5, horizontal/vertical/long-label overflow
in dark/light/system. Horizontal capture: build/compact-dock-only.log. One vertical
attempt could not focus the controlled target; later targeted captures succeeded:
build/compact-final-dock-vertical.log and compact-final-dock-overflow-only.log.
Vertical was recaptured after sharing its header row. Inspected horizontal dark/
light, vertical dark and overflow dark. Overflow fixture uses 64 long labels at
200% Dock scale. Pictures show layout/paging controls, not button execution/page
clicks or a full multi-monitor DPI matrix. No private pages or user data touched.

## Manual checks

1. Inspect your real labels horizontally and vertically: tight padding, no overlap.
2. Change Dock scale and move between monitors; check text/padding and screen bounds.
3. Try a short Dock label, a long title and an ampersand; check legibility/ellipsis.
4. With many buttons, navigate every page and verify order and access to all items.
5. Verify click execution, grip double-click, drag, collapse, menu and opacity.

No release/main promotion. Existing browser-navigation and roadmap acceptance
limitations remain; this change does not resolve the historical shutdown issue.
