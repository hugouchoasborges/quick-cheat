# Collapsible shortcut categories — 0.2.0-alpha.6

Requested: category name and divider above shortcut cards, expandable/collapsible sections, last state saved for each category of each app.

Use native ListView groups with themed header drawing and native state/accessibility. Keep responsive tiles below each header, removing redundant category captions. Header click toggles; native focused groups support Left/Right/Space. Skip hidden shortcuts in selection/execution and search arrows.

Persist view preferences separately in ui-state.ini using executable/category keys encoded as hexadecimal, case normalized. Read defaults as expanded. Search shows matches with temporary expanded sections, preserving saved states. Save only changed states on interaction and before context replacement, refresh, hide or shutdown; no polling, dependency or schema migration. New/renamed categories start expanded; stale preferences are harmless and retained.

Build Release with SkipTests, inspect isolated Dark/Light captures in bounded passes, document in Library and prepare a Gitflow release branch. Manual restart, multi-app, keyboard and accessibility acceptance remains with the user.
