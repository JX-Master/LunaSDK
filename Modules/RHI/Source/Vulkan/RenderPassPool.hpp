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
			// Must either be `Format::unknown`, or the same as `color_formats[i]`.
			Format resolve_formats[8] = { Format::unknown };
			Format depth_stencil_format = Format::unknown;
			LoadOp color_load_ops[8] = { LoadOp::dont_care };
			StoreOp color_store_ops[8] = { StoreOp::dont_care };
			LoadOp depth_load_op = LoadOp::dont_care;
			StoreOp depth_store_op = StoreOp::dont_care;
			LoadOp stencil_load_op = LoadOp::dont_care;
			StoreOp stencil_store_op = StoreOp::dont_care;
			bool depth_stencil_read_only = true;
			u8 sample_count = 1;

			bool operator==(const RenderPassKey& rhs) const
			{
				return !memcmp(color_formats, rhs.color_formats, sizeof(Format) * 8) &&
					!memcmp(resolve_formats, rhs.resolve_formats, sizeof(Format) * 8) &&
					depth_stencil_format == rhs.depth_stencil_format &&
					!memcmp(color_load_ops, rhs.color_load_ops, sizeof(LoadOp) * 8) &&
					!memcmp(color_store_ops, rhs.color_store_ops, sizeof(StoreOp) * 8) &&
					depth_load_op == rhs.depth_load_op &&
					depth_store_op == rhs.depth_store_op &&
					stencil_load_op == rhs.stencil_load_op &&
					stencil_store_op == rhs.stencil_store_op &&
					depth_stencil_read_only == rhs.depth_stencil_read_only &&
					sample_count == rhs.sample_count;
			}
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

	namespace RHI
	{
		struct Device;
		struct RenderPassPool
		{
			VkDevice m_device;
			PFN_vkCreateRenderPass m_vkCreateRenderPass;
			PFN_vkDestroyRenderPass m_vkDestroyRenderPass;

			HashMap<RenderPassKey, VkRenderPass> m_render_passes;

			R<VkRenderPass> get_render_pass(const RenderPassKey& key);

			void clean_up();
		};
	}
}