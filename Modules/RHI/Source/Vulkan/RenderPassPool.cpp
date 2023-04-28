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
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "ResolveTargetView.hpp"

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
					VkAttachmentDescription& dest = attachments[attachment_index];
					dest.flags = 0;
					dest.format = encode_format(key.color_formats[i]);
					dest.samples = encode_sample_count(key.sample_count);
					dest.loadOp = encode_load_op(key.color_load_ops[i]);
					dest.storeOp = encode_store_op(key.color_store_ops[i]);
					dest.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dest.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dest.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					dest.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					color_attachment_refs[i].attachment = attachment_index;
					color_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					resolve_attachment_refs[i].attachment = VK_ATTACHMENT_UNUSED;
					resolve_attachment_refs[i].layout = VK_IMAGE_LAYOUT_UNDEFINED;
					++attachment_index;
				}
				// Encode resolve if needed.
				for (usize i = 0; i < num_resolve_targets; ++i)
				{
					VkAttachmentDescription& dest = attachments[attachment_index];
					dest.flags = 0;
					dest.format = encode_format(key.resolve_formats[resolve_targets[i]]);
					dest.samples = VK_SAMPLE_COUNT_1_BIT;
					dest.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dest.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					dest.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dest.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dest.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					dest.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					resolve_attachment_refs[resolve_targets[i]].attachment = attachment_index;
					resolve_attachment_refs[resolve_targets[i]].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					++attachment_index;
				}
				// Encode depth stencil if needed.
				if (depth_stencil_attachment_present)
				{
					VkAttachmentDescription& dest = attachments[attachment_index];
					dest.flags = 0;
					dest.format = encode_format(key.depth_stencil_format);
					dest.samples = encode_sample_count(key.sample_count);
					dest.loadOp = encode_load_op(key.depth_load_op);
					dest.storeOp = encode_store_op(key.depth_store_op);
					dest.stencilLoadOp = encode_load_op(key.stencil_load_op);
					dest.stencilStoreOp = encode_store_op(key.stencil_store_op);
					dest.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					dest.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
		R<VkFramebuffer> RenderPassPool::get_frame_buffer(const FrameBufferKey& key)
		{
			auto iter = m_framebuffers.find(key);
			if (iter != m_framebuffers.end()) return iter->second;
			lutry
			{
				VkFramebufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				info.flags = 0;
				info.renderPass = key.render_pass;
				// Collect attachments.
				VkImageView attachments[17] = { VK_NULL_HANDLE };
				u32 num_attachments = 0;
				u32 width = 0;
				u32 height = 0;
				u32 depth = 0;
				for (usize i = 0; i < 8; ++i)
				{
					if (key.color_attachments[i])
					{
						RenderTargetView* rtv = cast_objct<RenderTargetView>(key.color_attachments[i]->get_object());
						attachments[num_attachments] = rtv->m_view;
						++num_attachments;
						auto desc = rtv->m_resource->get_desc();
						width = (u32)desc.width_or_buffer_size;
						height = desc.height;
						depth = desc.depth_or_array_size;
					}
					else break;
				}
				for (usize i = 0; i < 8; ++i)
				{
					if (key.resolve_attachments[i])
					{
						ResolveTargetView* rsv = cast_objct<ResolveTargetView>(key.resolve_attachments[i]->get_object());
						attachments[num_attachments] = rsv->m_view;
						++num_attachments;
					}
				}
				if (key.depth_stencil_attachment)
				{
					DepthStencilView* depth_stencil_attachment = cast_objct<DepthStencilView>(key.depth_stencil_attachment->get_object());
					attachments[num_attachments] = depth_stencil_attachment->m_view;
					++num_attachments;
					auto desc = depth_stencil_attachment->m_resource->get_desc();
					width = (u32)desc.width_or_buffer_size;
					height = desc.height;
					depth = desc.depth_or_array_size;
				}
				info.pAttachments = attachments;
				info.attachmentCount = num_attachments;
				info.width = width;
				info.height = height;
				info.layers = depth;
				VkFramebuffer fbo = VK_NULL_HANDLE;
				luexp(encode_vk_result(m_vkCreateFramebuffer(m_device, &info, nullptr, &fbo)));
				iter = m_framebuffers.insert(make_pair(key, fbo)).first;
			}
			lucatchret;
			return iter->second;
		}
		RenderPassPool::~RenderPassPool()
		{
			for (auto& p : m_framebuffers)
			{
				if (p.second != VK_NULL_HANDLE)
				{
					m_vkDestroyFramebuffer(m_device, p.second, nullptr);
					p.second = VK_NULL_HANDLE;
				}
			}
			m_framebuffers.clear();
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