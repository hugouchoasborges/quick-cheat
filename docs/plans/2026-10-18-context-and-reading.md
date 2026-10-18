# Context retention and reading space — 0.2.0-alpha.2

Requested: retain the last foreground app while interacting with Quick Cheat; improve reading with many shortcuts/workflows.
Cause found in code: onForeground overwrites target even when identify returns empty, including null/unidentifiable transitional foreground windows. Popup activation also clears selection even when no valid context changed. Layout is fixed at 740x480, with a 104px workflow detail field.

1. Ignore own, transient shell/menu, null and unidentifiable foreground windows. Retain last valid app; continue detecting actual external app switches. Keep execution safeguards against stale/closed targets.
2. Use a larger default popup (1040x760 logical pixels), constrained to monitor work area. Add native edge resizing, header dragging, Expand/Restore and session-size retention.
3. At wide sizes, put workflow tree and full step reader side by side; at narrow sizes stack proportionally. Wrap shortcut descriptions with measured row heights.
4. Preserve themes, data schema v3, shortcuts/workflows and native controls; no polling or new frameworks.
5. Compile without running/adding tests. Inspect a bounded batch of isolated demo captures. Record acceptance cases and create a new Library child page; keep main on published alpha.4.