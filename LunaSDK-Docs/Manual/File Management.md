```c++
#include <Luna/Runtime/File.hpp>
```

## File IO

`IFile` represents one system-level file handle that can be used to read and write file data. The user can create one `IFile` interface using `open_file`, and the file handle will be closed when its reference count drops to `0`. Data in the file can be read by `IFile::read`, and can be written by `IFile::write`. The current file cursor for IO operations can be fetched by `IFile::tell`, and can be set by `IFile::seek`. The whole size of the file can be fetched by `IFile::get_size`, and can be set by `IFile::set_size`.

In most cases, file data written by `IFile::write` will not be written to storage media immediately, but being cached in driver buffer and written back to the storage in next hardware flush. The user can wait such flush by calling `IFile::flush`, which blocks the current thread until all written data is successfully written back to storage and visible to other processes.

If you simply want to load file data after opening one file, you can pass `IFile` to `load_file_data` immediately after it is opened, which loads the whole file binary data and returns the data as one `Blob` object.

## File operations

Use `file_attribute` to fetch the attribute of one specified file, like its creation time, last modified time, whether it is a directory, etc. 

Use `copy_file` and `move_file` for coping and moving files and directories. Use `delete_file` for deleting one file or directory.

Use `open_dir` to create a file iterator (`IFileIterator`) that can be used to iterate over files and directories in the specified directory. Use `create_dir` to create a new empty directory on the specified directory.

Use `get_current_dir` and `set_current_dir` to get and set the current working directory of the current process. Use `get_process_path` to get process executable file's absolute directory, ended with application executable name.

