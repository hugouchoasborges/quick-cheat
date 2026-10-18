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

Each shortcut contains one key combination. Docks, community sharing, an installer and automatic updates are planned for later consideration.

## Workflows — 0.2.0-alpha.5 candidate

The development build adds local step-by-step guides for each application. In the shortcut library, choose an application and click **Manage workflows**. Enter a title, add your steps, reorder them with **Move up / Move down**, and click **Save workflow**. Changes to the step text are included when you save the workflow.

Write step explanations in Markdown: headings, **bold**, *italic*, lists, checklists, quotes, code and links render in the popup. Click **Attach image...** in the step editor to copy a PNG, JPEG, BMP or GIF into your local library and insert its Markdown reference. Save the workflow, then select the step in the popup to see the formatted explanation and image. GIFs use the first frame.

Images stay in `%LOCALAPPDATA%\QuickCheat\attachments`. Back up the entire data folder to preserve them; the `.qc` backup alone contains only text and image references. Removing a reference does not delete the image file. Imports accept up to 10 MiB and 24 megapixels, normalize to PNG up to 1600×1600 and 4 MiB, and display at most eight images per step. Steps retain the 4096-character limit. Remote images, HTML execution, tables and math rendering are not supported. Web links open only when clicked. The reader uses native Windows Rich Edit; no browser runtime is needed.

![Expandable workflow guides in the popup](docs/images/workflow-popup-dark.png)

The popup opens larger and fits the current monitor. Drag its edges to resize, drag the header to move it, or use **Expand / Restore**. Your preferred size is kept until the app exits. **PIN** is off by default: losing focus hides the popup while the tray app keeps running. Turn it on to keep the popup open; **Close** appears beside PIN. With PIN on, executing shortcuts or pressing the activation hotkey keeps the popup open. Press Escape while the popup is focused, or click Close, to hide it. PIN is remembered only for the current session. Clicking Quick Cheat retains the last valid external application context.

Applications with saved workflows show **Shortcuts / Workflows** tabs in the popup. Expand a guide and select a step to read its full text. At wider sizes, guides and step text appear side by side; smaller windows stack them. Shortcuts use responsive cards, with category, description and keycaps kept together. Long descriptions or key combinations have a full-text reader. Search filters workflow titles. These are reference guides; they do not execute commands.

The active application's name is larger and centered above search. Use the pin icon in the top-right corner to keep the popup open; the red **×** beside it closes the popup while leaving Quick Cheat in the tray. Hover over either icon for its label.

![Local workflow editor](docs/images/workflow-editor-dark.png)

This candidate is awaiting manual acceptance and is not the published download above. It reads existing v1/v2 data and writes v3 on the next save. Before upgrading an existing file, it preserves `shortcuts.qc.pre-v3.bak`, in addition to the normal previous-save backup. Older releases cannot read v3: to roll back, exit the app, keep a copy of the v3 file and restore the pre-v3 backup as `shortcuts.qc`. Workflows created in v3 are not available in older versions.

## Build from source

Install Visual Studio 2022 with **Desktop development with C++**, the Windows SDK and CMake tools. In PowerShell, run `./scripts/build.ps1` to build Windows x64 Release and run the headless tests. The executable is written to `build/Release/QuickCheat.exe`.

Run `./scripts/create-desktop-shortcut.ps1` to create a desktop shortcut to that build. `qc_preview.exe` can capture the popup and library using separate demo data; see [validation notes](docs/validation-0.1.0-alpha.4.md).

Development uses Gitflow: `develop`, `feature/*`, `release/*` and `hotfix/*`. `main` tracks published releases.

## License

[MIT](LICENSE). Markdown parsing uses [MD4C](THIRD_PARTY_NOTICES.md), also MIT licensed.
