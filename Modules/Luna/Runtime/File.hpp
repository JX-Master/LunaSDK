/*!
* This file is a portion of Luna SDK.
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
namespace Luna
{
	enum class FileAttributeFlag : u32
	{
		none = 0x00,
		read_only = 0x01,			// This file is a read-only file. One read-only directory denotes a read-only file system.
		hidden = 0x02,				// This file is a hidden file.
		directory = 0x04,			// This file is a directory.
		character_special = 0x08,	// This file is an character special file on UNIX/Linux systems.
        block_special = 0x10,       // This file is an block special file on UNIX/Linux systems.
	};

	enum class FileOpenFlag : u32
	{
		none = 0x00,
		//! Grants read access to the file so that `read` operations can be performed.
		read = 0x01,
		//! Grants write access to the file so that `write` operations can be performed.
		write = 0x02,
		//! Opens the file using user-mode buffering. The user-mode buffering allocates a user-mode buffer to 
		//! buffer the data read from file or to be written to file, so reduces system calls if lots of 
		//! small size reads/writes need to be performed.
		//! 
		//! When user-mode buffer is used, `flush` or `seeks` should be called after a write operation and before
		//! a read operation, and `seek` should be called after a read operation and before a write operation.
		user_buffering = 0x04,
	};

	enum class FileCreationMode : u32
	{
		//! Always creates a new file and opens it. If the file already exists, the old file content
		//! will be discarded and the file is treated as a new empty file.
		create_always = 1,
		//! Only creates a file and opens it when it does not exist. If the file already exists, the
		//! call fails with `BasicError::already_exists`.
		create_new = 2,
		//! Always opens a file. If the file already exists, the file will be open with its data preserved,
		//! if the file does not exist, it will be created and opened.
		open_always = 3,
		//! Only opens a file when it does exists. If the file does not exist, the call fails with `BasicError::not_found`.
		open_existing = 4,
		//! Only opens a file when it does exists, and discards the file data so the file is treated as a new
		//! file.
		open_existing_as_new = 5
	};

	struct FileAttribute
	{
		u64 size;
		u64 creation_time;
		u64 last_access_time;
		u64 last_write_time;
		FileAttributeFlag attributes;
	};

	//! @interface IFile
	//! `IFile` interface represents a opened file object of the platform or the virtual file system.
	struct IFile : virtual ISeekableStream
	{
		luiid("{c61fbf68-89dc-4776-8a99-cc855ff8289e}");

		//! Clears all buffers for this stream and causes all buffered data to be written to the underlying device.
		virtual void flush() = 0;
	};

	//! @interface IFileIterator
	//! Represents a directory stream that can be used to iterate all files and directories
	//! in the specified directory. See `open_dir` for details.
	struct IFileIterator : virtual Interface
	{
		luiid("{97643c3c-4681-4b24-9aee-320f88379edc}");

		//! Checks if this iterator points to a valid file item in the directory stream.
		virtual bool is_valid() = 0;

		//! Returns the name of the file the iterator currently points to.
		//! Returns `nullptr` if the file iterator is invalid.
		virtual const c8* get_filename() = 0;

		//! Returns the file attribute of the file the iterator currently points to.
		//! Returns EFileAttributeFlag::none if the file iterator is invalid.
		virtual FileAttributeFlag get_attribute() = 0;

		//! Moves to the next file in the directory.
		//! @return Returns `true` if the file iterator is valid after this call, 
		//! returns `false` otherwise.
		//! If the return value is `false`, then the iterator has reached the end of the 
		//! directory stream.
		virtual bool move_next() = 0;
	};

	//! Opens one file.
	//! @param[in] path The path of the file.
	//! @param[in] flags The file open flags.
	//! @param[in] creation Specify whether to create a file if the file does not exist.
	//! @return If succeeded, returns the new opened file object. If failed, returns one of the following error codes:
	//! * BasicError::bad_arguments
	//! * BasicError::access_denied
	//! * BasicError::not_found
	//! * BasicError::not_directory
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<Ref<IFile>>	open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation);

	//! Returns the data in the specified file as a blob object.
	//! @param[in] file The file handle. The file handle must have read access.
	//! @return Returns the data of the file.
	LUNA_RUNTIME_API R<Blob> load_file_data(IFile* file);

	//! Gets the file or directory attribute.
	//! @param[in] path The path of the file to check.
	//! @return Returns the file attribute structure, or one of the following error codes if failed:
	//! * BasicError::access_denied
	//! * BasicError::not_found
	//! * BasicError::not_directory
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<FileAttribute> get_file_attribute(const c8* path);

	enum class FileCopyFlag : u32
	{
		none = 0x00,
		//! The operation fails with `BasicError::already_exists` if the destination path exists.
		fail_if_exists = 0x01,
	};

	//! Copies the file or directory from the source path to the destination path.
	//! @param[in] from_path Source file or directory path.
	//! @param[in] to_path Destination file or directory path.
	//! @param[in] flags Additional flags.
	//! @return This call returns one of the following errors:
	//! * BasicError::bad_arguments
	//! * BasicError::already_exists
	//! * BasicError::access_denied
	//! * BasicError::not_found
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags = FileCopyFlag::none);

	enum class FileMoveFlag : u32
	{
		none = 0x00,
		//! The operation fails with `BasicError::already_exists` if the destination path exists.
		fail_if_exists = 0x01,
	};

	//! Moves the file or directory from the source path to the destination path. This call can also be used to rename a file.
	//! @param[in] from_path Source file path.
	//! @param[in] to_path Destination file path.
	//! @return This call returns one of the following errors:
	//! * BasicError::bad_arguments
	//! * BasicError::already_exists
	//! * BasicError::access_denied
	//! * BasicError::not_found
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags = FileMoveFlag::none);

	//! Deletes the specified file or directory.
	//! @param[in] file_path The file or directory to delete. If this is a non-empty directory, all its contexts will also be removed.
	//! @return This call returns one of the following errors:
	//! * BasicError::bad_arguments
	//! * BasicError::not_found
	//! * BasicError::access_denied
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	delete_file(const c8* file_path);
	//! Creates a file iterator that can be used to iterate all files in the specified directory.
	//! @param[in] path The directory path to open.
	//! @return Returns a file iterator object if succeeded. The result will be one of the following:
	//! * BasicError::not_found
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<Ref<IFileIterator>> open_dir(const c8* path);
	//! Creates one empty directory.
	//! @param[in] path The path of the directory.
	//! @return This call returns one of the following errors:
	//! * BasicError::already_exists
	//! * BasicError::not_found
	//! * BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	create_dir(const c8* path);
	//! Get the current working directory path for the underlying system.
	//! The default current working directory is set to the path that contains the executable file.
	//! @param[in] buffer_length The length of the buffer for the current directory string, including the null terminator.
	//! @param[in] A pointer to the buffer that receives the current directory string. To gets the required buffer size, specify
	//! `buffer` to `nullptr` and `buffer_length` to 0.
	//! @return Returns the number of characters copied into the buffer, including the null terminator. If `buffer_length` is 0 and
	//! `buffer` is `nullptr`, returns the required buffer size to fetch the current directory, including the null terminator. 
	LUNA_RUNTIME_API u32 get_current_dir(u32 buffer_length, c8* buffer);
	//! Set the current working directory path for the underlying system. The current directory will be set for the process scope.
	LUNA_RUNTIME_API RV set_current_dir(const c8* path);
	//! Get the application executable file's absolute directory, ended with application executable name.
	//! @return Returns the process path. The string is static and valid until the SDK is closed.
	LUNA_RUNTIME_API const c8* get_process_path();
}
