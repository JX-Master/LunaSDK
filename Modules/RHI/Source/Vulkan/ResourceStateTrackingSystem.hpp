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

		class ResourceStateTrackingSystem
		{
		public:

			u32 m_queue_family_index;

			//! Tables for unresolved resources. Unlike most implementations in other library, because
			//! we don't know when the list will be submitted to the queue, we defer the resolving of this 
			//! to the time when the list is actually submitted.
			HashMap<ImageResourceKey, ResourceStateFlag> m_unresolved_image_states;

			//! Tables for the current state of resources.
			HashMap<BufferResource*, ResourceStateFlag> m_current_buffer_states;
			HashMap<ImageResourceKey, ResourceStateFlag> m_current_image_states;

			Vector<VkBufferMemoryBarrier> m_buffer_barriers;
			Vector<VkImageMemoryBarrier> m_image_barriers;
			ResourceStateFlag m_src_state_flags = ResourceStateFlag::none;
			ResourceStateFlag m_dest_state_flags = ResourceStateFlag::none;

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
				m_src_state_flags = ResourceStateFlag::none;
				m_dest_state_flags = ResourceStateFlag::none;
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
			void append_buffer_transition(BufferResource* res, ResourceStateFlag before, ResourceStateFlag after);
			void append_image_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after);
			void append_buffer_aliasing(BufferResource* res, ResourceStateFlag after);
			void append_image_aliasing(ImageResource* res, ResourceStateFlag after);
			void append_image_resolve_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before, ResourceStateFlag after);
			void append_image_finish_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag before);
		public:

			//! Appends one barrier that transits the specified subresources' state to after
			//! state, and records the change into the tracking system.
			void pack_buffer_transition(BufferResource* res, ResourceStateFlag after);
			void pack_image_transition(ImageResource* res, const SubresourceIndex& subresource, ResourceStateFlag after);

			//! The aliasing barrier will discard the old data of the resource.
			void pack_buffer_aliasing(BufferResource* res, ResourceStateFlag after);
			void pack_image_aliasing(ImageResource* res, ResourceStateFlag after);

			//! Appends any barrier.
			void pack_barrier(const ResourceBarrierDesc& desc)
			{
				switch (desc.type)
				{
				case ResourceBarrierType::transition:
				{
					IResource* res = desc.transition.resource;
					auto d = res->get_desc();
					if (d.type == ResourceType::buffer)
					{
						BufferResource* r = cast_objct<BufferResource>(res->get_object());
						pack_buffer_transition(r, desc.transition.after);
					}
					else
					{
						ImageResource* r = cast_objct<ImageResource>(res->get_object());
						pack_image_transition(r, desc.transition.subresource, desc.transition.after);
					}
					break;
				}
				case ResourceBarrierType::aliasing:
				{
					auto d = desc.aliasing.resource->get_desc();
					if (d.type == ResourceType::buffer)
					{
						BufferResource* res = cast_objct<BufferResource>(desc.aliasing.resource->get_object());
						pack_buffer_aliasing(res, desc.aliasing.after);
					}
					else
					{
						ImageResource* res = cast_objct<ImageResource>(desc.aliasing.resource->get_object());
						pack_image_aliasing(res, desc.aliasing.after);
					}
					break;
				}
				case ResourceBarrierType::uav:
				{
					auto d = desc.uav.resource->get_desc();
					if (d.type == ResourceType::buffer)
					{
						BufferResource* res = cast_objct<BufferResource>(desc.uav.resource->get_object());
						VkBufferMemoryBarrier barrier{};
						barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
						barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
						barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
						barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						barrier.buffer = res->m_buffer;
						barrier.offset = 0;
						barrier.size = VK_WHOLE_SIZE;
						m_buffer_barriers.push_back(barrier);
					}
					else
					{
						ImageResource* res = cast_objct<ImageResource>(desc.uav.resource->get_object());
						VkImageMemoryBarrier barrier{};
						barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
						barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
						barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
						barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
						barrier.image = res->m_image;
						barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						barrier.subresourceRange.baseMipLevel = 0;
						barrier.subresourceRange.baseArrayLayer = 0;
						barrier.subresourceRange.levelCount = d.mip_levels;
						barrier.subresourceRange.layerCount = d.type == ResourceType::texture_3d ? 1 : d.depth_or_array_size;
						m_image_barriers.push_back(barrier);
					}
					m_src_state_flags |= ResourceStateFlag::unordered_access_write;
					m_dest_state_flags |= ResourceStateFlag::unordered_access_read | ResourceStateFlag::unordered_access_write;
					break;
				}
				default:
					lupanic();
					break;
				}
			}

			//! Resolves all unresolved transitions into m_transitions based on their current state.
			void resolve();

			//! Generates barriers that should be inserted at the end of the command buffer.
			void generate_finish_barriers();

			//! Applies all after state back to the resource global state.
			void apply(CommandQueueType type);
		};
	}
}