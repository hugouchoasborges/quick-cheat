# 0.6.0-alpha.1 — local source checkpoint

Public download remains v0.5.0-alpha.3. The installed Release executable remains
0.5.0-alpha.4 because the new link failed; source/resource version is 0.6.0-alpha.1.
No promotion, publication, deployment or user-data installation was performed.

## Implemented source

- Tray action opens a native resizable catalog window without a network request.
- Explicit Search, current application/site filter, pagination and collection
  author/license/language/layout/version/content details.
- One managed request worker; cancel/close invalidates the UI generation, and
  the UI joins completion before accessing results. No detached catalog threads.
- Download validation uses the bounded HTTPS/hash/collection reader. Installation
  reuses Add/Replace, duplicate handling, image rollback and pending-draft guards.
- Record immutable baseline/source after a successful store/image installation;
  report source-record failure separately without claiming the install failed.
- Catalog credentials are not implemented. It uploads nothing. Public catalog,
  any-Google publication/votes and English/pt-BR are confirmed policy decisions.

## Evidence and limits

`scripts/build.ps1 -SkipTests` compiled community_window.cpp and collection_ui.cpp.
Link failed with LNK1104 against build/Release/QuickCheat.exe, held by residual
PID 33632. Log: build/community-ui-build.log. Existing website_editor.cpp C4324
alignment warning remains. No automated tests added or run.

The DEMO capture helper now includes the idle catalog window, without networking.
It has not been run against this candidate: screenshots and visual/runtime claims
are pending until a successful link. No real API, import, cache or cancel behavior
was exercised. Worker cancellation is cooperative and may wait for a network
timeout; prompt shutdown is not proven. The earlier residual-process bug is open.

## Manual checks after building and deploying the service

- Open Community collections from the tray; opening alone should not fetch.
- Search all collections and filter the active app/site; inspect details and pages.
- Check service-unavailable feedback, exact-query offline cache and no-cache case.
- Cancel/close during requests, reopen, and ensure stale results never install.
- Preview a download; cancel leaves profiles unchanged. Check Add, duplicates,
  Replace confirmation, pending shortcut/workflow/Dock drafts and image rollback.
- Check source-record persistence and distinct reporting of source-write failures.
- Inspect light/dark/high contrast, keyboard, scaling and narrow-window layouts.
- Exit normally; report whether a residual process remains. Never force-kill the
  real app from automation. Existing desktop shortcut must continue to work.

Service registration/deployment, actual published content and end-to-end evidence
remain part of 0.6. Accounts/publication/votes (0.7), update merging (0.8) and
localization/beta (0.9) remain pending in the execution plan.
