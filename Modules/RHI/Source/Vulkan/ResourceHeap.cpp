/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResourceHeap.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "ResourceHeap.hpp"

namespace Luna
{
	namespace RHI
	{
		RV ResourceHeap::init(const ResourceHeapDesc& desc)
		{
			VmaAllocationCreateInfo info{};
			switch (desc.type)
			{
			case ResourceHeapType::local:
				info.usage = VMA_MEMORY_USAGE_AUTO;
				info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
				info.priority = 1.0f;
				break;
			case ResourceHeapType::upload:
				info.usage = VMA_MEMORY_USAGE_AUTO;
				info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
				break;
			case ResourceHeapType::readback:
				info.usage = VMA_MEMORY_USAGE_AUTO;
				info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
				break;
			}
			VkMemoryRequirements memory_requirements;
			memory_requirements.size = desc.size;
			memory_requirements.alignment = 0;
			vmaAllocateMemory(m_device->m_allocator, &memory_requirements, &info, &m_allocation, &m_allocation_info);

		}
		ResourceHeap::~ResourceHeap()
		{
			if (m_allocation != VK_NULL_HANDLE)
			{
				vmaFreeMemory(m_device->m_allocator, m_allocation);
				m_allocation = VK_NULL_HANDLE;
			}
		}
	}
}