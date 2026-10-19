# Compact Dock

User specifies the layout: size buttons to their text with 5 px padding on every
side, and minimize the rest of the Dock. Interpret pixels at 100% DPI/scale;
preserve native Windows DPI and existing Dock scale behavior.

Measure the actual font/text, remove the 180 px width and 38 px height, use 2 px
outer margins/gaps, a 12 px grip and compact collapse/page buttons. Pack variable
widths horizontally or vertically; retain bounded paging/order and no scrollbars.
Long labels can ellipsize only when constrained by available monitor space.
Preserve execution, double-click, menu, drag, opacity and user configuration.

Build SkipTests in the same Release path; no automated tests. Generate new DEMO
documentation captures, inspect a bounded visual pass and record limitations.
Document a new local version in README/changelog/Library/Harness; no publication.
