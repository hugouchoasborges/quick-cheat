# 0.3.0-alpha.1 — portable collections

Candidate for manual acceptance. Main remains on the published 0.2.0-alpha.8 release.

## Completed evidence

- Release compilation: `./scripts/build.ps1 -SkipTests` succeeded on 2026-10-18; final log: `build/collections-final-build.log`.
- Independent inspection of bounded parsing, safe paths, merge/replace and image rollback found no concrete defect.
- UI review found Export was not enabled after the first save. The fix updates availability after shortcut/workflow changes and checks selection bounds; the reviewer confirmed the correction.
- Existing `qc_preview` generated isolated dark/light/system demo captures and exported `build/qa/quick-cheat-demo.quickcheat.json`. Dark and light shortcut settings and dark workflow settings captures were inspected visually.
- No tests were added or run. Import round trips and failure handling await manual acceptance.

## Manual acceptance

1. Export a saved application with shortcuts, workflows, Markdown and an attached image. Import into a separate `--data-dir` and check the content and image rendering.
2. Import the same file with Add twice: exact duplicates must be skipped. Different content with the same name must remain available.
3. Replace an existing application; confirm only that application changes, while theme, hotkey and other applications remain unchanged. Cancel both preview and replacement confirmation.
4. Save the first shortcut/workflow in a new application and check Export becomes available without resizing. Remove the last content and check it becomes disabled.
5. Try exporting with pending edits; save-first feedback should appear. Cancel import at the pending-edit prompt and confirm no data changes.
6. Reject corrupt JSON, unsupported versions, unsafe paths, missing/invalid images and oversized collections without changing saved data.
7. An existing PNG with the same path and different bytes must abort import without overwriting it. A failed save must remove only newly installed images.
8. Restart and check imported content persists. Check light, dark and System settings, keyboard navigation and minimum window size.

The generated DEMO targets the controlled `qc_integration.exe` application, not a real editor. It contains seven shortcuts, two workflows and a referenced PNG. Import it to inspect the editing interface; export one of your own application profiles for real shortcut execution.

## Library
Version decisions and evidence: [0.3.0-alpha.1](tgslib://5e6d2d592646481a).

## Manual acceptance and publication
Hugo confirmed all implemented changes work on 2026-10-18 and authorized publication as v0.3.0-alpha.2. Earlier candidate/pending statements above describe the implementation checkpoint. This release includes both portable collections and inline settings. Automated tests were not added or run during release preparation.
