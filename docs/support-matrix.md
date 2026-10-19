# Support matrix and measurement protocol

0.5.0-alpha.1 is a local engineering candidate. No new compatibility guarantee.
The code targets Windows 10 APIs (`_WIN32_WINNT=0x0A00`); this does not establish
the minimum supported OS. Win32/C++20, native controls, Windows x64 only.

| Area | Evidence / required validation |
| --- | --- |
| Host OS | Local build host reports display 26H2, build 26300.9711, x64; legacy ProductName says Windows 10. This is one development host, not stable OS support certification. |
| Windows releases | Windows 10/11 stable minimum builds remain unverified. Record exact edition/build on each validation machine. |
| Edge / Chrome | Native UIA only. Exact installed versions and real browsing acceptance remain to record; no extension, title inference, clipboard or Ctrl+L. Firefox excluded. |
| DPI / monitors | Pending: 100/150/200%, mixed DPI, negative coordinates, disconnect/reconnect, Dock pagination and placement. |
| Keyboard | Pending: US and PT-BR ABNT2, AltGr/dead keys, Tab/Space and non-ASCII editing. |
| Permissions | Pending: standard user, elevated target, unavailable provider; fail to generic profile and refuse unconfirmed site execution. |
| Data | Schema v4 / pre-v4 backup; collection 2 reads 1. Migration, corruption, missing attachments and large libraries still need manual recovery checks. |

Use docs/validation-0.4.0-alpha.1.md and alpha.2 cumulatively, recording OS/browser
versions, DPI, layout, permissions, exact steps and observed/expected result.
Check changed tabs/windows, typed uncommitted address, internal pages, hidden
toolbar, pinned popup, canceled confirmation and stale provider results.
Do not equate preview captures with live browser recognition or shortcut safety.

For a performance baseline, use a separate empty DEMO data directory and a
representative fictional large collection; avoid personal data in reports.
Record commit/build configuration, CPU/RAM, Windows/browser versions, collection
counts and whether browser/provider is cold. Wait 30 seconds after startup.
Using Task Manager/Process Explorer, record private bytes, working set, threads,
handles and CPU-time delta over 60 seconds with popup hidden and Dock disabled.
Repeat with pinned popup, Dock enabled, and after repeated open/hide cycles.
Idle CPU fraction = CPU-seconds delta / wall seconds / logical processor count.
Separately record hotkey-to-visible and context-read latency, cold and warm,
with sample count, measurement method, median and worst observed value.
Human timings are estimates; sub-millisecond precision must not be claimed.
Compare like-for-like builds/data on the same host. No telemetry or navigation
history collection. No performance budget is agreed yet; discuss measured values
with Hugo before selecting limits. A live user session is only an observation,
not a controlled idle baseline. This delivery adds no polling to the application.

Observed existing process only (2026-10-19, PID 42112, previous development build):
private bytes 4,812,800; working set 29,749,248; 319 handles; 6 threads.
This single snapshot during development is not a 0.5 baseline. CPU-idle and
latency measurements, isolated workload and a second machine remain pending.
