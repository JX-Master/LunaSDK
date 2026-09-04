VFS separates filesystem storage from virtual path routing. An application creates an `IFileSystem` object for its storage, uses that object directly when appropriate, and mounts it into the VFS namespace when a shared virtual path is needed. The router retains instances and forwards operations to them.

## Concepts

A **filesystem instance** owns the state for one storage root. Its public interface is in `Luna/VFS/FileSystem.hpp`. Native directories and Pak files have concrete factory functions; custom implementations provide the same interface. There is no filesystem-driver registration step.

A **mount** maps one VFS root path to an existing instance. Multiple mount paths may reference the same instance. Mounting does not create, configure or flush storage. Unmounting removes a reference after a successful flush; it does not destroy a caller-owned instance.

An **instance-relative path** addresses a file or directory beneath that object's root. An empty `Path` addresses the root. A **VFS path** is resolved against the mounted namespace and then translated to an instance-relative path.

## Programming guide

### Initialize and create a native instance

Add `DependsOn("Runtime", "VFS")` to your target, initialize Runtime, register `module_vfs()` and initialize modules. Include `<Luna/VFS/VFS.hpp>` for routing and `<Luna/VFS/NativeFileSystem.hpp>` for native directory instances.

```cpp
lupanic_if_failed(Luna::init());
lupanic_if_failed(Luna::add_modules({Luna::module_vfs()}));
lupanic_if_failed(Luna::init_modules());
{
    auto created = Luna::VFS::new_native_file_system("Data");
    lupanic_if_failed(created);
    auto file_system = created.get();
    lupanic_if_failed(Luna::VFS::mount(file_system, "/Assets"));
    {
        auto file = Luna::VFS::open_file("/Assets/config.json",
            Luna::FileOpenFlag::read, Luna::FileCreationMode::open_existing);
        lupanic_if_failed(file);
        // Read the file here.
    }
    lupanic_if_failed(Luna::VFS::unmount("/Assets"));
}
Luna::close();
```

The native directory must already exist. Its path is resolved at creation, so later working-directory changes do not affect it. Direct operations use paths such as `file_system->open_file("config.json", ...)`. Absolute paths, named roots and unresolved parent components are invalid for instance methods. Runtime `Path` normalization still applies. Native permissions and symbolic-link behavior come from the platform filesystem.

Native handles are ordinary Runtime `IFile` objects. Their `flush()` retains its normal Runtime meaning. Native `IFileSystem::flush()` has no deferred device-level work. Pak instances provide package-level publication through this same interface; see [[Mounting Pak Files]].

### Mount, route and remount

Call `VFS::mount(file_system, mount_path)`. Null instances are rejected, and an occupied mount path returns `E_ALREADY_EXISTS`. A mount retains the object, so the local factory result can be released afterwards. Preserve a reference if the application needs direct access or explicit instance flush.

Routing selects the longest matching mount prefix with matching path roots and absolute/relative flags. A more specific nested mount wins regardless of mounting order. `VFS::remount(from, to)` changes only the namespace location; an occupied destination is rejected.

Several paths may mount the same instance. They share storage and access settings. VFS first offers copies and moves to the source instance, passing the destination instance as the optional third argument. Native instances support native file copying and file/directory moving between native roots. Pak instances provide optimized operations within the same instance, including between mount aliases. Other combinations fall back to bounded file streaming; fallback moves copy then delete. The generic streaming path does not move directories.

Copies do not overwrite an existing destination. If streaming fails, VFS removes a partial destination only if that operation created it, and reports cleanup failure when it cannot remove it.

### Flush and release

Call `file_system->flush()` to save one instance. Its implementation defines deferred persistence and busy-handle requirements. `VFS::flush_all()` calls every distinct mounted instance once and returns the first failure after attempting the others. It does not discover unmounted objects.

Checked unmount requires the selected mount's VFS files and iterators to be released. It calls instance flush before releasing the mount reference. Flush failure retains the mount. Other mounts or caller-held references keep the instance alive and usable afterwards.

At shutdown, VFS releases underlying handles held by all of its wrappers, flushes each distinct mounted instance once, logs failures, and releases mounts. Caller-owned instances and directly opened handles are not closed by the router. They must follow their own lifetime/persistence requirements. Release application-held objects before shutting down Runtime.

### Implement a filesystem

Implement `IFileSystem` as a reflected Runtime object. Supply `open_file`, `get_file_attribute` and `open_dir`, and override the mutation/native-path methods that the storage supports. Unsupported optional operations return `E_NOT_SUPPORTED`. The default `flush()` succeeds without work; an implementation with deferred writes overrides it and preserves retryable state on error.

Construct and configure the object through an application factory or constructor before mounting. No driver name, `DriverDesc`, erased parameter object, or mount/unmount callback is involved. The object's destructor releases resources. Returned handles must retain the storage they need, and the implementation must own synchronization and busy checks for direct use as well as routed use.

`copy_file(from, to, destination)` and `move_file(from, to, destination)` may handle another instance directly; an omitted or null destination selects the current instance. VFS always tries these methods before streaming. Returning `E_NOT_SUPPORTED` requests generic streaming and must leave storage unchanged. Other errors propagate directly. Concrete implementations decide which destination objects they support; the router keeps no type registry. `get_native_path` returns `E_NOT_SUPPORTED` for entries without independent native paths.

VFS routing and device operations use a global mutex. Its handle wrappers also serialize operations within each mount while coordinating unmount/shutdown. An implementation's direct methods and handles follow that implementation's synchronization policy. Methods invoked by VFS must not reenter the router; provider callbacks must also respect their owning instance's reentrancy rules.

## Migrating earlier VFS code

| Earlier API | Instance API |
| --- | --- |
| Driver registration and `DriverDesc` callbacks | A concrete `IFileSystem` implementation and factory |
| Native driver name plus path passed to `mount` | `new_native_file_system(native_path)`, then `mount(instance, mount_path)` |
| Pak mount descriptor and `mount_pak` | `new_pak_file_system(path_or_storage, mode, options)`, then `mount(instance, mount_path)` |
| `VFS::flush(mount_path)` | `instance->flush()` |
| Cleanup callbacks owned by VFS | Reference-counted instance/handle lifetimes |

Update callers and rebuild against the new public headers. The driver registry, driver-name lookup functions, descriptor and path-based single-device flush API have been removed.
