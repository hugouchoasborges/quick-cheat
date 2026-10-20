# 0.6.0-alpha.6 — packaged Edge apps

The live user capture showed no toolbar Edit in the installed Monday app, while
the same hostname was detected in a normal Edge tab. The new adapter recognizes
WebAppOriginText only when it matches a registered Edge PWA's manifest start host.
It checks the external-navigation header, single-tab layout and complete UIA walk.
The manifest alone never authorizes a website context.

No data/profile migration. Existing msedge.exe + hostname profiles, Dock and fresh
execution confirmation remain in use. No extension, browsing history, renderer
content traversal, clipboard or polling. Manifest XML is local and bounded, with
external entities/DTDs disabled. Installation changes are read on each fallback.

Validation:
- Policy/parser regression test failed before implementation, then passed.
- Real installed Monday: original reader returned empty; modified reader returned
  tgsgaming.monday.com using the actual NativeBrowserContext worker.
- Ordinary Edge: reader returned the active chatgpt.com hostname independently.
- Tests cover missing/mismatched installation, conflicting headers, external bar,
  multiple tabs, incomplete traversal, address presence and invalid/DTD XML.
- All five CTest tests passed after the final changes. The rebuilt Release popup
  displayed tgsgaming.monday.com — msedge.exe over the actual installed app.
- Hugo confirmed that Monday shortcuts appear and a keyboard shortcut executes
  successfully in the installed app (2026-10-20).
- Hugo could not navigate to another domain in that same app window; actual
  external-navigation behavior remains unvalidated. Synthetic policy tests cover
  rejection when the native external-navigation header is populated.
- Independent review found that depth truncation could hide disqualifying native
  chrome. Fixed by completing the app chrome walk up to depth 14 and rejecting
  any truncated fallback. Reviewer confirmed resolution; normal tabs retain 9.
  Native class names remain an Edge adapter detail, not a general PWA API.

Open URL shortcuts are blocked in installed apps: Ctrl+L/text injection requires
an editable browser address field. Use a normal tab for these actions.
Unpackaged apps/other browsers are outside this fallback. No release publication.
