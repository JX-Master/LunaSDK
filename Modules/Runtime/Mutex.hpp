/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.hpp
* @author JXMaster
* @date 2019/3/14
* @brief IMutex interface represents a recursive mutex object.
*/
#pragma once
#include "Waitable.hpp"
#include "Ref.hpp"

namespace Luna
{
	//! @interface IMutex
	//! @threadsafe
	//! The mutex or critical section is an important object that provides
	//! synchronization functionality for multiple threads that wants to 
	//! access the same resource without data race. The mutex can be "owned"
	//! by at most one thread, when other threads wants to acquire the mutex,
	//! it must wait until the thread that currently owns the mutex to release
	//! the mutex. The threads that waits on the mutex will probably be suspended
	//! by system.
	//! 
	//! The mutex can be acquired recursively, that is, the thread that already owns
	//! the mutex can make additional calls to acquire the mutex, so long as it makes 
	//! one release call for each acquire call to finally release the mutex.
	//! 
	//! Acquiring the mutex from one thread and releasing from another thread is not allowed.
	struct IMutex : virtual IWaitable
	{
		luiid("{eff5c37a-8994-4136-a841-3f494a75385e}");

		virtual void unlock() = 0;
	};

	//! Create a new mutex object.
	//! @return Returns the new created mutex object.
	LUNA_RUNTIME_API Ref<IMutex> new_mutex();

	//! RAII wrapper for mutex.
	class MutexGuard
	{
		Ref<IMutex> m_mtx;
	public:
		MutexGuard() :
			m_mtx(nullptr) {}
		MutexGuard(IMutex* mtx) :
			m_mtx(mtx)
		{
			m_mtx->wait();
		}
		bool locked()
		{
			return m_mtx ? true : false;
		}
		void lock(IMutex* mtx)
		{
			unlock();
			m_mtx = mtx;
			m_mtx->wait();
		}
		void unlock()
		{
			if (m_mtx)
			{
				m_mtx->unlock();
				m_mtx = nullptr;
			}
		}
		MutexGuard(const MutexGuard&) = delete;
		MutexGuard(MutexGuard&& rhs)
		{
			m_mtx = rhs.m_mtx;
			rhs.m_mtx = nullptr;
		}
		MutexGuard& operator=(const MutexGuard&) = delete;
		MutexGuard& operator=(MutexGuard&& rhs)
		{
			unlock();
			m_mtx = rhs.m_mtx;
			rhs.m_mtx = nullptr;
			return *this;
		}
		~MutexGuard()
		{
			unlock();
		}
	};
}