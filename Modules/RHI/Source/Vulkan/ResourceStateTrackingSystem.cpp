/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResourceStateTrackingSystem.cpp
* @author JXMaster
* @date 2023/4/27
*/
#include "ResourceStateTrackingSystem.hpp"

namespace Luna
{
	namespace RHI
	{
		void ResourceStateTrackingSystem::append_buffer_transition(BufferResource* res, ResourceStateFlag before, ResourceStateFlag after)
		{
			// Early out for unnecessary calls.
			if (before == after)
			{
				return;
			}
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = encode_access_flags(before);
			barrier.dstAccessMask = encode_access_flags(after);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = res->m_buffer;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			m_buffer_barriers.push_back(barrier);
			m_src_state_flags |= before;
			m_dest_state_flags |= after;
		}
		void ResourceStateTrackingSystem::append_image_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after)
		{
			// Early out for unnecessary calls.
			if (before == after)
			{
				return;
			}
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = encode_access_flags(before);
			barrier.dstAccessMask = encode_access_flags(after);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = encode_image_layout(before);
			barrier.newLayout = encode_image_layout(after);
			barrier.image = res->m_image;
			if (res->m_desc.pixel_format == Format::d16_unorm || res->m_desc.pixel_format == Format::d32_float)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (res->m_desc.pixel_format == Format::d24_unorm_s8_uint || res->m_desc.pixel_format == Format::d32_float_s8_uint_x24)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			barrier.subresourceRange.baseMipLevel = subresource.mip_slice;
			barrier.subresourceRange.baseArrayLayer = subresource.array_slice;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;
			m_image_barriers.push_back(barrier);
			m_src_state_flags |= before;
			m_dest_state_flags |= after;
		}
		void ResourceStateTrackingSystem::append_image_resolve_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = encode_access_flags(after);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = encode_image_layout(before);
			barrier.newLayout = encode_image_layout(after);
			barrier.image = res->m_image;
			if (res->m_desc.pixel_format == Format::d16_unorm || res->m_desc.pixel_format == Format::d32_float)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (res->m_desc.pixel_format == Format::d24_unorm_s8_uint || res->m_desc.pixel_format == Format::d32_float_s8_uint_x24)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			barrier.subresourceRange.baseMipLevel = subresource.mip_slice;
			barrier.subresourceRange.baseArrayLayer = subresource.array_slice;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;
			m_image_barriers.push_back(barrier);
			m_dest_state_flags |= after;
		}
		void ResourceStateTrackingSystem::append_image_finish_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = encode_access_flags(before);
			barrier.dstAccessMask = 0;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = encode_image_layout(before);
			barrier.newLayout = encode_image_layout(before);
			barrier.image = res->m_image;
			if (res->m_desc.pixel_format == Format::d16_unorm || res->m_desc.pixel_format == Format::d32_float)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (res->m_desc.pixel_format == Format::d24_unorm_s8_uint || res->m_desc.pixel_format == Format::d32_float_s8_uint_x24)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			barrier.subresourceRange.baseMipLevel = subresource.mip_slice;
			barrier.subresourceRange.baseArrayLayer = subresource.array_slice;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;
			m_image_barriers.push_back(barrier);
			m_src_state_flags |= before;
		}
		void ResourceStateTrackingSystem::append_buffer_aliasing(BufferResource* res, ResourceStateFlag after)
		{
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = encode_access_flags(after);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = res->m_buffer;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			m_buffer_barriers.push_back(barrier);
			m_dest_state_flags |= after;
		}
		void ResourceStateTrackingSystem::append_image_aliasing(ImageResource* res, ResourceStateFlag after)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = encode_access_flags(after);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = encode_image_layout(after);
			barrier.image = res->m_image;
			if (res->m_desc.pixel_format == Format::d16_unorm || res->m_desc.pixel_format == Format::d32_float)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (res->m_desc.pixel_format == Format::d24_unorm_s8_uint || res->m_desc.pixel_format == Format::d32_float_s8_uint_x24)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.levelCount = res->m_desc.mip_levels;
			barrier.subresourceRange.layerCount = res->m_desc.type == ResourceType::texture_3d ? 1 : res->m_desc.depth_or_array_size;
			m_image_barriers.push_back(barrier);
			m_dest_state_flags |= after;
		}
		void ResourceStateTrackingSystem::pack_buffer_transition(BufferResource* res, ResourceStateFlag after)
		{
			// Checks if the subresource is resolved.
			auto iter = m_current_buffer_states.find(res);
			if (iter == m_current_buffer_states.end())
			{
				// The subresource is not resolved.
				append_buffer_transition(res, ResourceStateFlag::none, after);
				m_current_buffer_states.insert(make_pair(res, after));
			}
			else
			{
				// The subresource is resolved.
				// Insert a transition always.
				append_buffer_transition(res, iter->second, after);
				iter->second = after;
			}
		}
		void ResourceStateTrackingSystem::pack_image_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag after)
		{
			if (subresource == RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				if (res->m_desc.type == ResourceType::texture_3d)
				{
					for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
					{
						SubresourceIndex index;
						index.array_slice = 0;
						index.mip_slice = mip_slice;
						pack_image_transition(res, index, after);
					}
				}
				else
				{
					for (u32 array_slice = 0; array_slice < res->m_desc.depth_or_array_size; ++array_slice)
					{
						for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
						{
							SubresourceIndex index;
							index.array_slice = array_slice;
							index.mip_slice = mip_slice;
							pack_image_transition(res, index, after);
						}
					}
				}
			}
			else
			{
				// Checks if the subresource is resolved.
				ImageResourceKey key;
				key.m_res = res;
				key.m_subres = subresource;
				auto iter = m_current_image_states.find(key);
				if (iter == m_current_image_states.end())
				{
					// The subresource is not resolved.
					m_unresolved_image_states.insert(make_pair(key, after));
					m_current_image_states.insert(make_pair(key, after));
				}
				else
				{
					// The subresource is resolved.
					// Insert a transition always.
					append_image_transition(res, subresource, iter->second, after);
					iter->second = after;
				}
			}
		}
		void ResourceStateTrackingSystem::pack_buffer_aliasing(BufferResource* res, ResourceStateFlag after)
		{
			append_buffer_aliasing(res, after);
			m_current_buffer_states.insert_or_assign(res, after);
		}
		void ResourceStateTrackingSystem::pack_image_aliasing(ImageResource* res, ResourceStateFlag after)
		{
			append_image_aliasing(res, after);
			if (res->m_desc.type == ResourceType::texture_3d)
			{
				for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
				{
					ImageResourceKey key;
					key.m_res = res;
					key.m_subres.mip_slice = mip_slice;
					key.m_subres.array_slice = 0;
					m_current_image_states.insert_or_assign(key, after);
				}
			}
			else
			{
				for (u32 array_slice = 0; array_slice < res->m_desc.depth_or_array_size; ++array_slice)
				{
					for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
					{
						ImageResourceKey key;
						key.m_res = res;
						key.m_subres.mip_slice = mip_slice;
						key.m_subres.array_slice = array_slice;
						m_current_image_states.insert_or_assign(key, after);
					}
				}
			}
		}
		void ResourceStateTrackingSystem::resolve()
		{
			begin_new_barriers_batch();
			for (auto& i : m_unresolved_image_states)
			{
				ImageResource* res = i.first.m_res;
				luassert(!res->m_states.empty());
				u32 subresource_index = calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, res->m_desc.mip_levels);
				append_image_resolve_transition(res, i.first.m_subres, res->m_states[subresource_index], i.second);
			}
		}
		void ResourceStateTrackingSystem::generate_finish_barriers()
		{
			begin_new_barriers_batch();
			for (auto& i : m_current_buffer_states)
			{
				append_buffer_transition(i.first, i.second, ResourceStateFlag::none);
			}
			for (auto& i : m_current_image_states)
			{
				append_image_finish_transition(i.first.m_res, i.first.m_subres, i.second);
			}
		}
		void ResourceStateTrackingSystem::apply(CommandQueueType type)
		{
			for (auto& i : m_current_image_states)
			{
				ImageResource* res = i.first.m_res;
				u32 subresource_index = calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, res->m_desc.mip_levels);
				res->m_states[subresource_index] = i.second;
			}
		}
	}
}