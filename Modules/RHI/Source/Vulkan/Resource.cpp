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
		RV BufferResource::init_as_committed(const BufferDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				luset(m_buffer, m_device->create_vk_buffer(m_desc));
				VkMemoryRequirements memory_requirements;
				m_device->m_funcs.vkGetBufferMemoryRequirements(m_device->m_device, m_buffer, &memory_requirements);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc.heap_type);
				luset(m_memory, allocate_device_memory(m_device, memory_requirements, allocation));
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		RV BufferResource::init_as_aliasing(const BufferDesc& desc, DeviceMemory* memory)
		{
			lutry
			{
				m_desc = desc;
				luset(m_buffer, m_device->create_vk_buffer(m_desc));
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
				m_device->m_funcs.vkDestroyBuffer(m_device->m_device, m_buffer, nullptr);
				m_buffer = VK_NULL_HANDLE;
			}
		}
		R<void*> BufferResource::map(usize read_begin, usize read_end)
		{
			void* ret = nullptr;
			lutry
			{
				luexp(encode_vk_result(vmaMapMemory(m_device->m_allocator, m_memory->m_allocation, &ret)));
			}
			lucatchret;
			return ret;
		}
		void BufferResource::unmap(usize write_begin, usize write_end)
		{
			vmaUnmapMemory(m_device->m_allocator, m_memory->m_allocation);
		}
		RV ImageResource::post_init()
		{
			lutry
			{
				luexp(encode_vk_result(vmaBindImageMemory(m_device->m_allocator, m_memory->m_allocation, m_image)));
				u32 num_subresources = m_desc.mip_levels * m_desc.array_size;
				m_image_layouts.resize(num_subresources, VK_IMAGE_LAYOUT_UNDEFINED);
			}
			lucatchret;
			return ok;
		}
		RV ImageResource::init_as_committed(const TextureDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				luset(m_image, m_device->create_vk_image(m_desc));
				VkMemoryRequirements memory_requirements;
				m_device->m_funcs.vkGetImageMemoryRequirements(m_device->m_device, m_image, &memory_requirements);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc.heap_type);
				luset(m_memory, allocate_device_memory(m_device, memory_requirements, allocation));
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		RV ImageResource::init_as_aliasing(const TextureDesc& desc, DeviceMemory* memory)
		{
			lutry
			{
				m_desc = desc;
				luset(m_image, m_device->create_vk_image(m_desc));
				m_memory = memory;
				luexp(post_init());
			}
			lucatchret;
			return ok;
		}
		ImageResource::~ImageResource()
		{
			if (m_image != VK_NULL_HANDLE && m_memory)
			{
				m_device->m_funcs.vkDestroyImage(m_device->m_device, m_image, nullptr);
				m_image = VK_NULL_HANDLE;
			}
		}
	}
}