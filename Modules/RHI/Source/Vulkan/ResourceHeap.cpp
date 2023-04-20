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
			VkMemoryAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			info.allocationSize = desc.size;
			
		}
		ResourceHeap::~ResourceHeap()
		{
			if (m_memory != VK_NULL_HANDLE)
			{
				vkFreeMemory(m_device->m_device, m_memory, nullptr);
				m_memory = VK_NULL_HANDLE;
			}
		}
	}
}