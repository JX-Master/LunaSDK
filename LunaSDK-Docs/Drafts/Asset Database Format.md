The centralized asset database is a UTF-8 JSON file containing all records beneath one VFS root. It uses the existing Variant serialization infrastructure and requires no additional database library. VFS and Pak treat it as an ordinary file; only Asset interprets its contents.

See [[Asset Databases]] for provider lifetime and save operations, and [[Converting Asset Metadata]] for generating this file from sidecars.

## Concepts

The database has its own `format_version`, currently `1`, and an `assets` array. It does not embed the VFS root. Each record uses a relative asset path, so the same database and payload tree can be mounted at a different root without rewriting records.

| Field | Meaning |
| --- | --- |
| `guid` | A nonzero asset GUID using Runtime's two-element unsigned 64-bit array, `[low, high]`. |
| `path` | The nonempty, canonical relative asset path using `/` separators. |
| `type` | The nonempty asset type name. |
| `data_units` | Optional array of named units, each containing a nonempty `id` and `loader`. |

The main data unit is implicit in `type` and is not included in the named-unit array. Type and loader names need not be registered during discovery. The format contains no asset payload bytes or requirement that those files already exist.

The built-in writer sorts records by path and named units by ID. The in-memory provider indexes paths and GUIDs for record lookup and conflict checking; these indexes are rebuilt when opening the file.

## Programming guide

### Read and write through the provider

Use `open_file_database`, `create_file_database` and `export_asset_database` instead of editing JSON during normal application operation. A minimal file looks like:

```json
{
  "format_version": 1,
  "assets": [
    {
      "guid": [10681767224022494022, 2655911415428893589],
      "path": "Characters/Hero",
      "type": "Game.Character",
      "data_units": [
        { "id": "Editor", "loader": "Game.CharacterEditor" }
      ]
    }
  ]
}
```

Opening this file with root `/Game` resolves the asset to `/Game/Characters/Hero`. It does not read any files belonging to that asset. Tools that process the JSON independently must preserve unsigned 64-bit integer precision for GUIDs.

### Validate input

The reader checks the database version, field types, nonzero GUIDs, nonempty names, relative paths and unique GUID/path/data-unit-ID mappings. Absolute paths, parent traversal, noncanonical spellings, embedded NULs and invalid path components are rejected. Paths are case-sensitive at the Asset/VFS level; callers must also respect the underlying filesystem's naming behavior.

The database filename is configurable and relative to the root. It cannot use the `.meta` extension or the reserved `.luna-asset-` filename prefix. The database file itself, sidecars and reserved temporary filenames cannot be registered as asset paths in a file database. Payload enumeration excludes them so moving or deleting one asset cannot consume shared metadata.

Storage validation occurs before registration. Registry validation then checks root ownership, existing GUID/path mappings and loaded or busy data units before committing a complete batch. Errors retain the previous logical database and registry state.

### Preserve sidecar compatibility

Sidecar providers accept the existing metadata versions `1` and `2`; an absent sidecar version retains the legacy default. They write version `2`. Export preserves the GUID, type and named units and adds the asset path relative to the selected root. The centralized database version is independent of these sidecar versions.

Opening, exporting or registering a database never deletes the source sidecars. Close and flush providers according to [[Asset Databases]].
