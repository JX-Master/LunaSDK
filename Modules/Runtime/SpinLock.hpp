/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SpinLock.hpp
* @author JXMaster
* @date 2020/8/21
*/
#pragma once
#include "Base.hpp"
#include "Atomic.hpp"
#include "Thread.hpp"

namespace Luna
{
	//! @class SpinLock
	//! A spin lock is like a light-weight mutex. Both mutex and spin lock are used to give one thread exclusive access to some 
	//! specific resource, but they have the following differences:
	//! 1. The spin lock is implemented purely in user-mode by C++, while the mutex is implemented by the underlying platform/OS and 
	//! is usually implemented in kernel-mode as an OS component, which means locking and releasing one spin lock is much faster than locking
	//! and releasing one mutex, since the later is usually performed through a system call.
	//! 2. The spin lock will never suspend one thread, nor will it yield the time slice of the waiting thread. If one spin lock is already
	//! locked, the waiting thread will keep checking (busy-waiting) until it obtains the lock. In the other side, the mutex will usually suspends 
	//! or yields the current thread if the mutex is already locked to let other threads use the processor. 
	//! This makes the spin lock suitable for locking the resource for a very short period of time (hundreds or thousands of CPU-cycles), but not suitable 
	//! if the lock will be obtained for a long time (>100us).
	//! 3. Creating one spin lock creation consumes much less memory than creating one mutex (only 4 bytes for non-recursive spin lock). 
	//! Meanwhile, creating one spin lock does not need to allocate any dynamic memory, which makes it suitable for embedding into other objects.
	class SpinLock
	{
		volatile u32 counter;
	public:
		SpinLock() :
			counter(0) {}
		SpinLock(const SpinLock&) = delete;
		SpinLock(SpinLock&& rhs) = delete;
		SpinLock& operator=(const SpinLock&) = delete;
		SpinLock& operator=(SpinLock&& rhs) = delete;
		void lock()
		{
			while (atom_compare_exchange_u32(&counter, 1, 0) != 0)
			{
#if defined(LUNA_PLATFORM_X86) || defined(LUNA_PLATFORM_X86_64)
				_mm_pause(); // busy-waiting.
#endif
			}
		}
		bool try_lock()
		{
			u32 comp = atom_compare_exchange_u32(&counter, 1, 0);
			return comp == 0;
		}
		void unlock()
		{
			atom_exchange_u32(&counter, 0);
		}
	};

	//! @class RecursiveSpinLock
	//! Similar to SpinLock, but allows the lock to be obtained mutable times from the same thread.
	//! The user should release the lock the same times as obtaining the lock to finally release the lock.
	class RecursiveSpinLock
	{
		volatile IThread* th;
		volatile u32 counter;
	public:
		RecursiveSpinLock() :
			th(nullptr),
			counter(0) {}
		RecursiveSpinLock(const RecursiveSpinLock&) = delete;
		RecursiveSpinLock(RecursiveSpinLock&& rhs) = delete;
		RecursiveSpinLock& operator=(const RecursiveSpinLock&) = delete;
		RecursiveSpinLock& operator=(RecursiveSpinLock&& rhs) = delete;
		void lock()
		{
			IThread* t = get_current_thread();
			if (th == t)
			{
				++counter;
				return;
			}
			while (atom_compare_exchange_pointer(&th, t, nullptr) != nullptr)
			{
#if defined(LUNA_PLATFORM_X86) || defined(LUNA_PLATFORM_X86_64)
				_mm_pause(); // busy-waiting.
#endif
			}
		}
		bool try_lock()
		{
			IThread* t = get_current_thread();
			if (th == t)
			{
				++counter;
				return true;
			}
			volatile IThread* comp = atom_compare_exchange_pointer(&th, t, nullptr);
			return comp == nullptr;
		}
		void unlock()
		{
			if (counter)
			{
				--counter;
			}
			else
			{
				atom_exchange_pointer(&th, nullptr);
			}
		}
	};

	//! The RAII wrapper for fetching one spin lock.
	template <typename _SpinLock>
	class LockGuard
	{
		_SpinLock* _m;
	public:
		explicit LockGuard(_SpinLock& m) :
			_m(&m)
		{
			_m->lock();
		}
		LockGuard(const LockGuard&) = delete;
		LockGuard(LockGuard&&) = delete;
		LockGuard& operator=(const LockGuard&) = delete;
		LockGuard& operator=(LockGuard&&) = delete;
		void unlock()
		{
			if (_m)
			{
				_m->unlock();
				_m = nullptr;
			}
		}
		LockGuard& operator=(_SpinLock& m)
		{
			unlock();
			_m = &m;
			_m->lock();
			return *this;
		}
		~LockGuard()
		{
			unlock();
		}

	};
}