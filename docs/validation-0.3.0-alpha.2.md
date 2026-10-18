# Inline settings — 0.3.0-alpha.2

Candidate for manual acceptance.

## Implemented
- Removed Manage shortcuts/workflows from the popup footer.
- Added an authored native gear button immediately left of PIN, with accessible Settings text and tooltip.
- Settings reuses the main popup: General, Shortcuts and Workflows are child panels, rather than a separate top-level editor window.
- Back arrow returns to the application library after the existing unsaved-edit confirmation. Gear also returns while Settings is open.
- Escape, Close and unpinned focus loss hide the window to tray while retaining the settings draft. Tray Settings opens the same panel. Exiting the app still confirms unsaved edits, even when the popup is hidden.
- Native scrolling keeps forms reachable at smaller sizes. PIN, themes, application context, import/export and workflow editing remain available.

## Manual acceptance
1. Open Settings using the gear and from the tray; confirm the same main window is reused.
2. Check gear placement with PIN on/off, Back, accessible labels, keyboard navigation and Dark/Light/System.
3. Edit a shortcut or workflow, hide with Escape or focus loss, reopen and confirm the draft remains. Exit from tray and cancel the discard prompt.
4. Use Back with pending edits: cancel to retain editing, confirm to return. Confirm content shows the current external application afterward.
5. Open native file/import/replace dialogs while unpinned; the main window should remain available behind its own dialogs.
6. Resize to the minimum size and use native scrollbars to reach the settings controls. Check shortcut execution and workflow reading after returning.

No tests added or run. Compilation, independent inspection and demo captures are recorded below after completion.
## Evidence
Release build with SkipTests succeeded. Independent review found the hidden-draft exit guard still depended on child visibility; changed to settingsVisible, and the reviewer confirmed the correction plus IDOK routing and keyboard focus reveal. Existing demo helper adapted for child panels and parent-window captures. Dark/light/system DEMO captures generated; dark popup, dark General/Workflows and light Shortcuts inspected. Manual acceptance remains pending.

Library: [0.3.0-alpha.2](tgslib://37c03bacb1de3d45).

## Manual acceptance and publication
Hugo confirmed all implemented changes work on 2026-10-18 and authorized publication as v0.3.0-alpha.2. Earlier candidate/pending statements above describe the implementation checkpoint. This release includes both portable collections and inline settings. Automated tests were not added or run during release preparation.
