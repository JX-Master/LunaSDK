/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TSAssert.hpp
* @author JXMaster
* @date 2019/1/15
*/
#pragma once
#include "Assert.hpp"
#include "Atomic.hpp"
#include "Thread.hpp"
namespace Luna
{
	struct TSLock
	{
		IThread* m_owning_thread;

		TSLock() :
			m_owning_thread(nullptr) {}
	};

	struct TSGuard
	{
		IThread* m_last_thread;
		TSLock& m_lock;

		TSGuard(TSLock& lock) :
			m_lock(lock)
		{
			IThread* cur = get_current_thread();
			m_last_thread = atom_exchange_pointer((IThread* volatile*)(&(lock.m_owning_thread)), cur);
			luassert_msg_always((m_last_thread == nullptr) || (m_last_thread == cur), "Data race detected!");
		}

		~TSGuard()
		{
			atom_exchange_pointer((IThread* volatile*)(&(m_lock.m_owning_thread)), m_last_thread);
		}
	};
}

#ifdef LUNA_ENABLE_THREAD_SAFE_ASSERTION

#define lutsassert_lock() mutable Luna::TSLock m_tsassert_lock;

#define lutsassert() Luna::TSGuard _tsguard(this->m_tsassert_lock);

#else

#define lutsassert_lock() 

#define lutsassert() 

#endif
