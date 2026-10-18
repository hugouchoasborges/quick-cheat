# Local Workflows — implementation plan

Goal: deliver 0.2.0-alpha.1 with local per-application reference workflows, approved by Hugo on 2026-10-18.
Architecture: extend Profile with titled workflows and ordered text steps. Keep Win32 controls and event-driven behavior. Use separate workflow editor/popup modules with shared theme and existing persistence.

1. Add bounded Workflow model and schema v3 records; load v1/v2 unchanged. Preserve a one-time pre-v3 file before first migration plus normal previous-save backup.
2. Add native workflow editor reachable from the selected application in the shortcut library. Provide title, ordered steps, Add/Update/Remove/Move, New/Save/Delete and unsaved-change protection.
3. Show Shortcuts/Workflows tabs in popup only when the active application has workflows. Use a native expandable tree plus a read-only wrapping detail field. Hide Execute on Workflows; no automatic actions.
4. Keep focus, foreground identity, theme, DPI and keyboard behavior consistent. Prevent stale shortcut execution while Workflows is selected.
5. Bump candidate to 0.2.0-alpha.1, document migration/rollback/acceptance. Compile Release without running or adding tests, per session instruction. Inspect isolated demo screenshots if available; leave manual acceptance explicit.
6. Create a Library child entry with decisions, changes, evidence and limitations. Integrate feature into develop and create release/0.2.0-alpha.1; main remains the published alpha.4 until a later approved publication.

Acceptance: create/edit/delete a workflow; add/edit/remove/reorder steps; persist after restart; expand/collapse and read long steps; foreground switching; legacy shortcuts/theme/hotkey retained; pre-v3 backup created only on migration; no input injection from workflow navigation.