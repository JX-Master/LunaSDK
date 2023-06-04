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
		void ResourceStateTrackingSystem::append_buffer(BufferResource* res, VkAccessFlags before, VkAccessFlags after,
			u32 before_queue_family_index, u32 after_queue_family_index)
		{
			if (before == 0 && after == 0 && before_queue_family_index == after_queue_family_index)
			{
				return;
			}
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = before;
			barrier.dstAccessMask = after;
			barrier.srcQueueFamilyIndex = before_queue_family_index;
			barrier.dstQueueFamilyIndex = after_queue_family_index;
			barrier.buffer = res->m_buffer;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			m_buffer_barriers.push_back(barrier);
		}
		void ResourceStateTrackingSystem::append_image(ImageResource* res, const SubresourceIndex& subresource, const ImageState& before, const ImageState& after,
			u32 before_queue_family_index, u32 after_queue_family_index)
		{
			if (before.access_flags == 0 && after.access_flags == 0
				&& before.image_layout == after.image_layout
				&& before_queue_family_index == after_queue_family_index)
			{
				return;
			}
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = before.access_flags;
			barrier.dstAccessMask = after.access_flags;
			barrier.srcQueueFamilyIndex = before_queue_family_index;
			barrier.dstQueueFamilyIndex = after_queue_family_index;
			barrier.oldLayout = before.image_layout;
			barrier.newLayout = after.image_layout;
			barrier.image = res->m_image;
			if (res->m_desc.format == Format::d16_unorm || res->m_desc.format == Format::d32_float)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (res->m_desc.format == Format::d24_unorm_s8_uint || res->m_desc.format == Format::d32_float_s8_uint_x24)
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
		}
		void ResourceStateTrackingSystem::pack_buffer_internal(BufferResource* res, const BufferBarrier& barrier, 
			VkAccessFlags recorded_src_access_flags, VkPipelineStageFlags recorded_src_pipeline_stage_flags, 
			u32 before_queue_family_index, u32 after_queue_family_index)
		{
			if (test_flags(barrier.flags, ResourceBarrierFlag::aliasing))
			{
				VkAccessFlags before_flags = 0;
				VkAccessFlags after_flags = encode_access_flags(barrier.after);
				append_buffer(res, before_flags, after_flags, before_queue_family_index, after_queue_family_index);
				m_src_stage_flags |= (barrier.before == BufferStateFlag::automatic) ?
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : determine_pipeline_stage_flags(barrier.before, m_queue_type);
				m_dst_stage_flags |= determine_pipeline_stage_flags(barrier.after, m_queue_type);
			}
			else
			{
				VkAccessFlags after_flags = encode_access_flags(barrier.after);
				VkAccessFlags before_flags;
				VkPipelineStageFlags before_stages;
				if (barrier.before == BufferStateFlag::automatic)
				{
					before_flags = recorded_src_access_flags;
					before_stages = recorded_src_pipeline_stage_flags;
				}
				else
				{
					before_flags = encode_access_flags(barrier.before);
					before_stages = determine_pipeline_stage_flags(barrier.before, m_queue_type);
				}
				if (test_flags(barrier.flags, ResourceBarrierFlag::discard_content))
				{
					before_flags = 0;
				}
				append_buffer(res, before_flags, after_flags, before_queue_family_index, after_queue_family_index);
				m_src_stage_flags |= before_stages;
				m_dst_stage_flags |= determine_pipeline_stage_flags(barrier.after, m_queue_type);
			}
		}
		void ResourceStateTrackingSystem::pack_image_internal(ImageResource* res, const TextureBarrier& barrier,
			const ImageState& recorded_before_state, VkPipelineStageFlags recorded_src_pipeline_stage_flags,
			u32 before_queue_family_index, u32 after_queue_family_index)
		{
			if (test_flags(barrier.flags, ResourceBarrierFlag::aliasing))
			{
				// Aliasing.
				ImageState before_state;
				before_state.access_flags = 0;
				before_state.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
				ImageState after_state;
				after_state.access_flags = encode_access_flags(barrier.after);
				after_state.image_layout = encode_image_layout(barrier.after);
				append_image(res, barrier.subresource, before_state, after_state, before_queue_family_index, after_queue_family_index);
				m_src_stage_flags |= (barrier.before == TextureStateFlag::automatic) ?
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : determine_pipeline_stage_flags(barrier.before, m_queue_type);
				m_dst_stage_flags |= determine_pipeline_stage_flags(barrier.after, m_queue_type);
			}
			else
			{
				ImageState after_state;
				after_state.access_flags = encode_access_flags(barrier.after);
				after_state.image_layout = encode_image_layout(barrier.after);
				ImageState before_state;
				VkPipelineStageFlags before_stages;
				if (barrier.before == TextureStateFlag::automatic)
				{
					before_state = recorded_before_state;
					before_stages = recorded_src_pipeline_stage_flags;
				}
				else
				{
					before_state.access_flags = encode_access_flags(barrier.before);
					before_state.image_layout = encode_image_layout(barrier.before);
					before_stages = determine_pipeline_stage_flags(barrier.before, m_queue_type);
				}
				if (test_flags(barrier.flags, ResourceBarrierFlag::discard_content))
				{
					before_state.access_flags = 0;
					before_state.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				append_image(res, barrier.subresource, before_state, after_state, before_queue_family_index, after_queue_family_index);
				m_src_stage_flags |= before_stages;
				m_dst_stage_flags |= determine_pipeline_stage_flags(barrier.after, m_queue_type);
			}
		}
		void ResourceStateTrackingSystem::pack_buffer(const BufferBarrier& barrier)
		{
			BufferResource* res = cast_object<BufferResource>(barrier.buffer->get_object());
			auto iter = m_current_buffer_states.find(res);
			if (iter == m_current_buffer_states.end())
			{
				// This resource is used on the current buffer for the first time.
				m_unresolved_buffer_states.insert(make_pair(res, barrier));
				m_current_buffer_states.insert(make_pair(res, barrier.after));
			}
			else
			{
				pack_buffer_internal(res, barrier, 
					encode_access_flags(iter->second), determine_pipeline_stage_flags(iter->second, m_queue_type), 
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
				iter->second = barrier.after;
			}
		}
		void ResourceStateTrackingSystem::pack_image(const TextureBarrier& barrier)
		{
			ImageResource* res = cast_object<ImageResource>(barrier.texture->get_object());
			if (barrier.subresource == TEXTURE_BARRIER_ALL_SUBRESOURCES)
			{
				TextureBarrier sub_barrier = barrier;
				for (u32 array_slice = 0; array_slice < res->m_desc.array_size; ++array_slice)
				{
					for (u32 mip_slice = 0; mip_slice < res->m_desc.mip_levels; ++mip_slice)
					{
						sub_barrier.subresource.array_slice = array_slice;
						sub_barrier.subresource.mip_slice = mip_slice;
						pack_image(sub_barrier);
					}
				}
			}
			else
			{
				ImageResourceKey key;
				key.m_res = res;
				key.m_subres = barrier.subresource;
				auto iter = m_current_image_states.find(key);
				if (iter == m_current_image_states.end())
				{
					// This resource is used on the current buffer for the first time.
					m_unresolved_image_states.insert(make_pair(key, barrier));
					m_current_image_states.insert(make_pair(key, barrier.after));
				}
				else
				{
					ImageState tracked_state;
					tracked_state.access_flags = encode_access_flags(iter->second);
					tracked_state.image_layout = encode_image_layout(iter->second);
					pack_image_internal(res, barrier, 
						tracked_state, determine_pipeline_stage_flags(iter->second, m_queue_type),
						VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
					iter->second = barrier.after;
				}
			}
		}
		void ResourceStateTrackingSystem::resolve()
		{
			begin_new_barriers_batch();
			for (auto& i : m_unresolved_buffer_states)
			{
				u32 before_queue = i.first->m_owning_queue_family_index;
				if (before_queue == U32_MAX) before_queue = m_queue_family_index;
				pack_buffer_internal(i.first, i.second, 0, 0, before_queue, m_queue_family_index);
				if (before_queue != m_queue_family_index)
				{
					// queue ownership transfer.
					auto iter = m_queue_transfer_barriers.insert(make_pair(before_queue, QueueTransferBarriers())).first;
					iter->second.buffer_barriers.push_back(m_buffer_barriers.back());
				}
			}
			for (auto& i : m_unresolved_image_states)
			{
				LockGuard guard(i.first.m_res->m_image_views_lock);
				auto global_state = i.first.m_res->m_global_states[calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, i.first.m_res->m_desc.mip_levels)];
				u32 before_queue = global_state.m_owning_queue_family_index;
				if (before_queue == U32_MAX) before_queue = m_queue_family_index;
				ImageState before_state;
				before_state.access_flags = 0;
				before_state.image_layout = global_state.m_image_layout;
				pack_image_internal(i.first.m_res, i.second, before_state, 0, before_queue, m_queue_family_index);
				if (before_queue != m_queue_family_index)
				{
					// queue ownership transfer.
					auto iter = m_queue_transfer_barriers.insert(make_pair(before_queue, QueueTransferBarriers())).first;
					iter->second.image_barriers.push_back(m_image_barriers.back());
				}
			}
			if (m_src_stage_flags == 0) m_src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			if (m_dst_stage_flags == 0) m_dst_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
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
			if (m_src_stage_flags == 0)
			{
				m_src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			}
			m_dst_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		void ResourceStateTrackingSystem::apply()
		{
			for (auto& i : m_current_buffer_states)
			{
				BufferResource* res = i.first;
				res->m_owning_queue_family_index = m_queue_family_index;
			}
			for (auto& i : m_current_image_states)
			{
				ImageResource* res = i.first.m_res;
				u32 subresource_index = calc_subresource_state_index(i.first.m_subres.mip_slice, i.first.m_subres.array_slice, res->m_desc.mip_levels);
				res->m_global_states[subresource_index].m_image_layout = encode_image_layout(i.second);
				res->m_global_states[subresource_index].m_owning_queue_family_index = m_queue_family_index;
			}
		}
	}
}