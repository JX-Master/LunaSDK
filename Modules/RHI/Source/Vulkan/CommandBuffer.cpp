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

namespace Luna
{
	namespace RHI
	{
		RV CommandBuffer::init(CommandQueue* queue)
		{
			lutry
			{
				m_queue = queue;
				VkCommandPoolCreateInfo pool_info{};
				pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.queueFamilyIndex = queue->m_queue_family_index;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateCommandPool(m_device->m_device, &pool_info, nullptr, &m_command_pool)));
				VkCommandBufferAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				alloc_info.commandPool = m_command_pool;
				alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				alloc_info.commandBufferCount = 1;
				luexp(encode_vk_result(m_device->m_funcs.vkAllocateCommandBuffers(m_device->m_device, &alloc_info, &m_command_buffer)));
				VkFenceCreateInfo fence_create_info{};
				fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateFence(m_device->m_device, &fence_create_info, nullptr, &m_fence)));
				luexp(begin_command_buffer());
			}
			lucatchret;
			return ok;
		}
		CommandBuffer::~CommandBuffer()
		{
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
			}
			lucatchret;
			return ok;
		}
		void CommandBuffer::wait()
		{
			auto r = m_device->m_funcs.vkWaitForFences(m_device->m_device, 1, &m_fence, VK_TRUE, U64_MAX);
		}
		bool CommandBuffer::try_wait()
		{
			return m_device->m_funcs.vkGetFenceStatus(m_device->m_device, m_fence) == VK_SUCCESS;
		}
		void CommandBuffer::attach_device_object(IDeviceChild* obj)
		{
			m_objs.push_back(obj);
		}
		void CommandBuffer::set_pipeline_state(PipelineStateBindPoint bind_point, IPipelineState* pso)
		{
			VkPipelineBindPoint bind{};
			switch (bind_point)
			{
			case PipelineStateBindPoint::graphics: bind = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
			case PipelineStateBindPoint::compute: bind = VK_PIPELINE_BIND_POINT_COMPUTE; break;
			}
			PipelineState* ps = (PipelineState*)pso->get_object();
			m_device->m_funcs.vkCmdBindPipeline(m_command_buffer, bind, ps->m_pipeline);
			if (bind_point == PipelineStateBindPoint::graphics)
			{
				m_num_viewports = ps->m_num_viewports;
			}
		}
		void CommandBuffer::set_viewport(const Viewport& viewport)
		{
			set_viewports({ &viewport, 1 });
		}
		void CommandBuffer::set_viewports(Span<const Viewport> viewports)
		{
			VkViewport* vps = (VkViewport*)alloca(sizeof(VkViewport) * m_num_viewports);
			for (usize i = 0; i < m_num_viewports; ++i)
			{
				auto& d = vps[i];
				d.width = (float)m_rt_width;
				d.height = (float)m_rt_height;
				d.minDepth = 0.0f;
				d.maxDepth = 1.0f;
				d.x = 0.0f;
				d.y = 0.0f;
			}
			for (usize i = 0; i < viewports.size(); ++i)
			{
				auto& d = vps[i];
				auto& s = viewports[i];
				d.width = s.width;
				d.height = s.height;
				d.minDepth = s.min_depth;
				d.maxDepth = s.max_depth;
				d.x = s.top_left_x;
				d.y = s.top_left_y;
			}
			m_device->m_funcs.vkCmdSetViewport(m_command_buffer, 0, m_num_viewports, vps);
		}
		void CommandBuffer::set_scissor_rect(const RectI& rect)
		{
			set_scissor_rects({ &rect, 1 });
		}
		void CommandBuffer::set_scissor_rects(Span<const RectI> rects)
		{
			VkRect2D* r = (VkRect2D*)alloca(sizeof(VkRect2D) * m_num_viewports);
			for (usize i = 0; i < m_num_viewports; ++i)
			{
				auto& d = r[i];
				d.extent.width = m_rt_width;
				d.extent.height = m_rt_height;
				d.offset.x = 0;
				d.offset.y = 0;
			}
			for (usize i = 0; i < m_num_viewports; ++i)
			{
				auto& d = r[i];
				auto& s = rects[i];
				d.offset.x = s.offset_x;
				d.offset.y = m_rt_height - (s.height + s.offset_y);
				d.extent.width = s.width;
				d.extent.height = s.height;
			}
			m_device->m_funcs.vkCmdSetScissor(m_command_buffer, 0, m_num_viewports, r);
		}
		void CommandBuffer::set_blend_factor(Span<const f32, 4> blend_factor)
		{
			m_device->m_funcs.vkCmdSetBlendConstants(m_command_buffer, blend_factor.data());
		}
		void CommandBuffer::set_stencil_ref(u32 stencil_ref)
		{
			m_device->m_funcs.vkCmdSetStencilReference(m_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, stencil_ref);
		}
		void CommandBuffer::draw(u32 vertex_count, u32 start_vertex_location)
		{
			m_device->m_funcs.vkCmdDraw(m_command_buffer, vertex_count, 1, start_vertex_location, 0);
		}
		void CommandBuffer::draw_indexed(u32 index_count, u32 start_index_location, i32 base_vertex_location)
		{
			m_device->m_funcs.vkCmdDrawIndexed(m_command_buffer, index_count, 1, start_index_location, base_vertex_location, 0);
		}
		void CommandBuffer::draw_indexed_instanced(u32 index_count_per_instance, u32 instance_count, u32 start_index_location,
			i32 base_vertex_location, u32 start_instance_location)
		{
			m_device->m_funcs.vkCmdDrawIndexed(m_command_buffer, index_count_per_instance * instance_count, instance_count, 
				start_index_location, base_vertex_location, start_instance_location);
		}
		void CommandBuffer::draw_instanced(u32 vertex_count_per_instance, u32 instance_count, u32 start_vertex_location,
			u32 start_instance_location)
		{
			m_device->m_funcs.vkCmdDraw(m_command_buffer, vertex_count_per_instance * instance_count, instance_count, 
				start_vertex_location, start_instance_location);
		}


	}
}