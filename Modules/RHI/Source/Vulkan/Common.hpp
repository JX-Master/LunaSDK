// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file Common.hpp
* @author JXMaster
* @date 2022/10/27
*/
#pragma once
#include <Runtime/PlatformDefines.hpp>
#include "../../RHI.hpp"
#include <vulkan/vulkan.h>
#include <Runtime/Result.hpp>

namespace Luna
{
	namespace RHI
	{
		// enabled extensions.
		constexpr const c8* VK_DEVICE_ENTENSIONS[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		constexpr usize NUM_VK_DEVICE_ENTENSIONS = sizeof(VK_DEVICE_ENTENSIONS) / sizeof(const c8*);

		inline RV encode_vk_result(VkResult result)
		{
			switch (result)
			{
			case VK_SUCCESS: return ok;
			case VK_NOT_READY: return BasicError::not_ready();
			case VK_TIMEOUT: return BasicError::timeout();
			case VK_INCOMPLETE: return BasicError::not_ready();
			case VK_ERROR_OUT_OF_HOST_MEMORY:
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				return BasicError::out_of_memory();
			case VK_ERROR_INITIALIZATION_FAILED:
				return BasicError::bad_platform_call();
			case VK_ERROR_DEVICE_LOST:
				return RHIError::device_removed();
			case VK_ERROR_LAYER_NOT_PRESENT:
			case VK_ERROR_EXTENSION_NOT_PRESENT:
			case VK_ERROR_FEATURE_NOT_PRESENT:
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				return BasicError::not_supported();
			case VK_ERROR_TOO_MANY_OBJECTS:
				return BasicError::out_of_resource();
			case VK_ERROR_FORMAT_NOT_SUPPORTED:
				return BasicError::not_supported();
			default:
				return BasicError::bad_platform_call();
			}
		}

		inline VkFormat encode_pixel_format(Format f)
		{
			switch (f)
			{
			case Format::unknown:
				return VK_FORMAT_UNDEFINED;
			case Format::r8_unorm:
				return VK_FORMAT_R8_UNORM;
			case Format::r8_snorm:
				return VK_FORMAT_R8_SNORM;
			case Format::r8_uint:
				return VK_FORMAT_R8_UINT;
			case Format::r8_sint:
				return VK_FORMAT_R8_SINT;

			case Format::r16_unorm:
				return VK_FORMAT_R16_UNORM;
			case Format::r16_snorm:
				return VK_FORMAT_R16_SNORM;
			case Format::r16_uint:
				return VK_FORMAT_R16_UINT;
			case Format::r16_sint:
				return VK_FORMAT_R16_SINT;
			case Format::r16_float:
				return VK_FORMAT_R16_SFLOAT;
			case Format::rg8_unorm:
				return VK_FORMAT_R8G8_UNORM;
			case Format::rg8_snorm:
				return VK_FORMAT_R8G8_SNORM;
			case Format::rg8_uint:
				return VK_FORMAT_R8G8_UINT;
			case Format::rg8_sint:
				return VK_FORMAT_R8G8_SINT;

			case Format::r32_uint:
				return VK_FORMAT_R32_UINT;
			case Format::r32_sint:
				return VK_FORMAT_R32_SINT;
			case Format::r32_float:
				return VK_FORMAT_R32_SFLOAT;

			case Format::rg16_unorm:
				return VK_FORMAT_R16G16_UNORM;
			case Format::rg16_snorm:
				return VK_FORMAT_R16G16_SNORM;
			case Format::rg16_uint:
				return VK_FORMAT_R16G16_UINT;
			case Format::rg16_sint:
				return VK_FORMAT_R16G16_SINT;
			case Format::rg16_float:
				return VK_FORMAT_R16G16_SFLOAT;
			case Format::rgba8_unorm:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case Format::rgba8_unorm_srgb:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case Format::rgba8_snorm:
				return VK_FORMAT_R8G8B8A8_SNORM;
			case Format::rgba8_uint:
				return VK_FORMAT_R8G8B8A8_UINT;
			case Format::rgba8_sint:
				return VK_FORMAT_R8G8B8A8_SINT;
			case Format::bgra8_unorm:
				return VK_FORMAT_B8G8R8A8_UNORM;
			case Format::bgra8_unorm_srgb:
				return VK_FORMAT_B8G8R8A8_SRGB;
			case Format::rg32_uint:
				return VK_FORMAT_R32G32_UINT;
			case Format::rg32_sint:
				return VK_FORMAT_R32G32_SINT;
			case Format::rg32_float:
				return VK_FORMAT_R32G32_SFLOAT;
			case Format::rgba16_unorm:
				return VK_FORMAT_R16G16B16A16_UNORM;
			case Format::rgba16_snorm:
				return VK_FORMAT_R16G16B16A16_SNORM;
			case Format::rgba16_uint:
				return VK_FORMAT_R16G16B16A16_UINT;
			case Format::rgba16_sint:
				return VK_FORMAT_R16G16B16A16_SINT;
			case Format::rgba16_float:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case Format::rgba32_uint:
				return VK_FORMAT_R32G32B32A32_UINT;
			case Format::rgba32_sint:
				return VK_FORMAT_R32G32B32A32_SINT;
			case Format::rgba32_float:
				return VK_FORMAT_R32G32B32A32_SFLOAT;

			case Format::b5g6r5_unorm:
				return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case Format::bgr5a1_unorm:
				return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

			case Format::rgb10a2_unorm:
				return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case Format::rgb10a2_uint:
				return VK_FORMAT_A2B10G10R10_UINT_PACK32;
			case Format::rg11b10_float:
				return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			case Format::rgb9e5_float:
				return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

			case Format::d16_unorm:
				return VK_FORMAT_D16_UNORM;
			case Format::d32_float:
				return VK_FORMAT_D32_SFLOAT;
			case Format::d24_unorm_s8_uint:
				return VK_FORMAT_D24_UNORM_S8_UINT;
			case Format::d32_float_s8_uint_x24:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;

			case Format::bc1_rgba_unorm:
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case Format::bc1_rgba_unorm_srgb:
				return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			case Format::bc2_rgba_unorm:
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case Format::bc2_rgba_unorm_srgb:
				return VK_FORMAT_BC2_SRGB_BLOCK;
			case Format::bc3_rgba_unorm:
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case Format::bc3_rgba_unorm_srgb:
				return VK_FORMAT_BC3_SRGB_BLOCK;
			case Format::bc4_r_unorm:
				return VK_FORMAT_BC4_UNORM_BLOCK;
			case Format::bc4_r_snorm:
				return VK_FORMAT_BC4_SNORM_BLOCK;
			case Format::bc5_rg_unorm:
				return VK_FORMAT_BC5_UNORM_BLOCK;
			case Format::bc5_rg_snorm:
				return VK_FORMAT_BC5_SNORM_BLOCK;
			case Format::bc6h_rgb_sfloat:
				return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case Format::bc6h_rgb_ufloat:
				return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case Format::bc7_rgba_unorm:
				return VK_FORMAT_BC7_UNORM_BLOCK;
			case Format::bc7_rgba_unorm_srgb:
				return VK_FORMAT_BC7_SRGB_BLOCK;
			default:
				lupanic();
				return VK_FORMAT_UNDEFINED;
			}
		}
	}
}