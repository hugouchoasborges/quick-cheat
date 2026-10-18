# Contextual dock and websites — 0.4.0-alpha.1

## Design
Next roadmap increment after portable collections: native contextual Dock plus website-specific shortcuts/workflows for Edge and Chrome (Firefox deferred). User chose native Windows UI Automation, without extension. Dedicated bounded MTA worker; WinEvents and on-demand reads, no continuous browser polling. Initial supported browsers: Edge and Chrome. Firefox and Android deferred. Store normalized hostname only, no paths/query/history. General browser content remains visible alongside a distinct website section. Keep last valid context while interacting with Quick Cheat and fail to browser-only when no reliable site context is available. Browsers/windows must not share stale site context.

Dock: ordered shortcuts picked from a profile, horizontal/vertical, collapsible, movable, scale/opacity controls; visible only for its associated foreground context and own non-activating interaction. Use native buttons, WM_MOUSEACTIVATE=MA_NOACTIVATE, validate target before sending input. Persist preferences and shortcut references; deletion adjusts references. Website dock overrides browser dock only when configured. No timer-based idle redraw/polling.

Data: extend profile identity to executable + optional hostname; schema v4 with pre-v4 backup. Read older schemas. Collections support websites and dock config with backwards format-1 reading. Settings remain inline; add Dock tab and website creation/editing access; existing pending-edit guards preserved. Windows C++20/Win32, English UI and Dark/Light/System. Main remains published release until manual acceptance/publication request.

## Tasks
- [x] Extend bounded model/store/collections and migration backup.
- [x] Implement native UI Automation worker, bounded event-based context detection and integration docs.
- [x] Implement native contextual dock and settings controls.
- [x] Integrate site identity, combined browser/site display, editing and execution guards.
- [x] Compile SkipTests, inspect isolated demo captures and review. Do not add/run tests unless asked.
- [x] Update docs, new Library child and Harness; feature → develop → release/0.4.0-alpha.1.