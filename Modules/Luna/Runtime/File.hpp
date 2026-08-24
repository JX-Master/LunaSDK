/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.hpp
* @author JXMaster
* @date 2019/3/14
*/
#pragma once
#include "Stream.hpp"
#include "Ref.hpp"
#include "File.generated.hpp"
namespace Luna
{
    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeFile Files
    //! @}

    //! @addtogroup RuntimeFile
    //! @{ 
    
    //! Represents file binary attributes.
    enum class FileAttributeFlag : u32
    {
        none = 0x00,
        //! This file is a read-only file. One read-only directory denotes a read-only file system.
        read_only = 0x01,
        //! This file is a hidden file.
        hidden = 0x02,
        //! This file is a directory.            
        directory = 0x04,
        //! This file is an character special file on UNIX/Linux systems.
        character_special = 0x08,
        //! This file is an block special file on UNIX/Linux systems.
        block_special = 0x10,
    };

    //! Specifies attributes for one file open operation.
    enum class FileOpenFlag : u32
    {
        none = 0x00,
        //! Grants read access to the file so that @ref IFile::read operations can be performed.
        read = 0x01,
        //! Grants write access to the file so that @ref IFile::write operations can be performed.
        write = 0x02,
        //! Allocates one user-mode buffer for the opened file. 
        //! @details The user-mode buffer can be used to buffer the data read from file or to be written to file, 
        //! thus reduce system calls if lots of small-sized reads/writes need to be performed.
        user_buffering = 0x04,
    };

    //! Specifies file creation mmode.
    enum class FileCreationMode : u32
    {
        //! Always creates a new file and opens it. If the file already exists, the old file content
        //! will be discarded and the file is treated as a new empty file.
        create_always = 1,
        //! Only creates a file and opens it when it does not exist. If the file already exists, the
        //! call fails with @ref E_ALREADY_EXISTS.
        create_new = 2,
        //! Always opens a file. If the file already exists, the file will be open with its data preserved,
        //! if the file does not exist, it will be created and opened.
        open_always = 3,
        //! Only opens a file when it does exists. If the file does not exist, the call fails with @ref E_NOT_FOUND.
        open_existing = 4,
        //! Only opens a file when it does exists, and discards the file data so the file is treated as a new
        //! file.
        open_existing_as_new = 5
    };

    //! Specifies file attributes.
    struct FileAttribute
    {
        //! The size, in bytes, of the file.
        u64 size;
        //! The file creation tiem represented by UNIX timestamps.
        i64 creation_time;
        //! The file last access time represented by UNIX timestamps.
        i64 last_access_time;
        //! The file last write time represented by UNIX timestamps.
        i64 last_write_time;
        //! The file binary attributes represented by flags.
        FileAttributeFlag attributes;
    };

    //! @interface IFile
    //! Represents a opened file object.
    struct [[Luna::interface("{c61fbf68-89dc-4776-8a99-cc855ff8289e}")]] IFile : virtual ISeekableStream
    {
        //! Clears all buffers for this stream and causes all buffered data to be written to the underlying device.
        virtual void flush() = 0;
    };

    //! @interface IFileIterator
    //! Represents a directory stream that can be used to iterate all files and directories
    //! in the specified directory. See @ref open_dir for details.
    struct [[Luna::interface("{97643c3c-4681-4b24-9aee-320f88379edc}")]] IFileIterator : virtual Interface
    {
        //! Checks if this iterator points to a valid file in the directory stream.
        //! @return Returns `true` if the iterator points to a valid file, returns `false` otherwise.
        virtual bool is_valid() = 0;
        
        //! Gets the filename of the file that the iterator currently points to.
        //! @return Returns the name of the file that the iterator currently points to,
        //! returns `nullptr` if the file iterator is invalid. The returnd pointer is valid until 
        //! the file iterator is released or @ref move_next is called.
        virtual const c8* get_filename() = 0;

        //! Gets the file attribute flags of the file.
        //! @return Returns the file attribute flags of the file that the iterator currently points to.
        //! Returns @ref FileAttributeFlag::none if the file iterator is invalid.
        virtual FileAttributeFlag get_attributes() = 0;

        //! Moves the iterator to the next file in the directory.
        //! @return Returns `true` if the file iterator is valid after this call, 
        //! returns `false` otherwise.
        //! If the return value is `false`, then the iterator has reached the end of the 
        //! directory stream.
        virtual bool move_next() = 0;
    };

    //! Opens one file.
    //! @param[in] path The path of the file.
    //! @param[in] flags The file open flags.
    //! @param[in] creation The file creation mode.
    //! @return Returns the new opened file object.
    //! @par Possible Errors
    //! * @ref E_BAD_ARGUMENTS
    //! * @ref E_ACCESS_DENIED
    //! * @ref E_NOT_FOUND
    //! * @ref E_NOT_DIRECTORY
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API R<Ref<IFile>> open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation);
    //! Returns the data in the specified file as a blob object.
    //! @param[in] file The file to read.
    //! @return Returns the data of the file.
    //! @par Valid Usage
    //! * `file` must be opened with @ref FileOpenFlag::read flag.
    LUNA_RUNTIME_API R<Blob> load_file_data(IFile* file);
    //! Gets the file attribute.
    //! @param[in] path The path of the file.
    //! @return Returns the file attribute structure.
    //! @par Possible Errors
    //! * @ref E_ACCESS_DENIED
    //! * @ref E_NOT_FOUND
    //! * @ref E_NOT_DIRECTORY
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API R<FileAttribute> get_file_attribute(const c8* path);
    //! Copies the file from the source path to the destination path. This function cannot copy directories.
    //! @param[in] from_path Source file path. If `from_path` does not existm this operation failed with @ref E_NOT_FOUND.
    //! @param[in] to_path Destination file or path. If `to_path` already exists, this operation fails with @ref E_ALREADY_EXISTS
    //! and the existing file will not be modified.
    //! @par Possible Errors
    //! * @ref E_BAD_ARGUMENTS
    //! * @ref E_ALREADY_EXISTS
    //! * @ref E_ACCESS_DENIED
    //! * @ref E_NOT_FOUND
    //! * @ref E_IS_DIRECTORY
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API RV copy_file(const c8* from_path, const c8* to_path);
    //! Moves the file or directory from the source path to the destination path. This call can also be used to rename a file.
    //! @param[in] from_path Source file or directory path. If `from_path` does not existm this operation failed with @ref E_NOT_FOUND.
    //! @param[in] to_path Destination file or directory path. If `to_path` already exists, this operation fails with @ref E_ALREADY_EXISTS
    //! and the existing file will not be modified.
    //! @par Possible Errors
    //! * @ref E_BAD_ARGUMENTS
    //! * @ref E_ALREADY_EXISTS
    //! * @ref E_ACCESS_DENIED
    //! * @ref E_NOT_FOUND
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API RV move_file(const c8* from_path, const c8* to_path);
    //! Deletes the specified file or directory.
    //! @param[in] file_path The file or directory to delete.
    //! If this is a directory, it must be empty.
    //! @par Possible Errors
    //! * @ref E_BAD_ARGUMENTS
    //! * @ref E_NOT_FOUND
    //! * @ref E_ACCESS_DENIED
    //! * @ref E_DIRECTORY_NOT_EMPTY
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API RV delete_file(const c8* file_path);
    //! Creates a file iterator that can be used to iterate all files in the specified directory.
    //! @param[in] path The directory path to open.
    //! @return Returns a file iterator object. 
    //! @par Possible Errors
    //! * @ref E_NOT_FOUND
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API R<Ref<IFileIterator>> open_dir(const c8* path);
    //! Creates one empty directory.
    //! @param[in] path The path of the directory to create.
    //! @par Possible Errors
    //! * @ref E_ALREADY_EXISTS
    //! * @ref E_NOT_FOUND
    //! * @ref E_BAD_PLATFORM_CALL for all errors that cannot be identified.
    LUNA_RUNTIME_API RV create_dir(const c8* path);
    //! Get the current working directory path for the underlying system.
    //! @return Returns the current working directory path. The path should be freed by @ref release_current_dir.
    LUNA_RUNTIME_API const c8* get_current_dir();
    //! Releases the current work directory string returned by @ref get_current_dir.
    LUNA_RUNTIME_API void release_current_dir(const c8* path);
    //! Sets the current working directory path for the underlying system. The current directory will be set for the process scope.
    //! @param[in] path The current working directory path to set.
    LUNA_RUNTIME_API RV set_current_dir(const c8* path);
    //! Get the application executable file's absolute path.
    //! @return Returns the application executable file's absolute path.
    //! The returned string shall be freed by calling @ref release_process_path.
    LUNA_RUNTIME_API const c8* get_process_path();
    //! Releases the path string returned by @ref get_process_path.
    LUNA_RUNTIME_API void release_process_path(const c8* path);

    //! @}s
}
