# Community through beta implementation plan (0.6–0.9)

Goal: execute the newly authorized roadmap in small native Windows increments.
Architecture: existing Portal Express dynamic-app API + on-demand WinHTTP worker,
native controls, bounded local cache and explicit install/publication/update.
Stack: C++20/Win32/Windows JSON, WinHTTP/BCrypt, Portal Node22/Express4.
Authorization: Hugo requested current release delivery and starting all stages.
Future publication still needs applicable acceptance/request. SkipTests only.

## 0.6 — catalog
- [x] Inspect actual D:/dashboard Harness, auth and dynamic-router contract.
- [x] Read-only versioned API, bounded paging/filtering and immutable downloads.
- [x] Compile native codec, HTTPS client, offline cache and validated preparation.
- [x] Source/version/hash/baseline recording after confirmed installation.
- [ ] Native catalog view, compatible suggestions and explicit Add/Replace preview.
- [ ] Connect worker completion/cancellation to UI generation and draft guards.
- [ ] Register/deploy service after review; verify actual catalog and offline flow.
- [ ] Record screenshots/evidence and request acceptance before release.

## 0.7 — accounts, publishing and votes
- [ ] Resolve open Google-account policy versus existing authorized accounts.
- [ ] Separate scoped community credentials from broad MCP apps-management tokens.
- [ ] Implement browser sign-in/PKCE or device pairing, secure token storage/logout.
- [ ] Publication preview and author rights/license declaration; version ownership.
- [ ] Server-authorized publish/update/withdraw; idempotent per-account voting.
- [ ] Rate/size limits, pagination, reports/moderation and privacy/deletion policy.
- [ ] Local review, acceptance and publication gate; no automatic upload of profiles.

## 0.8 — preserve adjustments on updates
- [ ] Add stable item IDs and backward-compatible migration/collection format.
- [ ] Map published IDs to local IDs; retain installed baseline and local edits.
- [ ] Three-way diff: untouched fields update; local-only edits stay; explicit
      conflict decisions for both-changed/deleted items and workflow content.
- [ ] Preserve Dock choices; show preview, back up before commit and enable restore.
- [ ] Manual/on-demand version checks first; choose later frequency with Hugo.

## 0.9 — localization and beta
- [ ] Confirm first languages; English fallback and UI/content/layout separation.
- [ ] Extract authored strings without translating user data; persist language.
- [ ] Live refresh preserves drafts; recheck long labels, Unicode, keyboard and HC.
- [ ] Revisit Windows/browser/DPI matrix, migration and measured resource budgets.
- [ ] Define beta scope, installer/update decisions and explicit beta acceptance.

Not complete: API service is local, client library is not desktop UI integration.
Keep the known residual-process shutdown issue visible; do not infer it fixed.
No Android, extension, telemetry or cloud sync added. No automated tests add/run.
