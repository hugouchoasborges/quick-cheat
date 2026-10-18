# Quick Cheat 0.1.0-alpha.2

Approved: Raycast-like visual direction across popup and editor/settings, Dark + Light + System, explicit Execute button; clicking a row must only select.

1. Test schema-v1 migration to System, theme-v2 persistence, invalid theme rejection, search matching and selection/execution separation.
2. Introduce shared GDI palette/brushes/native-control rendering; use documented UISettings events for System and DWM titlebar attributes with graceful unsupported-OS handling.
3. Restyle popup as a searchable command palette, category headings, right-aligned keycaps, selection and explicit Execute. Preserve external app identity while search owns focus. Restore focus on closing.
4. Apply shared palette to native editor fields, lists, buttons and combos; add persistent theme selector.
5. Verify headless data/search tests and controlled desktop integration (selection does not execute; Execute does). Capture popup/editor in Light/Dark and System, inspect in one batch and correct findings in one batch, then confirm once.
6. Update version metadata, docs, Library child page and Harness. Build portable candidate, publish Gitflow branches; preserve main.
