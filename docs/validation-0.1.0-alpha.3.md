# 0.1.0-alpha.3 validation

- Authored vector keycap/Q icon inspected at 256px; generated PNG-backed ICO includes 16, 20, 24, 32, 40, 48, 64, 96, 128 and 256px, with transparent corners.
- RED: resource test rejected the prior executable, which had no custom icon. GREEN: native LoadImage/GetIconInfo loaded all ten embedded sizes and confirmed their dimensions after resource integration.
- Windows x64 Release build and all 4 headless tests passed (icon resources, core, large store, themes). Runtime shortcut/theme logic unchanged; no repeated desktop input tests needed for this resource change.
- App icon is resource 1; tray uses it and host/editor/popup classes load the same embedded resource. No image decoder, runtime asset path or new app dependency was added.
- Desktop shortcut created at C:/Users/hugou/Desktop/Quick Cheat.lnk and read back through WScript.Shell. Target and icon: D:/git/hugouchoasborges/quick-cheat/build/Release/QuickCheat.exe; working directory is its containing folder. Existing unrelated shortcut targets are preserved by the script.
- App was asked to exit its message loop normally to replace the locked executable; saved local data was preserved. Updated app is reopened after packaging.
