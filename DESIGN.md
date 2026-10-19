# Quick Cheat visual direction

Visual authority: Raycast screenshot supplied by Hugo; applies to popup and editor/settings. This is a Windows command palette, rendered with native controls and GDI. Existing product truth is preserved.
Application mark: an original lavender keyboard key with a geometric Q on a charcoal rounded square. Authored SVG is in assets/quick-cheat.svg; generate-icon.ps1 creates its PNG and ten-size Windows ICO. The executable embeds the ICO so distribution and desktop shortcuts need no loose icon asset.

- Compact charcoal or warm-white surfaces, thin separators, restrained rounded selections and keycaps. System defaults to Windows app color preference and updates through documented UISettings color-change events. Explicit Dark/Light remain pinned.
- Search is the popup's first visual/focus anchor. The application name remains visible as context. Categories are quiet group headings; command descriptions and right-aligned keycaps form compact scan lines.
- Selected and hovered rows have distinct tonal fills; accent is used for the primary Execute/Save actions and keyboard focus. A single click selects a row; double-clicking executes it. Execute is an actual native button, disabled without a selection.
- Footer keeps visible Execute and Manage controls. Native keyboard navigation and Escape are supported; Enter invokes the explicit primary action, not selection.
- Editor uses the same colors, typography, borders, field treatment and action hierarchy. Theme selector exposes System, Light, Dark in English and persists immediately.
- Segoe UI uses installed Windows fonts, with 12–18px hierarchy at 96 DPI. Layout scales by monitor DPI. Draw only on input/invalidation; no animation loops, blur, custom font assets or bitmap backgrounds.
- High Contrast uses Windows system colors. Common system dialogs and native scrollbars retain OS rendering where documented APIs do not expose full app-specific theming; do not use private uxtheme exports or registry theme hacks.

Dark tokens: background #1E1E1E, panel #252525, field #292929, selection #363636, border #444444, text #F4F4F4, secondary #B4B4B4, accent #CEB4FF.
Light tokens: background #FAF9F7, panel #F1F0ED, field #FFFFFF, selection #E6E3ED, border #D4D0CC, text #222126, secondary #625F68, accent #6941A5.

## Local workflows
Native themed buttons switch Shortcuts/Workflows only for profiles with saved workflows. A native tree provides expand/collapse and step selection; a wrapping, scrollable read-only field exposes full step text. Execute is hidden and disabled in this view. A separate owned native editor provides title, workflow and step lists, multiline editing, ordered-step actions and explicit Save workflow. Unsaved changes require confirmation before closing or switching guides.

## Reading space and context retention
Default popup: 1040x760 logical pixels, constrained to monitor work area. Native resize borders, header dragging and Expand/Restore retain preferred dimensions for the session. Workflows use a side-by-side guide/detail layout at 960 logical pixels or wider, and proportional stacking below that. Shortcut commands use responsive native ListView tiles with category, two-line description and nearby keycaps. Category is shown inside each card to avoid gaps between groups. A dedicated reader exposes descriptions or combinations that do not fit the card. Own UI, tray/menu transitions and failed foreground identification preserve the last valid external app context.

## PIN and popup dismissal
PIN is placed at the top right and defaults off at process startup. Unpinned focus loss hides only the popup; the tray process remains. Pinned popup stays visible across focus loss, execution and hotkey activation; Close is visible beside PIN only while pinned. Escape/Close hides it without clearing session PIN. Management windows are owned above the pinned popup.

The active-app title is centered on a dedicated header line, using the existing 19px display font instead of the small caption font. Search sits below it. Pin and Close are 36px native buttons with authored GDI strokes, DPI scaling, keyboard focus, accessible text and tooltips. Pin is tilted while off and upright on the accent fill while on; Close is a red X (system text color in High Contrast). Title bounds remain symmetric when Close appears and ellipsize long names.

## Markdown and attached images
Workflow step source remains editable Markdown text. A lazily created native Rich Edit reader renders basic formatting with the pinned MD4C parser, following the current palette and DPI. WIC normalizes attached images to managed local PNG files; only these relative references render. No browser engine, network image fetching or HTML execution. Clear rendered images on hide/tab switch and cache unchanged visible content; width changes use a one-shot resize debounce. Attach image... inserts a reference at the editor selection; Save workflow persists it. Unreferenced files are retained to preserve backup references.

## Shortcut categories
Native ListView groups place each category above its responsive cards, with a themed chevron, name and horizontal divider. The header is clickable across its width; native group focus and keyboard access remain. Cards omit the repeated category caption and use 92px tiles. Hidden items are excluded from selection/execution. Group labels differing only by case share a section; empty categories join General. Persist expansion per executable/category in ui-state.ini, with encoded case-normalized keys. New categories expand by default. Search temporarily opens matching groups without saving those temporary states. Save on user interaction and before context replacement/hide/exit; no polling or shortcut-schema migration.

## Settings navigation
One Settings window uses themed native General / Shortcuts / Workflows navigation buttons. General has separated preference rows with labels/help on the left and controls alongside. Content tabs share a clearly labeled application picker above a divider. Shortcuts places the list left and editing fields right, with one primary Save. Workflows embeds the existing editor as a native child panel, arranged into guides, steps and Markdown columns. Save workflow stays beside the step editor; global preferences do not appear on content tabs. Minimum window is 980×780 logical pixels. Native focus/Tab navigation is handled by the parent; pending edits require confirmation across page/app changes and close. Static labels have opaque themed backgrounds to avoid stale text during layout/theme transitions.

## Complete workflow reading
Selecting a workflow tree parent renders its title and all numbered steps, separated by compact horizontal rules; a child selection remains focused on that step. Parse each step independently into the same Rich Edit control, preserving Markdown boundaries, link offsets, palette, resize cache and scrolling. The text limit accommodates 128 stored steps. Keep eight attached-image previews per reader for bounded memory, with an explicit placeholder directing users to a step for further images. Clear all rendered content on hide/tab switch. Storage and editing remain unchanged.

## Portable collection actions
Shortcuts and Workflows share native Import/Export buttons beside the application selector. Import previews identity and counts with Add, Replace and Cancel; replacement requires confirmation. Export requires saved edits and is available only for a profile containing shortcuts or workflows. Native dialogs follow Windows appearance.

## Settings inside the main window (0.3.0-alpha.2)
The footer Manage actions are replaced by an authored GDI gear directly left of PIN. Settings is a child panel in the same popup HWND, with General/Shortcuts/Workflows tabs and an authored Back arrow in the header. Header icons retain accessible labels and tooltips. Back/gear return after discard confirmation. Escape/Close/unpinned focus loss retain the draft while hiding to tray. Own dialogs do not trigger dismissal; process exit checks hidden drafts. Native scrolling handles small windows. This supersedes older descriptions of a separate owned settings window.

## Dock and website context (0.4 candidate)
Fourth Settings tab Dock keeps explicit Apply, ordered checkbox list, orientation/scale/opacity and pending-edit guards. Website... adds an exact-host profile under the selected browser; profile selector labels identify executable plus host. Popup shortcuts group Browser/category and hostname/category; workflow tree uses Browser and Website roots. Dock is a nonactivating native topmost strip with grip, collapse and shortcut buttons. No continuous redraw or browser polling. Ambiguous native address reads use browser-only content.

## Dock refinements (0.4.0-alpha.2)
Browser heading puts hostname before executable, falling back to process only. Dock overflow uses Previous/Next pages without scrollbars; orientation, selected order and nonactivating mouse handling remain. Settings uses a behavior/appearance column and a wider ordered shortcut column, with native auto-checkboxes for enable/collapsed, selection count, clear Apply and pending-edit feedback. Preserve native accessibility and Space behavior while theming checkbox paint.
