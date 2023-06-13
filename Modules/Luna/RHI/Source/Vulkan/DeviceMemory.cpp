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
		RV DeviceMemory::init(MemoryType memory_type, bool allow_aliasing, const VkMemoryRequirements& pVkMemoryRequirements)
		{
			m_memory_type = memory_type;
			m_alignment = pVkMemoryRequirements.alignment;
			VmaAllocationCreateInfo allocation{};
			encode_allocation_info(allocation, memory_type, allow_aliasing);
			return encode_vk_result(vmaAllocateMemory(m_device->m_allocator, &pVkMemoryRequirements, &allocation, &m_allocation, &m_allocation_info));
		}
		DeviceMemory::~DeviceMemory()
		{
			if (m_allocation != VK_NULL_HANDLE)
			{
				vmaFreeMemory(m_device->m_allocator, m_allocation);
				m_allocation = VK_NULL_HANDLE;
			}
		}
	}
}