/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.hpp
* @author JXMaster
* @date 2018/12/22
* @brief Windows implementation of Sync System.
*/
#pragma once
#include "../Mutex.hpp"
#include "OS.hpp"
namespace Luna
{
	struct Mutex : IMutex
	{
		lustruct("Mutex", "{0df3d468-0d98-4aee-b11d-905ad291def2}");
		luiimpl();

		opaque_t m_handle;

		Mutex()
		{
			m_handle = OS::new_mutex();
		}
		~Mutex()
		{
			OS::delete_mutex(m_handle);
		}
		void wait()
		{
			OS::lock_mutex(m_handle);
		}
		bool try_wait()
		{
			return OS::try_lock_mutex(m_handle);
		}
		void unlock()
		{
			OS::unlock_mutex(m_handle);
		}
	};
}
