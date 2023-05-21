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
		RV BufferResource::init_as_committed(MemoryType memory_type, const BufferDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				luset(m_buffer, m_device->create_vk_buffer(m_desc));
				VkMemoryRequirements memory_requirements;
				m_device->m_funcs.vkGetBufferMemoryRequirements(m_device->m_device, m_buffer, &memory_requirements);
				auto memory = new_object<DeviceMemory>();
				memory->m_device = m_device;
				luexp(memory->init(memory_type, memory_requirements));
				m_memory = memory;
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
		bool compare_image_view_desc(const TextureViewDesc& lhs, const TextureViewDesc& rhs)
		{
			return
				lhs.texture == rhs.texture &&
				lhs.type == rhs.type &&
				lhs.format == rhs.format &&
				lhs.mip_slice == rhs.mip_slice &&
				lhs.mip_size == rhs.mip_size &&
				lhs.array_slice == rhs.array_slice &&
				lhs.array_size == rhs.array_size;
		}
		R<ImageView*> ImageResource::get_image_view(const TextureViewDesc& desc)
		{
			auto validated_desc = desc;
			validate_texture_view_desc(validated_desc);
			LockGuard guard(m_image_views_lock);
			for (auto& v : m_image_views)
			{
				if (compare_image_view_desc(v.first, validated_desc))
				{
					return v.second.get();
				}
			}
			// Create a new one.
			auto view = new_object<ImageView>();
			view->m_device = m_device;
			auto r = view->init(validated_desc);
			if (failed(r)) return r.errcode();
			m_image_views.push_back(make_pair(validated_desc, view));
			return view;
		}
		RV ImageResource::post_init()
		{
			lutry
			{
				luexp(encode_vk_result(vmaBindImageMemory(m_device->m_allocator, m_memory->m_allocation, m_image)));
				u32 num_subresources = m_desc.mip_levels * m_desc.array_size;
				m_global_states.resize(num_subresources);
			}
			lucatchret;
			return ok;
		}
		RV ImageResource::init_as_committed(MemoryType memory_type, const TextureDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				luset(m_image, m_device->create_vk_image(m_desc));
				VkMemoryRequirements memory_requirements;
				m_device->m_funcs.vkGetImageMemoryRequirements(m_device->m_device, m_image, &memory_requirements);
				auto memory = new_object<DeviceMemory>();
				memory->m_device = m_device;
				luexp(memory->init(memory_type, memory_requirements));
				m_memory = memory;
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