/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ReadWriteLock.cpp
* @author JXMaster
* @date 2022/8/29
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
	namespace OS
	{
		opaque_t new_read_write_lock()
		{
			SRWLOCK* srw = (SRWLOCK*)Luna::memalloc(sizeof(SRWLOCK), alignof(SRWLOCK));
			InitializeSRWLock(srw);
			return srw;
		}
		void delete_read_write_lock(opaque_t lock)
		{
			Luna::memfree(lock, alignof(SRWLOCK));
		}
		void acquire_read_lock(opaque_t lock)
		{
			AcquireSRWLockShared((SRWLOCK*)lock);
		}
		void acquire_write_lock(opaque_t lock)
		{
			AcquireSRWLockExclusive((SRWLOCK*)lock);
		}
		bool try_acquire_read_lock(opaque_t lock)
		{
			return TryAcquireSRWLockShared((SRWLOCK*)lock);
		}
		bool try_acquire_write_lock(opaque_t lock)
		{
			return TryAcquireSRWLockExclusive((SRWLOCK*)lock);
		}
		void release_read_lock(opaque_t lock)
		{
			ReleaseSRWLockShared((SRWLOCK*)lock);
		}
		void release_write_lock(opaque_t lock)
		{
			ReleaseSRWLockExclusive((SRWLOCK*)lock);
		}
	}
}