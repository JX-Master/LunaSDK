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
			case Format::unknown: return VK_FORMAT_UNDEFINED;
			case Format::r8_unorm: return VK_FORMAT_R8_UNORM;
			case Format::r8_snorm: return VK_FORMAT_R8_SNORM;
			case Format::r8_uint: return VK_FORMAT_R8_UINT;
			case Format::r8_sint: return VK_FORMAT_R8_SINT;

			case Format::r16_unorm: return VK_FORMAT_R16_UNORM;
			case Format::r16_snorm: return VK_FORMAT_R16_SNORM;
			case Format::r16_uint: return VK_FORMAT_R16_UINT;
			case Format::r16_sint: return VK_FORMAT_R16_SINT;
			case Format::r16_float: return VK_FORMAT_R16_SFLOAT;
			case Format::rg8_unorm: return VK_FORMAT_R8G8_UNORM;
			case Format::rg8_snorm: return VK_FORMAT_R8G8_SNORM;
			case Format::rg8_uint: return VK_FORMAT_R8G8_UINT;
			case Format::rg8_sint: return VK_FORMAT_R8G8_SINT;

			case Format::r32_uint: return VK_FORMAT_R32_UINT;
			case Format::r32_sint: return VK_FORMAT_R32_SINT;
			case Format::r32_float: return VK_FORMAT_R32_SFLOAT;

			case Format::rg16_unorm: return VK_FORMAT_R16G16_UNORM;
			case Format::rg16_snorm: return VK_FORMAT_R16G16_SNORM;
			case Format::rg16_uint: return VK_FORMAT_R16G16_UINT;
			case Format::rg16_sint: return VK_FORMAT_R16G16_SINT;
			case Format::rg16_float: return VK_FORMAT_R16G16_SFLOAT;
			case Format::rgba8_unorm: return VK_FORMAT_R8G8B8A8_UNORM;
			case Format::rgba8_unorm_srgb: return VK_FORMAT_R8G8B8A8_SRGB;
			case Format::rgba8_snorm: return VK_FORMAT_R8G8B8A8_SNORM;
			case Format::rgba8_uint: return VK_FORMAT_R8G8B8A8_UINT;
			case Format::rgba8_sint: return VK_FORMAT_R8G8B8A8_SINT;
			case Format::bgra8_unorm: return VK_FORMAT_B8G8R8A8_UNORM;
			case Format::bgra8_unorm_srgb: return VK_FORMAT_B8G8R8A8_SRGB;
			case Format::rg32_uint: return VK_FORMAT_R32G32_UINT;
			case Format::rg32_sint: return VK_FORMAT_R32G32_SINT;
			case Format::rg32_float: return VK_FORMAT_R32G32_SFLOAT;
			case Format::rgba16_unorm: return VK_FORMAT_R16G16B16A16_UNORM;
			case Format::rgba16_snorm: return VK_FORMAT_R16G16B16A16_SNORM;
			case Format::rgba16_uint: return VK_FORMAT_R16G16B16A16_UINT;
			case Format::rgba16_sint: return VK_FORMAT_R16G16B16A16_SINT;
			case Format::rgba16_float: return VK_FORMAT_R16G16B16A16_SFLOAT;
			case Format::rgba32_uint: return VK_FORMAT_R32G32B32A32_UINT;
			case Format::rgba32_sint: return VK_FORMAT_R32G32B32A32_SINT;
			case Format::rgba32_float: return VK_FORMAT_R32G32B32A32_SFLOAT;

			case Format::b5g6r5_unorm: return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case Format::bgr5a1_unorm: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

			case Format::rgb10a2_unorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case Format::rgb10a2_uint: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
			case Format::rg11b10_float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			case Format::rgb9e5_float: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

			case Format::d16_unorm: return VK_FORMAT_D16_UNORM;
			case Format::d32_float: return VK_FORMAT_D32_SFLOAT;
			case Format::d24_unorm_s8_uint: return VK_FORMAT_D24_UNORM_S8_UINT;
			case Format::d32_float_s8_uint_x24: return VK_FORMAT_D32_SFLOAT_S8_UINT;

			case Format::bc1_rgba_unorm: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case Format::bc1_rgba_unorm_srgb: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			case Format::bc2_rgba_unorm: return VK_FORMAT_BC2_UNORM_BLOCK;
			case Format::bc2_rgba_unorm_srgb: return VK_FORMAT_BC2_SRGB_BLOCK;
			case Format::bc3_rgba_unorm: return VK_FORMAT_BC3_UNORM_BLOCK;
			case Format::bc3_rgba_unorm_srgb: return VK_FORMAT_BC3_SRGB_BLOCK;
			case Format::bc4_r_unorm: return VK_FORMAT_BC4_UNORM_BLOCK;
			case Format::bc4_r_snorm: return VK_FORMAT_BC4_SNORM_BLOCK;
			case Format::bc5_rg_unorm: return VK_FORMAT_BC5_UNORM_BLOCK;
			case Format::bc5_rg_snorm: return VK_FORMAT_BC5_SNORM_BLOCK;
			case Format::bc6h_rgb_sfloat: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case Format::bc6h_rgb_ufloat: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case Format::bc7_rgba_unorm: return VK_FORMAT_BC7_UNORM_BLOCK;
			case Format::bc7_rgba_unorm_srgb: return VK_FORMAT_BC7_SRGB_BLOCK;
			default:
				lupanic();
				return VK_FORMAT_UNDEFINED;
			}
		}
		inline bool is_depth_stencil_format(Format format)
		{
			return (format == Format::d16_unorm ||
				format == Format::d24_unorm_s8_uint ||
				format == Format::d32_float ||
				format == Format::d32_float_s8_uint_x24) ? true : false;
		}
		inline VkPrimitiveTopology encode_primitive_topology(PrimitiveTopology primitive_topology)
		{
			switch (primitive_topology)
			{
			case PrimitiveTopology::point_list: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveTopology::line_list: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::line_strip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopology::triangle_list: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::triangle_strip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			}
		}
		inline VkCompareOp encode_compare_op(ComparisonFunc func)
		{
			switch (func)
			{
			case ComparisonFunc::never: return VK_COMPARE_OP_NEVER;
			case ComparisonFunc::less: return VK_COMPARE_OP_LESS;
			case ComparisonFunc::equal: return VK_COMPARE_OP_EQUAL;
			case ComparisonFunc::less_equal: return VK_COMPARE_OP_LESS_OR_EQUAL;
			case ComparisonFunc::greater: return VK_COMPARE_OP_GREATER;
			case ComparisonFunc::not_equal: return VK_COMPARE_OP_NOT_EQUAL;
			case ComparisonFunc::greater_equal: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case ComparisonFunc::always: return VK_COMPARE_OP_ALWAYS;
			}
		}
		inline VkStencilOp encode_stencil_op(StencilOp op)
		{
			switch (op)
			{
			case StencilOp::keep: return VK_STENCIL_OP_KEEP;
			case StencilOp::zero: return VK_STENCIL_OP_ZERO;
			case StencilOp::replace: return VK_STENCIL_OP_REPLACE;
			case StencilOp::incr_sat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case StencilOp::decr_sat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case StencilOp::invert: return VK_STENCIL_OP_INVERT;
			case StencilOp::incr: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case StencilOp::decr: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			}
		}
		inline VkLogicOp encode_logic_op(LogicOp op)
		{
			switch (op)
			{
			case LogicOp::clear: return VK_LOGIC_OP_CLEAR;
			case LogicOp::set: return VK_LOGIC_OP_SET;
			case LogicOp::copy: return VK_LOGIC_OP_COPY;
			case LogicOp::copy_inverted: return VK_LOGIC_OP_COPY_INVERTED;
			case LogicOp::invert: return VK_LOGIC_OP_INVERT;
			case LogicOp::and : return VK_LOGIC_OP_AND;
			case LogicOp::nand: return VK_LOGIC_OP_NAND;
			case LogicOp::or: return VK_LOGIC_OP_OR;
			case LogicOp::nor: return VK_LOGIC_OP_NOR;
			case LogicOp::xor: return VK_LOGIC_OP_XOR;
			case LogicOp::equiv: return VK_LOGIC_OP_EQUIVALENT;
			case LogicOp::and_reverse: return VK_LOGIC_OP_AND_REVERSE;
			case LogicOp::and_inverted: return VK_LOGIC_OP_AND_INVERTED;
			case LogicOp::or_reverse: return VK_LOGIC_OP_OR_REVERSE;
			case LogicOp::or_inverted: return VK_LOGIC_OP_OR_INVERTED;
			}
		}
		inline VkBlendFactor encode_blend_factor(BlendFactor factor)
		{
			switch (factor)
			{
			case BlendFactor::zero: return VK_BLEND_FACTOR_ZERO;
			case BlendFactor::one: return VK_BLEND_FACTOR_ONE;
			case BlendFactor::src_color: return VK_BLEND_FACTOR_SRC_COLOR;
			case BlendFactor::inv_src_color: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::src_alpha: return VK_BLEND_FACTOR_SRC_ALPHA;
			case BlendFactor::inv_src_alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::dest_color: return VK_BLEND_FACTOR_DST_COLOR;
			case BlendFactor::inv_dest_color: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::dest_alpha: return VK_BLEND_FACTOR_DST_ALPHA;
			case BlendFactor::inv_dest_alpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case BlendFactor::src_alpha_sat: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			case BlendFactor::blend_factor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
			case BlendFactor::inv_blend_factor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactor::src1_color: return VK_BLEND_FACTOR_SRC1_COLOR;
			case BlendFactor::inv_src1_color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
			case BlendFactor::src1_alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
			case BlendFactor::inv_src1_alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
			}
		}
		inline VkBlendOp encode_blend_op(BlendOp op)
		{
			switch (op)
			{
			case BlendOp::add: return VK_BLEND_OP_ADD;
			case BlendOp::subtract: return VK_BLEND_OP_SUBTRACT;
			case BlendOp::rev_subtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
			case BlendOp::min: return VK_BLEND_OP_MIN;
			case BlendOp::max: return VK_BLEND_OP_MAX;
			}
		}
		inline VkColorComponentFlags encode_color_component_flags(ColorWriteMask mask)
		{
			VkColorComponentFlags r = 0;
			if (test_flags(mask, ColorWriteMask::red)) r |= VK_COLOR_COMPONENT_R_BIT;
			if (test_flags(mask, ColorWriteMask::green)) r |= VK_COLOR_COMPONENT_G_BIT;
			if (test_flags(mask, ColorWriteMask::blue)) r |= VK_COLOR_COMPONENT_B_BIT;
			if (test_flags(mask, ColorWriteMask::alpha)) r |= VK_COLOR_COMPONENT_A_BIT;
			return r;
		}
		inline VkAttachmentLoadOp encode_load_op(LoadOp op)
		{
			switch (op)
			{
			case LoadOp::dont_care: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			case LoadOp::load: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case LoadOp::clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
		}
		inline VkAttachmentStoreOp encode_store_op(StoreOp op)
		{
			switch (op)
			{
			case StoreOp::dont_care: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			case StoreOp::store: return VK_ATTACHMENT_STORE_OP_STORE;
			}
		}
		inline VkSampleCountFlagBits encode_sample_count(u8 num_samples)
		{
			switch (num_samples)
			{
			case 0:
			case 1: return VK_SAMPLE_COUNT_1_BIT;
			case 2: return VK_SAMPLE_COUNT_2_BIT;
			case 4: return VK_SAMPLE_COUNT_4_BIT;
			case 8: return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			}
		}
		inline u32 calc_mip_levels(u32 width, u32 height, u32 depth)
		{
			return 1 + (u32)floorf(log2f((f32)max(width, max(height, depth))));
		}
		inline void encode_buffer_create_info(VkBufferCreateInfo& dest, const BufferDesc& desc)
		{
			dest.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			dest.size = desc.size;
			dest.usage = 0;
			if (test_flags(desc.usages, BufferUsageFlag::copy_source)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::copy_dest)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::read_buffer)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::uniform_buffer)) dest.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::read_write_buffer)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::index_buffer)) dest.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::vertex_buffer)) dest.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (test_flags(desc.usages, BufferUsageFlag::indirect_buffer)) dest.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		inline void encode_image_create_info(VkImageCreateInfo& dest, const TextureDesc& desc)
		{
			dest.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			switch (desc.type)
			{
			case TextureType::tex1d:
				dest.imageType = VK_IMAGE_TYPE_1D; break;
			case TextureType::tex2d:
				dest.imageType = VK_IMAGE_TYPE_2D; break;
			case TextureType::tex3d:
				dest.imageType = VK_IMAGE_TYPE_3D; break;
			default: lupanic();
			}
			dest.extent.width = desc.width;
			dest.extent.height = desc.height;
			dest.extent.depth = desc.depth;
			dest.mipLevels = desc.mip_levels;
			dest.arrayLayers = desc.array_size;
			dest.format = encode_format(desc.pixel_format);
			dest.tiling = VK_IMAGE_TILING_OPTIMAL;
			dest.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dest.usage = 0;
			if (test_flags(desc.usages, TextureUsageFlag::copy_source)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(desc.usages, TextureUsageFlag::copy_dest)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (test_flags(desc.usages, TextureUsageFlag::sampled_texture)) dest.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (test_flags(desc.usages, TextureUsageFlag::read_texture) ||
				test_flags(desc.usages, TextureUsageFlag::read_write_texture)) dest.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (test_flags(desc.usages, TextureUsageFlag::render_target)) dest.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (test_flags(desc.usages, TextureUsageFlag::depth_stencil)) dest.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			dest.samples = encode_sample_count(desc.sample_count);
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
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
		inline VkAccessFlags encode_access_flags(BufferStateFlag state)
		{
			VkAccessFlags f = 0;
			if (test_flags(state, BufferStateFlag::indirect_argument)) f |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
			if (test_flags(state, BufferStateFlag::vertex_buffer)) f |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			if (test_flags(state, BufferStateFlag::index_buffer)) f |= VK_ACCESS_INDEX_READ_BIT;
			if (test_flags(state, BufferStateFlag::uniform_buffer_vs) ||
				test_flags(state, BufferStateFlag::uniform_buffer_ps) ||
				test_flags(state, BufferStateFlag::uniform_buffer_cs)) f |= VK_ACCESS_UNIFORM_READ_BIT;
			if (test_flags(state, BufferStateFlag::shader_read_vs) ||
				test_flags(state, BufferStateFlag::shader_read_ps) ||
				test_flags(state, BufferStateFlag::shader_read_cs)) f |= VK_ACCESS_SHADER_READ_BIT;
			if (test_flags(state, BufferStateFlag::shader_write_cs)) f |= VK_ACCESS_SHADER_WRITE_BIT;
			if (test_flags(state, BufferStateFlag::copy_dest)) f |= VK_ACCESS_TRANSFER_WRITE_BIT;
			if (test_flags(state, BufferStateFlag::copy_source)) f |= VK_ACCESS_TRANSFER_READ_BIT;
			return f;
		}
		inline VkAccessFlags encode_access_flags(TextureStateFlag state)
		{
			VkAccessFlags f = 0;
			if (test_flags(state, TextureStateFlag::shader_read_vs) ||
				test_flags(state, TextureStateFlag::shader_read_ps) ||
				test_flags(state, TextureStateFlag::shader_read_cs)) f |= VK_ACCESS_SHADER_READ_BIT;
			if (test_flags(state, TextureStateFlag::shader_write_cs)) f |= VK_ACCESS_SHADER_WRITE_BIT;
			if (test_flags(state, TextureStateFlag::color_attachment_read)) f |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			if (test_flags(state, TextureStateFlag::color_attachment_write) ||
				test_flags(state, TextureStateFlag::resolve_attachment)) f |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			if (test_flags(state, TextureStateFlag::depth_stencil_attachment_read)) f |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			if (test_flags(state, TextureStateFlag::depth_stencil_attachment_write)) f |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			if (test_flags(state, TextureStateFlag::copy_dest)) f |= VK_ACCESS_TRANSFER_WRITE_BIT;
			if (test_flags(state, TextureStateFlag::copy_source)) f |= VK_ACCESS_TRANSFER_READ_BIT;
			return f;
		}
		inline VkImageLayout encode_image_layout(TextureStateFlag state)
		{
			if (test_flags(state, TextureStateFlag::color_attachment_read) ||
				test_flags(state, TextureStateFlag::color_attachment_write) ||
				test_flags(state, TextureStateFlag::resolve_attachment))
			{
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			// This must appear before depth_stencil_read case to handle read and write flags.
			if (test_flags(state, TextureStateFlag::depth_stencil_attachment_write))
			{
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			if (test_flags(state, TextureStateFlag::depth_stencil_attachment_read))
			{
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			if (test_flags(state, TextureStateFlag::copy_dest))
			{
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			}
			if (test_flags(state, TextureStateFlag::copy_source))
			{
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			}
			if ((test_flags(state, TextureStateFlag::shader_read_vs) ||
				 test_flags(state, TextureStateFlag::shader_read_ps) ||
				 test_flags(state, TextureStateFlag::shader_read_cs)) &&
				(!test_flags(state, TextureStateFlag::shader_write_cs))
				)
			{
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			if (test_flags(state, TextureStateFlag::present))
			{
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			return VK_IMAGE_LAYOUT_GENERAL;
		}
		inline VkPipelineStageFlags determine_pipeline_stage_flags(BufferStateFlag state, CommandQueueType queue_type)
		{
			VkPipelineStageFlags flags = 0;

			switch (queue_type)
			{
			case CommandQueueType::graphics:
			{
				if (test_flags(state, BufferStateFlag::vertex_buffer) ||
					test_flags(state, BufferStateFlag::index_buffer))
				{
					flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_vs) ||
					test_flags(state, BufferStateFlag::shader_read_vs))
				{
					flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_ps) ||
					test_flags(state, BufferStateFlag::shader_read_ps))
				{
					flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_cs) ||
					test_flags(state, BufferStateFlag::shader_read_cs) ||
					test_flags(state, BufferStateFlag::shader_write_cs))
				{
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				break;
			}
			case CommandQueueType::compute:
			{
				if (test_flags(state, BufferStateFlag::vertex_buffer) ||
					test_flags(state, BufferStateFlag::index_buffer))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_vs) ||
					test_flags(state, BufferStateFlag::shader_read_vs))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_ps) ||
					test_flags(state, BufferStateFlag::shader_read_ps))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, BufferStateFlag::uniform_buffer_cs) ||
					test_flags(state, BufferStateFlag::shader_read_cs) ||
					test_flags(state, BufferStateFlag::shader_write_cs))
				{
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				break;
			}
			case CommandQueueType::copy: 
				return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			default: break;
			}
			// Compatible with both compute and graphics queues
			if (test_flags(state, BufferStateFlag::indirect_argument))
			{
				flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			}
			if (test_flags(state, BufferStateFlag::copy_dest) ||
				test_flags(state, BufferStateFlag::copy_source))
			{
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			return flags;
		}
		VkPipelineStageFlags determine_pipeline_stage_flags(TextureStateFlag state, CommandQueueType queue_type)
		{
			VkPipelineStageFlags flags = 0;

			switch (queue_type)
			{
			case CommandQueueType::graphics:
			{
				if (test_flags(state, TextureStateFlag::shader_read_vs))
				{
					flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
				}
				if (test_flags(state, TextureStateFlag::shader_read_ps))
				{
					flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				if (test_flags(state, TextureStateFlag::shader_read_cs) ||
					test_flags(state, TextureStateFlag::shader_write_cs))
				{
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				if (test_flags(state, TextureStateFlag::color_attachment_read) ||
					test_flags(state, TextureStateFlag::color_attachment_write) ||
					test_flags(state, TextureStateFlag::resolve_attachment))
				{
					flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
				if (test_flags(state, TextureStateFlag::depth_stencil_attachment_read) ||
					test_flags(state, TextureStateFlag::depth_stencil_attachment_write))
				{
					flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				}
				break;
			}
			case CommandQueueType::compute:
			{
				if (test_flags(state, TextureStateFlag::shader_read_vs))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, TextureStateFlag::shader_read_ps))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, TextureStateFlag::color_attachment_read) ||
					test_flags(state, TextureStateFlag::color_attachment_write) ||
					test_flags(state, TextureStateFlag::resolve_attachment))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, TextureStateFlag::depth_stencil_attachment_read) ||
					test_flags(state, TextureStateFlag::depth_stencil_attachment_write))
				{
					flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
				if (test_flags(state, TextureStateFlag::shader_read_cs) ||
					test_flags(state, TextureStateFlag::shader_write_cs))
				{
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				break;
			}
			case CommandQueueType::copy:
				return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			default: break;
			}
			// Compatible with both compute and graphics queues
			if (test_flags(state, TextureStateFlag::copy_dest) ||
				test_flags(state, TextureStateFlag::copy_source))
			{
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			return flags;
		}
		VkDescriptorType encode_descriptor_type(DescriptorType type)
		{
			switch (type)
			{
			case DescriptorType::uniform_buffer_view: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
			case DescriptorType::read_buffer_view:
			case DescriptorType::read_write_buffer_view: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorType::sampled_texture_view: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case DescriptorType::read_texture_view:
			case DescriptorType::read_write_texture_view: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType::sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
			}
		}
		VkSamplerAddressMode encode_address_mode(TextureAddressMode mode)
		{
			switch (mode)
			{
			case TextureAddressMode::repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case TextureAddressMode::mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case TextureAddressMode::clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case TextureAddressMode::border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			}
		}
	}
}