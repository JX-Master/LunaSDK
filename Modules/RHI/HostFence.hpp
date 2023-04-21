/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HostFence.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include <Runtime/Waitable.hpp>
#include "DeviceChild.hpp"
namespace Luna
{
	namespace RHI
	{
		//! @interface IHostFence
		//! A host fence is synchronization primitive that can be used to insert a dependency between a queue operation and the host.
		//! Every host fence has two states: signaled and unsignaled. When the user submits one queue operation, she may specify one 
		//! host fence target that will be signaled when the operation is finished. The user can wait for the host fence to be signald by
		//! calling `IWaitable::wait`. Unlike device fences, wait operations on host fences will not reset the signaled state. Once one 
		//! host fence is signaled, it will stay in signaled state until `IHostFence::reset` is called, thus allows multiple 
		//! wait operations to be performed on the same host fence without signaling the host fence multiple times.
		struct IHostFence : virtual IDeviceChild, virtual IWaitable
		{
			luiid("{B8578CE7-78C2-4DEE-ADB4-F0F9BA6E99D0}");

			//! Resets the host fence to unsignaled state.
			virtual void reset() = 0;
		};
	}
}