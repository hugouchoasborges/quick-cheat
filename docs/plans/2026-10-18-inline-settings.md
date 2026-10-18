# Inline settings implementation plan — 0.3.0-alpha.2

## Design
User requests replacing Manage shortcuts/workflows with an accessible authored gear icon immediately left of PIN, opening settings in the same main window. Reuse the existing native editor as a child panel, preserving tabs, explicit saves, import/export and workflow editor. Header shows Settings, with Back at left and gear/PIN/optional red Close at right. Back confirms pending edits; Escape hides to tray, retaining the draft, as does unpinned focus loss. Owned app dialogs must not trigger focus dismissal. Tray Settings uses the same panel. Native scrolling keeps existing forms accessible in smaller windows. No changes to store schema or dependencies.

## Tasks
- [x] Embed Editor inside popup; add settings navigation state and Back, preserve hidden drafts and app context.
- [x] Replace Manage footer with gear; draw native GDI gear/back in existing icon style with labels/tooltips.
- [x] Adapt layout and scrolling, focus/keyboard, close/discard and owned-dialog handling.
- [x] Compile with SkipTests, inspect isolated demo theme captures and review changes. No tests added/run.
- [x] Update documentation, Library version child and Harness, integrate feature into develop and prepare release candidate.