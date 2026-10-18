# Quick Cheat

C++20, Win32, native Windows controls. Prioritize stability, performance and minimal memory. UI and code are English. MIT license.
Raycast is the approved visual reference for popup and editor/settings. Support System, Light and Dark through documented Windows APIs. A single click selects a shortcut; double-click, Execute or Enter executes it. Keep the last valid external application context throughout Quick Cheat UI interaction; own windows, null identification and transient tray/menu windows must not clear it. Workflows are local reference guides, never automatic command sequences. PRODUCT.md and DESIGN.md record this scope and visual system.
Gitflow: main contains delivered releases, develop integrates work, feature/* branches hold implementation, release/* prepare releases and hotfix/* fix delivered releases.
For every implemented version, create a child page under Apps > Quick Cheat in Hugo Library with decisions, changes, validation and limitations.
Do not add frameworks, polling loops, online dependencies or cross-platform abstractions without a demonstrated need.

## Memória da IA (Harness)
Harness > Quick Cheat. Repository key: github.com/hugouchoasborges/quick-cheat.
At task start: `node C:/Users/hugou/.claude/skills/hugo-library/scripts/harness.mjs context --cwd <repo>`.
Product documentation: Apps > Quick Cheat, `tgslib://51be8b49abe30ba3`.

Popup PIN defaults off and hides the popup on focus loss; PIN on keeps it open across focus loss/execution/hotkey and exposes Close beside PIN. Shortcut cards use native ListView tile view; retain category labels, single selection, intentional double-click/Execute/Enter and native keyboard access.

Workflow Markdown uses pinned MD4C + lazy native Rich Edit, with WIC-managed local PNG attachments. Schema v3 stores Markdown references; preserve the attachments directory in backups. Never fetch remote images or execute HTML. Clear rendered assets on hide; retain unreferenced files for backup safety. Include MD4C MIT notice in binary packages.

Shortcut categories use collapsible native ListView groups above compact cards, with a themed header/divider. Persist per-app/category state in ui-state.ini, separate from schema v3. Default expanded; search expands matching groups temporarily without modifying preferences. Never select/execute collapsed items. Category headers replace category labels inside cards (supersedes the alpha.3 layout).

Settings has General / Shortcuts / Workflows tabs. Global settings belong only in General; content tabs share app selection. WorkflowEditor is a child panel (ID 230), not a modal window. Route dialog keys through the settings parent and preserve discard confirmation. Accepting workflow discard reloads saved source; never just clear dirty flags. Native step editing converts stored LF to CRLF and back, with 4096 stored characters and allowance for CRLF expansion.

Selecting a workflow parent shows all steps with numbered headings and separators; a child shows one step. Render independent Markdown sections in the same native reader so malformed Markdown cannot consume later steps. Keep text capacity for the full 128×4096 workflow and eight image previews per reader, directing users to individual steps for extra images. No schema change.

Portable collection format 1 is documented in docs/collection-format.md. Keep global settings/view state out of exports. Import preserves other profiles and never overwrites conflicting images; rollback removes only files created by that import. Use native Windows JSON/Base64, without background polling. Candidate 0.3.0-alpha.1 awaits manual acceptance.
