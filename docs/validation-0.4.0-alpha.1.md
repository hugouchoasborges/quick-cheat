# 0.4.0-alpha.1 — contextual dock and websites

Candidate awaiting manual acceptance. Main remains the published 0.3.0-alpha.2 release, with its subsequent screenshot documentation correction.

## Evidence

- Release compilation with `./scripts/build.ps1 -SkipTests` succeeded. Final log: `build/contextual-dock-final-build.log`.
- Independent inspection covered model/migration/collection identity, UI Automation integration, dock input/focus and inline settings.
- Review fixes: cancel pending site execution on close/hotkey reopen; prevent parallel chord execution; schedule browser refresh without resetting an existing one-shot timer; read the address before popup takes focus; restore browser focus before confirming site execution; consume dock button mouse input without activating it; cancel captured child clicks when hiding; refresh dock before shell-context exclusions; synchronize the selected website profile in the combo.
- Existing DEMO capture helper generated isolated Dark/Light/System views. Dock dark, Dock settings dark and website editor light were inspected visually. Images use new versioned paths. The website editor sample is fictional content, not evidence of live browser recognition.
- No automated tests were added or run. Live browser navigation, multi-window correlation and shortcut execution await manual acceptance.

## Dock acceptance

1. Choose an application in Settings → Dock. Check shortcuts, arrange them, enable the dock and Apply. Confirm horizontal/vertical layout, scale 50–200% and opacity 20–100%.
2. Activate that app: the dock appears. Alt+Tab to another app, the desktop, taskbar or Quick Cheat: it disappears. Return: the configured dock reappears.
3. Click a dock shortcut; its app must keep focus and receive exactly one chord. Cancel a pressed button by switching apps; it must not execute later.
4. Drag by the grip, collapse/expand and restart. Check persisted placement, ordered selection and collapse state. Check multiple DPI monitors and long lists/scrolling.
5. Delete a shortcut referenced by the dock; it should disappear while remaining references still select the correct commands.
6. Cancel leaving Dock with unsaved edits, hide/reopen Settings and exit from tray with pending edits. Confirm discard guards.

## Website acceptance — Edge and Chrome on Windows

1. Choose a browser profile; use Website... and enter `example.com` or an HTTP(S) URL. Save a shortcut and workflow for that hostname. Verify that the profile selector distinguishes browser and website profiles.
2. Open that hostname in Edge/Chrome and activate Quick Cheat. The header should show the detected hostname. General browser shortcuts and website categories appear together; workflows have separate Browser/Website groups.
3. Switch tabs, navigate across hostnames and switch between multiple windows and browsers. No context should leak across HWND/process pairs. Check pinned popup and website-specific dock override/fallback.
4. Type an address without navigating, then activate Quick Cheat: ambiguous/focused address input must fall back to browser content, not treat typed text as the current page. Check internal browser pages, hidden toolbar/PWA windows and elevated browsers.
5. Click a website shortcut: browser focus is restored and the hostname is confirmed asynchronously before sending. A changed/unavailable hostname must refuse execution. Reopen or close during confirmation; no stuck request or later unwanted chord.
6. `example.com` and `www.example.com` are distinct. Paths/query/ports do not create separate profiles. International hostnames normalize to ASCII IDN. Firefox and Android are outside this candidate.

## Data and portability

1. Open existing v1/v2/v3 data and save: v4 is written and `shortcuts.qc.pre-v4.bak` preserves the original, alongside the normal backup. Back up attachments and ui-state.ini too.
2. Export/import a website profile with workflows/images/dock selection. Format 2 includes hostname and dock; older format-1 collections still import with generic application context and disabled dock.
3. Add deduplicates shortcuts and remaps dock references; Replace affects only the matching executable+hostname. Other sites and the generic browser profile stay unchanged. Invalid dock references/configuration abort import/save.

Native browser detection depends on accessibility exposed by the installed browser version. It cannot reliably identify private windows. See [browser integration](browser-integration.md) and [collection format](collection-format.md).

Final inspection also addressed destroyed browser windows: matching request flags clear before the window-liveness guard, and invalid requests return an empty result. No stale chord is sent.

Library: [0.4.0-alpha.1 decisions](tgslib://cafe2685dabd7233).
