/*!
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

#if defined(LUNA_PLATFORM_X86) || defined(LUNA_PLATFORM_X86_64)
#include <emmintrin.h>
#endif

namespace Luna
{
	//! @addtogroup RuntimeThread
	//! @{
	
	//! Provides one spin lock that can give one thread exclusive access to one resource in multi-thread environments.
	//! @details A spin lock is like a light-weight mutex. Both mutex and spin lock are used to give one thread exclusive access to some 
	//! specific resource, but they have the following differences:
	//! 1. The spin lock is implemented purely in user-mode by C++, while the mutex is implemented by the underlying platform/OS and 
	//! is usually implemented in kernel-mode as an OS component, which means locking and releasing one spin lock is much faster than locking
	//! and releasing one mutex, since the later is usually performed through a system call.
	//! 2. The spin lock will never suspend one thread, nor will it yield the time slice of the waiting thread. If one spin lock is already
	//! locked, the waiting thread will keep checking (not_ready-waiting) until it obtains the lock. In the other side, the mutex will usually suspends 
	//! or yields the current thread if the mutex is already locked to let other threads use the processor. 
	//! This makes the spin lock suitable for locking the resource for a very short period of time (hundreds or thousands of CPU-cycles), but not suitable 
	//! if the lock will be obtained for a long time (>100us).
	//! 3. Creating one spin lock creation consumes much less memory than creating one mutex (only 4 bytes for non-recursive spin lock). 
	//! Meanwhile, creating one spin lock does not need to allocate any dynamic memory, which makes it suitable for embedding into other objects.
	class SpinLock
	{
		volatile u32 counter;
	public:
		//! Constructs one spin lock. The spin lock is unlocked after creation.
		SpinLock() :
			counter(0) {}
		SpinLock(const SpinLock&) = delete;
		SpinLock(SpinLock&& rhs) = delete;
		SpinLock& operator=(const SpinLock&) = delete;
		SpinLock& operator=(SpinLock&& rhs) = delete;
		//! Locks the spin lock.
		//! @details This function blocks the current thread until the spin lock is successfully locked.
		//! Locking the same spin lock from the same thread twice causes deadlock. Use @ref RecursiveSpinLock
		//! if you need to lock the same spin lock multiple times from the same thread.
		void lock()
		{
			while (atom_compare_exchange_u32(&counter, 1, 0) != 0)
			{
#if defined(LUNA_PLATFORM_X86) || defined(LUNA_PLATFORM_X86_64)
				_mm_pause(); // not_ready-waiting.
#endif
			}
		}
		//! Tries to lock the spin lock.
		//! @return Returns `true` if the spin lock is successfully locked when the function returns. Returns 
		//! `false` otherwise.
		bool try_lock()
		{
			u32 comp = atom_compare_exchange_u32(&counter, 1, 0);
			return comp == 0;
		}
		//! Unlocks the spin lock.
		void unlock()
		{
			atom_exchange_u32(&counter, 0);
		}
	};

	//! Similar to @ref SpinLock, but allows the lock to be obtained mutable times from the same thread.
	class RecursiveSpinLock
	{
		volatile IThread* th;
		volatile u32 counter;
	public:
		//! Constructs one spin lock. The spin lock is unlocked after creation.
		RecursiveSpinLock() :
			th(nullptr),
			counter(0) {}
		RecursiveSpinLock(const RecursiveSpinLock&) = delete;
		RecursiveSpinLock(RecursiveSpinLock&& rhs) = delete;
		RecursiveSpinLock& operator=(const RecursiveSpinLock&) = delete;
		RecursiveSpinLock& operator=(RecursiveSpinLock&& rhs) = delete;
		//! Locks the spin lock.
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
				_mm_pause(); // not_ready-waiting.
#endif
			}
		}
		//! Tries to lock the spin lock.
		//! @return Returns `true` if the spin lock is successfully locked when the function returns. Returns 
		//! `false` otherwise.
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
		//! Unlocks the spin lock.
		//! @details If the lock is acquired from the same thread multiple times, the user should call this function 
		//! the same times as @ref lock to finally release the lock.
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

	//! The RAII wrapper that locks the specified lock upon construction, and unlocks the specified lock upon 
	//! destruction.
	//! @details This can be used for both @ref SpinLock and @ref RecursiveSpinLock
	template <typename _SpinLock>
	class LockGuard
	{
		_SpinLock* _m;
	public:
		//! Constructs one lock guard and acquires the specified lock.
		//! @param[in] lock The spin lock to acquire.
		explicit LockGuard(_SpinLock& lock) :
			_m(&lock)
		{
			_m->lock();
		}
		LockGuard(const LockGuard&) = delete;
		LockGuard(LockGuard&&) = delete;
		LockGuard& operator=(const LockGuard&) = delete;
		LockGuard& operator=(LockGuard&&) = delete;
		//! Releases the acquired spin lock manually.
		//! @details This function does nothing if the lock is already released.
		void unlock()
		{
			if (_m)
			{
				_m->unlock();
				_m = nullptr;
			}
		}
		//! Replaces the acquired spin lock.
		//! @details The prior lock will be released firstly if not released, then
		//! the new lock will be acquired.
		//! @param[in] lock The new spin lock to acquire.
		LockGuard& operator=(_SpinLock& lock)
		{
			unlock();
			_m = &lock;
			_m->lock();
			return *this;
		}
		~LockGuard()
		{
			unlock();
		}

	};

	//! @}
}