/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HostFence.cpp
* @author JXMaster
* @date 2023/4/21
*/
#include "HostFence.hpp"

namespace Luna
{
	namespace RHI
	{
		RV HostFence::init()
		{
			VkFenceCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = 0;
			return encode_vk_result(m_device->m_funcs.vkCreateFence(m_device->m_device, &info, nullptr, &m_fence));
		}
		HostFence::~HostFence()
		{
			if (m_fence != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyFence(m_device->m_device, m_fence, nullptr);
				m_fence = VK_NULL_HANDLE;
			}
		}
		void HostFence::wait()
		{
			auto r = m_device->m_funcs.vkWaitForFences(m_device->m_device, 1, &m_fence, VK_TRUE, U64_MAX);
		}
		bool HostFence::try_wait()
		{
			return m_device->m_funcs.vkGetFenceStatus(m_device->m_device, m_fence) == VK_SUCCESS;
		}
		void HostFence::reset()
		{
			luassert_msg_always(m_device->m_funcs.vkResetFences(m_device->m_device, 1, &m_fence) == VK_SUCCESS, "vkResetFences failed.");
		}
	}
}