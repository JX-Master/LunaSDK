/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceMemory.cpp
* @author JXMaster
* @date 2023/4/20
*/
#include "DeviceMemory.hpp"

namespace Luna
{
	namespace RHI
	{
		DeviceMemory::~DeviceMemory()
		{
			if (m_allocation != VK_NULL_HANDLE)
			{
				vmaFreeMemory(m_device->m_allocator, m_allocation);
				m_allocation = VK_NULL_HANDLE;
			}
		}
		R<Ref<DeviceMemory>> allocate_device_memory(Device* device, 
			const VkMemoryRequirements& pVkMemoryRequirements, 
			const VmaAllocationCreateInfo& pCreateInfo)
		{
			auto ret = new_object<DeviceMemory>();
			lutry
			{
				ret->m_device = device;
				luexp(encode_vk_result(vmaAllocateMemory(ret->m_device->m_allocator, &pVkMemoryRequirements, &pCreateInfo, &ret->m_allocation, &ret->m_allocation_info)));
			}
			lucatchret;
			return ret;
		}
	}
}