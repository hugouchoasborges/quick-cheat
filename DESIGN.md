# Quick Cheat visual direction

Visual authority: Raycast screenshot supplied by Hugo; applies to popup and editor/settings. This is a Windows command palette, rendered with native controls and GDI. Existing product truth is preserved.
Application mark: an original lavender keyboard key with a geometric Q on a charcoal rounded square. Authored SVG is in assets/quick-cheat.svg; generate-icon.ps1 creates its PNG and ten-size Windows ICO. The executable embeds the ICO so distribution and desktop shortcuts need no loose icon asset.

- Compact charcoal or warm-white surfaces, thin separators, restrained rounded selections and keycaps. System defaults to Windows app color preference and updates through documented UISettings color-change events. Explicit Dark/Light remain pinned.
- Search is the popup's first visual/focus anchor. The application name remains visible as context. Categories are quiet group headings; command descriptions and right-aligned keycaps form compact scan lines.
- Selected and hovered rows have distinct tonal fills; accent is used for the primary Execute/Save actions and keyboard focus. Clicking a row only selects it. Execute is an actual native button, disabled without a selection.
- Footer keeps visible Execute and Manage controls. Native keyboard navigation and Escape are supported; Enter invokes the explicit primary action, not selection.
- Editor uses the same colors, typography, borders, field treatment and action hierarchy. Theme selector exposes System, Light, Dark in English and persists immediately.
- Segoe UI uses installed Windows fonts, with 12–18px hierarchy at 96 DPI. Layout scales by monitor DPI. Draw only on input/invalidation; no animation loops, blur, custom font assets or bitmap backgrounds.
- High Contrast uses Windows system colors. Common system dialogs and native scrollbars retain OS rendering where documented APIs do not expose full app-specific theming; do not use private uxtheme exports or registry theme hacks.

Dark tokens: background #1E1E1E, panel #252525, field #292929, selection #363636, border #444444, text #F4F4F4, secondary #B4B4B4, accent #CEB4FF.
Light tokens: background #FAF9F7, panel #F1F0ED, field #FFFFFF, selection #E6E3ED, border #D4D0CC, text #222126, secondary #625F68, accent #6941A5.
