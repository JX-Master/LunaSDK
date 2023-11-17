/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RHIHelper.hpp
* @author JXMaster
* @date 2023/11/6
*/
#pragma once
#include <Luna/RHI/Texture.hpp>
#include "Image.hpp"
#include "DDSImage.hpp"

namespace Luna
{
	namespace Image
	{
		inline Image::ImageFormat get_rhi_desired_format(Image::ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::r8_unorm: return ImageFormat::r8_unorm;
			case ImageFormat::rg8_unorm: return ImageFormat::rg8_unorm;
			case ImageFormat::rgb8_unorm: return ImageFormat::rgba8_unorm;
			case ImageFormat::rgba8_unorm: return ImageFormat::rgba8_unorm;
			case ImageFormat::r16_unorm: return ImageFormat::r16_unorm;
			case ImageFormat::rg16_unorm: return ImageFormat::rg16_unorm;
			case ImageFormat::rgb16_unorm: return ImageFormat::rgba16_unorm;
			case ImageFormat::rgba16_unorm: return ImageFormat::rgba16_unorm;
			case ImageFormat::r32_float: return ImageFormat::r32_float;
			case ImageFormat::rg32_float: return ImageFormat::rg32_float;
			case ImageFormat::rgb32_float: return ImageFormat::rgba32_float;
			case ImageFormat::rgba32_float: return ImageFormat::rgba32_float;
			default: lupanic(); return format;
			}
		}
		inline RHI::Format image_to_rhi_format(Image::ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::r8_unorm: return RHI::Format::r8_unorm;
			case ImageFormat::rg8_unorm: return RHI::Format::rg8_unorm;
			case ImageFormat::rgb8_unorm: return RHI::Format::rgba8_unorm;
			case ImageFormat::rgba8_unorm: return RHI::Format::rgba8_unorm;
			case ImageFormat::r16_unorm: return RHI::Format::r16_unorm;
			case ImageFormat::rg16_unorm: return RHI::Format::rg16_unorm;
			case ImageFormat::rgb16_unorm: return RHI::Format::rgba16_unorm;
			case ImageFormat::rgba16_unorm: return RHI::Format::rgba16_unorm;
			case ImageFormat::r32_float: return RHI::Format::r32_float;
			case ImageFormat::rg32_float: return RHI::Format::rg32_float;
			case ImageFormat::rgb32_float: return RHI::Format::rgba32_float;
			case ImageFormat::rgba32_float: return RHI::Format::rgba32_float;
			default: lupanic(); return RHI::Format::unknown;
			}
		}
		inline Image::ImageFormat rhi_to_image_format(RHI::Format format)
		{
			switch (format)
			{
			case RHI::Format::r8_unorm: return ImageFormat::r8_unorm;
			case RHI::Format::rg8_unorm: return ImageFormat::rg8_unorm;
			case RHI::Format::rgba8_unorm: return ImageFormat::rgba8_unorm;
			case RHI::Format::r16_unorm: return ImageFormat::r16_unorm;
			case RHI::Format::rg16_unorm: return ImageFormat::rg16_unorm;
			case RHI::Format::rgba16_unorm: return ImageFormat::rgba16_unorm;
			case RHI::Format::r32_float: return ImageFormat::r32_float;
			case RHI::Format::rg32_float: return ImageFormat::rg32_float;
			case RHI::Format::rgba32_float: return ImageFormat::rgba32_float;
			default: return ImageFormat::unkonwn;
			}
		}
		inline RHI::Format dds_to_rhi_format(Image::DDSFormat format)
		{
			switch (format)
			{
			case DDSFormat::r32g32b32a32_float: return RHI::Format::rgba32_float;
			case DDSFormat::r32g32b32a32_uint: return RHI::Format::rgba32_uint;
			case DDSFormat::r32g32b32a32_sint: return RHI::Format::rgba32_sint;
			case DDSFormat::r32g32b32_float: return RHI::Format::rgb32_float;
			case DDSFormat::r32g32b32_uint: return RHI::Format::rgb32_uint;
			case DDSFormat::r32g32b32_sint: return RHI::Format::rgb32_sint;
			case DDSFormat::r16g16b16a16_float: return RHI::Format::rgba16_float;
			case DDSFormat::r16g16b16a16_unorm: return RHI::Format::rgba16_unorm;
			case DDSFormat::r16g16b16a16_uint: return RHI::Format::rgba16_uint;
			case DDSFormat::r16g16b16a16_snorm: return RHI::Format::rgba16_snorm;
			case DDSFormat::r16g16b16a16_sint: return RHI::Format::rgba16_sint;
			case DDSFormat::r32g32_float: return RHI::Format::rg32_float;
			case DDSFormat::r32g32_uint: return RHI::Format::rg32_uint;
			case DDSFormat::r32g32_sint: return RHI::Format::rg32_sint;
			case DDSFormat::d32_float_s8x24_uint: return RHI::Format::d32_float_s8_uint_x24;
			case DDSFormat::r10g10b10a2_unorm: return RHI::Format::rgb10a2_unorm;
			case DDSFormat::r10g10b10a2_uint: return RHI::Format::rgb10a2_uint;
			case DDSFormat::r11g11b10_float: return RHI::Format::rg11b10_float;
			case DDSFormat::r8g8b8a8_unorm: return RHI::Format::rgba8_unorm;
			case DDSFormat::r8g8b8a8_unorm_srgb: return RHI::Format::rgba8_unorm_srgb;
			case DDSFormat::r8g8b8a8_uint: return RHI::Format::rgba8_uint;
			case DDSFormat::r8g8b8a8_snorm: return RHI::Format::rgba8_snorm;
			case DDSFormat::r8g8b8a8_sint: return RHI::Format::rgba8_sint;
			case DDSFormat::r16g16_float: return RHI::Format::rg16_float;
			case DDSFormat::r16g16_unorm: return RHI::Format::rg16_unorm;
			case DDSFormat::r16g16_uint: return RHI::Format::rg16_uint;
			case DDSFormat::r16g16_snorm: return RHI::Format::rg16_snorm;
			case DDSFormat::r16g16_sint: return RHI::Format::rg16_sint;
			case DDSFormat::d32_float: return RHI::Format::d32_float;
			case DDSFormat::r32_float: return RHI::Format::r32_float;
			case DDSFormat::r32_uint: return RHI::Format::r32_uint;
			case DDSFormat::r32_sint: return RHI::Format::r32_sint;
			case DDSFormat::d24_unorm_s8_uint: return RHI::Format::d24_unorm_s8_uint;
			case DDSFormat::r8g8_unorm: return RHI::Format::rg8_unorm;
			case DDSFormat::r8g8_uint: return RHI::Format::rg8_uint;
			case DDSFormat::r8g8_snorm: return RHI::Format::rg8_snorm;
			case DDSFormat::r8g8_sint: return RHI::Format::rg8_sint;
			case DDSFormat::r16_float: return RHI::Format::r16_float;
			case DDSFormat::d16_unorm: return RHI::Format::d16_unorm;
			case DDSFormat::r16_unorm: return RHI::Format::r16_unorm;
			case DDSFormat::r16_uint: return RHI::Format::r16_uint;
			case DDSFormat::r16_snorm: return RHI::Format::r16_snorm;
			case DDSFormat::r16_sint: return RHI::Format::r16_sint;
			case DDSFormat::r8_unorm: return RHI::Format::r8_unorm;
			case DDSFormat::r8_uint: return RHI::Format::r8_uint;
			case DDSFormat::r8_snorm: return RHI::Format::r8_snorm;
			case DDSFormat::r8_sint: return RHI::Format::r8_sint;
			case DDSFormat::r9g9b9e5_sharedexp: return RHI::Format::rgb9e5_float;
			case DDSFormat::bc1_unorm: return RHI::Format::bc1_rgba_unorm;
			case DDSFormat::bc1_unorm_srgb: return RHI::Format::bc1_rgba_unorm_srgb;
			case DDSFormat::bc2_unorm: return RHI::Format::bc2_rgba_unorm;
			case DDSFormat::bc2_unorm_srgb: return RHI::Format::bc2_rgba_unorm_srgb;
			case DDSFormat::bc3_unorm: return RHI::Format::bc3_rgba_unorm;
			case DDSFormat::bc3_unorm_srgb: return RHI::Format::bc3_rgba_unorm_srgb;
			case DDSFormat::bc4_unorm: return RHI::Format::bc4_r_unorm;
			case DDSFormat::bc4_snorm: return RHI::Format::bc4_r_snorm;
			case DDSFormat::bc5_unorm: return RHI::Format::bc5_rg_unorm;
			case DDSFormat::bc5_snorm: return RHI::Format::bc5_rg_snorm;
			case DDSFormat::b5g6r5_unorm: return RHI::Format::b5g6r5_unorm;
			case DDSFormat::b5g5r5a1_unorm: return RHI::Format::bgr5a1_unorm;
			case DDSFormat::b8g8r8a8_unorm: return RHI::Format::bgra8_unorm;
			case DDSFormat::b8g8r8a8_unorm_srgb: return RHI::Format::bgra8_unorm_srgb;
			case DDSFormat::bc6h_uf16: return RHI::Format::bc6h_rgb_ufloat;
			case DDSFormat::bc6h_sf16: return RHI::Format::bc6h_rgb_sfloat;
			case DDSFormat::bc7_unorm: return RHI::Format::bc7_rgba_unorm;
			case DDSFormat::bc7_unorm_srgb: return RHI::Format::bc7_rgba_unorm_srgb;
			default: return RHI::Format::unknown;
			}
		}
		inline DDSFormat rhi_to_dds_format(RHI::Format format)
		{
			switch (format)
			{
			case RHI::Format::rgba32_float:			 return DDSFormat::r32g32b32a32_float;
			case RHI::Format::rgba32_uint:			 return DDSFormat::r32g32b32a32_uint;
			case RHI::Format::rgba32_sint:			 return DDSFormat::r32g32b32a32_sint;
			case RHI::Format::rgb32_float:			 return DDSFormat::r32g32b32_float;
			case RHI::Format::rgb32_uint:			 return DDSFormat::r32g32b32_uint;
			case RHI::Format::rgb32_sint:			 return DDSFormat::r32g32b32_sint;
			case RHI::Format::rgba16_float:			 return DDSFormat::r16g16b16a16_float;
			case RHI::Format::rgba16_unorm:			 return DDSFormat::r16g16b16a16_unorm;
			case RHI::Format::rgba16_uint:			 return DDSFormat::r16g16b16a16_uint;
			case RHI::Format::rgba16_snorm:			 return DDSFormat::r16g16b16a16_snorm;
			case RHI::Format::rgba16_sint:			 return DDSFormat::r16g16b16a16_sint;
			case RHI::Format::rg32_float:			 return DDSFormat::r32g32_float;
			case RHI::Format::rg32_uint:			 return DDSFormat::r32g32_uint;
			case RHI::Format::rg32_sint:			 return DDSFormat::r32g32_sint;
			case RHI::Format::d32_float_s8_uint_x24: return DDSFormat::d32_float_s8x24_uint;
			case RHI::Format::rgb10a2_unorm:		 return DDSFormat::r10g10b10a2_unorm;
			case RHI::Format::rgb10a2_uint:			 return DDSFormat::r10g10b10a2_uint;
			case RHI::Format::rg11b10_float:		 return DDSFormat::r11g11b10_float;
			case RHI::Format::rgba8_unorm:			 return DDSFormat::r8g8b8a8_unorm;
			case RHI::Format::rgba8_unorm_srgb:		 return DDSFormat::r8g8b8a8_unorm_srgb;
			case RHI::Format::rgba8_uint:			 return DDSFormat::r8g8b8a8_uint;
			case RHI::Format::rgba8_snorm:			 return DDSFormat::r8g8b8a8_snorm;
			case RHI::Format::rgba8_sint:			 return DDSFormat::r8g8b8a8_sint;
			case RHI::Format::rg16_float:			 return DDSFormat::r16g16_float;
			case RHI::Format::rg16_unorm:			 return DDSFormat::r16g16_unorm;
			case RHI::Format::rg16_uint:			 return DDSFormat::r16g16_uint;
			case RHI::Format::rg16_snorm:			 return DDSFormat::r16g16_snorm;
			case RHI::Format::rg16_sint:			 return DDSFormat::r16g16_sint;
			case RHI::Format::d32_float:			 return DDSFormat::d32_float;
			case RHI::Format::r32_float:			 return DDSFormat::r32_float;
			case RHI::Format::r32_uint:				 return DDSFormat::r32_uint;
			case RHI::Format::r32_sint:				 return DDSFormat::r32_sint;
			case RHI::Format::d24_unorm_s8_uint:	 return DDSFormat::d24_unorm_s8_uint;
			case RHI::Format::rg8_unorm:			 return DDSFormat::r8g8_unorm;
			case RHI::Format::rg8_uint:				 return DDSFormat::r8g8_uint;
			case RHI::Format::rg8_snorm:			 return DDSFormat::r8g8_snorm;
			case RHI::Format::rg8_sint:				 return DDSFormat::r8g8_sint;
			case RHI::Format::r16_float:			 return DDSFormat::r16_float;
			case RHI::Format::d16_unorm:			 return DDSFormat::d16_unorm;
			case RHI::Format::r16_unorm:			 return DDSFormat::r16_unorm;
			case RHI::Format::r16_uint:				 return DDSFormat::r16_uint;
			case RHI::Format::r16_snorm:			 return DDSFormat::r16_snorm;
			case RHI::Format::r16_sint:				 return DDSFormat::r16_sint;
			case RHI::Format::r8_unorm:				 return DDSFormat::r8_unorm;
			case RHI::Format::r8_uint:				 return DDSFormat::r8_uint;
			case RHI::Format::r8_snorm:				 return DDSFormat::r8_snorm;
			case RHI::Format::r8_sint:				 return DDSFormat::r8_sint;
			case RHI::Format::rgb9e5_float:			 return DDSFormat::r9g9b9e5_sharedexp;
			case RHI::Format::bc1_rgba_unorm:		 return DDSFormat::bc1_unorm;
			case RHI::Format::bc1_rgba_unorm_srgb:	 return DDSFormat::bc1_unorm_srgb;
			case RHI::Format::bc2_rgba_unorm:		 return DDSFormat::bc2_unorm;
			case RHI::Format::bc2_rgba_unorm_srgb:	 return DDSFormat::bc2_unorm_srgb;
			case RHI::Format::bc3_rgba_unorm:		 return DDSFormat::bc3_unorm;
			case RHI::Format::bc3_rgba_unorm_srgb:	 return DDSFormat::bc3_unorm_srgb;
			case RHI::Format::bc4_r_unorm:			 return DDSFormat::bc4_unorm;
			case RHI::Format::bc4_r_snorm:			 return DDSFormat::bc4_snorm;
			case RHI::Format::bc5_rg_unorm:			 return DDSFormat::bc5_unorm;
			case RHI::Format::bc5_rg_snorm:			 return DDSFormat::bc5_snorm;
			case RHI::Format::b5g6r5_unorm:			 return DDSFormat::b5g6r5_unorm;
			case RHI::Format::bgr5a1_unorm:			 return DDSFormat::b5g5r5a1_unorm;
			case RHI::Format::bgra8_unorm:			 return DDSFormat::b8g8r8a8_unorm;
			case RHI::Format::bgra8_unorm_srgb:		 return DDSFormat::b8g8r8a8_unorm_srgb;
			case RHI::Format::bc6h_rgb_ufloat:		 return DDSFormat::bc6h_uf16;
			case RHI::Format::bc6h_rgb_sfloat:		 return DDSFormat::bc6h_sf16;
			case RHI::Format::bc7_rgba_unorm:		 return DDSFormat::bc7_unorm;
			case RHI::Format::bc7_rgba_unorm_srgb:	 return DDSFormat::bc7_unorm_srgb;
			default: return DDSFormat::unknown;
			}
		}
	}
}