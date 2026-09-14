Metadata export converts a provider's complete logical record set into a centralized database. It preserves asset identities and relative paths, allowing a development directory with sidecars to produce a distribution directory or Pak containing one metadata file.

The exporter handles metadata only. Applications choose and copy asset payload files separately. See [[Asset Database Format]] for the result and [[Asset Databases]] for using it at runtime.

## Concepts

The **source provider** supplies records beneath one root. It need not be registered with the global Asset registry. A read-only sidecar provider is useful for packaging directly from project files without loading data units or registering asset types and loaders.

The **destination root** defines where the exported relative paths will resolve. The exporter creates a new centralized database there. It does not change the source provider, remove sidecars or rewrite payload files. The destination payload tree must retain the corresponding relative layout.

## Programming guide

### Export a directory tree

Initialize Asset and mount the source and destination filesystems. Create the destination root and the parent directory of the database file before export:

```cpp
#include <Luna/Asset/Database.hpp>

Luna::RV export_project_metadata()
{
    using namespace Luna;
    auto source = Asset::new_sidecar_database("/Project/Assets",
        Asset::DatabaseMode::read);
    if(failed(source)) return source.errcode();
    return Asset::export_asset_database(source.get(),
        "/Distribution/Assets", "assets.db");
}
```

Export collects and validates the complete snapshot before publishing the destination file. Duplicate GUIDs, invalid records or an existing destination are errors. Source sidecars remain untouched. An independently opened file database can also be used as the source; export includes its accepted but unflushed logical edits.

### Package payloads

Copy the selected payload files to the destination with the same relative paths. If the destination is a writable Pak filesystem, both payload copies and the exported database remain staged in that filesystem. Release file handles, check `IFileSystem::flush()` and then unmount or release it. Export itself does not flush the filesystem.

To export only selected records or named units, create an empty unregistered file database, obtain records with the source's `get_records`, filter them, write each accepted record with `write_record`, and check `flush`. Filtering a named-unit descriptor does not automatically remove files belonging to that unit; the packaging tool must choose its payload set consistently.

### Verify and adopt

Reopen the destination with `open_file_database` to validate the stored file. Registration is a separate runtime operation. If the same GUIDs are already registered from the source, unregister the old database before registering the destination; export and independent validation do not require this switch.

Use the destination provider as the authoritative metadata source within its root. Retain project sidecars for development. Sidecar removal, if desired, is a separate explicit application operation after verification.

Release independent providers before Runtime shutdown. Registered providers follow the checked flush and unregister sequence described in [[Asset Databases]].

## Validation

`AssetDatabaseTest` checks that export preserves GUIDs, relative paths and named units, refuses an existing destination and leaves source sidecar bytes unchanged. It also mounts an exported database inside Pak, edits and flushes it, then reopens the package at another VFS root.
