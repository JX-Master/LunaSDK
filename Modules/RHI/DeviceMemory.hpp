/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceMemory.hpp
* @author JXMaster
* @date 2023/5/16
*/
#pragma once
#include "DeviceChild.hpp"

namespace Luna
{
	namespace RHI
	{
		//! Represents one allocated device memory.
		struct IDeviceMemory : virtual IDeviceChild
		{
			luiid("{066D9159-5E46-4967-A92C-752C1530308E}");

			//! Gets the size of the memory in bytes.
			virtual u64 get_size() = 0;
		};
	}
}