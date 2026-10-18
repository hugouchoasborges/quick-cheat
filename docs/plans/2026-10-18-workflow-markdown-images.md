# Markdown and image attachments — 0.2.0-alpha.4

Goal: edit workflow steps as Markdown, render them in the main popup with inline local images, and seed the existing ChatGPT examples for acceptance.
Architecture: keep schema v3 strings; attachment references use ![alt](attachments/<id>.png). A vendored pinned MIT MD4C parser emits formatted runs into native Rich Edit; EM_INSERTIMAGE inserts normalized PNG streams. WIC imports/resizes images and loads only the selected step. No browser, HTML execution, automatic remote downloads or image generation service.

1. Vendor parser source/header/license at a fixed upstream tag/commit; include notices in distributables.
2. Add image import/validation/scaling: local PNG/JPEG/BMP/GIF first frame, input 10 MiB/24 MP, normalized PNG max1600px and4MiB; up to8 displayed per step. Managed attachment references only.
3. Add lazy native reader: headings, emphasis, lists, quotes, code, links and images; text selection/copy, themes, DPI, resize and memory release on hide. Links open only through explicit user click and safe protocols.
4. Add Attach image to step editor, inserting Markdown at caret. Keep existing text/step limits and save/unsaved behavior.
5. Create original sample diagrams and update only the three named ChatGPT demo workflows, preserving shortcuts, other workflows and a pre-edit backup.
6. Compile without adding/running tests. Capture isolated DEMO reader/editor, review, create Library version child and document limits. Gitflow feature/develop/release; no main or public publication yet.