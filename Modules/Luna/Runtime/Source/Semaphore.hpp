/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.hpp
* @author JXMaster
* @date 2018/12/22
* @brief Windows implementation of Sync System.
*/
#pragma once
#include "../Semaphore.hpp"
#include "OS.hpp"

namespace Luna
{
	struct Semaphore : ISemaphore
	{
		lustruct("Semaphore", "{4d155da3-acdb-4ac6-aecb-70e43a5faedf}");
		luiimpl();

		opaque_t m_handle;

		Semaphore(i32 initial_count, i32 max_count)
		{
			m_handle = OS::new_semaphore(initial_count, max_count);
		}
		~Semaphore()
		{
			OS::delete_semaphore(m_handle);
		}
		virtual void wait() override
		{
			OS::acquire_semaphore(m_handle);
		}
		virtual bool try_wait() override
		{
			return OS::try_acquire_semaphore(m_handle);
		}
		virtual void release() override
		{
			OS::release_semaphore(m_handle);
		}
	};
}
