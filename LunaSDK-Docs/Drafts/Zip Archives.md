The `Zip` module reads ordinary ZIP and ZIP64 archives through Runtime streams. It provides an archive index and sequential readers for individual entries without depending on VFS, Pak, or Asset. Store and Deflate are supported. libzip and zlib are pinned source dependencies downloaded into `SDKs` by setup and compiled locally by LunaBuild; applications do not need a system installation of either library.

See [[Editing Zip Archives]] for creation, compression selection, editing, and saving. The public API is declared in `Luna/Zip/Zip.hpp`.

## Concepts

An **archive session** is an `IArchive` object. `open_archive` opens an existing archive in `OpenMode::read` by default. `OpenMode::read_write` enables staged edits. The input is always read through `ISeekableStream`; opening an editable session does not require write access to that input.

An **entry** is a named item in the ZIP central directory. Its `EntryInfo` includes its name, session-local index, uncompressed size, compression method, directory marker, and encryption indicator. Compressed size and CRC32 are meaningful only when `has_compressed_size` and `has_crc32` are set. For edited data, these values may remain unknown until saving.

An **entry reader** is a read-only `IStream` returning uncompressed bytes. Reading is sequential for both Store and Deflate. The API does not promise seeking inside compressed entries. Read through EOF to validate an existing entry's stored CRC32; releasing a partially read stream does not validate unread data. Newly staged data has no stored checksum to compare until saved.

Entry indices identify entries only within one open session. Deletion leaves gaps, and saving closes the session. Reopen and look up entries again rather than persisting these indices.

## Programming guide

### Build and initialization

Run `setup.sh` on macOS or `setup.bat` on Windows to prepare libzip and zlib under `SDKs`, including when updating an existing checkout. Declare `DependsOn("Runtime", "Zip")` in the consuming LunaBuild target. Include `<Luna/Zip/Zip.hpp>`. After Runtime initialization, register `module_zip()` with `add_modules`, then call `init_modules`.

```cpp
lupanic_if_failed(Luna::init());
lupanic_if_failed(Luna::add_modules({Luna::module_zip()}));
lupanic_if_failed(Luna::init_modules());
```

libzip and zlib are implementation dependencies; application code does not include their headers or handle their native types.

### Opening and enumeration

Pass a seekable input to `Zip::open_archive`. An `IFile` is one implementation; memory streams and application-defined storage streams can also be used. The archive interprets the entire input from offset zero, regardless of its initial cursor. To open a subrange, provide a stream that exposes that range as a complete stream.

The session retains the input. Its contents and size must remain unchanged while retained. The module can move its cursor. Serialize all operations on an archive, its readers, and streams shared with other archives or application code.

Opening and enumeration read the ZIP directory and structural records without decoding the entry payloads or buffering the whole ZIP. libzip may read a bounded portion of the archive tail while searching for the end-of-directory record, so this is not a guarantee that every underlying read falls exclusively inside metadata bytes.

Use `get_entries()` to list surviving entries or `find_entry(name)` for exact, case-sensitive lookup. Names are UTF-8; ZIP encoding conversion is handled by libzip. Imported entries using unsupported compression or encryption can still be listed. Opening their data returns `E_NOT_SUPPORTED`.

### Reading and errors

Call `open_entry(index)` and repeatedly read until zero bytes are returned. Directory markers cannot be opened as files and return `E_IS_DIRECTORY`. Always check `RV` even when some bytes were returned: decoding and CRC errors may appear near EOF. libzip diagnostics are available through Runtime's `explain(result.errcode())`; use `unwrap_errcode` when comparing an error code that may carry an error object.

The following helper copies one ZIP entry to a stream without loading it completely into memory. Zip must already be initialized.

```cpp
#include <Luna/Zip/Zip.hpp>

Luna::RV copy_zip_entry(Luna::ISeekableStream* input,
    const Luna::c8* name, Luna::IStream* output)
{
    using namespace Luna;
    auto opened = Zip::open_archive(input);
    if(failed(opened)) return opened.errcode();
    auto index = opened.get()->find_entry(name);
    if(failed(index)) return index.errcode();
    auto reader = opened.get()->open_entry(index.get());
    if(failed(reader)) return reader.errcode();
    byte_t buffer[16384];
    while(true)
    {
        usize size = 0;
        auto read = reader.get()->read(buffer, sizeof(buffer), &size);
        if(failed(read)) return read.errcode();
        if(!size) break;
        auto written = output->write(buffer, size);
        if(failed(written)) return written.errcode();
    }
    return ok;
}
```

### Names and directories

ZIP names are archive keys, not validated filesystem paths. A name ending in `/` marks an explicit directory. A file such as `textures/stone.png` does not require a separate `textures/` entry. Renaming or deleting a directory marker does not recursively change its descendants.

The module does not extract files to a directory, normalize `..`, resolve symlinks, or enforce a virtual filesystem's naming rules. Code that maps entry names onto a filesystem must define and enforce its own path policy.

### Shutdown

Readers retain their archive. While a reader exists, edits, saving, and discarding return `E_BUSY`. Release readers first, then save or discard the archive. Releasing an unsaved session discards it without writing its input. Release all Zip objects before `Luna::close()`.

## Supported boundaries

ZIP64 is emitted when required by archive sizes or entry counts. Stream lengths are limited to the signed 64-bit offsets supported by Runtime seeking. Multi-volume ZIPs, password handling, encryption, codecs other than Store/Deflate, extraction, and asynchronous operations are outside the current API.

The archive uses memory proportional to its index and pending edit descriptions. Payloads are read through streams as needed. The API does not impose whole-entry buffering; callers control whether their source streams use memory, files, or another medium.
