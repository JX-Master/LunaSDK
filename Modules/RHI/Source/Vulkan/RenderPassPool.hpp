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
			Format rtv_formats[8];
			Format resolve_formats[8];
			Format dsv_format;
			LoadOp rt_load_ops[8];
			StoreOp rt_store_ops[8];
			LoadOp resolve_load_ops[8];
			StoreOp resolve_store_ops[8];
			LoadOp depth_load_op;
			StoreOp depth_store_op;
			LoadOp stencil_load_op;
			StoreOp stencil_store_op;
			u8 sample_count;
		};
	}

	template <>
	struct hash<RHI::RenderPassKey>
	{
		usize operator()(const RHI::RenderPassKey& k) const
		{
			usize h = memhash<usize>(k.rtv_formats, sizeof(RHI::Format) * 8);
			h = memhash<usize>(k.resolve_formats, sizeof(RHI::Format) * 8);
			h = memhash<usize>(&k.dsv_format, sizeof(RHI::Format), h);
			h = memhash<usize>(k.rt_load_ops, sizeof(RHI::LoadOp) * 8, h);
			h = memhash<usize>(k.rt_store_ops, sizeof(RHI::StoreOp) * 8, h);
			h = memhash<usize>(k.resolve_load_ops, sizeof(RHI::LoadOp) * 8, h);
			h = memhash<usize>(k.resolve_store_ops, sizeof(RHI::StoreOp) * 8, h);
			h = memhash<usize>(&k.depth_load_op, sizeof(RHI::LoadOp), h);
			h = memhash<usize>(&k.depth_store_op, sizeof(RHI::StoreOp), h);
			h = memhash<usize>(&k.stencil_load_op, sizeof(RHI::LoadOp), h);
			h = memhash<usize>(&k.stencil_store_op, sizeof(RHI::StoreOp), h);
			h = memhash<usize>(&k.sample_count, sizeof(u8), h);
			return h;
		}
	};

	namespace RHI
	{
		struct Device;
		struct RenderPassPool
		{
			VkDevice m_device;
			PFN_vkDestroyRenderPass m_vkDestroyRenderPass;

			HashMap<RenderPassKey, VkRenderPass> m_render_passes;

			R<VkRenderPass> get_render_pass(const RenderPassKey& key);
			
			~RenderPassPool();
		};
	}
}