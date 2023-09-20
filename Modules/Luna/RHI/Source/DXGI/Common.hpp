/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Common.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>

#ifdef LUNA_PLATFORM_WINDOWS

#include "../../Texture.hpp"

#include <wrl/client.h>
#include <dxgi.h>
using Microsoft::WRL::ComPtr;

namespace Luna
{
	namespace RHI
	{
		inline DXGI_FORMAT encode_format(Format f)
		{
			switch (f)
			{
			case Format::unknown:
				return DXGI_FORMAT_UNKNOWN;
			case Format::r8_unorm:
				return DXGI_FORMAT_R8_UNORM;
			case Format::r8_snorm:
				return DXGI_FORMAT_R8_SNORM;
			case Format::r8_uint:
				return DXGI_FORMAT_R8_UINT;
			case Format::r8_sint:
				return DXGI_FORMAT_R8_SINT;

			case Format::r16_unorm:
				return DXGI_FORMAT_R16_UNORM;
			case Format::r16_snorm:
				return DXGI_FORMAT_R16_SNORM;
			case Format::r16_uint:
				return DXGI_FORMAT_R16_UINT;
			case Format::r16_sint:
				return DXGI_FORMAT_R16_SINT;
			case Format::r16_float:
				return DXGI_FORMAT_R16_FLOAT;
			case Format::rg8_unorm:
				return DXGI_FORMAT_R8G8_UNORM;
			case Format::rg8_snorm:
				return DXGI_FORMAT_R8G8_SNORM;
			case Format::rg8_uint:
				return DXGI_FORMAT_R8G8_UINT;
			case Format::rg8_sint:
				return DXGI_FORMAT_R8G8_SINT;

			case Format::r32_uint:
				return DXGI_FORMAT_R32_UINT;
			case Format::r32_sint:
				return DXGI_FORMAT_R32_SINT;
			case Format::r32_float:
				return DXGI_FORMAT_R32_FLOAT;

			case Format::rg16_unorm:
				return DXGI_FORMAT_R16G16_UNORM;
			case Format::rg16_snorm:
				return DXGI_FORMAT_R16G16_SNORM;
			case Format::rg16_uint:
				return DXGI_FORMAT_R16G16_UINT;
			case Format::rg16_sint:
				return DXGI_FORMAT_R16G16_SINT;
			case Format::rg16_float:
				return DXGI_FORMAT_R16G16_FLOAT;
			case Format::rgba8_unorm:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case Format::rgba8_unorm_srgb:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case Format::rgba8_snorm:
				return DXGI_FORMAT_R8G8B8A8_SNORM;
			case Format::rgba8_uint:
				return DXGI_FORMAT_R8G8B8A8_UINT;
			case Format::rgba8_sint:
				return DXGI_FORMAT_R8G8B8A8_SINT;
			case Format::bgra8_unorm:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			case Format::bgra8_unorm_srgb:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case Format::rg32_uint:
				return DXGI_FORMAT_R32G32_UINT;
			case Format::rg32_sint:
				return DXGI_FORMAT_R32G32_SINT;
			case Format::rg32_float:
				return DXGI_FORMAT_R32G32_FLOAT;
			case Format::rgba16_unorm:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case Format::rgba16_snorm:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
			case Format::rgba16_uint:
				return DXGI_FORMAT_R16G16B16A16_UINT;
			case Format::rgba16_sint:
				return DXGI_FORMAT_R16G16B16A16_SINT;
			case Format::rgba16_float:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case Format::rgb32_uint:
				return DXGI_FORMAT_R32G32B32_UINT;
			case Format::rgb32_sint:
				return DXGI_FORMAT_R32G32B32_SINT;
			case Format::rgb32_float:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case Format::rgba32_uint:
				return DXGI_FORMAT_R32G32B32A32_UINT;
			case Format::rgba32_sint:
				return DXGI_FORMAT_R32G32B32A32_SINT;
			case Format::rgba32_float:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case Format::b5g6r5_unorm:
				return DXGI_FORMAT_B5G6R5_UNORM;
			case Format::bgr5a1_unorm:
				return DXGI_FORMAT_B5G5R5A1_UNORM;

			case Format::rgb10a2_unorm:
				return DXGI_FORMAT_R10G10B10A2_UNORM;
			case Format::rgb10a2_uint:
				return DXGI_FORMAT_R10G10B10A2_UINT;
			case Format::rg11b10_float:
				return DXGI_FORMAT_R11G11B10_FLOAT;
			case Format::rgb9e5_float:
				return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

			case Format::d16_unorm:
				return DXGI_FORMAT_D16_UNORM;
			case Format::d32_float:
				return DXGI_FORMAT_D32_FLOAT;
			case Format::d24_unorm_s8_uint:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case Format::d32_float_s8_uint_x24:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			
			case Format::bc1_rgba_unorm:
				return DXGI_FORMAT_BC1_UNORM;
			case Format::bc1_rgba_unorm_srgb:
				return DXGI_FORMAT_BC1_UNORM_SRGB;
			case Format::bc2_rgba_unorm:
				return DXGI_FORMAT_BC2_UNORM;
			case Format::bc2_rgba_unorm_srgb:
				return DXGI_FORMAT_BC2_UNORM_SRGB;
			case Format::bc3_rgba_unorm:
				return DXGI_FORMAT_BC3_UNORM;
			case Format::bc3_rgba_unorm_srgb:
				return DXGI_FORMAT_BC3_UNORM_SRGB;
			case Format::bc4_r_unorm:
				return DXGI_FORMAT_BC4_UNORM;
			case Format::bc4_r_snorm:
				return DXGI_FORMAT_BC4_SNORM;
			case Format::bc5_rg_unorm:
				return DXGI_FORMAT_BC5_UNORM;
			case Format::bc5_rg_snorm:
				return DXGI_FORMAT_BC5_SNORM;
			case Format::bc6h_rgb_sfloat:
				return DXGI_FORMAT_BC6H_SF16;
			case Format::bc6h_rgb_ufloat:
				return DXGI_FORMAT_BC6H_UF16;
			case Format::bc7_rgba_unorm:
				return DXGI_FORMAT_BC7_UNORM;
			case Format::bc7_rgba_unorm_srgb:
				return DXGI_FORMAT_BC7_UNORM_SRGB;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		inline Format decode_format(DXGI_FORMAT f)
		{
			switch (f)
			{
			case DXGI_FORMAT_UNKNOWN:
				return Format::unknown;

			case DXGI_FORMAT_R8_UNORM:
				return Format::r8_unorm;
			case DXGI_FORMAT_R8_SNORM:
				return Format::r8_snorm;
			case DXGI_FORMAT_R8_UINT:
				return Format::r8_uint;
			case DXGI_FORMAT_R8_SINT:
				return Format::r8_sint;

			case DXGI_FORMAT_R16_UNORM:
				return Format::r16_unorm;
			case DXGI_FORMAT_R16_SNORM:
				return Format::r16_snorm;
			case DXGI_FORMAT_R16_UINT:
				return Format::r16_uint;
			case DXGI_FORMAT_R16_SINT:
				return Format::r16_sint;
			case DXGI_FORMAT_R16_FLOAT:
				return Format::r16_float;
			case DXGI_FORMAT_R8G8_UNORM:
				return Format::rg8_unorm;
			case DXGI_FORMAT_R8G8_SNORM:
				return Format::rg8_snorm;
			case DXGI_FORMAT_R8G8_UINT:
				return Format::rg8_uint;
			case DXGI_FORMAT_R8G8_SINT:
				return Format::rg8_sint;

			case DXGI_FORMAT_R32_UINT:
				return Format::r32_uint;
			case DXGI_FORMAT_R32_SINT:
				return Format::r32_sint;
			case DXGI_FORMAT_R32_FLOAT:
				return Format::r32_float;
			case DXGI_FORMAT_R16G16_UNORM:
				return Format::rg16_unorm;
			case DXGI_FORMAT_R16G16_SNORM:
				return Format::rg16_snorm;
			case DXGI_FORMAT_R16G16_UINT:
				return Format::rg16_uint;
			case DXGI_FORMAT_R16G16_SINT:
				return Format::rg16_sint;
			case DXGI_FORMAT_R16G16_FLOAT:
				return Format::rg16_float;
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				return Format::rgba8_unorm;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				return Format::rgba8_unorm_srgb;
			case DXGI_FORMAT_R8G8B8A8_SNORM:
				return Format::rgba8_snorm;
			case DXGI_FORMAT_R8G8B8A8_UINT:
				return Format::rgba8_uint;
			case DXGI_FORMAT_R8G8B8A8_SINT:
				return Format::rgba8_sint;
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				return Format::bgra8_unorm;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				return Format::bgra8_unorm_srgb;

			case DXGI_FORMAT_R32G32_UINT:
				return Format::rg32_uint;
			case DXGI_FORMAT_R32G32_SINT:
				return Format::rg32_sint;
			case DXGI_FORMAT_R32G32_FLOAT:
				return Format::rg32_float;
			case DXGI_FORMAT_R16G16B16A16_UNORM:
				return Format::rgba16_unorm;
			case DXGI_FORMAT_R16G16B16A16_SNORM:
				return Format::rgba16_snorm;
			case DXGI_FORMAT_R16G16B16A16_UINT:
				return Format::rgba16_uint;
			case DXGI_FORMAT_R16G16B16A16_SINT:
				return Format::rgba16_sint;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
				return Format::rgba16_float;

			case DXGI_FORMAT_R32G32B32_UINT:
				return Format::rgb32_uint;
			case DXGI_FORMAT_R32G32B32_SINT:
				return Format::rgb32_sint;
			case DXGI_FORMAT_R32G32B32_FLOAT:
				return Format::rgb32_float;

			case DXGI_FORMAT_R32G32B32A32_UINT:
				return Format::rgba32_uint;
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return Format::rgba32_sint;
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
				return Format::rgba32_float;

			case DXGI_FORMAT_B5G6R5_UNORM:
				return Format::b5g6r5_unorm;
			case DXGI_FORMAT_B5G5R5A1_UNORM:
				return Format::bgr5a1_unorm;

			case DXGI_FORMAT_R10G10B10A2_UNORM:
				return Format::rgb10a2_unorm;
			case DXGI_FORMAT_R10G10B10A2_UINT:
				return Format::rgb10a2_uint;
			case DXGI_FORMAT_R11G11B10_FLOAT:
				return Format::rg11b10_float;
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
				return Format::rgb9e5_float;
			
			case DXGI_FORMAT_D16_UNORM:
				return Format::d16_unorm;
			case DXGI_FORMAT_D32_FLOAT:
				return Format::d32_float;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
				return Format::d24_unorm_s8_uint;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
				return Format::d32_float_s8_uint_x24;
			
			case DXGI_FORMAT_BC1_UNORM:
				return Format::bc1_rgba_unorm;
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				return Format::bc1_rgba_unorm_srgb;
			case DXGI_FORMAT_BC2_UNORM:
				return Format::bc2_rgba_unorm;
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				return Format::bc2_rgba_unorm_srgb;
			case DXGI_FORMAT_BC3_UNORM:
				return Format::bc3_rgba_unorm;
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				return Format::bc3_rgba_unorm_srgb;
			case DXGI_FORMAT_BC4_UNORM:
				return Format::bc4_r_unorm;
			case DXGI_FORMAT_BC4_SNORM:
				return Format::bc4_r_snorm;
			case DXGI_FORMAT_BC5_UNORM:
				return Format::bc5_rg_unorm;
			case DXGI_FORMAT_BC5_SNORM:
				return Format::bc5_rg_snorm;
			
			case DXGI_FORMAT_BC6H_UF16:
				return Format::bc6h_rgb_sfloat;
			case DXGI_FORMAT_BC6H_SF16:
				return Format::bc6h_rgb_ufloat;
			case DXGI_FORMAT_BC7_UNORM:
				return Format::bc7_rgba_unorm;
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return Format::bc7_rgba_unorm_srgb;
			default:
				return Format::unknown;
			}
		}
	}
}

#endif