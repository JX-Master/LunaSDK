/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file D3D12Common.hpp
* @author JXMaster
* @date 2019/8/10
*/
#pragma once
#include <d3d12.h>
#include "../DXGI/Common.hpp"
#include "../../CommandBuffer.hpp"
#include <Runtime/Unicode.hpp>
#include "../RHI.hpp"

namespace Luna
{
	namespace RHI
	{
		inline D3D12_RESOURCE_STATES encode_resource_state(ResourceState s)
		{
			switch (s)
			{
			case ResourceState::common:
				return D3D12_RESOURCE_STATE_COMMON;
			case ResourceState::vertex_and_constant_buffer:
				return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			case ResourceState::copy_dest:
				return D3D12_RESOURCE_STATE_COPY_DEST;
			case ResourceState::copy_source:
				return D3D12_RESOURCE_STATE_COPY_SOURCE;
			case ResourceState::depth_stencil_write:
				return D3D12_RESOURCE_STATE_DEPTH_WRITE; // If depth-writes are disabled, return D3D12_RESOURCE_STATE_DEPTH_WRITE
			case ResourceState::depth_stencil_read:
				return D3D12_RESOURCE_STATE_DEPTH_READ;
			case ResourceState::index_buffer:
				return D3D12_RESOURCE_STATE_INDEX_BUFFER;
			case ResourceState::indirect_argument:
				return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
				//case ResourceState::present:
					//return D3D12_RESOURCE_STATE_PRESENT;
			case ResourceState::render_target:
				return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case ResourceState::resolve_dest:
				return D3D12_RESOURCE_STATE_RESOLVE_DEST;
			case ResourceState::resolve_src:
				return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			case ResourceState::shader_resource_pixel:
				return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // Need the shader flags mask in case the SRV is used by non-PS
			case ResourceState::shader_resource_non_pixel:
				return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			case ResourceState::stream_out:
				return D3D12_RESOURCE_STATE_STREAM_OUT;
			case ResourceState::unordered_access:
				return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			default:
				lupanic();
				return D3D12_RESOURCE_STATE_COMMON;
			}
		}

		inline D3D12_FILTER encode_filter(FilterMode f)
		{
			switch (f)
			{
			case FilterMode::min_mag_mip_point:
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			case FilterMode::min_mag_point_mip_linear:
				return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			case FilterMode::min_point_mag_linear_mip_point:
				return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FilterMode::min_point_mag_mip_linear:
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			case FilterMode::min_linear_mag_mip_point:
				return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			case FilterMode::min_linear_mag_point_mip_linear:
				return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FilterMode::min_mag_linear_mip_point:
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			case FilterMode::min_mag_mip_linear:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			case FilterMode::anisotropic:
				return D3D12_FILTER_ANISOTROPIC;
			case FilterMode::comparison_min_mag_mip_point:
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			case FilterMode::comparison_min_mag_point_mip_linear:
				return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			case FilterMode::comparison_min_point_mag_linear_mip_point:
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FilterMode::comparison_min_point_mag_mip_linear:
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			case FilterMode::comparison_min_linear_mag_mip_point:
				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			case FilterMode::comparison_min_linear_mag_point_mip_linear:
				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FilterMode::comparison_min_mag_linear_mip_point:
				return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			case FilterMode::comparison_min_mag_mip_linear:
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			case FilterMode::comparison_anisotropic:
				return D3D12_FILTER_COMPARISON_ANISOTROPIC;
			case FilterMode::minimum_min_mag_mip_point:
				return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
			case FilterMode::minimum_min_mag_point_mip_linear:
				return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
			case FilterMode::minimum_min_point_mag_linear_mip_point:
				return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FilterMode::minimum_min_point_mag_mip_linear:
				return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
			case FilterMode::minimum_min_linear_mag_mip_point:
				return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
			case FilterMode::minimum_min_linear_mag_point_mip_linear:
				return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FilterMode::minimum_min_mag_linear_mip_point:
				return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
			case FilterMode::minimum_min_mag_mip_linear:
				return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
			case FilterMode::minimum_anisotropic:
				return D3D12_FILTER_MINIMUM_ANISOTROPIC;
			case FilterMode::maximum_min_mag_mip_point:
				return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
			case FilterMode::maximum_min_mag_point_mip_linear:
				return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
			case FilterMode::maximum_min_point_mag_linear_mip_point:
				return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FilterMode::maximum_min_point_mag_mip_linear:
				return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
			case FilterMode::maximum_min_linear_mag_mip_point:
				return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
			case FilterMode::maximum_min_linear_mag_point_mip_linear:
				return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FilterMode::maximum_min_mag_linear_mip_point:
				return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
			case FilterMode::maximum_min_mag_mip_linear:
				return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
			case FilterMode::maximum_anisotropic:
			default:
				lupanic();
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			}
		}

		inline D3D12_TEXTURE_ADDRESS_MODE encode_address_mode(TextureAddressMode mode)
		{
			switch (mode)
			{
			case TextureAddressMode::repeat:
				return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			case TextureAddressMode::mirror:
				return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			case TextureAddressMode::clamp:
				return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			case TextureAddressMode::border:
				return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			case TextureAddressMode::mirror_once:
			default:
				return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			}
		}

		inline D3D12_COMPARISON_FUNC encode_comparison_func(ComparisonFunc c)
		{
			switch (c)
			{
			case ComparisonFunc::never:
				return D3D12_COMPARISON_FUNC_NEVER;
			case ComparisonFunc::less:
				return D3D12_COMPARISON_FUNC_LESS;
			case ComparisonFunc::equal:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case ComparisonFunc::less_equal:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case ComparisonFunc::greater:
				return D3D12_COMPARISON_FUNC_GREATER;
			case ComparisonFunc::not_equal:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case ComparisonFunc::greater_equal:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case ComparisonFunc::always:
			default:
				return D3D12_COMPARISON_FUNC_ALWAYS;
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

		inline D3D12_RESOURCE_DESC encode_resource_desc(const ResourceDesc& desc)
		{
			D3D12_RESOURCE_DESC rd;
			switch (desc.type)
			{
			case ResourceType::buffer:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				break;
			case ResourceType::texture_1d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
				break;
			case ResourceType::texture_2d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				break;
			case ResourceType::texture_3d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
				break;
			default:
				lupanic();
				rd.Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
				break;
			}
			rd.Alignment = 0;
			rd.Width = desc.width_or_buffer_size;
			rd.Height = desc.height;
			rd.DepthOrArraySize = desc.depth_or_array_size;
			rd.MipLevels = desc.mip_levels;
			rd.Format = encode_pixel_format(desc.pixel_format);
			rd.SampleDesc.Count = desc.sample_count;
			rd.SampleDesc.Quality = desc.sample_quality;
			if (desc.type == ResourceType::buffer)
			{
				rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}
			else
			{
				rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			}
			rd.Flags = D3D12_RESOURCE_FLAG_NONE;
			if ((desc.usages & ResourceUsageFlag::render_target) != ResourceUsageFlag::none)
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			if ((desc.usages & ResourceUsageFlag::depth_stencil) != ResourceUsageFlag::none)
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			if ((desc.usages & ResourceUsageFlag::unordered_access) != ResourceUsageFlag::none)
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			if ((desc.flags & ResourceFlag::simultaneous_access) != ResourceFlag::none)
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
			}
			// The D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE flag will make Visual Studio graphic debug layer to crash.
#ifndef LUNA_PROFILE
			if (((desc.usages & ResourceUsageFlag::shader_resource) == ResourceUsageFlag::none) && ((desc.usages & ResourceUsageFlag::depth_stencil) != ResourceUsageFlag::none))
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}
#endif
			return rd;
		}

		inline void set_object_name(ID3D12Object* object, const Name& name)
		{
			usize len = utf8_to_utf16_len(name.c_str(), name.size());
			wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
			utf8_to_utf16((c16*)buf, len + 1, name.c_str(), name.size());
			object->SetName(buf);
		}

		inline ErrCode encode_d3d12_error(HRESULT code)
		{
			switch(code)
			{
			case D3D12_ERROR_ADAPTER_NOT_FOUND: 
			case DXGI_ERROR_NOT_FOUND: return BasicError::not_found();
			case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return BasicError::version_dismatch();
			case DXGI_ERROR_INVALID_CALL:
			case E_INVALIDARG: return BasicError::bad_arguments();
			case DXGI_ERROR_NONEXCLUSIVE:
			case DXGI_ERROR_WAS_STILL_DRAWING: 
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return BasicError::not_ready();
			case E_OUTOFMEMORY: return BasicError::out_of_memory();
			case E_NOTIMPL: 
			case DXGI_ERROR_UNSUPPORTED: return BasicError::not_supported();
			case DXGI_ERROR_ACCESS_DENIED: return BasicError::access_denied();
			case DXGI_ERROR_NAME_ALREADY_EXISTS:
			case DXGI_ERROR_ALREADY_EXISTS: return BasicError::already_exists();
			case DXGI_ERROR_DEVICE_HUNG: return RHIError::device_hung();
			case DXGI_ERROR_DEVICE_REMOVED: return RHIError::device_removed();
			case DXGI_ERROR_DEVICE_RESET: return RHIError::device_reset();
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return RHIError::driver_internal_error();
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return RHIError::frame_statistics_disjoint();
			case DXGI_ERROR_MORE_DATA: return BasicError::insufficient_user_buffer();
			case DXGI_ERROR_WAIT_TIMEOUT: return BasicError::timeout();
			default: return BasicError::bad_platform_call();
			}
		}
	}
}