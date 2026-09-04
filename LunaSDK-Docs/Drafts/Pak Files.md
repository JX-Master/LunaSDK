The `Pak` module presents a single ZIP/ZIP64 stream as a directory tree. It adds validated paths, seekable file handles and ordinary directory operations to Zip. It is useful for game packages and other grouped files, and can also open ordinary ZIP archives that conform to its naming and compression rules. It has no dependency on VFS or Asset and assigns no special meaning to asset metadata files.

The public API is in `Luna/Pak/Pak.hpp`. See [[Editing Pak Files]] for creation, optional compression, writable staging, repeated flush and custom staging storage.

## Concepts

A **package** is an `IPak` object with one root directory. `open_pak` opens a retained seekable stream; `OpenMode::read` is the default. File contents are decoded on demand. Opening and directory enumeration build and query an index without decoding file payloads. ZIP structural reads can overlap a bounded portion of the archive tail.

A **package path** is a case-sensitive UTF-8 string with `/` separators. The empty string and `/` identify the root. Public calls accept an optional leading `/`; a trailing `/` denotes a directory. Other empty components, `.` or `..` components, backslashes and colons are rejected. Unicode normalization is not performed, so differently encoded names remain distinct. Names are not native host paths.

Imported entry names are decoded by Zip/libzip before Pak validates and indexes them.

A **file handle** is a Runtime `IFile` with an independent cursor and read/write access flags. Several read-only handles can access one file, or one writable handle can access it exclusively. A handle keeps the package alive. Read-only files stream from Zip; files opened for writing use private staging storage.

A **directory iterator** is an `IFileIterator` snapshot of the direct children, sorted by UTF-8 byte order. It includes synthesized parent directories, omits `.` and `..`, and does not block changes or flush. Later changes do not alter an existing snapshot.

## Programming guide

### Initialize

Run setup to prepare Zip's libzip/zlib source SDKs. Add `DependsOn("Runtime", "Pak")` to your target rules. Register `module_pak()` after Runtime initialization; Pak registers Zip as a dependency.

```cpp
lupanic_if_failed(Luna::init());
lupanic_if_failed(Luna::add_modules({Luna::module_pak()}));
lupanic_if_failed(Luna::init_modules());
```

### Open and inspect

Pass a readable `ISeekableStream` to `Pak::open_pak`. The whole stream is interpreted from offset zero, regardless of its current cursor. An `IFile` or a custom memory/storage stream can supply it. The stream's contents and size must remain unchanged while the package retains it.

Use `get_file_attribute` to query size and file/directory flags, and `open_dir` to enumerate direct children. The root always exists. Parent directories omitted from the ZIP are synthesized from file names. File timestamps are zero because this interface does not expose ZIP timestamps. `get_file_compression` returns Store or Deflate for files.

The package rejects duplicate paths, file/directory conflicts, invalid paths, directory entries containing data, encrypted entries and unsupported compression methods. ZIP external permissions and symbolic-link attributes are not interpreted; their entries are ordinary file bytes, with no link traversal.

### Read and seek

Open a file with `FileOpenFlag::read` and `FileCreationMode::open_existing`. Read and check the returned `RV`, including the read that reaches EOF. `seek` accepts Runtime's beginning/current/end modes and permits a cursor beyond EOF. Negative positions and positions beyond signed 64-bit range are errors.

For a file still backed by Zip, forward reads decode sequentially. A forward seek skips bytes during the next read; a backward seek reopens the decoder and replays the prefix. This applies to Store as well as Deflate with the current Zip implementation. Seeking changes the logical cursor immediately but can make the next read expensive. It does not allocate a whole-file buffer.

Reading through EOF checks the original entry's CRC. Seeking past the end or releasing a partially read handle does not validate unread bytes. Data in writable staging is checked when it is encoded into a new ZIP and later read back.

```cpp
#include <Luna/Pak/Pak.hpp>

Luna::R<Luna::Ref<Luna::IFile>> open_packaged_config(Luna::ISeekableStream* source)
{
    using namespace Luna;
    auto package = Pak::open_pak(source);
    if(failed(package)) return package.errcode();
    return package.get()->open_file("config/settings.json",
        FileOpenFlag::read, FileCreationMode::open_existing);
}
```

The returned file retains its package and the source even after the local package reference is released.

### Concurrency and shutdown

Pak serializes its operations and file-handle operations with a package mutex. Independent read cursors are supported, but I/O within one package runs serially. If external code or another package shares the underlying source stream, that sharing still needs external synchronization. Do not modify a retained source or staging stream outside Pak.

Release all file handles before calling `flush` or `discard`; otherwise they return `E_BUSY`. Release packages and iterators before Runtime shutdown. Destruction discards pending changes and never writes a stream. For editable packages, explicitly check flush success as described in [[Editing Pak Files]].

## Current scope

This module supplies standalone package operations. A VFS driver and Asset metadata databases are separate integration work. Pak does not mount itself or discover assets. Encryption, codecs beyond Store/Deflate, symbolic links, native extraction and filesystem permission preservation are not provided.
