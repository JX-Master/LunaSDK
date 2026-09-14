The `Zip` module creates and edits standard ZIP archives with an explicit save operation. Edits remain in the session until the application chooses a destination. The input archive and entry data sources are never implicitly overwritten. This supports applications that build ordinary ZIP files as well as higher-level archive storage systems.

See [[Zip Archives]] for module initialization, entry readers, threading, names, and shutdown.

## Concepts

A **data source** is an `ISeekableStream` supplying one file's uncompressed contents. Adding or replacing an entry retains that stream and reads its entire contents from offset zero when needed. The source must stay unchanged until the archive releases it. Streams may be shared when access is serialized. Unchanged entries in an existing ZIP can be copied in their stored representation when saving.

A **compression selection** consists of a `CompressionMethod` and a level. `store` writes bytes without compression and requires level `0`. `deflate` supports levels `1` through `9`, or `0` for the library default. Compression is selected per entry. A caller building a package can use the same selection for all entries or choose individually. Reading detects each entry's method automatically.

A **destination** is a separate writable, seekable, resizable stream. Saving replaces its contents with the complete ZIP. Successful save consumes the session; reopen the destination to continue editing. A failed save preserves the session for retry or discard, but its destination can contain partial data.

## Programming guide

### Start a session

Initialize Runtime and Zip as described in [[Zip Archives]]. Use `new_archive()` for a new editable archive, or `open_archive(input, OpenMode::read_write)` for an existing ZIP. Empty archives are valid and are preserved when saved.

### Stage changes

| Operation | Behavior |
| --- | --- |
| `add_file(name, source, method, level)` | Adds an entry from uncompressed bytes. Duplicate names return `E_ALREADY_EXISTS`. |
| `add_directory(name)` | Adds an explicit directory marker and appends `/` if needed. Parent markers are not created. |
| `replace_file(index, source)` | Replaces a file's data and preserves its current compression method, using the default compression level. |
| `set_compression(index, method, level)` | Selects the encoding used at the next save, including recompression of existing data. |
| `rename_entry(index, name)` | Renames one entry. The trailing `/` must preserve its file/directory kind. |
| `delete_entry(index)` | Deletes one entry without renumbering the remaining indices. |

Enumeration and lookup reflect staged edits immediately. File readers can read staged source bytes before compression and saving. Release readers before further editing. Replacement may discard entry extra fields that libzip considers dependent on the old data; the initial API does not expose ZIP comments, timestamps, permissions, or arbitrary extra-field editing.

### Save or discard

Call `save(destination)` after closing all entry readers. The destination must not alias the original archive or any retained entry source, including separate handles to the same storage. Pointer-identical aliases are rejected; detecting aliases through different handles is the caller's responsibility.

Saving can read all surviving entry data and rewrite the entire destination. It is not an in-place append or a cheap per-entry flush. After success, `is_open()` returns `false`; entry operations return `E_BAD_CALLING_TIME`. Reopening gives a new session and fresh indices. Saving an unchanged archive also writes a complete output.

If saving fails, inspect the error and retry with a usable destination, or call `discard()`. Both an explicit discard and ordinary object destruction abandon pending edits. Neither performs a hidden save. `discard()` succeeds on an already closed session.

For filesystem publication, provide a temporary output file, save successfully, release relevant handles, and publish the result using the storage system's replacement procedure. Zip itself does not rename files, promise atomic replacement, or guarantee physical-storage durability. Runtime's generic stream interface has no durable synchronization operation. Applications must choose a publication and synchronization protocol appropriate for their storage.

### Example: create with optional compression

The source and destination below are supplied by the caller; no VFS module is involved. Zip must already be initialized.

```cpp
#include <Luna/Zip/Zip.hpp>

Luna::RV create_zip(Luna::ISeekableStream* contents,
    Luna::ISeekableStream* destination, bool compress)
{
    using namespace Luna;
    auto created = Zip::new_archive();
    if(failed(created)) return created.errcode();
    auto method = compress ? Zip::CompressionMethod::deflate
                           : Zip::CompressionMethod::store;
    auto added = created.get()->add_file("data/content.bin", contents, method);
    if(failed(added)) return added.errcode();
    return created.get()->save(destination);
}
```

For repeated saves, reopen each successfully saved output in `OpenMode::read_write`, stage the next group of edits, and save to a separate output. A higher-level component can manage temporary files and publication around this sequence.

### Shutdown

Release readers, save or discard sessions, and release all sources and destinations owned by the application before shutting down Runtime. External serialization is required for the session and every shared source stream.

## Validation

Build and run the module's regression tests with:

```sh
dotnet run --project LunaBuild.csproj -- run --target ZipTest
```

The tests cover compressed and stored entries, staged reads, shared source cursors, empty directories and files, ZIP64 entry counts, editing, reader lifetimes, malformed input, CRC errors, and save failures at initialization, writing, and commit.

For optional interoperability checks with Python's standard ZIP implementation, run `python3 Tests/ZipTest/Interop.py`. This launches ZipTest through LunaBuild, tests imported Store, Deflate, and ZIP64 data, checks rejection of an unsupported codec, and validates Luna's output with Python. Do not run it concurrently with another LunaBuild command.
