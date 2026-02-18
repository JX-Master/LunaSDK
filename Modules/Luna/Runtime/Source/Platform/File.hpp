/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../File.hpp"
#include "Result.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)

#endif

namespace Luna
{
    namespace Platform
    {
        struct File
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            opaque_t m_handle = nullptr;
            bool m_buffered = false;
#elif defined(LUNA_PLATFORM_POSIX)

#endif
            bool valid() const
            {
#if defined(LUNA_PLATFORM_WINDOWS)
                return m_handle != nullptr;
#endif
            }
        };

        //! Opens one file.
        //! @param[in] path The path of the file.
        //! @param[in] flags The file open flags.
        //! @param[in] creation Specify whether to create a file if the file does not exist.
        //! @param[out] out_file Returns the created file.
        //! @par Possible Errors
        //! * Result::bad_arguments
        //! * Result::access_denied
        //! * Result::not_found
        //! * Result::bad_platform_call for all errors that cannot be identified.
        Result open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, File& out_file);

        //! Closes one file opened by `open_file`.
        //! @param[in] file The file handle opened by `open_file`.
        void close_file(File& file);

        //! Reads data from the current position the cursor is pointing to and offsets the cursor back. If the data to be
        //! read is not ready, the platform suspends this thread until the data is ready.
        //! @param[in] file The file handle opened by `open_file`.
        //! @param[in] buffer The buffer used to store the read data.
        //! @param[in] size The size, in bytes, of the data to read.
        //! @param[out] read_bytes If this is not `nullptr`, the system sets the actual size of bytes being read to the buffer
        //! to this parameter.
        //! The actual size of bytes being read may be smaller than the size of bytes required to be read if the cursor
        //! reaches the end of the stream, but this is NOT an error. Specially, if one read operation is performed when
        //! the cursor is beyond or at the end of the stream, the read operation succeeds with 0 bytes being read. This 
        //! can be considered as an EOF symbol in stdlib.
        //! @return Returns success on success, returns the following error code on failure:
        //! * Result::not_supported
        Result read_file(File& file, void* buffer, usize size, usize* read_bytes = nullptr);

        //! Writes data to the current position the cursor is pointing to and offsets the cursor back. This call returns after
        //! all data have been written.
        //! @param[in] file The file handle opened by `open_file`.
        //! @param[in] buffer The buffer that holds the data to be written.
        //! @param[in] size The size, in bytes, of the data to write.
        //! @param[out] write_bytes If not `nullptr`, the system sets the actual size of bytes being written to this parameter.
        //! Mostly, if the cursor goes beyond the end of the stream buffer while writing data, the stream will be expanded so
        //! the succeeding data can be written, so unless an error occurs, the size of bytes written will always equal to the 
        //! size of bytes required by the user to write. However, if an error occurs while writing data, some of the data may have 
        //! already be written while others are not, in such case the `write_bytes` reported by system may not be equal to `size` 
        //! specified by the user.
        Result write_file(File& file, const void* buffer, usize size, usize* write_bytes = nullptr);

        //! Gets the size of the file in bytes.
        //! @param[in] file The file handle opened by `open_file`.
        //! @return The size of the file, or error code on failure.
        u64 get_file_size(File& file);

        //! Sets the size of the file in bytes.
        //! If the current file size is smaller than the size to set and this call succeeded, the stream will be extended to the size specified
        //! with data between the last size and current size be uninitialized. If the current file size is greater than the size to set and this 
        //! call succeeded, the stream will be truncated and the data between the last size and current size will be discarded.
        //! @param[in] file The file handle opened by `open_file`.
        //! @param[in] size The size to set, in bytes.
        Result set_file_size(File& file, u64 size);

        //! Gets the current position of the stream cursor. The position is number of bytes relative to the beginning of the 
        //! stream.
        //! @param[in] file The file handle opened by `open_file`.
        //! @param[out] out_cursor Returns the current position of the stream cursor.
        Result get_file_cursor(File& file, u64& out_cursor);

        //! Moves the cursor to a new position.
        //! @param[in] file The file handle opened by `open_file`.
        //! @param[in] offset Number of bytes to move relative to the position specified by `mode`.
        //! @param[in] mode The seek mode to use.
        //! @remark The cursor position is not bound to the size of the stream. If you move the cursor beyond the end of the stream, the cursor
        //! is still valid with the following behaviors:
        //! 1. All read operations succeed with 0 byte being read, just like EOF behaviors.
        //! 2. All write operations extends the size of the stream before the actual write operation performed.
        //! 
        //! The cursor value cannot be negative, if the new cursor position goes below 0, the seek operation fails.
        Result set_file_cursor(File& file, i64 offset, SeekMode mode);

        //! Clears all buffers for this file and causes all buffered data to be written to the underlying device.
        //! @param[in] file The file handle opened by `open_file`.
        void flush_file(File& file);

        //! Gets the attribute/status of one file or directory.
        //! @param[in] path The path of the file to get.
        //! @param[out] out_attribute Returns the attribute/status of one file or directory.
        //! @return The file attribute structure if succeeded, returns error code if failed.
        //! Possible errors:
        //! * Result::access_denied
        //! * Result::not_found
        //! * Result::bad_platform_call for all errors that cannot be identified.
        Result get_file_attribute(const c8* path, FileAttribute& out_attribute);

        //! Copies the file from the source path to the destination path.
        //! Refer to docs in `File.hpp`.
        Result copy_file(const c8* from_path, const c8* to_path);

        //! Moves the file from the source path to the destination path. This call can also be used to rename a file.
        //! Refer to docs in `File.hpp`.
        Result move_file(const c8* from_path, const c8* to_path);

        //! Deletes the specified file or directory.
        //! Refer to docs in `File.hpp`.
        Result delete_file(const c8* path);

        struct FileIterator
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            HANDLE m_h = INVALID_HANDLE_VALUE;
            WIN32_FIND_DATAW m_data;
            char m_file_name[512];    // Buffer to store the file name in UTF-8 format.
            bool m_allocated;
#elif defined(LUNA_PLATFORM_POSIX)

#endif
            bool valid() const
            {
#if defined(LUNA_PLATFORM_WINDOWS)
                return m_h != INVALID_HANDLE_VALUE;
#elif defined(LUNA_PLATFORM_POSIX)

#endif
            }
        };

        //! Creates a file iterator handle that can be used to iterate all files in the specified directory.
        //! @param[in] path The directory path to open.
        //! @param[out] out_dir_iter Returns a file iterator handle if succeeded.
        //! @par Possible Errors
        //! * BasicError::access_denied
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        Result open_dir(const c8* path, FileIterator& out_dir_iter);

        //! Closes the file iterator handle.
        void close_dir(FileIterator& dir_iter);

        //! Checks if this iterator points to a valid file item in the directory stream.
        //! @param[in] dir_iter The directory iterator handle.
        bool dir_iterator_is_valid(FileIterator& dir_iter);

        //! Returns the name of the file the iterator currently points to.
        //! Returns `nullptr` if the file iterator is invalid.
        //! @param[in] dir_iter The directory iterator handle.
        const c8* dir_iterator_get_filename(FileIterator& dir_iter);

        //! Returns the file attribute of the file the iterator currently points to.
        //! Returns EFileAttributeFlag::none if the file iterator is invalid.
        //! @param[in] dir_iter The directory iterator handle.
        FileAttributeFlag dir_iterator_get_attributes(FileIterator& dir_iter);

        //! Moves the file iterator to the next file in the directory.
        //! @param[in] dir_iter The directory iterator handle.
        //! @return Returns `true` if the file iterator is valid after this call, 
        //! returns `false` otherwise.
        //! If the return value is `false`, then the iterator has reached the end of the 
        //! directory stream.
        bool dir_iterator_move_next(FileIterator& dir_iter);

        //! Creates one directory.
        //! @param[in] path The path of the directory to create.
        //! @return Returns on of the following error codes on error:
        //! * BasicError::already_exists
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        Result create_dir(const c8* path);

        //! Get the current working directory path for the underlying system.
        //! The default current working directory is set to the path that contains the executable file.
        //! @param[out] buffer A pointer to the buffer that receives the current directory string. To gets the required buffer size, specify
        //! `buffer` to `nullptr` and `buffer_size` to 0.
        //! @param[in] buffer_size The length of the buffer for the current directory string, including the null terminator.
        //! @return The number of `c8` characters copied into the buffer, including the null terminator. If `buffer_size` is 0 and
        //! `buffer` is `nullptr`, returns the required buffer size to fetch the current directory, including the null terminator. 
        usize get_current_dir(c8* buffer, usize buffer_size);

        //! Set the current working directory path for the underlying system. The current directory will be set for the process scope.
        //! @param[in] path The path to set. This must be a null-terminated string.
        //! @return Returns `s_ok` on success, returns one of the following error codes on failure:
        //! * BasicError::bad_platform_call
        Result set_current_dir(const c8* path);

        //! Get the application executable file's absolute path.
        //! @param[out] buffer A pointer to the buffer that receives the path string. To gets the required buffer size, specify
        //! `buffer` to `nullptr` and `buffer_size` to 0.
        //! @param[in] buffer_size The length of the buffer for the path string, including the null terminator.
        //! @return The number of `c8` characters copied into the buffer, including the null terminator. If `buffer_size` is 0 and
        //! `buffer` is `nullptr`, returns the required buffer size to fetch the path, including the null terminator. 
        usize get_process_path(c8* buffer, usize buffer_size);
    }
}