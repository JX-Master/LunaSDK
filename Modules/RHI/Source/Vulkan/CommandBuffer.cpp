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
		void CommandBuffer::begin_render_pass(const RenderPassDesc& desc)
		{
			
		}

	}
}