# Edge installed-app context

Approved scope: recognize the Edge-installed Monday app, reuse existing website
shortcuts, preserve fresh execution checks, and validate normal/browser app switches.

The live capture found WebAppOriginText in native Edge chrome. The existing reader
only accepts a toolbar Edit. Prefer the existing address source whenever present;
fall back to the app header only for a registered Edge PWA whose manifest start URL
matches the header hostname. Installation metadata alone is never a current URL.

Reject fallback for incomplete/expired scans, edited or ambiguous address fields,
multiple app tabs, conflicting header names, and nonempty external-navigation
CustomTabBarTitleOriginView labels. Never inspect renderer/page descendants.
Keep the existing window/process/foreground, generation and execution rechecks.
No profile, collection or persistence schema change; no extension or polling.

Alternatives: installation URL alone cannot establish navigation context; reading
page contents would expand access unnecessarily. Reuse native chrome and verify
the registered installation instead.

Implementation and validation:
- [x] Add failing policy/parser tests and an optional real-window browser test.
- [x] Resolve AUMID to a registered PWA manifest using native Windows APIs/XML.
- [x] Collect native header/external-bar evidence within the existing UIA limits.
- [x] Run targeted tests, including the installed Monday and ordinary Edge window.
- [x] Build build/Release after normal exit; reopen and request manual acceptance.
- [x] Document the local candidate in Library, including remaining limitations.

Outcome: five CTest tests pass; Hugo accepted shortcut display and keyboard
execution in the installed app. External navigation in the same app window could
not be reproduced by Hugo and remains a documented validation limitation.

Risks: native class names and Edge's package metadata are adapter details and may
change. Unknown layouts fail to browser-only. Test external navigation explicitly;
do not claim every Edge PWA variant is covered by the single Monday installation.
