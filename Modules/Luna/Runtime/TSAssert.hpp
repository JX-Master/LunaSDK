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

//! @addtogroup Runtime
//! @{
//! @defgroup RuntimeTSAssert Thread safe assertion
//! @}

//! @addtogroup RuntimeTSAssert
//! @{

#ifdef LUNA_ENABLE_THREAD_SAFE_ASSERTION

//! Defines a thread safe assertion lock that marks one structure or class type that should never be accessed by multiple threads
//! without synchronization.
//! @remark Add this macro to the body of one structure or class definition, and add @ref lutsassert to member functions that should
//! be protected. For example:
//! ```
//! // The non-thread-safe type.
//! class MyType
//! {
//! 	lutsassert_lock();
//! 	i32 value; // The data that should be accessed by only one thread.
//! 	
//!		// Will crash the program if another thread is also calling functions of the same object
//!     // with lutsassert() set
//!		void set_value(i32 v)
//! 	{
//!			lutsassert();
//!			value = v;
//!		}
//!		// 
//! 	i32 get_value()
//!		{
//!			lutsassert();
//! 		return value;
//! 	}
//! };
//! ```
#define lutsassert_lock() mutable Luna::TSLock m_tsassert_lock;

//! Tests this function call for thread safety.
//! @remark See remarks of @ref lutsassert_lock for details.
#define lutsassert() Luna::TSGuard _tsguard(this->m_tsassert_lock);

#else

//! Defines a thread safe assertion lock that marks one structure or class type that should never be accessed by multiple threads
//! without synchronization.
//! @remark Add this macro to the body of one structure or class definition, and add @ref lutsassert to member functions that should
//! be protected. For example:
//! ```
//! // The non-thread-safe type.
//! class MyType
//! {
//! 	lutsassert_lock();
//! 	i32 value; // The data that should be accessed by only one thread.
//! 	
//!		// Will crash the program if another thread is also calling functions of the same object
//!     // with lutsassert() set
//!		void set_value(i32 v)
//! 	{
//!			lutsassert();
//!			value = v;
//!		}
//!		// 
//! 	i32 get_value()
//!		{
//!			lutsassert();
//! 		return value;
//! 	}
//! };
//! ```
#define lutsassert_lock() 

//! Tests this function call for thread safety.
//! @remark See remarks of @ref lutsassert_lock for details.
#define lutsassert()

#endif

//! @}