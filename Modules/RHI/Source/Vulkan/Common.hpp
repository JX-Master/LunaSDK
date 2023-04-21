/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Common.hpp
* @author JXMaster
* @date 2022/10/27
*/
#pragma once
#include <Runtime/PlatformDefines.hpp>
#include "../../RHI.hpp"
#include <volk.h>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>
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
		// Used for Vulkan RTTI.
		struct VkStructureHeader
		{
			VkStructureType sType;
			const void* pNext;
		};
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
		inline VkFormat encode_format(Format f)
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
		inline u32 calc_mip_levels(u32 width, u32 height, u32 depth)
		{
			return 1 + (u32)floorf(log2f((f32)max(width, max(height, depth))));
		}
		inline ResourceDesc validate_resource_desc(const ResourceDesc& desc)
		{
			ResourceDesc ret = desc;
			if (ret.type == ResourceType::buffer)
			{
				ret.pixel_format = Format::unknown;
				ret.height = 1;
				ret.depth_or_array_size = 1;
				ret.mip_levels = 1;
				ret.sample_count = 1;
				ret.sample_quality = 0;
			}
			else if (ret.type == ResourceType::texture_1d)
			{
				ret.height = 1;
				ret.sample_count = 1;
				ret.sample_quality = 0;
			}
			else if (ret.type == ResourceType::texture_3d)
			{
				ret.sample_count = 1;
				ret.sample_quality = 0;
			}
			if (!ret.mip_levels)
			{
				if (ret.type != ResourceType::texture_3d)
				{
					ret.mip_levels = calc_mip_levels((u32)desc.width_or_buffer_size, desc.height, 1);
				}
				else
				{
					ret.mip_levels = calc_mip_levels((u32)desc.width_or_buffer_size, desc.height, desc.depth_or_array_size);
				}
			}
			return ret;
		}
		inline void encode_buffer_create_info(VkBufferCreateInfo& dest, const ResourceDesc& validated_desc)
		{
			dest.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			dest.size = validated_desc.width_or_buffer_size;
			dest.usage = 0;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::copy_source)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::copy_dest)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::shader_resource)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::constant_buffer)) dest.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::unordered_access)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::index_buffer)) dest.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::vertex_buffer)) dest.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::indirect_buffer)) dest.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		inline void encode_image_create_info(VkImageCreateInfo& dest, const ResourceDesc& validated_desc)
		{
			dest.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			switch (validated_desc.type)
			{
			case ResourceType::texture_1d:
				dest.imageType = VK_IMAGE_TYPE_1D; break;
			case ResourceType::texture_2d:
				dest.imageType = VK_IMAGE_TYPE_2D; break;
			case ResourceType::texture_3d:
				dest.imageType = VK_IMAGE_TYPE_3D; break;
			default: lupanic();
			}
			dest.extent.width = (u32)validated_desc.width_or_buffer_size;
			dest.extent.height =
				(validated_desc.type == ResourceType::texture_2d || validated_desc.type == ResourceType::texture_3d) ? validated_desc.height : 1;
			dest.extent.depth = validated_desc.type == ResourceType::texture_3d ? validated_desc.depth_or_array_size : 1;
			dest.mipLevels = validated_desc.mip_levels;
			dest.arrayLayers =
				(validated_desc.type == ResourceType::texture_1d || validated_desc.type == ResourceType::texture_2d) ?
				validated_desc.depth_or_array_size : 1;
			dest.format = encode_format(validated_desc.pixel_format);
			dest.tiling = VK_IMAGE_TILING_OPTIMAL;
			dest.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dest.usage = 0;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::copy_source)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::copy_dest)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::shader_resource)) dest.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::unordered_access)) dest.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::render_target)) dest.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (test_flags(validated_desc.usages, ResourceUsageFlag::depth_stencil)) dest.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			switch (validated_desc.sample_count)
			{
			case 0:
			case 1: dest.samples = VK_SAMPLE_COUNT_1_BIT; break;
			case 2: dest.samples = VK_SAMPLE_COUNT_2_BIT; break;
			case 4: dest.samples = VK_SAMPLE_COUNT_4_BIT; break;
			case 8: dest.samples = VK_SAMPLE_COUNT_8_BIT; break;
			case 16: dest.samples = VK_SAMPLE_COUNT_16_BIT; break;
			case 32: dest.samples = VK_SAMPLE_COUNT_32_BIT; break;
			case 64: dest.samples = VK_SAMPLE_COUNT_64_BIT; break;
			}
		}
		inline void encode_allocation_info(VmaAllocationCreateInfo& dest, ResourceHeapType heap_type)
		{
			switch (heap_type)
			{
			case ResourceHeapType::local:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
				dest.priority = 1.0f;
				break;
			case ResourceHeapType::upload:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
				break;
			case ResourceHeapType::readback:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
				break;
			}
		}
	}
}