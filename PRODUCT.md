# Quick Cheat

## Platform
Windows desktop, C++20 + Win32 + native controls; x64 builds. Stability, responsiveness and low idle memory/CPU are priorities. English UI. MIT.

## Purpose
Help users remember and deliberately execute shortcuts for the foreground application. Users create and organize their own local shortcuts; profiles follow the external application even while Quick Cheat has keyboard focus.

## Confirmed scope
Popup, local profile/shortcut editor and settings. Hugo selected the supplied Raycast screenshot as visual reference for both popup and management UI, and explicitly requested Dark, Light and System themes. A single click selects a shortcut row; double-click, Execute or Enter executes it. Existing global toggle remains configurable. Local reference workflows are approved for 0.2: titled, ordered text steps with native expand/collapse and editing. Dock and community are deferred.

## Interaction
The popup is a compact command palette: search, categorized rows, keycaps, selected state, explicit Execute and management access. Search intentionally takes keyboard focus while preserving the external target identity. Closing restores that target when appropriate. Selection alone must never inject input.

## Delivery
Gitflow with main reserved for published releases. Each implemented version receives a Library child page with decisions, changes, validation and limitations. Product source: Apps > Quick Cheat (`tgslib://51be8b49abe30ba3`).

Context and readability: retain the last valid external app throughout Quick Cheat UI interaction. Popup supports native resizing, Expand/Restore, responsive workflow reading and wrapped shortcut descriptions. Window size preference is session-only; no storage schema change in alpha.2.

Alpha.3: PIN defaults off; popup hides on focus loss while tray stays running. PIN on keeps popup open until Escape/Close, including after shortcut execution. PIN state is session-only. Shortcut cards contain category, description and keys in a responsive grid.

Alpha.4: workflow steps accept basic Markdown and local image attachments, rendered in the main popup. A native attachment picker copies images into the data folder. Markdown and relative references remain schema v3 text. Backups include the attachments directory; guides remain local reference material. External image downloads and HTML execution are excluded.

Alpha.7: Settings separates General, Shortcuts and Workflows. General owns appearance, activation and data-folder access. Content tabs share the selected application; the workflow editor is embedded, with no second management window. Keep explicit saves and discard confirmation across tab/app changes and close. Popup actions enter their content tab; tray Settings enters General.

## Portable collections (0.3 candidate)
Per-application JSON import/export includes saved shortcuts, workflows and referenced PNG images. Add skips exact duplicates; Replace confirms and changes only the matching application. One active profile per executable; global settings and category view state remain local. Community publishing, voting and installed variants are future work.

## Settings navigation (0.3 candidate)
Gear beside PIN opens General in the same main window; tray Settings uses the same panel. Back returns to the contextual library, confirming pending edits. Escape/focus dismissal preserve drafts. The separate settings window and footer Manage actions are superseded.

## Contextual Dock and websites (0.4 candidate)
Native Windows Dock: ordered selected shortcuts, orientation, scale, opacity, grip position and collapse state; shows only for its foreground target and does not take focus when clicked. Website context is native UI Automation for Edge/Chrome, expressly chosen over extensions. Exact hostname per executable; browser content and website content displayed together. Browser-only fallback, fresh confirmation before site execution. Firefox and Android deferred. Local v4 + collection format2; candidate awaits acceptance.
