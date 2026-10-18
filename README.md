# Quick Cheat

Remember less. Keep your keyboard shortcuts close.

Quick Cheat is a small Windows app that shows the shortcuts you saved for the application you are using. Search by name, category or keys, then run a shortcut without leaving your workflow.

Built with C++ and native Windows controls, with a focus on speed and low memory use. Everything stays on your computer. No accounts, telemetry or internet connection required.

## Download

[Download Quick Cheat for Windows x64](https://github.com/hugouchoasborges/quick-cheat/releases/tag/v0.1.0-alpha.4)

Download the ZIP, extract it and run `QuickCheat.exe`. No installer or additional runtime is needed. This is an early alpha release.

## A shortcut library for each app

![Quick Cheat popup in dark mode](docs/images/popup-dark.png)

- Open a searchable popup for the current application.
- Organize your shortcuts into categories.
- Click once to select; double-click, press Enter or click **Execute** to run.
- Add, edit and record shortcuts in the local library.
- Choose **System**, **Light** or **Dark** appearance.

### Light theme

![Quick Cheat popup in light mode](docs/images/popup-light.png)

### Manage your shortcuts

![Quick Cheat shortcut library](docs/images/editor-dark.png)

Screenshots use isolated demo data. The sample shortcuts are illustrative; applications do not come with preloaded profiles.

## Getting started

1. Launch Quick Cheat. It runs in the Windows notification area.
2. Open an application and press **Ctrl+Alt+Space** to show its shortcuts.
3. Choose **Manage shortcuts** to add descriptions, categories and key combinations.
4. Search or select a shortcut, then double-click it, click **Execute** or press Enter. Press **Esc** to close the popup.

You can also open the library from the tray icon. If the default hotkey is already in use on first launch, Quick Cheat tries **Ctrl+Alt+Shift+Space** and notifies you. Change the activation hotkey in the library. Release held modifier keys before executing a shortcut.

## Local data

Shortcuts and settings are stored in `%LOCALAPPDATA%\QuickCheat\shortcuts.qc`. The previous save is kept as `shortcuts.qc.bak`. To restore it, exit the app, keep a copy of the current file and replace it with the backup.

For separate demo or portable data, launch `QuickCheat.exe --data-dir <directory>`. Only one instance can run per Windows session.

## Current scope

Windows x64 only. Windows 10 1809+ and Windows 11 are the intended API baseline; compatibility has not been verified on every Windows version or monitor setup. Elevated or protected applications may block shortcut execution. Profiles are matched by executable filename, so apps with the same filename share a profile.

Each shortcut contains one key combination. Workflows, docks, community sharing, an installer and automatic updates are planned for later consideration.

## Build from source

Install Visual Studio 2022 with **Desktop development with C++**, the Windows SDK and CMake tools. In PowerShell, run `./scripts/build.ps1` to build Windows x64 Release and run the headless tests. The executable is written to `build/Release/QuickCheat.exe`.

Run `./scripts/create-desktop-shortcut.ps1` to create a desktop shortcut to that build. `qc_preview.exe` can capture the popup and library using separate demo data; see [validation notes](docs/validation-0.1.0-alpha.4.md).

Development uses Gitflow: `develop`, `feature/*`, `release/*` and `hotfix/*`. `main` tracks published releases.

## License

[MIT](LICENSE)
