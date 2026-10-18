# PIN and responsive shortcut tiles — 0.2.0-alpha.3

User-approved behavior: PIN top-right, off by default. Unpinned popup hides on focus loss, leaving tray process alive. Pinned popup stays visible until Escape/Close; Close appears beside PIN only when pinned. Keep pinned popup visible after execution and hotkey reactivation. Management windows remain usable as owned windows above the pinned popup.

Layout: native ListView tile mode with category labels inside each card, compact cards with description and keycaps, automatic columns by available width. Native selection/keyboard/double-click, explicit Execute, shared themes and full-text reader for long descriptions. Keep all data schema v3 unchanged.

Steps: add pin/focus policy; replace popup listbox only with native tile view; adapt selection and layout; compile without running/adding tests; inspect one DEMO batch and at most one confirmation; review focus/selection hazards; document version and manual acceptance in Library; Gitflow feature/develop/release, no main promotion or public release before acceptance.