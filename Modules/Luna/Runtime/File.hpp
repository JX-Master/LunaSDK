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
		//! call fails with @ref BasicError::already_exists.
		create_new = 2,
		//! Always opens a file. If the file already exists, the file will be open with its data preserved,
		//! if the file does not exist, it will be created and opened.
		open_always = 3,
		//! Only opens a file when it does exists. If the file does not exist, the call fails with @ref BasicError::not_found.
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
	struct IFile : virtual ISeekableStream
	{
		luiid("{c61fbf68-89dc-4776-8a99-cc855ff8289e}");

		//! Clears all buffers for this stream and causes all buffered data to be written to the underlying device.
		virtual void flush() = 0;
	};

	//! @interface IFileIterator
	//! Represents a directory stream that can be used to iterate all files and directories
	//! in the specified directory. See @ref open_dir for details.
	struct IFileIterator : virtual Interface
	{
		luiid("{97643c3c-4681-4b24-9aee-320f88379edc}");

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
	//! * @ref BasicError::bad_arguments
	//! * @ref BasicError::access_denied
	//! * @ref BasicError::not_found
	//! * @ref BasicError::not_directory
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<Ref<IFile>>	open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation);

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
	//! * @ref BasicError::access_denied
	//! * @ref BasicError::not_found
	//! * @ref BasicError::not_directory
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<FileAttribute> get_file_attribute(const c8* path);

	//! Specify attributes for one file copy operation.
	enum class FileCopyFlag : u32
	{
		none = 0x00,
		//! If this is specified, the copy operation fails with @ref BasicError::already_exists if the destination file exists.
		//! If this is not specified, the destination file will be overwritten by the source file in the copy operation.
		fail_if_exists = 0x01,
	};

	//! Copies the file or directory from the source path to the destination path.
	//! @param[in] from_path Source file or directory path.
	//! @param[in] to_path Destination file or directory path.
	//! @param[in] flags The attributes for one file copy operation.
	//! @par Possible Errors
	//! * @ref BasicError::bad_arguments
	//! * @ref BasicError::already_exists
	//! * @ref BasicError::access_denied
	//! * @ref BasicError::not_found
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags = FileCopyFlag::none);

	//! Specify attributes for one file move operation.
	enum class FileMoveFlag : u32
	{
		none = 0x00,
		//! If this is specified, the move operation fails with @ref BasicError::already_exists if the destination file exists.
		//! If this is not specified, the destination file will be overwritten by the source file in the move operation.
		fail_if_exists = 0x01,
	};

	//! Moves the file or directory from the source path to the destination path. This call can also be used to rename a file.
	//! @param[in] from_path Source file or directory path.
	//! @param[in] to_path Destination file or directory path.
	//! @param[in] flags The attributes for one file move operation.
	//! @par Possible Errors
	//! * @ref BasicError::bad_arguments
	//! * @ref BasicError::already_exists
	//! * @ref BasicError::access_denied
	//! * @ref BasicError::not_found
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags = FileMoveFlag::none);

	//! Deletes the specified file or directory.
	//! @param[in] file_path The file or directory to delete. If this is a non-empty directory, all its contexts will also be deleted.
	//! @par Possible Errors
	//! * @ref BasicError::bad_arguments
	//! * @ref BasicError::not_found
	//! * @ref BasicError::access_denied
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	delete_file(const c8* file_path);
	//! Creates a file iterator that can be used to iterate all files in the specified directory.
	//! @param[in] path The directory path to open.
	//! @return Returns a file iterator object. 
	//! @par Possible Errors
	//! * @ref BasicError::not_found
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API R<Ref<IFileIterator>> open_dir(const c8* path);
	//! Creates one empty directory.
	//! @param[in] path The path of the directory to create.
	//! @par Possible Errors
	//! * @ref BasicError::already_exists
	//! * @ref BasicError::not_found
	//! * @ref BasicError::bad_platform_call for all errors that cannot be identified.
	LUNA_RUNTIME_API RV	create_dir(const c8* path);
	//! Gets the current working directory path for the underlying system.
	//! @details The default current working directory is set to the path that contains the executable file.
	//! @param[in] buffer_length The length of the buffer for the current directory string, including the null terminator.
	//! @param[in] buffer A pointer to the buffer that receives the current directory string. To gets the required buffer size, specify
	//! @ref buffer to `nullptr` and @ref buffer_length to 0.
	//! @return Returns the number of characters copied into the buffer, including the null terminator. The copied string is always null-terminated.
	//! If @ref buffer_length is `0` and @ref buffer is `nullptr`, returns the required buffer size to fetch the current directory, including the null terminator. 
	LUNA_RUNTIME_API u32 get_current_dir(u32 buffer_length, c8* buffer);
	//! Sets the current working directory path for the underlying system. The current directory will be set for the process scope.
	//! @param[in] path The current working directory path to set.
	LUNA_RUNTIME_API RV set_current_dir(const c8* path);
	//! Gets the full (absolute) path of the application's executable file.
	//! @return Returns the full (absolute) path of the application's executable file. 
	//! The returned pointer is valid until Luna SDK is closed.
	LUNA_RUNTIME_API const c8* get_process_path();

	//! @}
}
