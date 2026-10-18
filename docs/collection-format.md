# Portable collections — format 2 (0.4 candidate)

A `.quickcheat.json` file contains one application's saved shortcuts, workflows and referenced local PNG images. It does not contain global settings, category expansion state, or other applications. The 0.4 candidate uses local store schema v4. Format 1 remains readable.

```json
{
  "format": "quick-cheat.collection",
  "version": 2,
  "application": {"executable": "example.exe", "name": "Example", "website": ""},
  "shortcuts": [{"title": "Save", "category": "File", "modifiers": 2, "key": 83}],
  "workflows": [{"title": "Prepare a document", "steps": ["## Start\n\nOpen your **document**."]}],
  "images": [],
  "dock": {"enabled": false, "vertical": false, "collapsed": false, "scale": 100, "opacity": 100, "x": -1, "y": -1, "shortcuts": []}
}
```

Modifiers use Windows MOD_ALT=1, MOD_CONTROL=2, MOD_SHIFT=4 and MOD_WIN=8; key is the Windows virtual-key code. Each image has `reference` (a managed `attachments/name.png` path) and `pngBase64` (PNG bytes encoded as Base64). The image set must exactly match managed Markdown image references. Remote image URLs are retained as text and are not downloaded.

## Import and export

In Settings → Shortcuts or Workflows, select an application and use **Export collection...** after saving your edits. **Import collection...** previews the incoming application and content counts before offering:

- **Add:** keep existing content and skip exact duplicate shortcuts and workflows. Different content with the same title is retained. Incoming dock indices are remapped to the deduplicated shortcuts and appended to the existing ordered selection, preserving local dock preferences (64 selected shortcuts maximum).
- **Replace:** replace only the matching executable+hostname profile's shortcuts and workflows, after a second confirmation.
- **Cancel:** leave the library unchanged.

Profiles are identified by executable plus optional exact hostname. The generic browser profile has an empty website; each site has its own profile. Import preserves global settings and all other applications. This format provides local file sharing; community browsing and voting are future work.

## Limits and file handling

- 32 MiB collection file; 128 embedded images, 4 MiB per image and 8 MiB combined.
- Existing store limits still apply, including the 8 MiB serialized store budget and per-field limits.
- Invalid JSON, unsupported versions, unsafe attachment paths, missing images and invalid PNG data are rejected.
- Existing image bytes must match exactly; conflicting files are never overwritten.
- Newly created images are removed if installation or saving fails. Existing files remain untouched.
- Export writes a temporary sibling and replaces the destination after flushing. Import uses the existing local-save backup mechanism.

Windows.Data.Json, Crypt32 and WIC provide JSON, Base64 and PNG handling. MD4C discovers Markdown image references. No background service or new third-party package is required. File dialogs and the import preview use native Windows appearance.

## Website and dock fields
Format 2 requires application.website (empty for generic apps) and dock. Website profiles are restricted to browser executables and canonical hostnames. Import uses that identity, preserving other sites and the generic browser profile. Format-1 files default to an empty website and disabled dock.

Dock shortcuts are unique zero-based indices into the exported shortcuts array, at most 64. Scale is 50–200 and opacity 20–100 percent. Position x/y is a DPI-independent offset relative to the target window; -1 selects the default. Replace/new profiles use incoming dock configuration; Add preserves existing preferences while adding remapped selections. Unsafe/out-of-range references abort import.
