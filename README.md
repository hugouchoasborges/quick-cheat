# Quick Cheat

A small Windows shortcut reference and launcher. C++20, Win32 and native Windows controls; no web runtime, background polling or network access.

## Current candidate: 0.1.0-alpha.3

- Tray-resident app with a configurable global popup toggle (default **Ctrl+Alt+Space**).
- If that default is occupied on first launch, tries **Ctrl+Alt+Shift+Space** and shows a notification. Existing configured conflicts are reported; use the tray menu to change activation.
- Raycast-inspired popup with search, category headings, right-aligned keycaps and explicit **Execute**. Clicking a row selects it; selection never injects a shortcut.
- Shared **System**, **Light** and **Dark** appearance across the popup and shortcut library, saved from the Appearance selector. System follows Windows app colors through documented UISettings events; High Contrast uses system colors.
- Local editor: select the current app or browse to an executable, create/edit/delete shortcuts, record a combination or type it, and choose/type categories.
- English UI, Unicode data, versioned UTF-8 storage, durable replacement and previous-save backup.
- Original lavender keycap/Q icon embedded in the executable, tray and window classes, with ten sizes from 16 to 256px.

## Run

Run `QuickCheat.exe`. The application starts in the notification area; Windows may put its icon under **Show hidden icons**. Right-click it and choose **Manage shortcuts** (or double-click the icon).
Activate your application and press the popup toggle. Search receives keyboard focus while Quick Cheat retains the external application as its target. Type a description, category or key combination; use Up/Down or click a row to select it, then click **Execute** (or press Enter from search). Toggle again or use Esc to close and restore the target when possible. **Ctrl+,** from search opens management.
For a new application, open **Manage shortcuts**, enter a description, category and key combination, then click **Save shortcut**. Choose **Appearance > System / Light / Dark** to change the shared theme immediately. Reactivate the application and open its popup to use your shortcuts.
Use **New shortcut** to add another entry. Selecting a row edits that entry. Category names are entered directly or selected from the existing list; empty categories appear as **General**. **Browse application** adds an installed or portable `.exe` profile.
Key syntax includes `Ctrl+S`, `Ctrl+Shift+F10`, `Alt+Enter`, `Win+Left`, `Ctrl+Plus`, `PageDown`. The **Record** button captures supported single combinations; Esc cancels recording. Release physical modifier keys before invoking Execute.

## Data and recovery

Data: `%LOCALAPPDATA%\QuickCheat\shortcuts.qc`; previous save: `shortcuts.qc.bak`. The file is a schema-versioned, escaped UTF-8 format, with no third-party parser. Schema v2 stores appearance; v1 loads with System selected and is migrated on the next explicit save. Alpha.1 cannot read v2 files. Saves flush a temporary file and replace the original; failed saves preserve in-memory data and report an error. Invalid input files are rejected without being overwritten.
To recover, exit Quick Cheat, keep a copy of the damaged file and replace `shortcuts.qc` with a known-good `shortcuts.qc.bak`. Data stays on this computer; there are no accounts, telemetry or community downloads in this version. `--data-dir <directory>` selects a separate data directory for testing or portable use; only one instance runs per session.

## Build and verify

Install Visual Studio 2022 **Desktop development with C++**, Windows SDK and CMake tools. Run `./scripts/build.ps1` in PowerShell. The script builds Windows x64 Release with a static C++ runtime, then runs the core tests. Output: `build/Release/QuickCheat.exe`. The executable needs no separately installed .NET or VC++ runtime.
Run `./scripts/create-desktop-shortcut.ps1` after building to create **Quick Cheat** on the Windows desktop. It points directly at `build/Release/QuickCheat.exe`, so subsequent builds use the same shortcut; its icon also comes from the executable. Unrelated existing shortcuts are preserved. `./scripts/generate-icon.ps1` reproduces the authored SVG, PNG preview and multi-size ICO in `assets/` using Windows System.Drawing; the app needs no asset files alongside its executable.
For desktop integration checks, first exit Quick Cheat and run `build/Release/qc_integration.exe build/Release/QuickCheat.exe`. This opens controlled test windows using isolated temporary data and verifies native hotkey-message handling, editor persistence, popup focus, actual SendInput delivery, foreground switching and restart. It dispatches `WM_HOTKEY` directly; physical activation and conflicts with other software need manual verification. Do not interact with other windows during the check; Windows foreground restrictions can interrupt automation.
`build/Release/qc_preview.exe build/Release/QuickCheat.exe` captures only the app's popup/editor into `build/qa` and samples idle resource usage. It uses separate sample data. The GitHub workflow builds and runs the headless core tests; interactive tests require a desktop session.

## Compatibility and limitations

Windows x64 only. The intended API baseline is Windows 10 1809+ / Windows 11; validation so far covers the development machine, not a complete OS/monitor matrix. A manifest enables common controls v6, Per-Monitor V2 DPI and normal user privileges. No undocumented APIs or forced focus bypasses are used.
Profiles match the executable filename, case-insensitively. Different installations with the same filename share a profile; browser tabs and application versions are not separate contexts. Protected/elevated apps may block input; focus changes cancel stale clicks. The app does not elevate itself. A successful SendInput call means Windows accepted the events, not that the target application performed the expected command.
This alpha supports one key combination per shortcut. It has no hold-to-show mode, multi-step macros, workflows, dock, online sharing, automatic startup, installer or updater. System dialogs and native scrollbars follow the OS where documented APIs do not expose per-app theming. DWM titlebar color/corner attributes require supported Windows versions and degrade gracefully; no private uxtheme exports are used. Live OS-theme transitions, High Contrast and mixed-monitor/DPI behavior still require manual acceptance on additional machines.

## Development

Gitflow: `main` contains delivered releases; `develop` integrates completed features; `feature/*` holds work; `release/*` prepares release candidates; `hotfix/*` fixes delivered releases. The initial scaffold on main is preserved until release promotion. Each implemented version has a child page in Hugo Library under **Apps > Quick Cheat**, with decisions, changes, validation and limitations.
License: MIT; see [LICENSE](LICENSE).
