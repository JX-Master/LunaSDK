## Status
Proposed.

## Last updated
2026/9/4

## Background
LunaSDK needs ZIP archive support both for ordinary application code and as the foundation of a future Pak module. ZIP storage must remain independent of VFS mounting and Asset metadata. Reimplementing ZIP64, Deflate, archive editing, and format validation would duplicate existing library functionality.

## Decision
Introduce a `Zip` module, depending on `Runtime` and a pinned libzip dependency. Download libzip 1.11.4 and zlib 1.3.2 through `setup.sh` / `setup.bat` into the ignored `SDKs` directory, then compile them locally through LunaBuild. Third-party library sources must not be checked into the LunaSDK repository. The repository contains only download recipes, configuration templates, and build integration code. Keep third-party declarations out of the module's public API.

Setup verifies the pinned source archives with SHA-256 and prepares the libzip configuration headers and generated error table inside `SDKs`. It checks these source SDKs even when the platform SDK bundle is already installed. Normal builds do not download dependencies or require a system libzip, zlib, or CMake installation.

The module operates on ZIP archives and entries. It provides archive creation, opening, entry enumeration and lookup, sequential entry streams, adding and replacing entries from caller-provided streams, explicit directories, renaming, deleting, compression selection, saving, and discarding changes. Store and Deflate are supported initially; encrypted entry decoding and encoding are not enabled in this phase. Unsupported methods produce explicit errors.

### Streams and ownership
Archive input uses `ISeekableStream`. Opening reads archive metadata on demand rather than loading the entire archive into memory. Added entry sources are seekable streams and are retained until the archive releases them. Their data must remain unchanged until save or discard. This permits large inputs without unconditional whole-file buffering.

Entry readers provide `IStream` semantics. They do not promise seeking in compressed entries. An archive owns its libzip handle and readers retain their archive. An archive cannot be edited, saved, or discarded while a reader is open. Calls involving the same archive, its readers, or shared source streams must be externally serialized.

ZIP entry indices are session-local and must not be treated as persistent identifiers. Entries can be deleted without renumbering the remaining session entries; enumeration skips deleted entries.

### Editing and saving
Mutations are staged in the archive session. Neither mutation nor object destruction implicitly saves. Releasing an unsaved archive discards its changes.

Stream-based saving writes a complete archive to a distinct caller-owned output stream. It must not alias the archive input or any retained entry source, including aliases through separate handles. The output must support writing, seeking and resizing, and its previous contents are replaced. The original inputs are never overwritten by the module.

A failed save leaves the archive session available for retry or discard; the destination may contain incomplete data. A successful save consumes the session, as libzip closes its archive when committing. The caller can reopen the destination to continue editing. Empty archives are preserved as valid ZIP files.

This output-stream contract deliberately does not claim atomic replacement of a filesystem path or physical-storage durability. A caller that needs those guarantees supplies a temporary destination and publishes it using its storage-specific replacement and synchronization protocol. `ISeekableStream` does not expose a durable synchronization operation, and Runtime's current `IFile::flush` cannot report synchronization errors.

### Layering
`Zip` exposes archive-level edits such as replacing an entry with a data source. A future `Pak` module supplies directory-tree policy, file-style random writes, caching, stable logical file identities, and repeated flush sessions using these operations. VFS supplies its Pak driver. Asset stores its database as an ordinary VFS file. None of those higher-level features is part of this change.

The ZIP format is kept standard, including ZIP64 when required. File naming, directory-tree validation, and extraction policy must not be confused with archive parsing: this module does not extract entry paths onto a filesystem.

## Impact
Applications can read and write ordinary ZIP files without depending on Pak, VFS, or Asset. Store and Deflate archives remain interoperable with standard ZIP tools. Download recipes record third-party source versions and archive hashes; downloaded SDKs retain the upstream licenses. Optional codecs and AES backends are disabled to keep the initial dependency set bounded. The wrapper rejects encrypted data operations, including legacy ZIP encryption compiled into libzip's core.

Both third-party libraries are static targets even in a shared-module build. LunaBuild must honor an explicit `StaticLibrary` target kind when choosing output names, archive commands, Windows import-library outputs, and runtime staging. Normal Luna `SharedLibrary` targets continue to follow the global linkage option.

Users of stream-based save must provide a separate output and explicitly handle publication. Compressed entry readers are sequential. These boundaries avoid promising filesystem transactions and random-access decoding that neither a generic stream nor the selected library provides automatically.

## Alternatives considered
### Implement a private container immediately
A private container could optimize block-level random access and incremental commits, but those requirements are not part of the first Zip module. It would also lose direct interoperability with ordinary ZIP tools.

### Expose libzip directly
Exposing libzip types would couple application code and future Pak code to its allocation, error, and handle-lifetime conventions. A Luna interface keeps those details private while retaining archive-level capabilities.

### Hide ZIP support inside Pak or VFS
This would prevent business code from using ordinary ZIP archives independently and mix archive algorithms with filesystem policy.

## Version history
* **2026/9/4** Proposed and implemented for review. Introduce Zip first; future Pak depends on Zip, and VFS integrates Pak through a driver.
