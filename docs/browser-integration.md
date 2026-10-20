# Native website detection

Quick Cheat uses Windows UI Automation to read the address field of the active
Microsoft Edge or Google Chrome window, or a supported installed Edge app's
verified native origin header. No extension, store account, browser
launch flag, web server, clipboard access, simulated keypress, or page script is
required. Firefox and other browsers are not supported by this initial adapter.

Only the normalized hostname is retained in memory for contextual profiles. The
adapter does not save URLs, paths, query strings, page text, or browsing history.
Native accessibility cannot reliably distinguish private windows; the same
hostname detection can apply there. It does not grant access to page contents.

## Behavior and limits

Packaged Edge web apps can also reuse their website profiles without an omnibox.
The adapter requires the native WebAppOriginText header, known app/external-bar
chrome and a matching start host from the registered PWA manifest. It rejects
nonempty external-navigation headers, multiple tabs, mismatches and ambiguity.
This is a conservative adapter for known Edge chrome; installation metadata is
never used alone to infer the current site. Open URL actions require a normal
browser tab; keyboard shortcut execution retains fresh context confirmation.

- Detection runs asynchronously on a dedicated COM MTA worker, created on first use.
- Foreground changes, popup activation and browser UI events request a fresh read;
  there is no continuous polling. Requests coalesce and superseded results are discarded.
- The browser window handle and process ID must still match. A read is accepted
  only while that browser owns the foreground, both before and after the query.
  Popup activation waits for the initial query; executing a website shortcut
  returns focus to the browser before confirming its hostname again.
- A bounded walk inspects browser chrome, skips document/renderer subtrees, and
  reads the Value pattern of a unique visible toolbar edit control, or the verified
  installed-app origin described above. Localized
  address-bar names are not used.
- An address field currently being edited returns no website: typed text is not
  assumed to be a committed navigation.
- UI Automation connection and transaction timeouts are 150 ms. A 400 ms traversal
  budget is checked between provider calls; it is not a strict end-to-end deadline.
  Slow providers cannot block Quick Cheat's UI or application shutdown.
- If the browser does not expose a suitable address source, its chrome changes,
  a popup/devtools toolbar makes the address ambiguous, or access is denied,
  detection returns unknown and ordinary browser shortcuts remain available.
- Website matching is exact by hostname: `example.com` and `www.example.com`
  are separate contexts. Paths do not create separate profiles.

Browser accessibility trees can change independently of Windows. This adapter
fails to the general browser profile instead of guessing from window titles.
Native detection should be manually accepted against installed Chrome/Edge
versions, multiple windows, tab switches, navigation, private browsing and an
unresponsive/closed browser before release.

## Implementation references

Microsoft recommends a separate MTA thread for UI Automation clients:
[UI Automation threading](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading).
Provider timeout settings come from
[IUIAutomation2](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nn-uiautomationclient-iuiautomation2).
