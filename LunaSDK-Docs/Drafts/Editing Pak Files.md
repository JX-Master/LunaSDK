Pak stages changes to files and directories, then writes a complete package when explicitly flushed. The original package is read-only input even during editing. This lets applications control output storage and publication while continuing to edit the same `IPak` object across multiple flushes.

See [[Pak Files]] for initialization, paths, readers and directory enumeration.

## Concepts

`Options` selects the default compression of newly created files and the storage used for writable contents. Store is the default. Deflate accepts level `0` for the library default or levels `1` through `9`; Store requires level `0`. The default is a session setting, not a file hidden inside the package. Existing files keep their compression unless changed explicitly.

Selected levels remain in effect across flushes on the same Pak object. ZIP does not record the original encoder level, so opening an existing package uses level `0` for subsequent edits unless a level is selected explicitly.

A **staging stream** contains one file's uncompressed editable bytes. The built-in staging implementation uses memory with a default per-file limit of 256 MiB. A custom staging factory can supply temporary files or other streams. Staged data is retained until successful flush or discard. Multiple staged files can collectively exceed the per-file memory limit.

A **flush destination** is a separate readable, writable, seekable, resizable stream that receives a complete ZIP. Pak opens it after saving and adopts it as its source for subsequent reads and edits. This stream must remain unchanged while retained. The old source is released after successful adoption.

## Programming guide

### Create or edit

Use `new_pak(options)` to create an empty editable package. No output is written at creation. To edit an existing package, pass `OpenMode::read_write` and options to `open_pak`. A read-only package rejects write access, directory mutations, compression changes and flush.

Create parent directories first with `create_dir`. `open_file` supports Runtime's creation modes: `create_new`, `create_always`, `open_always`, `open_existing`, and `open_existing_as_new`. Creation or truncation requires write access. `FileOpenFlag::user_buffering` is accepted as a hint; Pak already manages its own decoding/staging.

Opening an existing file for writing copies its complete decoded contents into staging. Truncating it skips this copy. A staged file supports random reads, writes and resizing; extending it or writing beyond EOF zero-fills the gap. This touches staging storage only. Each file allows several readers or one writer, so opening a conflicting handle returns `E_BUSY`.

### Change the directory tree

| Operation | Behavior |
| --- | --- |
| `create_dir(path)` | Creates one directory; its parent must exist. |
| `copy_file(from, to)` | Copies one file's contents and compression selection to a new path using independent staging. |
| `move_file(from, to)` | Moves one file or a complete directory subtree; the destination must not exist. |
| `delete_file(path)` | Deletes a file or an empty directory. The root cannot be deleted. |
| `set_file_compression(path, method, level)` | Selects the encoding for the next flush, without immediate recompression. |
| `get_file_compression(path)` | Returns the current selected method. |

Moving/deleting an open file, moving a subtree with open files, truncating an open file, or changing an open file's compression returns `E_BUSY`. Unrelated paths remain editable. Copying from a file with an active writer is also busy. Directory removal is nonrecursive, and directory moves cannot move a directory into itself. Failed validation leaves the tree unchanged.

### Flush repeatedly

Release file handles, then call `flush(destination)`. It replaces the destination's contents with the complete package. The destination must not alias the current source or any retained staging stream, including through separate handles. Pak checks identical interface pointers; applications must exclude other storage aliases.

A successful flush keeps the Pak object open, adopts the destination as its new source, releases staged contents and clears `is_dirty()`. New and mutated packages are dirty. An unchanged editable package can also be flushed to create a complete copy. A second flush needs another destination: the previous destination is now the retained input.

A failed flush preserves the previous input, directory tree, staged contents and dirty state for retry. The destination may be incomplete. If writing succeeded but reopening the output failed, the output may be complete; it is still not adopted. Fix the output or supply another stream and retry. A normal file write to staging can fail after partially writing bytes; check its returned byte count and result as with Runtime files.

`IFile::flush()` only concerns the file's staging stream. It does not write the package and cannot report errors through its Runtime signature. Always use `IPak::flush` to commit the archive and check its result.

```cpp
#include <Luna/Pak/Pak.hpp>

Luna::RV create_game_package(Luna::ISeekableStream* destination, bool compress)
{
    using namespace Luna;
    Pak::Options options;
    options.compression = compress ? Pak::CompressionMethod::deflate
                                  : Pak::CompressionMethod::store;
    auto created = Pak::new_pak(options);
    if(failed(created)) return created.errcode();
    auto package = created.get();
    auto made = package->create_dir("config");
    if(failed(made)) return made.errcode();
    {
        auto opened = package->open_file("config/build.txt",
            FileOpenFlag::write, FileCreationMode::create_new);
        if(failed(opened)) return opened.errcode();
        const c8 text[] = "release";
        auto written = opened.get()->write(text, sizeof(text) - 1);
        if(failed(written)) return written.errcode();
    }
    return package->flush(destination);
}
```

### Customize writable storage

Set `Options::create_staging_stream` and optionally `Options::staging_userdata`. The userdata is retained by the package. The factory receives it as `object_t` and returns a private `ISeekableStream`. Pak resets the stream to zero length before use. It must support reading, writing, seeking and resizing, must remain exclusively controlled by Pak, and must not alias the package input or other staging streams.

The callback runs under the package mutex and must not reenter the package. Errors are returned to the initiating file operation. The factory can create temporary files and arrange their cleanup through the returned object's lifetime. The built-in `max_memory_file_size` limit does not apply to custom streams.

Without a factory, opening a larger existing file for writing or growing past the memory limit returns `E_OUT_OF_RANGE`. Read-only streaming does not have this limit. Choose a suitable limit or custom storage based on expected uncompressed file sizes.

### Publish and shut down

Pak flush does not rename native files or guarantee physical-storage synchronization. For disk publication, write a distinct temporary file, check flush, and use the storage system's checked replacement protocol. Do not delete the original before a replacement is ready. After flush the Pak object retains the output; account for platform handle-sharing requirements when publishing it. A publication failure is still pending work even when `is_dirty()` is false.

`discard()` closes a package without saving; repeat discard succeeds. Release handles before discard and release all objects before Runtime shutdown. Releasing the last package reference without flushing discards pending edits.

## Validation

Run `dotnet run --project LunaBuild.csproj -- run --target PakTest`. The tests cover tree validation, directory operations, access and sharing modes, seeking, zero-filled writes, compression, repeated flush, save failures and custom staging storage. They also read Pak output through Zip to check ordinary ZIP compatibility.

Run `python3 Tests/PakTest/Interop.py` for independent ZIP interoperability checks. It invokes PakTest through LunaBuild, imports Python Store/Deflate and ZIP64 data, saves through native file streams, and validates the resulting packages with Python. Do not run it concurrently with another LunaBuild command.
