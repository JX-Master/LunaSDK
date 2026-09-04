## Status
Proposed.

## Last updated
2026/9/4

## Background
Shipping a game as loose files increases deployment cost. A single package should represent a directory tree, support read-only and editable use, and defer physical writes until explicit flush or VFS unmount/shutdown. Asset discovery should read one metadata database instead of opening many `.meta` files.

These requirements cross three separate domains. VFS routes file operations between storage devices. Asset manages logical assets, including GUIDs, types, shared paths, and multiple data units. An archive is a general container of files and directories and must not depend on either domain. ADR-0018 supplies the independent Zip module used by this decision; ADR-0014 defines the asset records that the database must preserve.

The current VFS already has driver callbacks and an error-returning unmount operation, but no device flush operation. Its module shutdown clears mounts without calling their unmount callbacks. Runtime `IFile::flush` and module shutdown return no error. Current Asset metadata loading, saving, moving, and copying directly assume per-asset JSON `.meta` files. Adding only a package reader or a second discovery function would not address these lifecycle and persistence assumptions.

## Decision
Treat Pak storage, VFS integration, and Asset metadata databases as one architecture delivered in stages under this ADR. The first implementation stage introduces Pak and its documentation/tests. The following stages implement VFS and Asset changes described below and update this ADR's delivery record. They do not require separate competing architectural decisions.

### Module responsibilities
| Module | Responsibility | Dependencies relevant to this feature |
| --- | --- | --- |
| Zip | Standard ZIP/ZIP64 parsing, archive edits, codecs and checksums | Runtime, libzip; libzip uses zlib |
| Pak | Validated directory tree, seekable file handles, staged file writes, repeatable flush | Runtime, Zip |
| VFS | Pak driver, path routing, mount access, storage publication, unmount/shutdown | Runtime, Pak |
| Asset | Asset records and database implementations over ordinary VFS files | VFS and existing serialization facilities |

Pak does not define asset GUIDs, loaders, data units, `.meta` files, database filenames, or mount points. Applications can use Pak directly with arbitrary seekable streams. VFS mounting does not implicitly register assets. Application code first mounts storage, then opens/imports the appropriate Asset database through its VFS root.

### Package representation and paths
A phase-one `.pak` is a standard single-volume ZIP/ZIP64 archive with a validated filesystem naming profile. There is no mandatory private header, asset manifest, or reserved file. Ordinary ZIP tools can inspect it, and compatible ordinary ZIP archives can be opened as packages.

Store and Deflate are supported in the first phase. Creation accepts a default compression selection for new files; Store is the default. Individual files may override it. Existing files preserve their method unless explicitly changed. The default is a session policy, not a hidden package entry. Encryption and other codecs are rejected explicitly during package opening. Future codecs/encryption use ZIP method identifiers, flags and extra fields through Zip, with Pak defining their file-access policy. A format revision is required before adding semantics that existing readers must not silently ignore.

Paths are UTF-8, case-sensitive and slash-separated, with no Unicode normalization. Imported names are decoded by Zip/libzip before Pak validates and indexes them. The package has an implicit root and synthesizes parent directories missing from the ZIP directory. Duplicate explicit paths, file/directory collisions, non-empty directory entries, absolute ZIP entry names, empty/dot/parent components, backslashes and drive-style colons are rejected. Public operations accept an optional leading slash and a directory trailing slash; the empty path and `/` identify the root. No symbolic-link resolution or host filesystem permission interpretation is performed. ZIP entries marked as links by external attributes are ordinary file bytes here.

Opening scans the ZIP central directory and builds a memory index. It does not decode file payloads. Zip's bounded archive-tail reads may overlap payload bytes while locating structural records, so this is not a guarantee that every physical read lies exclusively in metadata.

### File operations and staging
Pak exposes Runtime file access/creation modes, file attributes, directory iteration, directory creation, file copying, subtree moves, and deletion. Parents must exist before creation. Copy operates on a file; deleting a directory requires it to be empty. Directory iteration is a sorted snapshot. ZIP timestamps and external attributes are not part of the phase-one Pak API; Runtime timestamp fields are zero.

Read-only files are decoded on demand through Zip, with independent logical cursors. Seeking forward skips decoded bytes; seeking backward reopens and replays decoding. This keeps read-only memory bounded but does not promise constant-time random access, even for Store with the current Zip interface. Reading through EOF verifies the ZIP CRC; a partial read does not validate unread data.

Writable opening materializes that file into a private seekable staging stream, unless its contents are being discarded. The default staging stream uses memory, with a configurable per-file limit (256 MiB initially). Applications can supply a staging-stream factory backed by temporary files or other storage for larger editable files. The limit applies to the built-in memory implementation, not to read-only decoding or custom staging. Extending a file zero-fills the gap. Staging never writes the source package.

A file permits multiple readers or one exclusive writable handle. Operations affecting an open file, and moves of a subtree containing open files, return `E_BUSY`. Other files remain editable. Handles retain their package; directory snapshots do not block mutations or flush. Operations on one package and its handles are serialized by a package mutex. Separately opened packages sharing the same underlying stream still require caller synchronization. Staging callbacks must not reenter the package or return aliased storage.

### Repeatable flush and error handling
`IPak::flush(destination)` writes a complete package to a distinct readable, writable, seekable and resizable stream. All package file handles must be released first. The output must not alias the current source or any retained staging stream, including through separate handles. Pointer-identical aliases are rejected. Output contents are replaced; input contents remain unchanged.

Pak retains its own directory and staged data independently of Zip's editing session. Flush opens a separate Zip edit session over the current source, applies changes, saves, and reopens the output before adopting it as the new source. Surviving unchanged ZIP entries can be copied without decoding/recompression. This isolates Pak from Zip's consuming save operation and allows repeated edit/flush cycles on the same Pak object.

On failure, Pak keeps its previous source, pending changes and dirty state for retry; the destination may be incomplete, or complete if reopening failed. On success, it releases old sources/staging, adopts the destination, and clears dirty state. Flushing an unchanged editable package still writes a complete output. A newly created empty package is dirty until first flush. Read-only packages reject flush. Explicit discard closes the package without writing, and ordinary destruction also discards pending changes.

This is stream serialization, not atomic filesystem replacement or durable synchronization. It deliberately takes an output stream rather than overwriting a source that libzip is still reading. Runtime has no generic transactional replacement API and `IFile::flush` cannot report synchronization failures.

### VFS integration stage
Add a Pak driver and typed mount parameters containing access mode, package options and storage configuration. Adapt Pak's file and directory interfaces to the existing driver operations. A package is one mounted directory. Its root and all descendants inherit read-only access when mounted read-only. Cross-device copies use VFS file streaming; moves across devices remain copy-then-delete operations, without a multi-device transaction guarantee.

Add an optional error-returning driver flush callback, a VFS flush operation for a mount, and a flush-all operation. File-handle `flush()` only completes writes to Pak staging; it does not commit the entire package and cannot substitute for error-returning device flush. An explicit writable unmount flushes/publishes first and removes the mount only on success. Busy handles and publication errors keep the mount available for retry.

For native package files, the driver saves to a sibling temporary file and then publishes it using a checked platform replacement operation. Runtime's existing no-overwrite `move_file` is not an atomic replacement primitive; do not implement publication as delete-original-then-move. The VFS stage must introduce or use a suitable storage-specific replacement facility, including Windows handle closure requirements. After Pak has adopted a temporary output, a failed publication remains a pending driver transaction; do not report the mount as clean merely because Pak's dirty bit cleared. Keep the original file and retryable temporary output until publication succeeds. A custom stream/storage mount supplies the corresponding publication policy without imposing local paths on Pak.

VFS shutdown must visit every mount and invoke its unmount/cleanup path before unregistering drivers. It attempts pending flushes and reports failures through logging because `Module::on_close` cannot return errors. Applications needing checked persistence explicitly release file handles, flush and unmount before Runtime shutdown. Shutdown is a fallback attempt, not a promise that disk-full or I/O failures can be recovered after the process exits. Driver replacement and mount lifetimes must also stop invalidating live driver data. These are required lifecycle changes in the VFS stage.

### Asset database stage
Introduce an Asset-owned metadata database interface, with two implementations: the existing per-asset sidecar layout and one centralized database file. Both use only VFS file/directory APIs. Keep existing asset GUID, type, path, loader and data-unit semantics. No Asset code branches on a Pak driver name or includes Pak/Zip headers.

One centralized database covers a specified root and its complete descendant tree, as selected by the user. For example, `/Assets/assets.db` contains records whose asset paths are relative to `/Assets`. Each record contains the asset GUID, relative path, type and named data-unit descriptors; the main data unit remains implicit in the asset type. The database has an independent format version and uses the existing JSON/Variant serialization infrastructure initially. The filename is configurable and is an ordinary file to VFS/Pak.

Opening a centralized database reads and validates the one database file, resolves paths beneath the chosen root, and registers records without reading any asset payload files or scanning for `.meta` files. When stored in Pak, the database itself is a file payload that must be read; the requirement is to avoid decoding the other asset payloads. The same database works in a normal directory.

Database roots have explicit ownership: reject overlapping registered roots initially. An explicitly selected database is authoritative within its root; do not silently merge sidecars and centralized records, or fall back to scanning after a corrupt database. Validate versions, relative paths, duplicate GUID/path mappings, record contents and existing-registry conflicts before applying a batch. Metadata updates retain the existing busy/loaded data-unit checks. Metadata failure must not leave a partially registered batch.

Route metadata load/save and asset create/move/copy/delete through the selected database. A discovery-only second API is insufficient because the existing maintenance operations unconditionally write or move `.meta` files. In centralized mode, metadata save updates a database record and database flush writes the file; generic asset file enumeration must not treat that shared file as one asset's payload. Moves across database roots transfer the record through both providers and report partial physical-operation failures; no global filesystem transaction is implied. Sidecar mode preserves the existing developer workflow and old metadata compatibility.

Conversion is an explicit tooling operation: collect and validate sidecar records, write the centralized database, verify it, and only remove sidecars when explicitly requested. Packaging can leave source sidecars untouched while emitting only the database and payload files. Database authority is never inferred solely from a `.pak` extension.

### Persistence order across the layers
Applications save asset data units and metadata records, flush the Asset database into ordinary VFS files, release file handles, then flush/unmount the VFS device. For Pak, database and asset file updates are then serialized into the same package output. Closing Asset precedes closing VFS, which precedes Pak/Zip dependencies. VFS never calls back into Asset to discover pending metadata. Checked application shutdown must follow this order; Pak cannot save Asset changes that were never written to its files.

## Impact
Applications gain ordinary filesystem operations on a single archive without depending on VFS or Asset. Asset databases remain usable on any VFS driver, and ZIP tools remain useful for inspection and interoperability. No additional third-party dependency is required beyond the setup-managed Zip dependencies.

An archive flush is proportional to the resulting package size and is not an append journal or block transaction. Read-only compressed seeking can be expensive; writable files require staging proportional to their uncompressed size. Custom staging limits memory use but adds application storage responsibilities. Block compression, encryption, extraction, background compaction, filesystem permissions and crash-durable generic stream transactions are outside the first stage.

VFS and Asset will need public API additions and internal lifecycle changes. Existing sidecar workflows remain supported, but centralized mode requires database-aware maintenance and explicit flush ordering. This ADR describes those changes as required follow-up implementation, not capabilities already present after the Pak stage.

## Alternatives considered
### A private binary Pak format
A private format could optimize block access and incremental commits, but would add format tooling and validation work before those requirements are demonstrated. Standard ZIP/ZIP64 plus a strict directory profile satisfies the current scope and already supports codec extension identifiers.

### Asset records in the ZIP central directory
This would couple general archive storage to logical assets and prevent the same metadata implementation from working in loose directories. A normal database file keeps the responsibilities separate.

### Expose Zip directly as the VFS driver
Zip has entry-level edits, sequential readers, nonrecursive directory markers and a consuming save. A Pak layer provides the file semantics, tree validation and repeated flush lifecycle that a VFS driver needs, and makes those capabilities reusable outside VFS.

### Buffer the entire archive or overwrite it in place
Whole-archive buffering scales with package size. In-place generic stream rewrites can destroy source bytes still needed for a flush and cannot promise recovery. Separate destinations and per-file staging give explicit ownership and retry behavior.

## Version history
* **2026/9/4** Proposed the complete Pak/VFS/Asset design and implemented the standalone Pak stage for review, with API documentation, programming guides, PakTest and Python ZIP interoperability checks. Build and tests passed on macOS arm64. VFS integration and Asset database implementations remain the next delivery stages under this ADR.
