# 0.5.0-alpha.2 — Edge native address traversal

Local candidate, not accepted/published. User reports msedge.exe without website.
Running development executable confirmed 0.4.0-alpha.2; Edge 155.0.4283.62.

Read-only UIA inspection of the live Edge window found an unfocused Edit
OmniboxViewViews under a toolbar at depth 8 (element 80). Traversal visited 204
elements in 148 ms; the existing limit of 128 discards even a found candidate
because traversal has not completed. No address, page title or history logged.
This identifies a concrete traversal-limit failure, not all possible UIA failures.

Change: 512 element budget, retaining the 400 ms deadline and existing depth
limit, ambiguity/focus rejection, renderer/document exclusion and HWND/PID guards.
No clipboard, keyboard simulation for detection, extension, polling or schema change.
The read-only inspection used .NET UIA; its timing is not native app latency.

Release compilation with SkipTests completed successfully on 2026-10-19:
build/edge-context-build.log; isolated binary version 0.5.0-alpha.2.
No automated tests added or run. Existing live app/data/drafts preserved.

Manual validation after normal exit of the previous instance:
1. Launch build/Release/QuickCheat.exe; confirm alpha.2 version.
2. In Edge, activate a normal HTTP(S) page with address bar unfocused. Open the
   popup by hotkey: expect hostname — msedge.exe, without needing a saved profile.
3. Change tabs/sites/windows; verify header updates and website shortcuts.
4. Type an uncommitted address, use internal pages or hide toolbar: require safe
   browser-only fallback. Close/reopen during confirmation; no stale execution.
5. Run cumulative 0.4 and 0.5 alpha.1 acceptance, including Chrome and pinned popup.

The initial isolated build preserved the old running executable. Hugo subsequently
requested the usual build/Release path for compilation and launch; this is now the
user-facing build convention. Native end-to-end acceptance remains pending.

Library: [0.5.0-alpha.2 evidence](tgslib://7e06deadd974f132).
