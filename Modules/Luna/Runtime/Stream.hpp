/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Stream.hpp
* @author JXMaster
* @date 2020/1/28
*/
#pragma once
#include "Interface.hpp"
#include "Result.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	
	//! Specify the seek mode for one seekable stream.
	enum class SeekMode : u32
	{
		//! The offset is relative to the beginning of the stream.
		begin = 1,
		//! The offset is relative to the current position of the cursor.
		current = 2,
		//! The offset is relative to the end of the stream.
		end = 3
	};

	//! @interface IStream
	//! Represents a serial stream sequence of bytes and supports read/write operations on them.
	//! @details Common implementations of streams include file, memory buffer, web socket and so on.
	//! 
	//! This object is not thread safe and the I/O operations on this object is not asynchronous (will suspend
	//! the current thread until the operation is done or failed).
	struct IStream : virtual Interface
	{
		luiid("{0345f636-ca5c-4b4d-8416-29834377d239}");

		//! Reads data from the current position the cursor is pointing to and advances the cursor. 
		//! @details If the data to be read is not ready, the platform suspends the calling thread until the data is ready.
		//! @param[in] buffer The buffer to accept the read data.
		//! @param[in] size The size, in bytes, to read from the stream.
		//! @param[out] read_bytes If not `nullptr`, the system sets the actual size of bytes being read to the buffer 
		//! to this parameter.
		//! 
		//! The actual size of bytes being read may be smaller than the size of bytes required to be read if the cursor
		//! reaches the end of the stream, but this is NOT an error. Specially, if one read operation is performed when
		//! the cursor is beyond or at the end of the stream, the read operation succeeds with 0 bytes being read. This 
		//! can be considered as an EOF symbol in stdlib.
		virtual RV read(void* buffer, usize size, usize* read_bytes = nullptr) = 0;

		//! Writes data to the current position the cursor is pointing to and advances the cursor. 
		//! @details This call returns after all data have been written.
		//! @param[in] buffer The buffer that holds the data to be written.
		//! @param[in] size The size, in bytes, to write to the stream.
		//! @param[out] write_bytes If not `nullptr`, the system sets the actual size of bytes being written to this parameter.
		//! 
		//! Mostly, if the cursor goes beyond the end of the stream buffer while writing data, the stream will be expanded so
		//! the succeeding data can be written, so unless an error occurs, the size of bytes written will always equal to the 
		//! size of bytes required by the user to write. However, if an error occurs while writing data, some of the data may have 
		//! already be written while others are not, in such case the `write_bytes` reported by system may not be equal to `size` 
		//! specified by the user.
		virtual RV write(const void* buffer, usize size, usize* write_bytes = nullptr) = 0;
	};

	//! @interface ISeekableStream
	//! Represents one stream object that supports setting the cursor position.
	struct ISeekableStream : virtual IStream
	{
		luiid("{42F66080-C388-4EE0-9C4D-1EEC1B82F692}");

		//! Gets the current position of the stream cursor. 
		//! @return Returns the current position of the stream cursor. The position is number of bytes relative to the beginning of the stream.
		virtual R<u64> tell() = 0;

		//! Moves the read/write cursor to a new position.
		//! @param[in] offset Number of bytes to move relative to the position specified by `mode`.
		//! @param[in] mode The seek mode to use.
		//! @remark The cursor position is not bound to the size of the stream. If you move the cursor beyond the end of the stream, the cursor
		//! is still valid with the following behaviors:
		//! 1. All read operations succeed with 0 byte being read, just like EOF behaviors.
		//! 2. All write operations extends the size of the stream before the actual write operation performed.
		//! 
		//! The cursor value cannot be negative, if the new cursor position goes below 0, the seek operation fails.
		virtual RV seek(i64 offset, SeekMode mode) = 0;

		//! Gets the size of the stream buffer in bytes.
		//! @return Returns the of the stream buffer in bytes. Returns `0` if the underlying stream is invalid or does not have a specific size.
		virtual u64 get_size() = 0;

		//! Sets the size of the stream buffer.
		//! @details If the current stream buffer size is smaller than the size to set and this call succeeded, the stream buffer will be extended to the 
		//! size specified, with data between the last size and current size be uninitialized. 
		//! If the current stream buffer size is greater than the size to set and this call succeeded, the stream buffer will be truncated and 
		//! the data between the last size and current size will be discarded.
		//! @param[in] size The size to set, in bytes.
		virtual RV set_size(u64 size) = 0;
	};

	//! @}
}