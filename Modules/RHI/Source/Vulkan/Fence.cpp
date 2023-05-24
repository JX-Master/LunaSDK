/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Fence.cpp
* @author JXMaster
* @date 2023/4/21
*/
#include "Fence.hpp"

namespace Luna
{
	namespace RHI
	{
		RV Fence::init()
		{
			VkSemaphoreCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			return encode_vk_result(m_device->m_funcs.vkCreateSemaphore(m_device->m_device, &info, nullptr, &m_semaphore));
		}
		Fence::~Fence()
		{
			if (m_semaphore != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroySemaphore(m_device->m_device, m_semaphore, nullptr);
				m_semaphore = VK_NULL_HANDLE;
			}
		}
	}
}