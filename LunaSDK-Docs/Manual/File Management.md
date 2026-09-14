```c++
#include <Luna/Runtime/File.hpp>
```

## File IO

`IFile` represents one system-level file handle that can be used to read and write file data. The user can create one `IFile` interface using `open_file`, and the file handle will be closed when its reference count drops to `0`. Data in the file can be read by `IFile::read`, and can be written by `IFile::write`. The current file cursor for IO operations can be fetched by `IFile::tell`, and can be set by `IFile::seek`. The whole size of the file can be fetched by `IFile::get_size`, and can be set by `IFile::set_size`.

`IFile::flush` has different strength depending on how the file was opened. For a file opened with `FileOpenFlag::user_buffering`, it drains the Runtime/C-library user-mode buffer to the operating system, but does not request durable storage synchronization. For an unbuffered file, the current platform implementations call `fsync` or `FlushFileBuffers` to request that the operating system flush pending file data. Device caches and platform guarantees still determine the final durability semantics, so do not describe `flush` as an unconditional physical-media commit.

If you simply want to load file data after opening one file, you can pass `IFile` to `load_file_data` immediately after it is opened, which loads the whole file binary data and returns the data as one `Blob` object.

## File operations

Use `get_file_attribute` to fetch the attributes of a specified file or directory, such as its creation time, last modified time, and whether it is a directory.

Use `copy_file` to copy one file. It does not copy directories. Use `move_file` to move or rename a file or directory. Use `delete_file` to delete a file or an empty directory; a non-empty directory must be emptied first.

`move_file(from, to, flags)` accepts optional `FileMoveFlag` options. Omitting the flags preserves the default behavior: an existing destination causes `E_ALREADY_EXISTS` and remains unchanged.

| Option | Behavior |
| --- | --- |
| `none` | Keeps the default move behavior and does not overwrite the destination. |
| `allow_overwrite` | Allows a file to replace an existing file. Directories cannot replace existing files or be overwritten; moving a directory to an absent destination is still allowed. |
| `no_copy` | Requires a native move on the same filesystem, without copy/delete fallback. A cross-filesystem move returns `E_NOT_SUPPORTED`. |

Options can be combined with `|`. Windows permits cross-volume file copying unless `no_copy` is set; POSIX always requires a native rename. For example, `move_file(from, to, FileMoveFlag::allow_overwrite)` enables ordinary file replacement. To publish a prepared file on the same filesystem, VFS Pak storage uses:

```cpp
RV result = move_file("game.pak.tmp", "game.pak",
    FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy);
// Check result before treating the prepared file as published.
```

This combination performs checked native replacement without deleting the original file first or falling back to copying. The destination may also be absent. The source and destination must identify distinct files or directories, and callers must exclude concurrent changes to either entry. Close handles that prevent native moves first. The operation does not synchronize file data or promise power-loss recovery. Without `no_copy`, cross-volume copying follows native platform semantics and does not provide the same publication guarantees.

Use `open_dir` to create a file iterator (`IFileIterator`) that can be used to iterate over files and directories in the specified directory. Use `create_dir` to create a new empty directory on the specified directory.

Use `get_current_dir` and `set_current_dir` to get and set the current working directory of the process. The string returned by `get_current_dir` must be released with `release_current_dir`.

Use `get_process_path` to get the absolute path of the process executable, including the executable name. The returned string must be released with `release_process_path`.
