# Compact popup chrome

Hugo requested minimal header/footer based on the supplied mono application
screenshot. Single compact row: app context, search and 30px native icon buttons.
Keep full accessible context text with visual ellipsis. Preserve keyboard access,
PIN/close/settings, explicit Execute and Expand. Remove redundant Escape button
and full-width header/footer separators; keep a small count and inline actions.

Share content-top/tabs/footer metrics across shortcuts and workflow layouts so
long descriptions, split readers and settings remain aligned. Target body start
50px instead of118px; footer reserve38px instead of68px. No data/schema changes.
Settings keeps its own title/back flow with a46px header. C++20/Win32 only.

Use local0.5.0-alpha.4 on feature branch, without automatic publication. Compile
SkipTests in build/Release. Existing DEMO helper captures Dark/Light/System;
inspect popup, workflows and Settings in one batch; at most one corrective pass.
The Impeccable launcher does not recognize Win32; native manual review applies,
without changing the project to a web platform. No automated tests added/run.
