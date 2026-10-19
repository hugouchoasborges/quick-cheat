# 0.5.0-alpha.4 — compact popup

Local candidate; public release remains alpha.3. Hugo asked to minimize the
header/footer using a screenshot of the mono profile. Adopted one compact row:
context, search and 30px Settings/PIN/Close controls. Context text ellipsizes
visually while retaining its full accessible name. No separate footer panel or
Escape button; Esc still hides through existing dialog handling. Count and
24px-high Expand/Execute actions are embedded at the bottom.

Content begins at50 logical pixels instead of118, or84 with workflow tabs instead
of152. Bottom reserve38 instead of68. Settings header46 instead of54.
Shared metrics keep long shortcut details and workflow readers aligned.
No schema, input-execution or browser detection change.

Evidence: Release SkipTests succeeded, build/compact-popup-build.log.
Existing DEMO helper completed (build/compact-popup-preview.log);33 new PNGs
in docs/images/v0.5.0-alpha.4/. One batched visual pass inspected Dark/Light popup,
Dark complete workflow and General Settings. No corrective visual round needed.
Native Win32 manual review used; Impeccable platform detector does not recognize
this platform. No automated tests added or run. Other environments remain manual.

Validate: one shortcut/no workflows; long executable/hostname; narrow/expanded
window; PIN/Close; search; Expand/Execute; Esc and keyboard focus; workflows with
long/image content; Settings Back and draft preservation; multiple DPI monitors.
Known residual-process shutdown issue remains open; this is not a shutdown fix.

Hugo subsequently confirmed that alpha.4 opened and that he closed it himself.
The later absence of a window is therefore not evidence of startup failure.
This confirmation does not cover all compact-layout acceptance checks.

Library: [0.5.0-alpha.4 evidence](tgslib://fc607c940f74e3ce).
