VFS represents a Pak file as an `IFileSystem` instance. Applications create the instance, optionally mount it as a virtual directory, and use the same file APIs as native directories. Read-only instances suit shipped data; editable instances stage changes and publish a new package on instance flush, checked unmount or VFS shutdown.

See [[File System Instances]] for the general instance/mount model, and [[Pak Files]] and [[Editing Pak Files]] for package paths, compression, seeking and staging storage. Creating or mounting a filesystem does not discover or register assets.

## Concepts

A **Pak filesystem** owns one opened package, its access mode, storage provider and pending publication. It is usable before mounting and after unmounting. Several VFS mount paths can reference the same instance and share its data, mode and file-handle constraints.

A **storage provider** is an `IPakStorage`. It supplies the existing source stream, a fresh output stream for each save, and the operation that publishes a completed output. The built-in provider uses native files; custom providers can use memory or other application storage.

A **pending publication** is a complete saved package whose publication failed. It remains readable through the instance and all its mount aliases. New edits are blocked until a retry publishes the same output.

## Programming guide

### Initialize and create

Run setup to prepare Zip's libzip/zlib SDKs. Add `DependsOn("Runtime", "VFS")` to your target. Include `<Luna/VFS/VFS.hpp>` and `<Luna/VFS/PakFileSystem.hpp>`. VFS registers Pak and its Zip dependency automatically.

```cpp
lupanic_if_failed(Luna::init());
lupanic_if_failed(Luna::add_modules({Luna::module_vfs()}));
lupanic_if_failed(Luna::init_modules());
{
    auto opened = Luna::VFS::new_pak_file_system("Game.pak");
    lupanic_if_failed(opened);
    auto file_system = opened.get();
    lupanic_if_failed(Luna::VFS::mount(file_system, "/Game"));
    // Use VFS files here, releasing them before unmounting.
    lupanic_if_failed(Luna::VFS::unmount("/Game"));
}
Luna::close();
```

`new_pak_file_system(native_path, mode, options)` opens an existing package. The defaults are read-only access and default `Pak::Options`. Create a new backing package through `Pak::new_pak` and flush it before opening a filesystem.

Native paths are resolved at instance creation. Later working-directory changes and mount/remount operations do not change the backing location. An editable instance requires exclusive control of its backing storage, including aliases through other objects, processes or paths. Reusing one instance at multiple VFS paths is supported; opening independent editable instances over the same backing storage is not.

### Access and mount aliases

Direct instance paths are relative to its root:

```cpp
auto direct = file_system->open_file("config/settings.json",
    Luna::FileOpenFlag::read, Luna::FileCreationMode::open_existing);
auto routed = Luna::VFS::open_file("/Game/config/settings.json",
    Luna::FileOpenFlag::read, Luna::FileCreationMode::open_existing);
```

The empty `Path` names the instance root. Absolute paths, named roots and remaining parent components are rejected by instance methods. Runtime `Path` normalization occurs before routing and validation. VFS paths select the longest matching mount prefix in the same root and absolute/relative namespace.

Read/write flags, creation modes, attributes and directory operations follow Pak semantics. Read-only instances reject mutations. Timestamps are zero. Package entries have no independent native path, so `get_native_path` returns `E_NOT_SUPPORTED`.

Mounting the same instance again creates an alias:

```cpp
lupanic_if_failed(Luna::VFS::mount(file_system, "/GameAlias"));
```

Edits through either path or through the instance are immediately visible through the others. Copies and directory moves within the same instance use Pak operations even between aliases; file copies preserve their source compression. Copies between Pak and another instance stream file bytes and use the destination's defaults. These cross-instance moves copy then delete and have no multi-filesystem transaction guarantee. The generic path copies files only; native-to-native operations can use the native implementation's copy/move support.

### Edit and flush

```cpp
Luna::Pak::Options options;
options.compression = Luna::Pak::CompressionMethod::deflate;
auto opened = Luna::VFS::new_pak_file_system("UserData.pak",
    Luna::Pak::OpenMode::read_write, options);
lupanic_if_failed(opened);
auto file_system = opened.get();
lupanic_if_failed(Luna::VFS::mount(file_system, "/UserData"));
{
    auto file = Luna::VFS::open_file("/UserData/save.bin",
        Luna::FileOpenFlag::write, Luna::FileCreationMode::create_always);
    lupanic_if_failed(file);
    const Luna::c8 contents[] = "saved state";
    lupanic_if_failed(file.get()->write(contents, sizeof(contents) - 1));
}
lupanic_if_failed(file_system->flush());
lupanic_if_failed(Luna::VFS::unmount("/UserData"));
```

`IFileSystem::flush()` acts directly on the instance without routing a mount path. It requires **all of this instance's file handles** to be released, including direct handles and handles from every mount alias. Otherwise it returns `E_BUSY`. Directory snapshots can remain open. `IFile::flush()` only flushes a file's staging storage and does not publish the package.

`VFS::flush_all()` flushes each distinct mounted instance once, even when it has several aliases. It attempts the later instances after a failure and preserves the first error's diagnostic. Unmounted instances must be flushed directly by their owner.

Clean and read-only instances need no output. A dirty native instance serializes into a sibling temporary file, then calls Runtime `move_file` with `FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy` for checked same-filesystem replacement. Successful flush keeps the instance open for further edits.

### Handle save failures

Serialization failure preserves the original package and staged edits, and removes the incomplete native output. Publication failure preserves the original backing file and the completed temporary output; its path is included in the error. Reads remain available, while new mutations return `E_BUSY`.

Call the same instance's `flush()` to retry. After publication failure it retries the completed output without recompressing. Release any readers opened since the failure before retrying. Both the pending state and busy checks belong to the instance, so direct operations and every mount alias follow the same rules.

Replacement follows native filesystem semantics. It does not guarantee physical-media durability or provide a multi-file transaction. Do not externally rename, edit or replace an opened backing package.

### Custom storage

Implement all three pure virtual methods of `IPakStorage` and pass it to the storage overload of the factory. The interface provides no default implementations. Read-only providers must explicitly implement `create_output()` and `publish(output)` to return `E_NOT_SUPPORTED`.

```cpp
Luna::R<Luna::Ref<Luna::VFS::IFileSystem>> open_application_package(
    Luna::VFS::IPakStorage* storage)
{
    return Luna::VFS::new_pak_file_system(storage, Luna::Pak::OpenMode::read_write);
}
```

| Method | Contract |
| --- | --- |
| `open_source()` | Returns a readable/seekable stream containing the existing package. Its bytes and size remain unchanged while retained. |
| `create_output()` | Returns a fresh readable, writable, seekable, resizable stream that does not alias a retained input or staging stream. Read-only instances never call it. |
| `publish(output)` | Publishes the completed output. Failure preserves the prior publication and keeps the output usable for retry. Success also keeps that same output stream readable because Pak has adopted it. |

The filesystem retains its provider. Callbacks are synchronous and must not reenter the owning filesystem or VFS. Returned stream lifetimes own cleanup, including incomplete outputs; the provider defines retention when publication fails during shutdown. The built-in provider preserves logical stream identity while closing native handles for file replacement and reopening on later reads.

### Unmount and shutdown

Checked `VFS::unmount` requires that mount's VFS file handles and directory iterators to be released, then flushes its instance. The Pak flush can still return `E_BUSY` for files opened directly or through another alias. Failure retains the mount for retry. Success releases only that mount's reference: the instance remains usable through caller references and other mounts.

VFS shutdown first releases the underlying files/iterators of all VFS wrappers, then flushes each distinct mounted instance once and releases its references. It logs failures. It does not close caller-owned instances or direct file handles. Release direct handles before shutdown so they do not block Pak saving. Complete native outputs awaiting failed publication remain available for recovery.

Unloading the last instance reference discards unsaved edits. Applications using an instance without mounting must flush it explicitly. Release application-held Runtime objects before `Luna::close()`; shutdown invalidation does not make wrappers usable after Runtime ends.

When integrating with Asset, save data and metadata into ordinary files, release file handles, then flush the filesystem or unmount. A mount does not ask Asset to write pending records.

## Validation

Run `dotnet run --project LunaBuild.csproj -- run --target VFSPakTest`. The suite covers native/custom storage, direct instance operations, shared mount aliases, deferred publication and retries, file/iterator lifetimes, routing, bounded copies and shutdown ownership. `AssetTest` checks existing sidecar workflows through the updated VFS.
