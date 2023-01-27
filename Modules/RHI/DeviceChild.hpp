/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeviceChild.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include <Runtime/Interface.hpp>

namespace Luna
{
	namespace RHI
	{
		struct IDevice;

		struct IDeviceChild : virtual Interface
		{
			luiid("{BE9F147B-9C53-4103-9E8D-1F5CEC6459BA}");

			virtual IDevice* get_device() = 0;
		};
	}
}