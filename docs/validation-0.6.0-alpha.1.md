# 0.6.0-alpha.1 — local candidate

Public download remains v0.5.0-alpha.3. The local Release executable and its
resource version are 0.6.0-alpha.1 at build/Release/QuickCheat.exe.
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

Initial link failed with LNK1104 because residual PID 33632 held the executable.
Hugo manually ended it. `scripts/build.ps1 -SkipTests` then succeeded, including
the final theme correction (build/community-ui-final-build.log). No warnings in
that incremental build; earlier website_editor.cpp C4324 is not claimed fixed.
No automated tests added or run.

Full DEMO capture completed (build/community-ui-preview-final.log); 36 fresh PNGs
in docs/images/v0.6.0-alpha.1. A later full capture failed to restore its Dock
target, so the final catalog captures used the narrower --community-only option
(build/community-only-preview.log), successful in light/dark/system without network.
Dark controls use existing native checkbox rendering/backgrounds. Catalog and
popup visually inspected. Full-helper focus remains environment-sensitive.
No real API, import, cache or cancel behavior was exercised. Cancellation may wait
for network timeout; the earlier residual-process shutdown bug remains open.

Real-data launch used the usual Release path (PID 20752). Hugo confirmed that it
opened and he closed it himself. The later absence of host/windows is not evidence
of startup failure. This confirms opening, not full catalog/install acceptance.

On 2026-10-19 Hugo reported "Está tudo funcionando" in response to the manual
checklist: compact popup, shortcuts/workflows, Settings/Dock, drafts, native catalog
layout/navigation and normal exit. Record this as user-reported acceptance of that
local scope. Process disappearance was not independently measured and no shutdown
fix was implemented; the earlier residual-process issue is not proven resolved.
Real catalog service/search/install, Google accounts, publishing/votes, updates
and localization were explicitly excluded from that checklist and remain pending.
This feedback does not request publication or main promotion.

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
