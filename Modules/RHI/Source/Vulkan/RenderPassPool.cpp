/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RenderPassPool.cpp
* @author JXMaster
* @date 2023/4/21
*/
#include "RenderPassPool.hpp"
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		R<VkRenderPass> RenderPassPool::get_render_pass(const RenderPassKey& key)
		{
			auto iter = m_render_passes.find(key);
			if (iter != m_render_passes.end()) return iter->second;
			lutry
			{
				VkAttachmentDescription attachments[17];
				

				VkSubpassDescription subpass{};
				subpass.flags = 0;
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.inputAttachmentCount = 0;
				subpass.pInputAttachments = nullptr;

				VkRenderPassCreateInfo create_info{};

			}
			lucatchret;
			return iter->second;
		}
		RenderPassPool::~RenderPassPool()
		{
			for (auto& p : m_render_passes)
			{
				if (p.second != VK_NULL_HANDLE)
				{
					m_vkDestroyRenderPass(m_device, p.second, nullptr);
					p.second = VK_NULL_HANDLE;
				}
			}
			m_render_passes.clear();
		}
	}
}