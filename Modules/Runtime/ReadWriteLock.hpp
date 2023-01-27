/*
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
	//! @interface IReadWriteLock
	//! @threadsafe
	//! Represents one system-level read write lock.
	struct IReadWriteLock : virtual Interface
	{
		luiid("{F74B6F8A-AF52-4ABA-B2D7-58EBFBEC7CBA}");

		virtual void acquire_read() = 0;
		virtual void acquire_write() = 0;
		virtual bool try_acquire_read() = 0;
		virtual bool try_acquire_write() = 0;
		virtual void release_read() = 0;
		virtual void release_write() = 0;
	};

	LUNA_RUNTIME_API Ref<IReadWriteLock> new_read_write_lock();
}