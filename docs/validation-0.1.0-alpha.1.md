# 0.1.0-alpha.1 validation

Development host: Windows build 26300, x64; Visual Studio 2022/MSVC 19.38 and Windows SDK 10.0.26100.0. Intended minimum Windows 10 1809; not verified on Windows 10 or multiple OS builds.

- Core test RED: parser stub failed the expected Ctrl+Shift+S assertion before implementation.
- Core test GREEN: valid/invalid chords, filename normalization, Unicode/escape roundtrip, unsupported schemas, truncated/trailing corruption rejection, durable save and previous-save backup.
- Review found mismatched read/write size limits. A failing regression reproduced it; a shared 8 MiB UTF-8 limit now permits a 16,000-shortcut ASCII store to save/reload and rejects oversized multibyte data without replacing the previous file. The regression passes.
- Release build completed without compiler warnings in the final build.
- Controlled desktop integration passed: WM_HOTKEY dispatch, empty popup, native editor save, non-activating popup, actual F6 delivered through SendInput on click, foreground change, restart persistence and clean shutdown. Tests use isolated data and terminate only their own child processes.
- Actual Ctrl+Alt+Space registration was occupied on this machine. First-run fallback to Ctrl+Alt+Shift+Space added; configured conflicts are reported without a blocking startup dialog. Automated physical-input activation was inconsistent in this desktop environment, so the repeatable integration test dispatches WM_HOTKEY. Physical-key activation remains a manual acceptance check.
- Interactive focus tests can be interrupted by activity in unrelated windows and by Windows foreground restrictions. The harness transfers focus between its controlled targets; the product does not bypass focus restrictions.
- Native popup and editor captures inspected: readable labels, keycaps, application selector, category combo and editor controls, no observed clipping at the captured 96-DPI layout. Other DPI values and mixed-monitor setups remain manual acceptance checks.
- One idle sample before opening UI: working set 11.0 MiB, private commit 2.11 MiB, CPU time 0 ms over a 1-second interval. This is one local sample, not a maximum or a universal performance guarantee. Executable approximately 330 KiB before packaging; exact size/hash recorded in the package manifest.

## Manual acceptance

1. Activate a normal application and test physical activation, toggle and click execution with its actual shortcuts.
2. Record chords (including modifiers and function keys), save, edit, delete and restart; confirm categories and activation changes persist.
3. Try an occupied global hotkey, held modifiers, an elevated target, a target that closes during interaction and Alt+Tab while a mouse button is down. No input should be sent to an unintended app.
4. Move between monitors at 100%, 125%, 150% and 200% scale; verify popup bounds, editor resizing, text and clicks.
5. Confirm notification-icon recovery after Explorer restart and recovery from a copied corrupt data file using its backup.
