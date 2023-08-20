/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceChild.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>

namespace Luna
{
	namespace RHI
	{
		struct IDevice;

		struct IDeviceChild : virtual Interface
		{
			luiid("{BE9F147B-9C53-4103-9E8D-1F5CEC6459BA}");

			//! Gets the device that creates this object.
			virtual IDevice* get_device() = 0;

			//! Sets the name of the device object. This name is for use in debug diagnostics and tools.
			virtual void set_name(const c8* name) = 0;
		};
	}
}
