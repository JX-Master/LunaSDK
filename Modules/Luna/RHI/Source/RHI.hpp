/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.hpp
* @author JXMaster
* @date 2019/7/10
* @brief D3D12 implementation of GraphicSystem
*/
#pragma once
#include "../RHI.hpp"

namespace Luna
{
	namespace RHI
	{
		//! Implemented by the rendering API to initialize the rendering infrastructure.
		RV render_api_init();
		//! Implemented by the rendering API to clean up the rendering infrastructure.
		void render_api_close();

		inline u32 calc_mip_levels(u32 width, u32 height, u32 depth)
		{
			return 1 + (u32)floorf(log2f((f32)max(width, max(height, depth))));
		}
		inline bool is_depth_stencil_format(Format format)
		{
			return (format == Format::d16_unorm ||
				format == Format::d24_unorm_s8_uint ||
				format == Format::d32_float ||
				format == Format::d32_float_s8_uint_x24) ? true : false;
		}
		inline RV validate_texture_desc(TextureDesc& desc)
		{
			if(desc.width == 0 || desc.height == 0 || desc.depth == 0 || desc.array_size == 0)
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: width=%u, height=%u, depth=%u and array_size=%u must not be 0.", desc.width, desc.height, desc.depth, desc.array_size);
			if(desc.type == TextureType::tex1d && (desc.height != 1 || desc.depth != 1))
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: height=%u and depth=%u must be 1 for TextureType::tex1d.", desc.height, desc.depth);
			if(desc.type == TextureType::tex2d && desc.depth != 1)
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: depth=%u must be 1 for TextureType::tex2d.", desc.depth);
			if(desc.type == TextureType::tex3d && desc.array_size != 1)
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: array_size=%u must be 1 for TextureType::tex3d.", desc.array_size);
			if(test_flags(desc.usages, TextureUsageFlag::cube) && (desc.array_size % 6) != 0)
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: array_size=%u must be times of 6 if TextureUsageFlag::cube is set.", desc.array_size);
			if(desc.type != TextureType::tex2d && desc.sample_count != 1)
				return set_error(BasicError::bad_arguments(), "Invalid TextureDesc: sample_count=%u must be 1 if type is not TextureType::tex2d.", desc.sample_count);
			if (desc.mip_levels == 0)
			{
				if (is_depth_stencil_format(desc.format))
				{
					desc.mip_levels = 1;
				}
				else
				{
					desc.mip_levels = calc_mip_levels(desc.width, desc.height, desc.depth);
				}
			}
			return ok;
		}
		inline void validate_texture_view_desc(const TextureDesc& texture_desc, TextureViewDesc& desc)
		{
			if (desc.type == TextureViewType::unspecified)
			{
				if (texture_desc.type == TextureType::tex2d)
				{
					if(texture_desc.sample_count != 1)
					{
						desc.type = texture_desc.array_size == 1 ? TextureViewType::tex2dms : TextureViewType::tex2dmsarray;
					}
					else if(test_flags(texture_desc.usages, TextureUsageFlag::cube))
					{
						desc.type = texture_desc.array_size <= 6 ? TextureViewType::texcube : TextureViewType::texcubearray;
					}
					else
					{
						desc.type = texture_desc.array_size == 1 ? TextureViewType::tex2d : TextureViewType::tex2darray;
					}
				} 
				else if (texture_desc.type == TextureType::tex3d) desc.type = TextureViewType::tex3d;
				else if (texture_desc.type == TextureType::tex1d) desc.type = texture_desc.array_size == 1 ? TextureViewType::tex1d : TextureViewType::tex1darray;
				else { lupanic(); }
			}
			if (desc.format == Format::unknown) desc.format = texture_desc.format;
			if (desc.mip_size == U32_MAX) desc.mip_size = texture_desc.mip_levels - desc.mip_slice;
			if (desc.array_size == U32_MAX) desc.array_size = texture_desc.array_size - desc.array_slice;
			if (desc.type == TextureViewType::tex1d ||
				desc.type == TextureViewType::tex2d ||
				desc.type == TextureViewType::tex3d)
			{
				desc.array_size = 1;
			}
		}
	}
}