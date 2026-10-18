# Quick Cheat 0.1.0-alpha.1 Implementation Plan

**Goal:** Deliver a usable Windows popup and local shortcut editor.
**Architecture:** C++20 domain and storage separated from Win32 UI and foreground/input integration. One tray process, one blocking message loop, no background polling or network dependency.
**Tech Stack:** MSVC, Windows SDK, CMake, Win32 common controls, GDI, static C++ runtime. Existing MIT license retained.

1. Connect existing remote; preserve main; branch develop and feature/0.1.0-local-shortcuts.
2. Test shortcut parsing/formatting, profile identity, versioned serialization, corrupt-file rejection and durable local persistence before implementation.
3. Build a tray-resident application using RegisterHotKey and foreground events. Default toggle: Ctrl+Alt+Space. Report conflicts and permit reconfiguration.
4. Render a no-activation popup with categorized keycap rows, empty state, click-to-execute, close and access to the editor. Track the external foreground application while ignoring our own windows.
5. Add native profile management and shortcut create/edit/delete with input validation, grouping and durable save. Preserve target window identity and verify foreground before SendInput; never inject while physical modifiers are pressed.
6. Provide a Windows x64 executable, reproducible build script, manifest, version metadata and CI build/test workflow. Initially target Windows 10 1809+ and Windows 11 through documented APIs; actual OS coverage must be reported separately.
7. Verify model/storage tests, executable launch/tray lifecycle, actual foreground tracking and click execution on a controlled window. Measure idle memory/CPU without claiming a universal budget.
8. Record decisions, deliverables, validation and limitations in a Library child page for 0.1.0-alpha.1 and initialize Harness. Commit on the feature branch; main advances only through the release process.

## Scope
Local profiles, shortcut CRUD, categories, contextual popup and keyboard injection. Workflows, dock and online community are deferred. Activation uses a toggle in this first version; hold activation remains a later enhancement. Do not publish a GitHub Release or update main until the candidate is validated.

## Validation
Targeted domain/storage tests, CMake Release build, Windows integration smoke checks. No bundled third-party runtime, network calls or undocumented Windows APIs. Performance improvements must preserve correctness; disk writes happen only on explicit save.
