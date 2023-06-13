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
#include "../ReadWriteLock.hpp"
#include "OS.hpp"

namespace Luna
{
	struct ReadWriteLock : IReadWriteLock
	{
		lustruct("ReadWriteLock", "{CF24C77D-6022-4777-9D68-A454DA2E209F}");
		luiimpl();

		opaque_t m_handle;

		ReadWriteLock()
		{
			m_handle = OS::new_read_write_lock();
		}
		~ReadWriteLock()
		{
			OS::delete_read_write_lock(m_handle);
		}
		virtual void acquire_read() override
		{
			OS::acquire_read_lock(m_handle);
		}
		virtual void acquire_write() override
		{
			OS::acquire_write_lock(m_handle);
		}
		virtual bool try_acquire_read() override
		{
			return OS::try_acquire_read_lock(m_handle);
		}
		virtual bool try_acquire_write() override
		{
			return OS::try_acquire_write_lock(m_handle);
		}
		virtual void release_read() override
		{
			OS::release_read_lock(m_handle);
		}
		virtual void release_write() override
		{
			OS::release_write_lock(m_handle);
		}
	};
}