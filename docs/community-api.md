# Community API v1 — local 0.6 foundation

Implementation exists locally, not deployed or wired into the desktop UI.
Windows executable remains the compact 0.5.0-alpha.4 candidate. Compiled library:
qc_community. Portal source: D:/dashboard/app/apps/quick-cheat/.
The Portal's existing dirty core/catalog/compose changes are preserved.

Proposed live base follows the actual dynamic-app contract:
https://portal.hugoborges.dev.br/apps/quick-cheat/api/v1
No top-level /api/quick-cheat route is assumed or published.

- GET /catalog: q, executable, hostname, language, keyboardLayout, cursor.
- GET /collections/<id>/versions/<version>/download: immutable JSON by hash.
- Page: schemaVersion1, revision, items[<=20], total[<=500], nextCursor.
- Item: stable id/version, title/summary, public author id/displayName, target
  executable+hostname, compatibility platform/windows, architecture/x64,
  minimumClientVersion, language/layout, format1/2, counts, votes, license,
  updatedAt and SHA256. No account email in the response.
- Filters combine exact website and generic browser content. Revision-bound
  pagination rejects stale cursors; reset cursor after changing filters.

Native client: explicit worker calls only, pinned HTTPS host, redirects refused,
bounded WinHTTP timeouts/body size, strict UTF-8/JSON and shape limits. No automatic
network operation is wired to foreground changes. One query-keyed offline page
cache (<=1MiB on disk, response<=256KiB); cache warnings remain explicit.

prepare() downloads<=32MiB, checks SHA256, minimum client version, profile target
and counts, and reuses readCollection for existing limits/validation. Temporary
files are owned and cleaned; no profile/image installation occurs in prepare().
Caller must show Add/Replace preview, protect drafts and use existing merge,
attachment rollback and App::commit. Only after success may recordInstallation()
write source metadata and immutable baseline; it requires the saved target profile.
This prepares provenance, not update merging or stable per-item IDs yet.

Server module is deliberately not in apps.json: no live Portal exposure/restart.
It reads only explicit public/published records, validates IDs/realpaths/hash,
caps metadata/artifact sizes and permits at most2 concurrent requests.
No auth/write/vote endpoints or new access/navigation logs are added.

Evidence: Windows Release SkipTests compilation and Node syntax checks. No
automated tests or live service/end-to-end validation. Native catalog UI,
installation wiring, Portal registration/deployment and account policy are next.
