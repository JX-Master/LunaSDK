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
		};

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

		class ResourceStateTrackingSystem
		{
		public:

			CommandQueueType m_queue_type = CommandQueueType::graphics;

			//! Tables for unresolved resources. Unlike most implementations in other library, because
			//! we don't know when the list will be submitted to the queue, we defer the resolving of this 
			//! to the time when the list is actually submitted.
			HashMap<ImageResourceKey, ResourceStateFlag> m_unresolved_image_states;

			//! Tables for the current state of resources.
			HashMap<BufferResource*, ResourceStateFlag> m_current_buffer_states;
			HashMap<ImageResourceKey, ResourceStateFlag> m_current_image_states;

			Vector<VkBufferMemoryBarrier> m_buffer_barriers;
			Vector<VkImageMemoryBarrier> m_image_barriers;
			VkPipelineStageFlags m_src_stage_flags = 0;
			VkPipelineStageFlags m_dest_stage_flags = 0;

			ResourceStateTrackingSystem() {}

			void reset()
			{
				m_unresolved_image_states.clear();
				m_current_buffer_states.clear();
				m_current_image_states.clear();
			}

			void begin_new_barriers_batch()
			{
				m_buffer_barriers.clear();
				m_image_barriers.clear();
				m_src_stage_flags = 0;
				m_dest_stage_flags = 0;
			}

			R<ResourceStateFlag> get_buffer_state(BufferResource* res) const
			{
				auto iter = m_current_buffer_states.find(res);
				if (iter == m_current_buffer_states.end())
				{
					return BasicError::not_found();
				}
				return iter->second;
			}

			R<ResourceStateFlag> get_image_state(ImageResource* res, const SubresourceIndex& subresource) const
			{
				ImageResourceKey k;
				k.m_res = res;
				k.m_subres = subresource;
				auto iter = m_current_image_states.find(k);
				if (iter == m_current_image_states.end())
				{
					return BasicError::not_found();
				}
				return iter->second;
			}

		private:
			void append_buffer(BufferResource* res, VkAccessFlags before, VkAccessFlags after);
			void append_image(ImageResource* res, const SubresourceIndex& subresource, const ImageState& before, const ImageState& after);
		public:

			//! Appends one barrier that transits the specified subresources' state to after
			//! state, and records the change into the tracking system.
			void pack_buffer(BufferResource* res, ResourceStateFlag before, ResourceStateFlag after);
			void pack_image(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after);

			//! Appends any barrier.
			void pack_barrier(const ResourceBarrierDesc& desc)
			{
				auto d = desc.resource->get_desc();
				if (d.type == ResourceType::buffer)
				{
					BufferResource* res = cast_objct<BufferResource>(desc.resource->get_object());
					pack_buffer(res, desc.before, desc.after);
				}
				else
				{
					ImageResource* res = cast_objct<ImageResource>(desc.resource->get_object());
					pack_image(res, desc.subresource, desc.before, desc.after);
				}
			}

			//! Resolves all unresolved transitions into m_transitions based on their current state.
			void resolve();

			//! Generates barriers that should be inserted at the end of the command buffer.
			void generate_finish_barriers();

			//! Applies all after state back to the resource global state.
			void apply();
		};
	}
}