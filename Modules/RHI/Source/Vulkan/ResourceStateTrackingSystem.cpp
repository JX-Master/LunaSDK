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
		void ResourceStateTrackingSystem::append_buffer(BufferResource* res, VkAccessFlags before, VkAccessFlags after)
		{
			// Early out for unnecessary calls.
			if (before == after)
			{
				return;
			}
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = before;
			barrier.dstAccessMask = after;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = res->m_buffer;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
		}
		void ResourceStateTrackingSystem::append_image(ImageResource* res, const SubresourceIndex& subresource, const ImageState& before, const ImageState& after)
		{
			// Early out for unnecessary calls.
			if (before == after)
			{
				return;
			}
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = before.access_flags;
			barrier.dstAccessMask = after.access_flags;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = before.image_layout;
			barrier.newLayout = after.image_layout;
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
		}
		void ResourceStateTrackingSystem::pack_buffer(BufferResource* res, ResourceStateFlag before, ResourceStateFlag after)
		{
			auto iter = m_current_buffer_states.find(res);
			VkAccessFlags after_flags = encode_access_flags(after);
			if (iter == m_current_buffer_states.end())
			{
				// The resource is used for the first time.
				VkAccessFlags before_flags = 0;
				if (test_flags(before, ResourceStateFlag::automatic))
				{
					before_flags = encode_access_flags(before);
				}
				append_buffer(res, before_flags, after_flags);
				m_current_buffer_states.insert(make_pair(res, after));
				m_dest_stage_flags |= determine_pipeline_stage_flags(after, m_queue_type);
			}
			else
			{
				ResourceStateFlag before_state = before;
				if (before_state == ResourceStateFlag::automatic) before_state = iter->second;
				append_buffer(res, encode_access_flags(before_state), after_flags);
				iter->second = after;
				m_src_stage_flags |= determine_pipeline_stage_flags(before_state, m_queue_type);
				m_dest_stage_flags |= determine_pipeline_stage_flags(after, m_queue_type);
			}
		}
		void ResourceStateTrackingSystem::pack_image(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after)
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
						pack_image(res, index, before, after);
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
							pack_image(res, index, before, after);
						}
					}
				}
			}
			else
			{
				ImageResourceKey key;
				key.m_res = res;
				key.m_subres = subresource;
				auto iter = m_current_image_states.find(key);
				if (iter == m_current_image_states.end())
				{
					// The subresource is used for the first time.
					if(test_flags(before, ResourceStateFlag::automatic))
					{
						// defer the resolve of this barrier to execution time.
						m_unresolved_image_states.insert(make_pair(key, after));
					}
					else
					{
						ImageState before_state;
						before_state.access_flags = encode_access_flags(before);
						before_state.image_layout = encode_image_layout(before);
						ImageState after_state;
						after_state.access_flags = encode_access_flags(after);
						after_state.image_layout = encode_image_layout(after);
						append_image(res, subresource, before_state, after_state);
						m_src_stage_flags |= determine_pipeline_stage_flags(before, m_queue_type);
						m_dest_stage_flags |= determine_pipeline_stage_flags(after, m_queue_type);
					}
					m_current_image_states.insert(make_pair(key, after));
				}
				else
				{
					// The subresource is resolved.
					// Insert a transition always.
					ResourceStateFlag before_resolved;
					if (test_flags(before, ResourceStateFlag::automatic))
					{
						before_resolved = iter->second;
					}
					else
					{
						before_resolved = before;
					}
					ImageState before_state;
					before_state.access_flags = encode_access_flags(before_resolved);
					before_state.image_layout = encode_image_layout(before_resolved);
					ImageState after_state;
					after_state.access_flags = encode_access_flags(after);
					after_state.image_layout = encode_image_layout(after);
					append_image(res, subresource, before_state, after_state);
					m_src_stage_flags |= determine_pipeline_stage_flags(before_resolved, m_queue_type);
					m_dest_stage_flags |= determine_pipeline_stage_flags(after, m_queue_type);
					iter->second = after;
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
				ImageState before;
				u32 subresource_index = calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, res->m_desc.mip_levels);
				before.access_flags = 0;
				before.image_layout = res->m_image_layouts[subresource_index];
				ImageState after;
				after.access_flags = encode_access_flags(i.second);
				after.image_layout = encode_image_layout(i.second);
				append_image(res, i.first.m_subres, before, after);
				m_dest_stage_flags |= determine_pipeline_stage_flags(i.second, m_queue_type);
			}
		}
		void ResourceStateTrackingSystem::generate_finish_barriers()
		{
			begin_new_barriers_batch();
			for (auto& i : m_current_buffer_states)
			{
				append_buffer(i.first, encode_access_flags(i.second), 0);
				m_src_stage_flags |= determine_pipeline_stage_flags(i.second, m_queue_type);
			}
			for (auto& i : m_current_image_states)
			{
				ImageState before;
				before.access_flags = encode_access_flags(i.second);
				before.image_layout = encode_image_layout(i.second);
				ImageState after;
				after.access_flags = 0;
				after.image_layout = encode_image_layout(i.second);
				append_image(i.first.m_res, i.first.m_subres, before, after);
				m_src_stage_flags |= determine_pipeline_stage_flags(i.second, m_queue_type);
			}
			m_dest_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		void ResourceStateTrackingSystem::apply()
		{
			for (auto& i : m_current_image_states)
			{
				ImageResource* res = i.first.m_res;
				u32 subresource_index = calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, res->m_desc.mip_levels);
				res->m_image_layouts[subresource_index] = encode_image_layout(i.second);
			}
		}
	}
}