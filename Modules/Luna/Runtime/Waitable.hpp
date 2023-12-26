/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Waitable.hpp
* @author JXMaster
* @date 2018/12/8
*/
#pragma once
#include "Interface.hpp"
namespace Luna
{
	//! @addtogroup RuntimeThread
	//! @{
	
	//! @interface IWaitable
	//! Represents a waitable object used for multi-thread synchronization. 
	//! @details Objects that implements `IWaitable` cannot be used cross process boundary.
	struct IWaitable : virtual Interface
	{	
		luiid("{3dcaabdc-f4d5-4aa6-bc30-904f7875964a}");
		
		//! Waits for this object to be signaled. 
		//! @details This will block the current thread until the wait condition is satisfied.
		virtual void wait() = 0;

		//! Tries to wait for this object to be signaled. 
		//! @details This will not suspend the current thread, if the condition is not satisfied, this call returns immediately with `false`.
		virtual bool try_wait() = 0;
	};

	//! @}
}