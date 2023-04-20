/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceMemory.hpp
* @author JXMaster
* @date 2023/4/20
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DeviceMemory
		{
			lustruct("RHI::DeviceMemory", "{F99F86B6-3339-4C28-A82A-13B63ADAFBBC}");

			Ref<Device> m_device;
			VmaAllocation m_allocation = VK_NULL_HANDLE;
			VmaAllocationInfo m_allocation_info;
			~DeviceMemory();
		};

		R<Ref<DeviceMemory>> allocate_device_memory(Device* device, 
			const VkMemoryRequirements& pVkMemoryRequirements, 
			const VmaAllocationCreateInfo& pCreateInfo);
	}
}