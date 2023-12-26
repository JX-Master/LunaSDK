/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.hpp
* @author JXMaster
* @date 2019/3/14
* @brief Semaphore interface represents a semaphore object.
*/
#pragma once
#include "Waitable.hpp"
#include "Ref.hpp"
namespace Luna
{
	//! @addtogroup RuntimeThread
	//! @{
	
	//! @interface ISemaphore
	//! Represents one system-level semaphore object.
	struct ISemaphore : virtual IWaitable
	{
		luiid("{ef6a7782-0a6c-4a40-abc9-a6d2381a3397}");

		//! Increases the counter value of the semaphore by one.
		virtual void release() = 0;
	};

	//! Create a new semaphore object.
	//! @param[in] initial_count The initial count of the semaphore.
	//! @param[in] max_count The max count the specified semaphore can reach to.
	//! @return Returns the new created semaphore object.
	LUNA_RUNTIME_API Ref<ISemaphore> new_semaphore(i32 initial_count, i32 max_count);

	//! @}
}