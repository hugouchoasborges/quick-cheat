# 0.1.0-alpha.2 validation

Development host: Windows build 26300 x64, MSVC 19.38, Windows SDK 10.0.26100.0. Alpha candidate; not a complete OS/DPI matrix.

- Theme test RED observed before persistence/search implementation. GREEN: v1 defaults to System, v2 roundtrips explicit themes, invalid mode rejected, multi-term title/category/chord matching.
- Release build and 3 headless tests passed: core, large-store regression and themes/search. No warnings in final build.
- Controlled desktop integration passed: popup search focus, empty Execute disabled, theme changes persisted through native UI, matching/nonmatching searches, row clicks never execute, Execute button delivers F6 through SendInput to the retained external target, foreground switching and restart persistence.
- Independent review found a pending foreground-event race when reactivating the popup. WM_MOUSEACTIVATE now synchronously captures the external foreground before activation and clears/disables selection when context changes; row selection/arrow navigation deliberately re-enables Execute. Button-down snapshots and refresh invalidation retain the existing stale-click protection.
- Final captures inspected in one batched initial round and one confirmation round: popup/editor in Dark, Light and System at 96 DPI. Corrected dark-button corner backgrounds, titlebar theme colors and secondary labels. Native system dialogs/scrollbars can retain OS appearance.
- Impeccable detector returned no findings for the changed C++ UI targets; native visual inspection remains the primary evidence.
- Idle sample before opening UI: working set 13.64 MiB, private commit 2.68 MiB, 0 ms CPU over 1 second. One local sample, not a resource ceiling. Fonts are cached per popup DPI; no polling, animation loop, web framework or third-party runtime added.
- Documented Windows UISettings color notifications implement System mode. Actual live Windows theme/High Contrast transitions were not forced during testing; they remain manual checks, along with 100/125/150/200% mixed-monitor scaling, physical activation, assistive technology and protected/elevated applications.
- Integration uses WM_HOTKEY dispatch rather than simulating physical global activation. Interactive checks require a quiet desktop and can be interrupted by unrelated foreground activity.

## Manual acceptance

Switch Appearance through all three values and restart. In System, change Windows app mode and check both surfaces. Try High Contrast and alternate DPI monitors. Confirm search keyboard navigation, Escape restoration, mouse selection without execution, Execute and held modifiers. Alt+Tab during a button press must not inject into a substituted target. Check Ctrl+, and native keyboard/assistive-control behavior.
