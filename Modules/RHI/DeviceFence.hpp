/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceFence.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "DeviceChild.hpp"
namespace Luna
{
	namespace RHI
	{
		//! @interface IDeviceFence
		//! A device fence is a synchronization primitive that can be used to insert a dependency between queue operations. 
		//! When the user submits one queue operation, she may specify multiple device fences as either wait targets or signal targets. 
		//! Before the device processes the operation, it will firstly waits all wait targets to be signaled. After the device 
		//! processes the operation, it will signal all signal targets, so that other operations waiting on these device fences can be 
		//! processed.
		//! 
		//! Every device fence has two states: signaled and unsignaled. The device fence state is managed by the device automatically, 
		//! one device fence is created in unsignaled state, every signal operation changes the device fence state from unsignaled to 
		//! signaled, and every wait operation resets the device fence state to unsignaled. To use device fence objects properly,
		//! the user must ensure that signal operations and wait operations on the same device fence should occur in discrete 1:1 pairs 
		//! (every two signal operations should have one wait operations in between, every two wait operations should one signal operations in between).
		struct IDeviceFence : virtual IDeviceChild
		{
			luiid("{126700A9-A8CC-45FE-AC5F-C68879B8D7FD}");

		};
	}
}