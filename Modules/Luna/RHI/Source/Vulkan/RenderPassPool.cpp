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
				VkAttachmentReference color_attachment_refs[8] = { 0 };
				VkAttachmentReference resolve_attachment_refs[8] = { 0 };
				VkAttachmentReference depth_stencil_attachment_ref {};
				u8 num_color_attachments = 0;
				for (usize i = 0; i < 8; ++i)
				{
					if (key.color_formats[i] != Format::unknown)
					{
						++num_color_attachments;
					}
					else
					{
						break;
					}
				}
				u8 resolve_targets[8];
				u8 num_resolve_targets = 0;
				for (usize i = 0; i < num_color_attachments; ++i)
				{
					if (key.resolve_formats[i] != Format::unknown)
					{
						resolve_targets[num_resolve_targets] = i;
						++num_resolve_targets;
					}
				}
				bool depth_stencil_attachment_present = key.depth_stencil_format != Format::unknown;
				// Encode attachment.
				u8 attachment_index = 0;
				for (usize i = 0; i < num_color_attachments; ++i)
				{
					VkAttachmentDescription& dst = attachments[attachment_index];
					dst.flags = 0;
					dst.format = encode_format(key.color_formats[i]);
					dst.samples = encode_sample_count(key.sample_count);
					dst.loadOp = encode_load_op(key.color_load_ops[i]);
					dst.storeOp = encode_store_op(key.color_store_ops[i]);
					dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dst.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					dst.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					color_attachment_refs[i].attachment = attachment_index;
					color_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					resolve_attachment_refs[i].attachment = VK_ATTACHMENT_UNUSED;
					resolve_attachment_refs[i].layout = VK_IMAGE_LAYOUT_UNDEFINED;
					++attachment_index;
				}
				// Encode resolve if needed.
				for (usize i = 0; i < num_resolve_targets; ++i)
				{
					VkAttachmentDescription& dst = attachments[attachment_index];
					dst.flags = 0;
					dst.format = encode_format(key.resolve_formats[resolve_targets[i]]);
					dst.samples = VK_SAMPLE_COUNT_1_BIT;
					dst.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dst.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					dst.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					resolve_attachment_refs[resolve_targets[i]].attachment = attachment_index;
					resolve_attachment_refs[resolve_targets[i]].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					++attachment_index;
				}
				// Encode depth stencil if needed.
				if (depth_stencil_attachment_present)
				{
					VkAttachmentDescription& dst = attachments[attachment_index];
					dst.flags = 0;
					dst.format = encode_format(key.depth_stencil_format);
					dst.samples = encode_sample_count(key.sample_count);
					dst.loadOp = encode_load_op(key.depth_load_op);
					dst.storeOp = encode_store_op(key.depth_store_op);
					dst.stencilLoadOp = encode_load_op(key.stencil_load_op);
					dst.stencilStoreOp = encode_store_op(key.stencil_store_op);
					if (key.depth_stencil_read_only)
					{
						dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					}
					else
					{
						dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					}
					depth_stencil_attachment_ref.attachment = attachment_index;
					depth_stencil_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					++attachment_index;
				}
				VkSubpassDescription subpass{};
				subpass.flags = 0;
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.inputAttachmentCount = 0;
				subpass.pInputAttachments = nullptr;
				if (num_color_attachments)
				{
					subpass.colorAttachmentCount = num_color_attachments;
					subpass.pColorAttachments = color_attachment_refs;
				}
				else
				{
					subpass.colorAttachmentCount = 0;
					subpass.pColorAttachments = VK_NULL_HANDLE;
				}
				if (num_resolve_targets)
				{
					subpass.pResolveAttachments = resolve_attachment_refs;
				}
				else
				{
					subpass.pResolveAttachments = VK_NULL_HANDLE;
				}
				if (depth_stencil_attachment_present)
				{
					subpass.pDepthStencilAttachment = &depth_stencil_attachment_ref;
				}
				else
				{
					subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
				}
				subpass.preserveAttachmentCount = 0;
				subpass.pPreserveAttachments = nullptr;
				VkRenderPassCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				create_info.flags = 0;
				if (attachment_index)
				{
					create_info.pAttachments = attachments;
					create_info.attachmentCount = attachment_index;
				}
				else
				{
					create_info.pAttachments = nullptr;
					create_info.attachmentCount = 0;
				}
				create_info.subpassCount = 1;
				create_info.pSubpasses = &subpass;
				create_info.dependencyCount = 0;
				create_info.pDependencies = nullptr;
				VkRenderPass render_pass = VK_NULL_HANDLE;
				luexp(encode_vk_result(m_vkCreateRenderPass(m_device, &create_info, nullptr, &render_pass)));
				iter = m_render_passes.insert(make_pair(key, render_pass)).first;
			}
			lucatchret;
			return iter->second;
		}
		void RenderPassPool::clean_up()
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