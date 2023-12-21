/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.hpp
* @author JXMaster
* @date 2022/8/29
*/
#pragma once
#include "Interface.hpp"
#include "Ref.hpp"

namespace Luna
{
	//! @addtogroup RuntimeThread
	//! @{
	
	//! @interface IReadWriteLock
	//! Represents one system-level read write lock.
	//! @details One read write lock allows multiple threads to access the same resource in read mode, or at most one thread to access the resource in 
	//! write mode. One read write lock is created in unlocked mode, and can be transferred to read mode by acquiring read ownership of the lock, or 
	//! transferred to write mode by acquiring write ownership of the lock. When the read write lock is in read mode, successing acquires of read ownership
	//! succeeds, but acquires of write ownership will fail or be blocked until all read ownerships are released; When the read write lock is in write mode,
	//! all succeeding acquires of read and write ownerships will fail or be blocked until the write ownership is released. The write ownership is not recursive:
	//! succeeding acquire of write ownership from the thread that already acquired the write ownership causes deadlock and shall not be performed.
	//! @threadsafe
	struct IReadWriteLock : virtual Interface
	{
		luiid("{F74B6F8A-AF52-4ABA-B2D7-58EBFBEC7CBA}");

		//! Acquires one read ownership of the lock.
		//! @details This operation blocks the current thread until the ownership is acquired.
		virtual void acquire_read() = 0;
		//! Acquires one write ownership of the lock.
		//! @details This operation blocks the current thread until the ownership is acquired.
		virtual void acquire_write() = 0;
		//! Tries to acquire one read ownership of the lock.
		//! @details This operation does not block the current thread, it returns immediately no matter whether the ownership is 
		//! acquired.
		//! @return Returns `true` if the ownership is acquired when this function returns. Returns `false` otherwise.
		virtual bool try_acquire_read() = 0;
		//! Tries to acquire one write ownership of the lock.
		//! @details This operation does not block the current thread, it returns immediately no matter whether the ownership is 
		//! acquired.
		//! @return Returns `true` if the ownership is acquired when this function returns. Returns `false` otherwise.
		virtual bool try_acquire_write() = 0;
		//! Releases the read ownership acquired by @ref acquire_read or @ref try_acquire_read.
		virtual void release_read() = 0;
		//! Releases the write ownership acquired by @ref acquire_write or @ref try_acquire_write.
		virtual void release_write() = 0;
	};

	//! Creates one new read write lock.
	//! @return Returns the created read write lock.
	LUNA_RUNTIME_API Ref<IReadWriteLock> new_read_write_lock();

	//! @}
}