# Dock grip context menu implementation plan

Goal: native right-click menu on the grip: checked Enable Dock, Orientation,
Scale, Opacity, separator, Settings. User explicitly added Settings last.
Architecture: Win32 popup/submenus, immediate Store commit for the displayed
profile; keep noactivation shortcut clicks, bounded resources and existing schema.
Tech stack: C++20/Win32; Release SkipTests at build/Release/QuickCheat.exe.

1. Add isolated dock_menu.cpp and declarations; use presets and include any current
   non-preset percentage. Scale 50–200%; opacity 20–100%, as existing Settings.
2. Freeze dock rebuild while menu is open; retain HWND/PID/profile identity and
   cancel stale results. Settings opens Dock tab and matching profile through guards.
3. Preserve pending settings drafts; reject quick changes while edits are pending.
   Commit before rendering; disabling hides Dock, re-enable through Settings.
4. Bump local candidate to 0.5.0-alpha.3; update docs/changelog/Library/Harness.
5. Add only DEMO menu capture to existing screenshot helper, compile SkipTests,
   inspect native menu capture, then reopen actual app at the same Release path.
6. Keep local feature branch until Hugo accepts/requests publication. No automated
   tests added/executed. No community, Android, site execution or schema changes.

User validation of alpha.2: Edge sites and Dock OK; remaining flows reported OK.
Uncommitted address recognized and accepted by user; record discrepancy rather
than claim fallback works. Site-specific execution not actually tested.
