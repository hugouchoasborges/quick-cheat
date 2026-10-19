# 0.6.0-alpha.3 — website links and Dock labels

Hugo approved same-tab navigation and an optional Dock-only label. Local candidate;
public release remains v0.5.0-alpha.3. No private document was fetched or opened.

## Behavior

Settings > Shortcuts: select a website profile, choose Open URL, enter a complete
HTTP(S) URL and save. Use the normal Dock settings list to add that action.
Dock label is optional for keyboard and URL actions. Empty uses the full default
button text; populated uses only the label. Popup/settings retain the full title.
Action, URL and Dock label edits participate in existing draft/discard protection.

Execution copies the action before async UIA confirmation of the source hostname.
Window/process/foreground must still match. Native Ctrl+L, Unicode URL and Enter
are submitted as a single input batch to navigate the active browser's same tab.
No extension, default-browser launch or clipboard write. Held modifiers/keys,
invalid URL, stale website/target and focus failures refuse navigation. A partial
input failure reports it and releases injected keys; inspect the address bar then.
Input queues/focus are not a browser transaction; live acceptance remains required.

## Persistence

Store v5 preserves a pre-v5 backup on first save and reads v1–v4. No eager rewrite.
URL length limit 2048, Dock label 120. URL actions require a website profile and
zero chord fields. Existing keyboard shortcuts preserve their virtual-key values.
Collection v3 adds url and dockLabel; reads v1/v2 with empty defaults. Add skips
exact matches including URL/label; Replace retains existing confirmation/backup.
Old clients cannot read v5/v3. Native catalog accepts collection-format metadata3;
Portal deployment/catalog format3 support is outside this local UI delivery.

## Validation

Build with scripts/build.ps1 -SkipTests in the existing build/Release path.
Release SkipTests succeeded: build/website-links-final-build.log, version0.6alpha3.
No warnings in the final incremental build; earlier C4324 is not claimed fixed.
No automated tests added/executed. Nine fresh PNGs in docs/images/v0.6.0-alpha.3;
--links-only DEMO capture succeeded (build/website-links-forms.log). Inspected URL
editor and label fields. Earlier helper used cross-process LVM_SETITEMSTATE without
marshaling and faulted the DEMO control; replaced with pointer-free key selection.
Capture dimensions now honor DPI. Temporary product tracing was removed.
Images show controls/fixture persistence, not real navigation, migration or imports.

## Manual checklist

1. Under library.tgsgaming.com create Open URL for the requested /doc/... URL.
2. Add it to the Dock; click and verify the SAME tab navigates, without extra tabs.
3. Repeat using Execute/Enter in the popup. Try a different active site: refuse.
4. Set a short Dock label; verify only Dock text changes. Clear to restore default.
5. Reopen settings; confirm saved action/URL/label, keyboard actions and Dock order.
6. Leave an unsaved URL/label edit and verify existing discard/draft protection.
7. Verify clipboard unchanged, migration backup, collection export/Add/Replace,
   invalid/oversized URLs and browser focus changes before execution.

Accounts/community deployment, update merging and localization remain pending.
