# Portable collections — format 1

A `.quickcheat.json` file contains one application's saved shortcuts, workflows and referenced local PNG images. It does not contain global settings, category expansion state, or other applications. The local store remains schema v3.

```json
{
  "format": "quick-cheat.collection",
  "version": 1,
  "application": {"executable": "example.exe", "name": "Example"},
  "shortcuts": [{"title": "Save", "category": "File", "modifiers": 2, "key": 83}],
  "workflows": [{"title": "Prepare a document", "steps": ["## Start\n\nOpen your **document**."]}],
  "images": []
}
```

Modifiers use Windows MOD_ALT=1, MOD_CONTROL=2, MOD_SHIFT=4 and MOD_WIN=8; key is the Windows virtual-key code. Each image has `reference` (a managed `attachments/name.png` path) and `pngBase64` (PNG bytes encoded as Base64). The image set must exactly match managed Markdown image references. Remote image URLs are retained as text and are not downloaded.

## Import and export

In Settings → Shortcuts or Workflows, select an application and use **Export collection...** after saving your edits. **Import collection...** previews the incoming application and content counts before offering:

- **Add:** keep existing content and skip exact duplicate shortcuts and workflows. Different content with the same title is retained.
- **Replace:** replace only the matching application's shortcuts and workflows, after a second confirmation.
- **Cancel:** leave the library unchanged.

Only one active profile per executable is supported. Import preserves global settings and all other applications. This format provides local file sharing; community browsing and voting are future work.

## Limits and file handling

- 32 MiB collection file; 128 embedded images, 4 MiB per image and 8 MiB combined.
- Existing store limits still apply, including the 8 MiB serialized store budget and per-field limits.
- Invalid JSON, unsupported versions, unsafe attachment paths, missing images and invalid PNG data are rejected.
- Existing image bytes must match exactly; conflicting files are never overwritten.
- Newly created images are removed if installation or saving fails. Existing files remain untouched.
- Export writes a temporary sibling and replaces the destination after flushing. Import uses the existing local-save backup mechanism.

Windows.Data.Json, Crypt32 and WIC provide JSON, Base64 and PNG handling. MD4C discovers Markdown image references. No background service or new third-party package is required. File dialogs and the import preview use native Windows appearance.
