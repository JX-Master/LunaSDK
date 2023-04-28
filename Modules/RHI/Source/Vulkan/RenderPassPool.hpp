/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RenderPassPool.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
	namespace RHI
	{
		struct RenderPassKey
		{
			Format color_formats[8] = { Format::unknown };
			// Must either be `Format::unknown`, or the same as `rtv_formats[i]`.
			Format resolve_formats[8] = { Format::unknown };
			Format depth_stencil_format = Format::unknown;
			LoadOp color_load_ops[8] = { LoadOp::dont_care };
			StoreOp color_store_ops[8] = { StoreOp::dont_care };
			LoadOp depth_load_op = LoadOp::dont_care;
			StoreOp depth_store_op = StoreOp::dont_care;
			LoadOp stencil_load_op = LoadOp::dont_care;
			StoreOp stencil_store_op = StoreOp::dont_care;
			u8 sample_count = 1;
		};

		struct FrameBufferKey
		{
			VkRenderPass render_pass = VK_NULL_HANDLE;
			IRenderTargetView* color_attachments[8] = { nullptr };
			IResolveTargetView* resolve_attachments[8] = { nullptr };
			IDepthStencilView* depth_stencil_attachment = nullptr;
		};
	}

	template <>
	struct hash<RHI::RenderPassKey>
	{
		usize operator()(const RHI::RenderPassKey& k) const
		{
			usize h = memhash<usize>(k.color_formats, sizeof(RHI::Format) * 8);
			h = memhash<usize>(k.resolve_formats, sizeof(RHI::Format) * 8, h);
			h = memhash<usize>(&k.depth_stencil_format, sizeof(RHI::Format), h);
			h = memhash<usize>(k.color_load_ops, sizeof(RHI::LoadOp) * 8, h);
			h = memhash<usize>(k.color_store_ops, sizeof(RHI::StoreOp) * 8, h);
			h = memhash<usize>(&k.depth_load_op, sizeof(RHI::LoadOp), h);
			h = memhash<usize>(&k.depth_store_op, sizeof(RHI::StoreOp), h);
			h = memhash<usize>(&k.stencil_load_op, sizeof(RHI::LoadOp), h);
			h = memhash<usize>(&k.stencil_store_op, sizeof(RHI::StoreOp), h);
			h = memhash<usize>(&k.sample_count, sizeof(u8), h);
			return h;
		}
	};

	template <>
	struct hash<RHI::FrameBufferKey>
	{
		usize operator()(const RHI::FrameBufferKey& k) const
		{
			usize h = memhash<usize>(&k.render_pass, sizeof(VkRenderPass));
			h = memhash<usize>(k.color_attachments, sizeof(RHI::IRenderTargetView*) * 8, h);
			h = memhash<usize>(k.resolve_attachments, sizeof(RHI::IResolveTargetView*) * 8, h);
			h = memhash<usize>(&k.depth_stencil_attachment, sizeof(RHI::IDepthStencilView*), h);
			return h;
		}
	};

	namespace RHI
	{
		struct Device;
		struct RenderPassPool
		{
			VkDevice m_device;
			PFN_vkCreateRenderPass m_vkCreateRenderPass;
			PFN_vkDestroyRenderPass m_vkDestroyRenderPass;
			PFN_vkCreateFramebuffer m_vkCreateFramebuffer;
			PFN_vkDestroyFramebuffer m_vkDestroyFramebuffer;

			HashMap<RenderPassKey, VkRenderPass> m_render_passes;
			HashMap<FrameBufferKey, VkFramebuffer> m_framebuffers;

			R<VkRenderPass> get_render_pass(const RenderPassKey& key);
			R<VkFramebuffer> get_frame_buffer(const FrameBufferKey& key);
			
			~RenderPassPool();
		};
	}
}