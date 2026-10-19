# 0.6.0-alpha.2 — Ctrl + backslash

Local candidate only. Public release remains v0.5.0-alpha.3.

## Cause and correction

Hugo reported that Ctrl+backslash could not be registered. Native observation:
active foreground keyboard layout 0x04160416, VkKeyScanEx(backslash) = 0x00E2.
The key-name table omitted VK_OEM_102 (0xE2), so Record's formatChord guard
rejected it. The text parser accepted Backslash but rejected a literal backslash.

Add the stable IntlBackslash name for VK_OEM_102. Literal backslash input uses
VkKeyScanExW/GetKeyboardLayout and preserves required Shift/Ctrl/Alt modifiers;
reject unavailable/unsupported mappings. Keep existing Backslash = VK_OEM_5.
Explicit names round-trip independently of layout. Store/collection virtual-key
numbers and formats are unchanged; no user data rewritten for this correction.
Older clients may reject profiles/collections containing the newly supported key.

Native reference: [Microsoft virtual-key codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).

## Evidence and limits

Build: scripts/build.ps1 -SkipTests, existing build/Release/QuickCheat.exe.
Log: build/backslash-build.log, successful Release build. Existing C4324 alignment
warning in website_editor.cpp remains. No automated tests added or executed.
No new UI layout; existing alpha.1 screenshots remain representative. No claims
of manual end-to-end verification until Hugo reports capture/save/execution.
Community service and later roadmap stages remain pending. No public release.

## Manual validation

1. Settings > Shortcuts: select the intended application and enter a description.
2. Type Ctrl+backslash in Keys, then Save. On ABNT2 it should show Ctrl+IntlBackslash.
3. Select New shortcut, click Record and press Ctrl+backslash; save another entry.
4. Change selection/reopen Settings; verify the stored combination remains.
5. In an application where that combination has a known effect, run the saved
   shortcut via Quick Cheat and confirm the target receives it.
6. Existing Ctrl+S and named Backslash entries should retain their key values.

Opening, persistence, execution and behavior on other layouts need manual evidence.
