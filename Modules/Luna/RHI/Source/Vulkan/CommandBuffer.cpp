/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CommandBuffer.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "CommandBuffer.hpp"
#include "PipelineState.hpp"
#include "Resource.hpp"
#include "PipelineLayout.hpp"
#include "DescriptorSet.hpp"
#include "QueryHeap.hpp"
#include "Fence.hpp"
#include "Instance.hpp"
#include "../RHI.hpp"
namespace Luna
{
	namespace RHI
	{
		RV QueueTransferTracker::init(u32 queue_family_index)
		{
			lutry
			{
				VkCommandPoolCreateInfo pool_info{};
				pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.queueFamilyIndex = queue_family_index;
				luexp(encode_vk_result(m_funcs->vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool)));
				VkCommandBufferAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				alloc_info.commandPool = m_command_pool;
				alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				alloc_info.commandBufferCount = 1;
				luexp(encode_vk_result(m_funcs->vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer)));
				VkSemaphoreCreateInfo semaphore_info{};
				semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				luexp(encode_vk_result(m_funcs->vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_semaphore)));
				VkFenceCreateInfo fence_info{};
				fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			}
			lucatchret;
			return ok;
		}
		R<VkSemaphore> QueueTransferTracker::submit_barrier(VkQueue queue, IMutex* queue_mtx, Span<const VkBufferMemoryBarrier> buffer_barriers, Span<const VkImageMemoryBarrier> texture_barriers)
		{
			lutry
			{
				luexp(encode_vk_result(m_funcs->vkResetCommandPool(m_device, m_command_pool, 0)));
				VkCommandBufferBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				begin_info.pInheritanceInfo = nullptr;
				luexp(encode_vk_result(m_funcs->vkBeginCommandBuffer(m_command_buffer, &begin_info)));
				m_funcs->vkCmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
					(u32)buffer_barriers.size(), buffer_barriers.data(), (u32)texture_barriers.size(), texture_barriers.data());
				luexp(encode_vk_result(m_funcs->vkEndCommandBuffer(m_command_buffer)));
				VkSubmitInfo submit{};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.waitSemaphoreCount = 0;
				submit.pWaitSemaphores = nullptr;
				submit.pWaitDstStageMask = nullptr;
				submit.signalSemaphoreCount = 1;
				submit.pSignalSemaphores = &m_semaphore;
				submit.commandBufferCount = 1;
				submit.pCommandBuffers = &m_command_buffer;
				MutexGuard guard(queue_mtx);
				luexp(encode_vk_result(m_funcs->vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE)));
			}
			lucatchret;
			return m_semaphore;
		}
		QueueTransferTracker::~QueueTransferTracker()
		{
			if (m_command_buffer != VK_NULL_HANDLE)
			{
				m_funcs->vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
				m_command_buffer = VK_NULL_HANDLE;
			}
			if (m_command_pool != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroyCommandPool(m_device, m_command_pool, nullptr);
				m_command_pool = VK_NULL_HANDLE;
			}
			if (m_semaphore != VK_NULL_HANDLE)
			{
				m_funcs->vkDestroySemaphore(m_device, m_semaphore, nullptr);
				m_semaphore = VK_NULL_HANDLE;
			}
		}
		RV CommandBuffer::init(u32 command_queue_index)
		{
			lutry
			{
				if (command_queue_index >= m_device->m_queues.size()) return BasicError::bad_arguments();
				m_queue = m_device->m_queues[command_queue_index];
				m_queue_index = command_queue_index;
				VkCommandPoolCreateInfo pool_info{};
				pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.queueFamilyIndex = m_queue.queue_family_index;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateCommandPool(m_device->m_device, &pool_info, nullptr, &m_command_pool)));
				VkCommandBufferAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				alloc_info.commandPool = m_command_pool;
				alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				alloc_info.commandBufferCount = 2;
				VkCommandBuffer buffers[2] = { VK_NULL_HANDLE };
				luexp(encode_vk_result(m_device->m_funcs.vkAllocateCommandBuffers(m_device->m_device, &alloc_info, buffers)));
				m_resolve_buffer = buffers[0];
				m_command_buffer = buffers[1];
				VkFenceCreateInfo fence_create_info{};
				fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateFence(m_device->m_device, &fence_create_info, nullptr, &m_fence)));
				luexp(begin_command_buffer());
				m_track_system.m_queue_type = m_queue.desc.type;
				m_track_system.m_queue_family_index = m_queue.queue_family_index;
			}
			lucatchret;
			return ok;
		}
		CommandBuffer::~CommandBuffer()
		{
			for (VkFramebuffer fbo : m_fbos)
			{
				m_device->m_funcs.vkDestroyFramebuffer(m_device->m_device, fbo, nullptr);
			}
			m_fbos.clear();
			if (m_command_buffer != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkFreeCommandBuffers(m_device->m_device, m_command_pool, 1, &m_command_buffer);
				m_command_buffer = VK_NULL_HANDLE;
			}
			if (m_resolve_buffer != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkFreeCommandBuffers(m_device->m_device, m_command_pool, 1, &m_resolve_buffer);
				m_resolve_buffer = VK_NULL_HANDLE;
			}
			if (m_command_pool != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyCommandPool(m_device->m_device, m_command_pool, nullptr);
				m_command_pool = VK_NULL_HANDLE;
			}
			if (m_fence != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyFence(m_device->m_device, m_fence, nullptr);
				m_fence = VK_NULL_HANDLE;
			}
		}
		RV CommandBuffer::begin_command_buffer()
		{
			lutry
			{
				VkCommandBufferBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				begin_info.pInheritanceInfo = nullptr;
				luexp(encode_vk_result(m_device->m_funcs.vkBeginCommandBuffer(m_command_buffer, &begin_info)));
				m_recording = true;
			}
			lucatchret;
			return ok;
		}
		R<QueueTransferTracker*> CommandBuffer::get_transfer_tracker(u32 queue_family_index)
		{
			QueueTransferTracker* ret;
			lutry
			{
				auto iter = m_transfer_trackers.find(queue_family_index);
				if (iter == m_transfer_trackers.end())
				{
					UniquePtr<QueueTransferTracker> tracker(memnew<QueueTransferTracker>());
					tracker->m_device = m_device->m_device;
					tracker->m_funcs = &m_device->m_funcs;
					luexp(tracker->init(queue_family_index));
					iter = m_transfer_trackers.insert(make_pair(queue_family_index, move(tracker))).first;
				}
				ret = iter->second.get();
			}
			lucatchret;
			return ret;
		}
		void CommandBuffer::write_timestamp(IQueryHeap* heap, u32 index)
		{
			QueryHeap* h = (QueryHeap*)heap->get_object();
			m_device->m_funcs.vkCmdResetQueryPool(m_command_buffer, h->m_query_pool, index, 1);
			m_device->m_funcs.vkCmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, h->m_query_pool, index);
		}
		void CommandBuffer::begin_pipeline_statistics_query(IQueryHeap* heap, u32 index)
		{
			QueryHeap* h = (QueryHeap*)heap->get_object();
			m_device->m_funcs.vkCmdResetQueryPool(m_command_buffer, h->m_query_pool, index, 1);
			m_device->m_funcs.vkCmdBeginQuery(m_command_buffer, h->m_query_pool, index, 0);
		}
		void CommandBuffer::end_pipeline_statistics_query(IQueryHeap* heap, u32 index)
		{
			QueryHeap* h = (QueryHeap*)heap->get_object();
			m_device->m_funcs.vkCmdEndQuery(m_command_buffer, h->m_query_pool, index);
		}
		void CommandBuffer::wait()
		{
			auto r = m_device->m_funcs.vkWaitForFences(m_device->m_device, 1, &m_fence, VK_TRUE, U64_MAX);
		}
		bool CommandBuffer::try_wait()
		{
			return m_device->m_funcs.vkGetFenceStatus(m_device->m_device, m_fence) == VK_SUCCESS;
		}
		RV CommandBuffer::reset()
		{
			lutry
			{
				luexp(encode_vk_result(m_device->m_funcs.vkResetFences(m_device->m_device, 1, &m_fence)));
				if (m_recording)
				{
					// Close the command buffer.
					luexp(encode_vk_result(m_device->m_funcs.vkEndCommandBuffer(m_command_buffer)));
					m_recording = false;
				}
				luexp(encode_vk_result(m_device->m_funcs.vkResetCommandPool(m_device->m_device, m_command_pool, 0)));
				luexp(begin_command_buffer());
				m_track_system.reset();
				m_objs.clear();
				m_rt_width = 0;
				m_rt_height = 0;
				m_graphics_pipeline_layout = nullptr;
				m_compute_pipeline_layout = nullptr;
				for (VkFramebuffer fbo : m_fbos)
				{
					m_device->m_funcs.vkDestroyFramebuffer(m_device->m_device, fbo, nullptr);
				}
				m_fbos.clear();
			}
			lucatchret;
			return ok;
		}
		void CommandBuffer::attach_device_object(IDeviceChild* obj)
		{
			m_objs.push_back(obj);
		}
		void CommandBuffer::begin_event(const c8* event_name)
		{
			if (g_enable_validation_layer)
			{
				VkDebugUtilsLabelEXT markerInfo = {};
				markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				markerInfo.pLabelName = event_name;
				vkCmdBeginDebugUtilsLabelEXT(m_command_buffer, &markerInfo);
			}
		}
		void CommandBuffer::end_event()
		{
			if (g_enable_validation_layer)
			{
				vkCmdEndDebugUtilsLabelEXT(m_command_buffer);
			}
		}
		struct FramebufferDesc
		{
			VkRenderPass render_pass = VK_NULL_HANDLE;
			ImageView* color_attachments[8] = { nullptr };
			ImageView* resolve_attachments[8] = { nullptr };
			ImageView* depth_stencil_attachment = nullptr;

			bool operator==(const FramebufferDesc& rhs) const
			{
				return render_pass == rhs.render_pass &&
					!memcpy((void*)color_attachments, (void*)rhs.color_attachments, sizeof(ImageView*) * 8) &&
					!memcpy((void*)resolve_attachments, (void*)rhs.resolve_attachments, sizeof(ImageView*) * 8) &&
					depth_stencil_attachment == rhs.depth_stencil_attachment;
			}
		};
		static VkFramebuffer new_frame_buffer(Device* device, const FramebufferDesc& desc)
		{
			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.flags = 0;
			info.renderPass = desc.render_pass;
			// Collect attachments.
			VkImageView attachments[17] = { VK_NULL_HANDLE };
			u32 num_attachments = 0;
			u32 width = 0;
			u32 height = 0;
			u32 depth = 0;
			for (usize i = 0; i < 8; ++i)
			{
				if (desc.color_attachments[i])
				{
					attachments[num_attachments] = desc.color_attachments[i]->m_image_view;
					++num_attachments;
					ImageResource* image = cast_object<ImageResource>(desc.color_attachments[i]->m_desc.texture->get_object());
					width = image->m_desc.width;
					height = image->m_desc.height;
					depth = image->m_desc.depth;
				}
				else break;
			}
			for (usize i = 0; i < 8; ++i)
			{
				if (desc.resolve_attachments[i])
				{
					attachments[num_attachments] = desc.resolve_attachments[i]->m_image_view;
					++num_attachments;
				}
			}
			if (desc.depth_stencil_attachment)
			{
				attachments[num_attachments] = desc.depth_stencil_attachment->m_image_view;
				++num_attachments;
				ImageResource* image = cast_object<ImageResource>(desc.depth_stencil_attachment->m_desc.texture->get_object());
				width = (u32)image->m_desc.width;
				height = image->m_desc.height;
				depth = image->m_desc.depth;
			}
			info.pAttachments = attachments;
			info.attachmentCount = num_attachments;
			info.width = width;
			info.height = height;
			info.layers = depth;
			VkFramebuffer fbo = VK_NULL_HANDLE;
			device->m_funcs.vkCreateFramebuffer(device->m_device, &info, nullptr, &fbo);
			return fbo;
		}
		void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
		{
			lucheck_msg(!m_render_pass_begin && !m_copy_pass_begin && !m_compute_pass_begin, "begin_render_pass can only be called when no other pass is open.");
			lutry
			{
				RenderPassKey rp;
				FramebufferDesc fb;
				u32 width = 0;
				u32 height = 0;
				u32 num_color_attachments = 0;
				u32 num_resolve_targets = 0;
				for (usize i = 0; i < 8; ++i)
				{
					if (desc.color_attachments[i].texture)
					{
						auto& src = desc.color_attachments[i];
						TextureViewDesc view;
						view.texture = src.texture;
						view.type = src.view_type;
						view.format = src.format;
						view.mip_slice = src.mip_slice;
						view.mip_size = 1;
						view.array_slice = src.array_slice;
						view.array_size = desc.array_size;
						lulet(view_object, cast_object<ImageResource>(view.texture->get_object())->get_image_view(view));
						fb.color_attachments[i] = view_object;
					}
					if (desc.resolve_attachments[i].texture)
					{
						auto& src = desc.resolve_attachments[i];
						TextureViewDesc view;
						view.texture = src.texture;
						view.type = TextureViewType::unspecified;
						view.format = fb.color_attachments[i]->m_desc.format;
						view.mip_slice = src.mip_slice;
						view.mip_size = 1;
						view.array_slice = src.array_slice;
						view.array_size = 1;
						lulet(view_object, cast_object<ImageResource>(view.texture->get_object())->get_image_view(view));
						fb.resolve_attachments[i] = view_object;
					}
					m_color_attachments[i] = fb.color_attachments[i];
					m_resolve_attachments[i] = fb.resolve_attachments[i];
					if (desc.color_attachments[i].texture)
					{
						++num_color_attachments;
						auto d = desc.color_attachments[i].texture->get_desc();
						rp.color_formats[i] = d.format;
						rp.color_load_ops[i] = desc.color_attachments[i].load_op;
						rp.color_store_ops[i] = desc.color_attachments[i].store_op;
						if (desc.resolve_attachments[i].texture)
						{
							rp.resolve_formats[i] = d.format;
							++num_resolve_targets;
						}
						width = d.width;
						height = d.height;
					}
					else break;
				}
				if (desc.depth_stencil_attachment.texture)
				{
					auto& src = desc.depth_stencil_attachment;
					TextureViewDesc view;
					view.texture = src.texture;
					view.type = src.view_type;
					view.format = src.format;
					view.mip_slice = src.mip_slice;
					view.mip_size = 1;
					view.array_slice = src.array_slice;
					view.array_size = desc.array_size;
					lulet(view_object, cast_object<ImageResource>(view.texture->get_object())->get_image_view(view));
					fb.depth_stencil_attachment = view_object;
				}
				m_dsv = fb.depth_stencil_attachment;
				bool use_depth_stencil = false;
				if (desc.depth_stencil_attachment.texture)
				{
					use_depth_stencil = true;
					auto d = desc.depth_stencil_attachment.texture->get_desc();
					rp.depth_stencil_format = d.format;
					rp.depth_load_op = desc.depth_stencil_attachment.depth_load_op;
					rp.depth_store_op = desc.depth_stencil_attachment.depth_store_op;
					rp.stencil_load_op = desc.depth_stencil_attachment.stencil_load_op;
					rp.stencil_store_op = desc.depth_stencil_attachment.stencil_store_op;
					if (!width) width = d.width;
					if (!height) height = d.height;
				}
				rp.sample_count = desc.sample_count;
				rp.depth_stencil_read_only = desc.depth_stencil_attachment.read_only;
				LockGuard guard(m_device->m_render_pass_pool_lock);
				lulet(render_pass, m_device->m_render_pass_pool.get_render_pass(rp));
				guard.unlock();
				fb.render_pass = render_pass;
				VkFramebuffer fbo = new_frame_buffer(m_device, fb);
				m_fbos.push_back(fbo);

				VkRenderPassBeginInfo begin_info{};
				begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				begin_info.renderPass = render_pass;
				begin_info.framebuffer = fbo;
				begin_info.renderArea.offset.x = 0;
				begin_info.renderArea.offset.y = 0;
				begin_info.renderArea.extent.width = width;
				begin_info.renderArea.extent.height = height;

				u32 num_attachments = num_color_attachments + num_resolve_targets;
				if (use_depth_stencil) ++num_attachments;
				VkClearValue* clear_values = (VkClearValue*)alloca(sizeof(VkClearValue) * num_attachments);
				u32 attachment_index = 0;
				for (usize i = 0; i < num_color_attachments; ++i)
				{
					auto& dst = clear_values[attachment_index];
					auto& src = desc.color_attachments[i].clear_value;
					dst.color.float32[0] = src.x;
					dst.color.float32[1] = src.y;
					dst.color.float32[2] = src.z;
					dst.color.float32[3] = src.w;
					++attachment_index;
				}
				for (usize i = 0; i < num_resolve_targets; ++i)
				{
					auto& dst = clear_values[attachment_index];
					memzero(&dst, sizeof(VkClearValue));
					++attachment_index;
				}
				if (use_depth_stencil)
				{
					auto& dst = clear_values[attachment_index];
					dst.depthStencil.depth = desc.depth_stencil_attachment.depth_clear_value;
					dst.depthStencil.stencil = desc.depth_stencil_attachment.stencil_clear_value;
					++attachment_index;
				}
				begin_info.clearValueCount = num_attachments;
				begin_info.pClearValues = clear_values;
				m_occlusion_query_heap_attachment = desc.occlusion_query_heap;
				m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
				m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
				m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
				m_pipeline_statistics_query_heap_attachment = desc.pipeline_statistics_query_heap;
				m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
				if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
				{
					write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
				}
				if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
				{
					begin_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
				}
				m_device->m_funcs.vkCmdBeginRenderPass(m_command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
				m_rt_width = width;
				m_rt_height = height;
				m_num_color_attachments = num_color_attachments;
				m_num_resolve_attachments = num_resolve_targets;
				m_render_pass_begin = true;
			}
			lucatch
			{

			}
		}
		void CommandBuffer::set_graphics_pipeline_layout(IPipelineLayout* pipeline_layout)
		{
			assert_graphcis_context();
			m_graphics_pipeline_layout = pipeline_layout;
		}
		void CommandBuffer::set_graphics_pipeline_state(IPipelineState* pso)
		{
			assert_graphcis_context();
			PipelineState* ps = (PipelineState*)pso->get_object();
			m_device->m_funcs.vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ps->m_pipeline);
		}
		void CommandBuffer::set_vertex_buffers(u32 start_slot, Span<const VertexBufferView> views)
		{
			assert_graphcis_context();
			VkBuffer* bufs = (VkBuffer*)alloca(sizeof(VkBuffer) * views.size());
			VkDeviceSize* vk_offsets = (VkDeviceSize*)alloca(sizeof(VkDeviceSize) * views.size());
			for (u32 i = 0; i < views.size(); ++i)
			{
				BufferResource* res = cast_object<BufferResource>(views[i].buffer->get_object());
				bufs[i] = res->m_buffer;
				vk_offsets[i] = views[i].offset;
			}
			m_device->m_funcs.vkCmdBindVertexBuffers(m_command_buffer, start_slot, (u32)views.size(), bufs, vk_offsets);
		}
		void CommandBuffer::set_index_buffer(const IndexBufferView& view)
		{
			assert_graphcis_context();
			BufferResource* res = cast_object<BufferResource>(view.buffer->get_object());
			VkIndexType index_type;
			switch (view.format)
			{
			case Format::r16_uint:
			case Format::r16_sint:
				index_type = VK_INDEX_TYPE_UINT16; break;
			case Format::r32_uint:
			case Format::r32_sint:
				index_type = VK_INDEX_TYPE_UINT32; break;
			}
			m_device->m_funcs.vkCmdBindIndexBuffer(m_command_buffer, res->m_buffer, view.offset, index_type);
		}
		void CommandBuffer::set_graphics_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
		{
			assert_graphcis_context();
			VkPipelineLayout layout = VK_NULL_HANDLE;
			PipelineLayout* playout = (PipelineLayout*)m_graphics_pipeline_layout->get_object();
			layout = playout->m_pipeline_layout;
			VkDescriptorSet* sets = (VkDescriptorSet*)alloca(sizeof(VkDescriptorSet) * descriptor_sets.size());
			for (u32 i = 0; i < descriptor_sets.size(); ++i)
			{
				auto s = (DescriptorSet*)(descriptor_sets[i]->get_object());
				sets[i] = s->m_desc_set;
			}
			m_device->m_funcs.vkCmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 
				start_index, (u32)descriptor_sets.size(), sets, 0, nullptr);
		}
		void CommandBuffer::set_viewport(const Viewport& viewport)
		{
			set_viewports({ &viewport, 1 });
		}
		void CommandBuffer::set_viewports(Span<const Viewport> viewports)
		{
			assert_graphcis_context();
			u32 max_num_viewports = m_device->m_physical_device_properties.limits.maxViewports;
			VkViewport* vps = (VkViewport*)alloca(sizeof(VkViewport) * max_num_viewports);
			for (usize i = 0; i < max_num_viewports; ++i)
			{
				auto& d = vps[i];
				d.x = 0.0f;
				d.y = (float)m_rt_height;
				d.width = (float)m_rt_width;
				d.height = -((float)m_rt_height);
				d.minDepth = 0.0f;
				d.maxDepth = 1.0f;
			}
			for (usize i = 0; i < viewports.size(); ++i)
			{
				auto& d = vps[i];
				auto& s = viewports[i];
				d.x = s.top_left_x;
				d.y = s.top_left_y + s.height;
				d.width = s.width;
				d.height = -s.height;
				d.minDepth = s.min_depth;
				d.maxDepth = s.max_depth;
			}
			m_device->m_funcs.vkCmdSetViewport(m_command_buffer, 0, max_num_viewports, vps);
		}
		void CommandBuffer::set_scissor_rect(const RectI& rect)
		{
			set_scissor_rects({ &rect, 1 });
		}
		void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
		{
			assert_graphcis_context();
			u32 max_num_viewports = m_device->m_physical_device_properties.limits.maxViewports;
			VkRect2D* r = (VkRect2D*)alloca(sizeof(VkRect2D) * max_num_viewports);
			for (usize i = 0; i < max_num_viewports; ++i)
			{
				auto& d = r[i];
				d.offset.x = 0;
				d.offset.y = 0;
				d.extent.width = m_rt_width;
				d.extent.height = m_rt_height;
			}
			for (usize i = 0; i < rects.size(); ++i)
			{
				auto& d = r[i];
				auto& s = rects[i];
				d.offset.x = s.offset_x;
				d.offset.y = s.offset_y;
				d.extent.width = s.width;
				d.extent.height = s.height;
			}
			m_device->m_funcs.vkCmdSetScissor(m_command_buffer, 0, max_num_viewports, r);
		}
		void CommandBuffer::set_blend_factor(const Float4U& blend_factor)
		{
			assert_graphcis_context();
			f32 factor[] = {blend_factor.x, blend_factor.y, blend_factor.z, blend_factor.w};
			m_device->m_funcs.vkCmdSetBlendConstants(m_command_buffer, factor);
		}
		void CommandBuffer::set_stencil_ref(u32 stencil_ref)
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdSetStencilReference(m_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, stencil_ref);
		}
		void CommandBuffer::draw(u32 vertex_count, u32 start_vertex_location)
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdDraw(m_command_buffer, vertex_count, 1, start_vertex_location, 0);
		}
		void CommandBuffer::draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location)
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdDrawIndexed(m_command_buffer, index_count, 1, start_index_location, base_vertex_location, 0);
		}
		void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
			u32 start_instance_location)
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdDraw(m_command_buffer, vertex_count_per_instance * instance_count, instance_count,
				start_vertex_location, start_instance_location);
		}
		void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
			i32 base_vertex_location, u32 start_instance_location)
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdDrawIndexed(m_command_buffer, index_count_per_instance * instance_count, instance_count, 
				start_index_location, base_vertex_location, start_instance_location);
		}
		void CommandBuffer::begin_occlusion_query(OcclusionQueryMode mode, u32 index)
		{
			assert_graphcis_context();
			QueryHeap* h = (QueryHeap*)m_occlusion_query_heap_attachment->get_object();
			m_device->m_funcs.vkCmdResetQueryPool(m_command_buffer, h->m_query_pool, index, 1);
			m_device->m_funcs.vkCmdBeginQuery(m_command_buffer, h->m_query_pool, index, mode == OcclusionQueryMode::counting ? VK_QUERY_CONTROL_PRECISE_BIT : 0);
		}
		void CommandBuffer::end_occlusion_query(u32 index)
		{
			assert_graphcis_context();
			QueryHeap* h = (QueryHeap*)m_occlusion_query_heap_attachment->get_object();
			m_device->m_funcs.vkCmdEndQuery(m_command_buffer, h->m_query_pool, index);
		}
		void CommandBuffer::end_render_pass()
		{
			assert_graphcis_context();
			m_device->m_funcs.vkCmdEndRenderPass(m_command_buffer);
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				end_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
			m_occlusion_query_heap_attachment = nullptr;
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_pipeline_statistics_query_heap_attachment = nullptr;
			m_pipeline_statistics_query_index = DONT_QUERY;
			m_render_pass_begin = false;
			m_rt_width = 0;
			m_rt_height = 0;
			m_num_color_attachments = 0;
			m_num_resolve_attachments = 0;
			memzero(m_color_attachments, sizeof(ImageView*) * 8);
			memzero(m_resolve_attachments, sizeof(ImageView*) * 8);
			m_dsv = nullptr;
		}
		void CommandBuffer::begin_compute_pass(const ComputePassDesc& desc)
		{
			lucheck_msg(!m_render_pass_begin && !m_copy_pass_begin && !m_compute_pass_begin, "begin_compute_pass can only be called when no other pass is open.");
			m_compute_pass_begin = true;
			m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
			m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
			m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
			m_pipeline_statistics_query_heap_attachment = desc.pipeline_statistics_query_heap;
			m_pipeline_statistics_query_index = desc.pipeline_statistics_query_write_index;
			if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				begin_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
		}
		void CommandBuffer::set_compute_pipeline_layout(IPipelineLayout* pipeline_layout)
		{
			assert_compute_context();
			m_compute_pipeline_layout = pipeline_layout;
		}
		void CommandBuffer::set_compute_pipeline_state(IPipelineState* pso)
		{
			assert_compute_context();
			PipelineState* ps = (PipelineState*)pso->get_object();
			m_device->m_funcs.vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ps->m_pipeline);
		}
		void CommandBuffer::set_compute_descriptor_sets(u32 start_index, Span<IDescriptorSet*> descriptor_sets)
		{
			assert_compute_context();
			VkPipelineLayout layout = VK_NULL_HANDLE;
			PipelineLayout* playout = (PipelineLayout*)m_compute_pipeline_layout->get_object();
			layout = playout->m_pipeline_layout;
			VkDescriptorSet* sets = (VkDescriptorSet*)alloca(sizeof(VkDescriptorSet) * descriptor_sets.size());
			for (u32 i = 0; i < descriptor_sets.size(); ++i)
			{
				auto s = (DescriptorSet*)descriptor_sets[i]->get_object();
				sets[i] = s->m_desc_set;
			}
			m_device->m_funcs.vkCmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout,
				start_index, (u32)descriptor_sets.size(), sets, 0, nullptr);
		}
		void CommandBuffer::dispatch(u32 thread_group_count_x, u32 thread_group_count_y, u32 thread_group_count_z)
		{
			assert_compute_context();
			m_device->m_funcs.vkCmdDispatch(m_command_buffer, thread_group_count_x, thread_group_count_y, thread_group_count_z);
		}
		void CommandBuffer::end_compute_pass()
		{
			lucheck_msg(m_compute_pass_begin, "Calling end_compute_pass without prior call to begin_compute_pass.");
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			if (m_pipeline_statistics_query_heap_attachment && m_pipeline_statistics_query_index != DONT_QUERY)
			{
				end_pipeline_statistics_query(m_pipeline_statistics_query_heap_attachment, m_pipeline_statistics_query_index);
			}
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_pipeline_statistics_query_heap_attachment = nullptr;
			m_pipeline_statistics_query_index = DONT_QUERY;
			m_compute_pass_begin = false;
		}
		void CommandBuffer::begin_copy_pass(const CopyPassDesc& desc)
		{
			lucheck_msg(!m_render_pass_begin && !m_copy_pass_begin && !m_compute_pass_begin, "begin_copy_pass can only be called when no other pass is open.");
			m_copy_pass_begin = true;
			m_timestamp_query_heap_attachment = desc.timestamp_query_heap;
			m_timestamp_query_begin_index = desc.timestamp_query_begin_pass_write_index;
			m_timestamp_query_end_index = desc.timestamp_query_end_pass_write_index;
			if (m_timestamp_query_heap_attachment && m_timestamp_query_begin_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_begin_index);
			}
		}
		void CommandBuffer::copy_resource(IResource* dst, IResource* src)
		{
			assert_copy_context();
			BufferResource* s = cast_object<BufferResource>(src->get_object());
			BufferResource* d = cast_object<BufferResource>(dst->get_object());
			if (s && d)
			{
				VkBufferCopy copy{};
				copy.srcOffset = 0;
				copy.dstOffset = 0;
				copy.size = d->m_desc.size;
				m_device->m_funcs.vkCmdCopyBuffer(m_command_buffer, s->m_buffer, d->m_buffer, 1, &copy);
			}
			else
			{
				ImageResource* ts = cast_object<ImageResource>(src->get_object());
				ImageResource* td = cast_object<ImageResource>(dst->get_object());
				// The copy is performed one per mips.
				u32 mip_levels = td->m_desc.mip_levels;
				u32 array_count = td->m_desc.array_size;
				VkImageCopy* copies = (VkImageCopy*)alloca(sizeof(VkImageCopy) * mip_levels);
				for (u32 mip = 0; mip < mip_levels; ++mip)
				{
					VkImageCopy& copy = copies[mip];
					copy.srcSubresource.aspectMask =
						is_depth_stencil_format(td->m_desc.format) ?
						VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
						VK_IMAGE_ASPECT_COLOR_BIT;
					copy.srcSubresource.baseArrayLayer = 0;
					copy.srcSubresource.layerCount = array_count;
					copy.srcSubresource.mipLevel = mip;
					copy.srcOffset.x = 0;
					copy.srcOffset.y = 0;
					copy.srcOffset.z = 0;
					copy.dstSubresource = copy.srcSubresource;
					copy.dstOffset = copy.srcOffset;
					copy.extent.width = max<u32>(td->m_desc.width >> mip, 1);
					copy.extent.height = max<u32>(td->m_desc.height >> mip, 1);
					copy.extent.depth = max<u32>(td->m_desc.depth >> mip, 1);
				}
				VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				VkImageLayout dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				m_device->m_funcs.vkCmdCopyImage(m_command_buffer, ts->m_image, src_layout, td->m_image, dst_layout, mip_levels, copies);
			}
		}
		void CommandBuffer::copy_buffer(
			IBuffer* dst, u64 dst_offset,
			IBuffer* src, u64 src_offset,
			u64 copy_bytes)
		{
			assert_copy_context();
			BufferResource* s = cast_object<BufferResource>(src->get_object());
			BufferResource* d = cast_object<BufferResource>(dst->get_object());
			VkBufferCopy copy{};
			copy.srcOffset = src_offset;
			copy.dstOffset = dst_offset;
			copy.size = copy_bytes;
			m_device->m_funcs.vkCmdCopyBuffer(m_command_buffer, s->m_buffer, d->m_buffer, 1, &copy);
		}
		void CommandBuffer::copy_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			assert_copy_context();
			ImageResource* s = cast_object<ImageResource>(src->get_object());
			ImageResource* d = cast_object<ImageResource>(dst->get_object());
			// The copy is performed one per mips.
			VkImageCopy copy{};
			copy.srcSubresource.aspectMask =
				is_depth_stencil_format(d->m_desc.format) ?
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
				VK_IMAGE_ASPECT_COLOR_BIT;
			copy.srcSubresource.baseArrayLayer = src_subresource.array_slice;
			copy.srcSubresource.layerCount = 1;
			copy.srcSubresource.mipLevel = src_subresource.mip_slice;
			copy.srcOffset.x = src_x;
			copy.srcOffset.y = src_y;
			copy.srcOffset.z = src_z;
			copy.dstSubresource.aspectMask = copy.srcSubresource.aspectMask;
			copy.dstSubresource.baseArrayLayer = dst_subresource.array_slice;
			copy.dstSubresource.layerCount = 1;
			copy.dstSubresource.mipLevel = dst_subresource.mip_slice;
			copy.dstOffset.x = dst_x;
			copy.dstOffset.y = dst_y;
			copy.dstOffset.z = dst_z;
			copy.extent.width = copy_width;
			copy.extent.height = copy_height;
			copy.extent.depth = copy_depth;
			VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			VkImageLayout dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			m_device->m_funcs.vkCmdCopyImage(m_command_buffer, s->m_image, src_layout, d->m_image, dst_layout, 1, &copy);
		}
		void CommandBuffer::copy_buffer_to_texture(
			ITexture* dst, SubresourceIndex dst_subresource, u32 dst_x, u32 dst_y, u32 dst_z,
			IBuffer* src, u64 src_offset, u32 src_row_pitch, u32 src_slice_pitch,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			assert_copy_context();
			BufferResource* s = cast_object<BufferResource>(src->get_object());
			ImageResource* d = cast_object<ImageResource>(dst->get_object());
			VkBufferImageCopy copy{};
			copy.bufferOffset = src_offset;
			copy.bufferRowLength = src_row_pitch * 8 / bits_per_pixel(d->m_desc.format);
			copy.bufferImageHeight = src_slice_pitch * 8 / bits_per_pixel(d->m_desc.format);
			copy.imageSubresource.aspectMask = is_depth_stencil_format(d->m_desc.format) ?
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
				VK_IMAGE_ASPECT_COLOR_BIT;
			copy.imageSubresource.baseArrayLayer = dst_subresource.array_slice;
			copy.imageSubresource.layerCount = 1;
			copy.imageSubresource.mipLevel = dst_subresource.mip_slice;
			copy.imageOffset.x = dst_x;
			copy.imageOffset.y = dst_y;
			copy.imageOffset.z = dst_z;
			copy.imageExtent.width = copy_width;
			copy.imageExtent.height = copy_height;
			copy.imageExtent.depth = copy_depth;
			m_device->m_funcs.vkCmdCopyBufferToImage(m_command_buffer, s->m_buffer, d->m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		}
		void CommandBuffer::copy_texture_to_buffer(
			IBuffer* dst, u64 dst_offset, u32 dst_row_pitch, u32 dst_slice_pitch,
			ITexture* src, SubresourceIndex src_subresource, u32 src_x, u32 src_y, u32 src_z,
			u32 copy_width, u32 copy_height, u32 copy_depth)
		{
			assert_copy_context();
			ImageResource* s = cast_object<ImageResource>(src->get_object());
			BufferResource* d = cast_object<BufferResource>(dst->get_object());
			VkBufferImageCopy copy{};
			copy.imageSubresource.aspectMask = is_depth_stencil_format(s->m_desc.format) ?
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
				VK_IMAGE_ASPECT_COLOR_BIT;
			copy.imageSubresource.baseArrayLayer = src_subresource.array_slice;
			copy.imageSubresource.layerCount = 1;
			copy.imageSubresource.mipLevel = src_subresource.mip_slice;
			copy.imageOffset.x = src_x;
			copy.imageOffset.y = src_y;
			copy.imageOffset.z = src_z;
			copy.imageExtent.width = copy_width;
			copy.imageExtent.height = copy_height;
			copy.imageExtent.depth = copy_depth;
			copy.bufferOffset = dst_offset;
			copy.bufferRowLength = dst_row_pitch * 8 / bits_per_pixel(s->m_desc.format);
			copy.bufferImageHeight = dst_slice_pitch * 8 / bits_per_pixel(s->m_desc.format);
			m_device->m_funcs.vkCmdCopyImageToBuffer(m_command_buffer, s->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				d->m_buffer, 1, &copy);
		}
		void CommandBuffer::end_copy_pass()
		{
			lucheck_msg(m_copy_pass_begin, "Calling end_copy_pass without prior call to begin_copy_pass.");
			if (m_timestamp_query_heap_attachment && m_timestamp_query_end_index != DONT_QUERY)
			{
				write_timestamp(m_timestamp_query_heap_attachment, m_timestamp_query_end_index);
			}
			m_timestamp_query_heap_attachment = nullptr;
			m_timestamp_query_begin_index = DONT_QUERY;
			m_timestamp_query_end_index = DONT_QUERY;
			m_copy_pass_begin = false;
		}
		void CommandBuffer::resource_barrier(Span<const BufferBarrier> buffer_barriers, Span<const TextureBarrier> texture_barriers)
		{
			assert_non_render_pass();
			m_track_system.begin_new_barriers_batch();
			for (auto& barrier : buffer_barriers)
			{
				m_track_system.pack_buffer(barrier);
			}
			for (auto& barrier : texture_barriers)
			{
				m_track_system.pack_image(barrier);
			}
			if (!m_track_system.m_buffer_barriers.empty() || !m_track_system.m_image_barriers.empty())
			{
				if (m_track_system.m_src_stage_flags == 0)
				{
					m_track_system.m_src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				}
				if (m_track_system.m_dst_stage_flags == 0)
				{
					m_track_system.m_dst_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				}
				m_device->m_funcs.vkCmdPipelineBarrier(m_command_buffer,
					m_track_system.m_src_stage_flags, m_track_system.m_dst_stage_flags, 0, 0, nullptr,
					m_track_system.m_buffer_barriers.size(), m_track_system.m_buffer_barriers.data(),
					m_track_system.m_image_barriers.size(), m_track_system.m_image_barriers.data());
			}
		}
		RV CommandBuffer::submit(Span<IFence*> wait_fences, Span<IFence*> signal_fences, bool allow_host_waiting)
		{
			lucheck_msg(!m_render_pass_begin && !m_copy_pass_begin && !m_compute_pass_begin, "submit can only be called when no render, compute or copy pass is open.");
			if (!m_recording) return BasicError::bad_calling_time();
			lutry
			{
				// Finish barrier.
				m_track_system.generate_finish_barriers();
				if (!m_track_system.m_buffer_barriers.empty() || !m_track_system.m_image_barriers.empty())
				{
					m_device->m_funcs.vkCmdPipelineBarrier(m_command_buffer,
						m_track_system.m_src_stage_flags, m_track_system.m_dst_stage_flags, 0, 0, nullptr,
						m_track_system.m_buffer_barriers.size(), m_track_system.m_buffer_barriers.data(),
						m_track_system.m_image_barriers.size(), m_track_system.m_image_barriers.data());
				}

				// Close the command buffer.
				luexp(encode_vk_result(m_device->m_funcs.vkEndCommandBuffer(m_command_buffer)));
				m_recording = false;

				Vector<VkSemaphore> wait_semaphores;
				Vector<VkPipelineStageFlags> wait_stages;

				bool resolve_enabled = false;
				if (!m_track_system.m_unresolved_image_states.empty() || !m_track_system.m_unresolved_buffer_states.empty())
				{
					// Resolve image states.
					m_track_system.resolve();
					if (!m_track_system.m_buffer_barriers.empty() || !m_track_system.m_image_barriers.empty())
					{
						resolve_enabled = true;
						VkCommandBufferBeginInfo begin_info{};
						begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
						begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
						begin_info.pInheritanceInfo = nullptr;
						luexp(encode_vk_result(m_device->m_funcs.vkBeginCommandBuffer(m_resolve_buffer, &begin_info)));
						m_device->m_funcs.vkCmdPipelineBarrier(m_resolve_buffer,
							m_track_system.m_src_stage_flags, m_track_system.m_dst_stage_flags, 0, 0, nullptr,
							m_track_system.m_buffer_barriers.size(), m_track_system.m_buffer_barriers.data(),
							m_track_system.m_image_barriers.size(), m_track_system.m_image_barriers.data());
						luexp(encode_vk_result(m_device->m_funcs.vkEndCommandBuffer(m_resolve_buffer)));
						// Queue ownership transfer.
						for (auto& transfer_barriers : m_track_system.m_queue_transfer_barriers)
						{
							lulet(transfer_tracker, get_transfer_tracker(transfer_barriers.first));
							VkQueue queue = VK_NULL_HANDLE;
							IMutex* queue_mtx = nullptr;
							for (auto& q : m_device->m_queues)
							{
								if (q.queue_family_index == transfer_barriers.first)
								{
									queue = q.queue;
									queue_mtx = q.queue_mtx;
									break;
								}
							}
							luassert(queue != VK_NULL_HANDLE);
							lulet(sema, transfer_tracker->submit_barrier(queue, queue_mtx,
								{ transfer_barriers.second.buffer_barriers.data(), transfer_barriers.second.buffer_barriers.size() },
								{ transfer_barriers.second.image_barriers.data(), transfer_barriers.second.image_barriers.size() }
							));
							wait_semaphores.push_back(sema);
							wait_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
						}
					}
				}
				// Submit the command buffer.
				for (usize i = 0; i < wait_fences.size(); ++i)
				{
					Fence* fence = (Fence*)wait_fences[i]->get_object();
					wait_semaphores.push_back(fence->m_semaphore);
					wait_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
				}
				VkSubmitInfo submit{};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.waitSemaphoreCount = (u32)wait_semaphores.size();
				submit.pWaitSemaphores = wait_semaphores.data();
				submit.pWaitDstStageMask = wait_stages.data();
				submit.signalSemaphoreCount = (u32)signal_fences.size();
				VkSemaphore* signal_semaphores = nullptr;
				if (!signal_fences.empty())
				{
					signal_semaphores = (VkSemaphore*)alloca(sizeof(VkSemaphore) * signal_fences.size());
					for (usize i = 0; i < signal_fences.size(); ++i)
					{
						Fence* fence = (Fence*)signal_fences[i]->get_object();
						signal_semaphores[i] = fence->m_semaphore;
					}
				}
				submit.pSignalSemaphores = signal_semaphores;
				VkCommandBuffer buffers[2] = { m_resolve_buffer , m_command_buffer };
				if (resolve_enabled)
				{
					submit.commandBufferCount = 2;
					submit.pCommandBuffers = buffers;
				}
				else
				{
					submit.commandBufferCount = 1;
					submit.pCommandBuffers = &m_command_buffer;
				}
				VkFence fence = VK_NULL_HANDLE;
				if (allow_host_waiting)
				{
					fence = m_fence;
				}
				MutexGuard guard(m_queue.queue_mtx);
				luexp(encode_vk_result(m_device->m_funcs.vkQueueSubmit(m_queue.queue, 1, &submit, fence)));
				m_track_system.apply();
			}
			lucatchret;
			return ok;
		}
	}
}