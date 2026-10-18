# Portable collections implementation plan — 0.3.0-alpha.1

## Goal
Implement the next recorded roadmap step: portable per-application collections with local import/export. Keep Windows-only C++/Win32 and the existing settings tabs. No community service or multiple installed collection variants in this increment.

## Contract
A versioned .quickcheat.json contains one application's identity, shortcuts, workflows and referenced PNG images encoded as Base64. Global activation/theme settings and category expansion state are excluded. Use Windows.Data.Json and Crypt32, provided by Windows; no new third-party dependency or idle service. Limit files to 32 MiB, embedded images to 8 MiB total, 4 MiB each and 128 files; retain current profile/text limits.

Import shows application identity/counts before changes and offers Add, Replace this application's content, or Cancel. Add skips exact duplicates; differing content with identical names is retained. Replace requires explicit confirmation. Other profiles and global settings remain intact. Reject unsupported formats/versions, missing or invalid images and unsafe paths. Existing image files are never overwritten: identical bytes are reused, conflicting bytes abort. Newly installed images are rolled back if validation/storage fails. Existing storage backup remains in use.

## Tasks
- [x] Implement bounded JSON serialization/parsing, Markdown image-reference discovery and merged-store preparation in collections.cpp/.hpp.
- [x] Add rollback-managed image installation, reusing current WIC validation and safe managed-reference rules.
- [x] Add Import collection / Export collection actions beside the application selector in Shortcuts and Workflows, with native dialogs and preview. Export requires saved edits; importing preserves discard guards.
- [x] Compile with SkipTests. No tests added/run unless explicitly requested. Inspect UI with isolated demo captures and get an independent code review of file handling/merge behavior.
- [x] Document format, limits, manual acceptance and decisions; create a new version child in Library. Integrate feature → develop → release/0.3.0-alpha.1; leave main on the published release.
