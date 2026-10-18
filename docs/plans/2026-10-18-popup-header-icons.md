# Popup header refinement — 0.2.0-alpha.5

Requested: enlarge and center the application name; replace PIN with a pin icon and CLOSE with a red X. Preserve the existing native appearance and behavior.

Implementation: dedicate the first header line to the centered active-app title, move search below it, and adjust shortcut/workflow content offsets. Draw icons in the existing owner-drawn native buttons, with DPI scaling, accessible button names, hover tooltips and existing focus/selection states. Keep Close conditional on PIN. Use system colors in High Contrast.

Delivery: build Release with SkipTests, inspect isolated Dark/Light popup and workflow captures, document the version in Library and retain Gitflow. No storage migration or dependency needed.
