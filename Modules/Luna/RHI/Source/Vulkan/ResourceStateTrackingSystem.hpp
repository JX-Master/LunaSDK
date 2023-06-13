/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResourceStateTrackingSystem.hpp
* @author JXMaster
* @date 2023/4/27
*/
#pragma once
#include "Device.hpp"
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ImageResourceKey
		{
			ImageResource* m_res;
			SubresourceIndex m_subres;

			bool operator==(const ImageResourceKey& rhs) const
			{
				return m_res == rhs.m_res && m_subres == rhs.m_subres;
			}
		};
	}

	template<>
	struct hash<RHI::ImageResourceKey>
	{
		usize operator()(const RHI::ImageResourceKey& value)
		{
			usize h = (usize)value.m_res;
			h ^= hash<RHI::SubresourceIndex>()(value.m_subres);
			return h;
		}
	};

	namespace RHI
	{
		inline constexpr u32 calc_subresource_state_index(u32 mip_slice, u32 array_slice, u32 mip_levels)
		{
			if (mip_slice == U32_MAX && array_slice == U32_MAX)
			{
				return U32_MAX;
			}
			return mip_slice + array_slice * mip_levels;
		}

		struct ImageState
		{
			VkAccessFlags access_flags;
			VkImageLayout image_layout;
			bool operator==(const ImageState& rhs) const
			{
				return access_flags == rhs.access_flags && image_layout == rhs.image_layout;
			}
		};

		struct QueueTransferBarriers
		{
			Vector<VkBufferMemoryBarrier> buffer_barriers;
			Vector<VkImageMemoryBarrier> image_barriers;
		};

		class ResourceStateTrackingSystem
		{
		public:

			CommandQueueType m_queue_type = CommandQueueType::graphics;
			u32 m_queue_family_index;

			//! Tables for unresolved resources. Unlike most implementations in other library, because 
			//! we don't know when the list will be submitted to the queue, we defer the resolving of this 
			//! to the time when the list is actually submitted.
			HashMap<BufferResource*, BufferBarrier> m_unresolved_buffer_states;
			HashMap<ImageResourceKey, TextureBarrier> m_unresolved_image_states;

			//! Tables for the current state of resources.
			HashMap<BufferResource*, BufferStateFlag> m_current_buffer_states;
			HashMap<ImageResourceKey, TextureStateFlag> m_current_image_states;

			Vector<VkBufferMemoryBarrier> m_buffer_barriers;
			Vector<VkImageMemoryBarrier> m_image_barriers;
			VkPipelineStageFlags m_src_stage_flags = 0;
			VkPipelineStageFlags m_dst_stage_flags = 0;
			HashMap<u32, QueueTransferBarriers> m_queue_transfer_barriers;

			ResourceStateTrackingSystem() {}

			void reset()
			{
				m_unresolved_buffer_states.clear();
				m_unresolved_image_states.clear();
				m_current_buffer_states.clear();
				m_current_image_states.clear();
			}

			void begin_new_barriers_batch()
			{
				m_buffer_barriers.clear();
				m_image_barriers.clear();
				m_src_stage_flags = 0;
				m_dst_stage_flags = 0;
				m_queue_transfer_barriers.clear();
			}

			VkImageLayout get_image_layout(ImageResource* res, const SubresourceIndex& subresource) const
			{
				ImageResourceKey k;
				k.m_res = res;
				k.m_subres = subresource;
				auto iter = m_current_image_states.find(k);
				if (iter == m_current_image_states.end())
				{
					u32 subresource_index = calc_subresource_state_index(subresource.mip_slice, subresource.array_slice, res->m_desc.mip_levels);
					return res->m_global_states[subresource_index].m_image_layout;
				}
				return encode_image_layout(iter->second);
			}

		private:
			void append_buffer(BufferResource* res, VkAccessFlags before, VkAccessFlags after, 
				u32 before_queue_family_index = VK_QUEUE_FAMILY_IGNORED, u32 after_queue_family_index = VK_QUEUE_FAMILY_IGNORED);
			void append_image(ImageResource* res, const SubresourceIndex& subresource, const ImageState& before, const ImageState& after,
				u32 before_queue_family_index = VK_QUEUE_FAMILY_IGNORED, u32 after_queue_family_index = VK_QUEUE_FAMILY_IGNORED);

			void pack_buffer_internal(BufferResource* res, const BufferBarrier& barrier, 
				VkAccessFlags recorded_src_access_flags, VkPipelineStageFlags recorded_src_pipeline_stage_flags, 
				u32 before_queue_family_index, u32 after_queue_family_index);
			void pack_image_internal(ImageResource* res, const TextureBarrier& barrier,
				const ImageState& recorded_before_state, VkPipelineStageFlags recorded_src_pipeline_stage_flags,
				u32 before_queue_family_index, u32 after_queue_family_index);
		public:

			//! Appends one barrier that transits the specified subresources' state to after
			//! state, and records the change into the tracking system.
			void pack_buffer(const BufferBarrier& barrier);
			void pack_image(const TextureBarrier& barrier);

			//! Resolves all unresolved transitions into m_transitions based on their current state.
			void resolve();

			//! Generates barriers that should be inserted at the end of the command buffer.
			void generate_finish_barriers();

			//! Applies all after state back to the resource global state.
			void apply();
		};
	}
}