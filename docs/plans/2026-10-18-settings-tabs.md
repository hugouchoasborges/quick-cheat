# Settings tabs — 0.2.0-alpha.7

Reorganize the existing native settings window into General, Shortcuts and Workflows. Preserve the approved Raycast palette, native controls, local storage and low idle cost.

General contains appearance, activation shortcut and data-folder access. Shortcuts contains a shared application selector, shortcut list and focused editing form. Workflows uses the same application selector and embeds the existing workflow editor in the tab, with separate workflow, step and Markdown editing columns. Avoid a second management window and competing global/application controls.

Use themed native navigation buttons with selected states and keyboard focus. Keep application selection across tabs. Preserve explicit saves and confirmation before discarding pending workflow/shortcut/global edits during navigation or close. Popup management actions enter the appropriate tab; tray Settings enters General. Create workflow controls only when needed, and retain existing Markdown/image functionality.

Compile Release with SkipTests; inspect a bounded batch of isolated demo captures for all three tabs in Light/Dark. Document limitations and pending manual acceptance in Library. Use feature → develop → release Gitflow; leave main on the published version.
