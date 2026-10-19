# Browser header and dock pages — 0.4.0-alpha.2

User requested hostname before browser process and no dock scrollbars. Browser headings use hostname — executable; unknown site uses executable alone. Other application headings retain existing names. Settings retains its Settings heading.

Dock removes native scroll styles and handlers. Existing wrapping is retained when content fits. Oversized docks show bounded pages with Previous/Next, preserving configured scale, orientation, shortcut order and nonactivating input. Pagination is session state, resets for a different profile and clamps when layout changes. Dock settings also receives two-column organization, native checkboxes for enabled/collapsed, appearance controls and a selected count beside the ordered shortcut list. Preserve explicit Apply and all pending-edit guards. No data/schema change.

- [x] Centralize browser heading formatting, including missing profile/site fallback.
- [x] Replace dock overflow scrolling with pages, within monitor work area.
- [x] Compile SkipTests, inspect normal/overflow DEMO, update docs and Library child.
- [x] Integrate feature → develop → release/0.4.0-alpha.2. Main remains published0.3.

No tests added/run. Existing preview helper gains a DEMO fixture option for 64 dock shortcuts at 200% scale; it captures UI without assertions.