# 0.6.0-alpha.4 — Dock interaction and defaults

Hugo approved grip/free-surface double-click, immediate single-click buttons,
automatic selection of new manual shortcuts and enabled Dock for new editor
profiles. Preserve existing disabled profiles and deselected buttons on edits.

## Implementation

Dock surface uses native CS_DBLCLKS/WM_LBUTTONDBLCLK. Cancel drag capture, verify
window/process/foreground, preserve pending draft/discard checks, select Shortcuts
and use the existing popup opening/site-refresh path. Recheck context after modal
discard prompts. No button double-click behavior or execution delay was added.
Repeated opening requests do not cancel an outstanding popup website lookup.
Mouse-up saves coordinates only after an actual move, avoiding writes on clicks.

Save of a new shortcut appends its index to Dock selection if below 64, preserving
order. At capacity the shortcut is saved without a Dock button and status explains
how to adjust selection. Edits preserve current membership. Validation/save failure
restores the prior draft, including Dock selection. Keyboard and URL actions use
the same path. New automatic/Browse/website editor profiles enable Dock; existing
and imported profile settings remain untouched. No migration/schema/layout change.

## Evidence and limits

Release build uses scripts/build.ps1 -SkipTests and build/Release/QuickCheat.exe.
Release SkipTests passed (build/dock-defaults-final-build.log), resource version
0.6.0-alpha.4. Final incremental build has no warnings; earlier C4324 is not fixed.
No automated tests added/executed.
Layout remains identical to alpha.3; its screenshots are representative. No new
DEMO capture or synthesized double-click/creation verification was performed.
Manual acceptance of the new behavior remains pending. No release/main promotion.

## Manual validation

1. Double-click grip/free space: open Shortcuts for the active app/site, including
   after viewing Workflows. Buttons should still execute immediately on one click.
2. Try a new website tab/context and verify popup content matches the current site.
3. Try unsaved Settings drafts: refusing discard preserves them and prevents switching.
4. Save a new keyboard or URL shortcut: it should enter Dock selection at the end.
5. Remove its Dock selection, edit/save it: it must remain deselected.
6. Check new app/site profiles enable Dock; old disabled ones remain disabled.
7. At 64 Dock buttons, a new shortcut still saves, with limit feedback and no removal.
8. Confirm dragging, right-click menu, collapse, paging and imported Dock order.

Real browser navigation from alpha.3 and community deployment remain pending.
