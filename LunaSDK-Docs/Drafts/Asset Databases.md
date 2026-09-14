Asset databases store the persistent identity and structure of assets. A database covers one VFS root and its complete descendant tree. Applications can keep the development layout of one `.meta` file per asset, or store all records in one file for distribution. Both layouts use ordinary VFS operations and work with native directories, Pak files and other filesystem implementations.

See [[Asset Database Format]] for the centralized format, [[Converting Asset Metadata]] for export, and [[Mounting Pak Files]] for package storage.

## Concepts

An **asset record** contains a persistent GUID, a path relative to the database root, an asset type and named data-unit descriptors. The main data unit is implicit in the type. Metadata discovery does not require payload files, registered types or registered loaders; loading a data unit does require the corresponding type and loader.

A **database provider** implements `IAssetDatabase`. A sidecar provider reads and writes individual `.meta` files. A file database holds the complete logical record set in memory and writes one JSON file on flush. Providers can also be used independently of the runtime asset registry, for example by packaging tools.

The **asset registry** associates records with stable runtime handles and loaded objects. Registering a database retains its provider and registers its records as one validated batch. Database roots must not overlap, and a GUID cannot be registered at multiple paths, including paths through VFS mount aliases. Register the database before discovering or creating assets within its root. Assets outside explicitly registered roots continue using the legacy sidecar workflow.

The **logical snapshot** includes accepted metadata edits that may not yet have reached storage. A fresh storage snapshot is separate: it can be validated against the registry before replacing either view.

## Programming guide

### Initialize and select a provider

Add `DependsOn("Runtime", "Asset", "VFS")` to the application target. Initialize Runtime, add `module_asset()` and initialize modules. Asset initializes its VFS dependency. Create and mount the desired filesystem before opening its metadata database.

Include `<Luna/Asset/Database.hpp>` for the provider and registration APIs:

```cpp
#include <Luna/Asset/Database.hpp>

Luna::R<Luna::Ref<Luna::Asset::IAssetDatabase>> open_game_assets()
{
    using namespace Luna;
    auto opened = Asset::open_file_database("/Assets", "assets.db",
        Asset::DatabaseMode::read_write);
    if(failed(opened)) return opened.errcode();
    auto database = opened.get();
    auto registered = Asset::register_asset_database(database);
    if(failed(registered)) return registered.errcode();
    return database;
}
```

Choose a factory according to the storage layout:

| Factory | Behavior |
| --- | --- |
| `new_sidecar_database(root, mode)` | Creates a provider without scanning. Defaults to read/write. Registration scans sidecars under the root. |
| `open_file_database(root, filename, mode)` | Reads and validates the centralized file only. Defaults to read-only access. Registration uses its already loaded logical snapshot. |
| `create_file_database(root, filename)` | Publishes an empty writable database. Requires an existing parent directory and refuses an existing file. |

These factories do not register assets. The default centralized filename is `assets.db`; it is configurable and relative to the root. `DatabaseMode` controls metadata writes independently of the filesystem's access mode. Selecting read/write access does not make a read-only mount writable.

Keep a registered provider's VFS bindings and backing files stable. Before changing the mount, unregister the database. After deliberately changing its contents externally, use the checked reload API below.

### Discover and edit metadata

| API | Database behavior |
| --- | --- |
| `register_asset_database(database)` | Validates all records and ownership before registering the complete snapshot. Unregistered GUID placeholders retain their handles. |
| `load_assets_meta(path, allow_overwrite)` | Uses registered database snapshots within their roots; scans legacy sidecars elsewhere. Validates the complete batch before publication. |
| `load_asset_meta(asset)` | Reconciles one asset from its provider's current logical record. |
| `new_asset(path, type, save_meta)` | Creates metadata through the selected provider when `save_meta` is true. |
| `save_asset_meta(asset)` | Saves one runtime descriptor into its provider, including type and named data units. |
| `get_asset_database(path)` | Returns the explicitly registered provider covering a path; legacy fallback paths return `E_NOT_FOUND`. |

Changing a runtime descriptor does not implicitly save it. After changing the type or named data units, call `save_asset_meta`. `set_asset_path` changes the runtime path only; a subsequent metadata save relocates the record from its last saved location. Use `move_asset` when payload files must move as well.

Within an explicit database root, that provider is authoritative. Asset does not merge stray sidecars into a centralized database or silently fall back after a database error.

Use `reload_asset_database(database)` to reread storage. It rejects a dirty provider with `E_BUSY`, checks the complete candidate against existing ownership and loaded/busy data units, and then replaces the provider snapshot and registry view. Invalid records or conflicts leave the registry and any cached provider snapshot intact. Sidecar providers read files directly and do not cache an old storage snapshot. Records removed from storage are unregistered only when their data units are unloaded and idle. A failed reload never partially installs earlier records in the batch.

### Move, copy and delete assets

`get_asset_files` enumerates payload files associated with the asset base name. It excludes `.meta` files, centralized database files and reserved metadata temporary files. Metadata is maintained through the provider instead of being treated as one of these payload files.

`move_asset` preserves the GUID, transfers payload files and relocates the record. Moves between database roots transfer metadata through the source and destination providers. `copy_asset` creates an independent record and normally a new GUID; it may also populate a caller-supplied unregistered GUID handle. `delete_asset` removes payloads and the record. Metadata writes to read-only providers fail before payload changes; copying from a read-only source to a writable destination is permitted.

Maintenance reserves affected assets against concurrent data-unit operations. Moves and copies attempt to roll back transferred payloads after a later failure, and report paths whose rollback failed. These operations do not provide a transaction spanning multiple physical files or filesystems. Deletion can likewise fail after deleting some payloads; check every result.

### Save and publish

Use this order for a checked save:

1. Save changed asset data units through their loaders.
2. Call `save_asset_meta` for changed runtime descriptors.
3. Call `database->flush()` or `flush_asset_databases()`.
4. Release filesystem file handles.
5. Call the filesystem instance's `flush()`, or checked `VFS::unmount`.

Sidecar saves write immediately. File databases stage changes until database flush. A flush serializes the complete JSON into a sibling temporary file and publishes it through checked VFS replacement. Failed publication preserves the previous database file and leaves the provider dirty for retry. Temporary-file cleanup is attempted; if cleanup fails, the error reports the retained path. Custom filesystems must support the requested move/replacement operation without streaming fallback. These saves do not promise physical-media durability.

Database flush writes ordinary VFS files. In a Pak filesystem these edits are still staged until the filesystem itself flushes. A mount or filesystem flush never asks Asset to save pending runtime descriptors or database records. Both save steps must succeed before a package is ready to distribute.

`flush_asset_databases()` attempts every registered provider and returns the first failure with its diagnostic. It does not call `save_asset_meta` or save data units implicitly.

### Unregister and shut down

Unload all owned data units and finish their operations before calling `unregister_asset_database`. It flushes the provider, then removes its registrations. Failure retains registration for retry. Existing GUID handles remain valid unregistered placeholders, and the filesystem remains mounted.

Module shutdown closes Asset before VFS. Asset attempts database flush and logs failures because `Asset::close()` cannot return an error. It does not save unsaved runtime descriptors or data-unit objects. Use the checked save sequence for application shutdown, then release application-held providers and other Runtime objects before `Luna::close()`.

### Implement a custom provider

Implement every pure virtual method of `IAssetDatabase`. Record paths are relative to its immutable root. `get_records` exposes the logical snapshot; `read_snapshot` reads a candidate from storage without mutating that snapshot; `accept_snapshot` validates and installs a clean candidate without writing storage. A dirty provider rejects snapshot acceptance.

Implement `write_record`, `remove_record`, `is_dirty` and `flush` with consistent logical-view and retry semantics. `is_metadata_file` identifies files that asset payload maintenance must exclude. A read-only provider rejects record writes and removals before changing storage.

Synchronize provider operations and do not reenter Asset from provider methods. For a registered provider, let coordinating Asset APIs perform record mutations and reloads so ownership and the runtime registry remain consistent. Independent tools may call provider methods directly. Explicit `flush` is valid in either use.

## Validation

Run `AssetDatabaseTest` through LunaBuild for centralized discovery, snapshot validation, metadata maintenance, export, read-only access, failed-publication retry and Pak persistence. `AssetTest` covers existing sidecar and data-unit workflows.
