/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Fence.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "DeviceChild.hpp"
namespace Luna
{
	namespace RHI
	{
		//! @interface IFence
		//! A fence is a synchronization primitive that can be used to insert a dependency between queue operations. 
		//! When the user submits one queue operation, she may specify multiple fences as either wait targets or signal targets. 
		//! Before the device processes the operation, it will firstly waits for all wait targets to be signaled. After the device 
		//! processes the operation, it will signal all signal targets, so that other operations waiting on these fences can be 
		//! processed.
		//! 
		//! Every fence has two states: signaled and unsignaled. The fence state is managed by the device automatically, 
		//! one fence is created in unsignaled state, every signal operation changes the fence state from unsignaled to 
		//! signaled, and every wait operation resets the fence state to unsignaled. To use fence objects properly,
		//! the user must ensure that signal operations and wait operations on the same fence should occur in discrete 1:1 pairs 
		//! (every two signal operations should have one wait operations in between, every two wait operations should one signal 
		//! operations in between).
		struct IFence : virtual IDeviceChild
		{
			luiid("{126700A9-A8CC-45FE-AC5F-C68879B8D7FD}");

		};
	}
}