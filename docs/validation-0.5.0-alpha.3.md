# 0.5.0-alpha.3 — Dock grip context menu

Local candidate, not published/accepted. Public version remains alpha.2.
Right-click the grip/background strip (not shortcut buttons): Enable Dock,
Orientation (Horizontal/Vertical), Scale (50–200%), Opacity (20–100%),
separator and Settings last. Presets include any current non-preset value.
Changes apply immediately and persist to the profile whose Dock was displayed.
For websites this may be the website override; disabling it restores an enabled
generic browser Dock if configured. Re-enable through Settings → Dock.
Settings opens the matching profile and Dock tab through existing draft guards.
Quick changes require pending Settings edits to be saved/discarded first.
No shortcut input is sent by these commands; menu tracks captured HWND/PID/key.

Release SkipTests succeeded; log: build/dock-menu-build.log. Version resource
confirmed 0.5.0-alpha.3 in build/Release/QuickCheat.exe. DEMO capture completed
(build/dock-menu-preview.log); 33 new PNGs in docs/images/v0.5.0-alpha.3/,
including native grip menus. Dark/Light root menus inspected: checked Enable Dock,
Orientation, Scale, Opacity, separator and Settings last. Win32 menu colors follow
Windows, independently of the application's selected theme.
The first build compiled the new Dock code but linking failed (LNK1104): previous
alpha.2 process PID 30408 remained alive after normal exit, with host/window gone.
Hugo chose to end it manually; no forced process termination by the agent.
This shutdown issue is recorded, not claimed fixed by the menu change.
Existing screenshot helper is extended only to document the native menu.
No automated tests added/executed; same build/Release path and existing user data.

Manual checks:
1. Right-click grip in expanded/collapsed Dock; confirm all entries and checks.
2. Change orientation, scale and opacity; check layout, transparency and saved
   values after restart. Verify the selected value in each submenu.
3. Disable Dock; re-enable in Settings. Verify generic/website profile isolation.
4. Settings last opens Dock for the displayed profile. Cancel discard prompts;
   preserve drafts when hidden and reject quick edits until resolved.
5. Dismiss with Esc/outside click; switch apps while menu is open. No stale
   change, hidden-window action, stuck drag or unintended shortcut execution.
6. Verify normal noactivation shortcut clicks, drag, collapse, paging, DPI,
   keyboard menu navigation and Dark/Light/System/High Contrast.

Alpha.2 user feedback: Edge sites and Dock OK; other flows reported functioning.
Typed uncommitted address was recognized and accepted by Hugo. This differs from
the intended conservative fallback; no browser detection/execution change here.
Site-specific shortcut execution was not tested; it remains unverified.

Library: [0.5.0-alpha.3 implementation and evidence](tgslib://9ffae05939e7d7f0).
