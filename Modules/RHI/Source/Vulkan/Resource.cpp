/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Resource.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		RV BufferResource::post_init()
		{
			lutry
			{
				luexp(encode_vk_result(vmaBindBufferMemory(m_device->m_allocator, m_memory->m_allocation, m_buffer)));
			}
			lucatchret;
			return ok;
		}
		RV BufferResource::init_as_committed(const ResourceDesc& desc)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				luexp(create_vk_buffer(m_device->m_device, m_desc, &m_buffer));
				VkMemoryRequirements memory_requirements;
				vkGetBufferMemoryRequirements(m_device->m_device, m_buffer, &memory_requirements);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc.heap_type);
				luset(m_memory, allocate_device_memory(m_device, memory_requirements, allocation));
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		RV BufferResource::init_as_aliasing(const ResourceDesc& desc, DeviceMemory* memory)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				luexp(create_vk_buffer(m_device->m_device, m_desc, &m_buffer));
				m_memory = memory;
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		BufferResource::~BufferResource()
		{
			if (m_buffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(m_device->m_device, m_buffer, nullptr);
				m_buffer = VK_NULL_HANDLE;
			}
		}
		RV ImageResource::post_init()
		{
			lutry
			{
				luexp(encode_vk_result(vmaBindImageMemory(m_device->m_allocator, m_memory->m_allocation, m_image)));
			}
			lucatchret;
			return ok;
		}
		RV ImageResource::init_as_committed(const ResourceDesc& desc)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				luexp(create_vk_image(m_device->m_device, m_desc, &m_image));
				VkMemoryRequirements memory_requirements;
				vkGetImageMemoryRequirements(m_device->m_device, m_image, &memory_requirements);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc.heap_type);
				luset(m_memory, allocate_device_memory(m_device, memory_requirements, allocation));
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		RV ImageResource::init_as_aliasing(const ResourceDesc& desc, DeviceMemory* memory)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				luexp(create_vk_image(m_device->m_device, m_desc, &m_image));
				m_memory = memory;
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		ImageResource::~ImageResource()
		{
			if (m_image != VK_NULL_HANDLE)
			{
				vkDestroyImage(m_device->m_device, m_image, nullptr);
				m_image = VK_NULL_HANDLE;
			}
		}
	}
}