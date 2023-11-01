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
#include <dxgi1_5.h>
#include "../DXGI/Common.hpp"
#include "../../CommandBuffer.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include "../RHI.hpp"
#include <D3D12MemAlloc.h>

namespace Luna
{
	namespace RHI
	{
		//! Calculates the subresource index of the specified subresource.
		inline constexpr u32 calc_subresource_index(u32 mip_slice, u32 array_slice, u32 mip_levels)
		{
			return mip_slice + array_slice * mip_levels;
		}
		//! Calculates the mipmap slice and array slice from subresource index.
		inline constexpr void calc_mip_array_slice(u32 subresource, u32 mip_levels, u32& mip_slice, u32& array_slice)
		{
			mip_slice = subresource % mip_levels;
			array_slice = subresource / mip_levels;
		}
		inline D3D12_RESOURCE_STATES encode_buffer_state(BufferStateFlag s)
		{
			D3D12_RESOURCE_STATES r = D3D12_RESOURCE_STATE_COMMON;
			if (test_flags(s, BufferStateFlag::indirect_argument)) r |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
			if (test_flags(s, BufferStateFlag::vertex_buffer) || 
				test_flags(s, BufferStateFlag::uniform_buffer_cs) ||
				test_flags(s, BufferStateFlag::uniform_buffer_vs) ||
				test_flags(s, BufferStateFlag::uniform_buffer_ps))  r |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			if (test_flags(s, BufferStateFlag::index_buffer)) r |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
			if (test_flags(s, BufferStateFlag::shader_write_ps) ||
				test_flags(s, BufferStateFlag::shader_write_cs)) r |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			if((r & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == 0)
			{
				if (test_flags(s, BufferStateFlag::shader_read_vs) ||
					test_flags(s, BufferStateFlag::shader_read_cs)) r |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				if (test_flags(s, BufferStateFlag::shader_read_ps)) r |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}
			if (test_flags(s, BufferStateFlag::copy_dest)) r |= D3D12_RESOURCE_STATE_COPY_DEST;
			if (test_flags(s, BufferStateFlag::copy_source)) r |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			return r;
		}
		inline D3D12_RESOURCE_STATES encode_texture_state(TextureStateFlag s)
		{
			D3D12_RESOURCE_STATES r = D3D12_RESOURCE_STATE_COMMON;
			if (test_flags(s, TextureStateFlag::color_attachment_read) ||
				test_flags(s, TextureStateFlag::color_attachment_write)) r |= D3D12_RESOURCE_STATE_RENDER_TARGET;
			if (test_flags(s, TextureStateFlag::depth_stencil_attachment_write)) r |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			if (test_flags(s, TextureStateFlag::depth_stencil_attachment_read) &&
				!test_flags(s, TextureStateFlag::depth_stencil_attachment_write)) r |= D3D12_RESOURCE_STATE_DEPTH_READ;
			if (test_flags(s, TextureStateFlag::resolve_attachment)) r |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
			if (test_flags(s, TextureStateFlag::shader_write_ps) ||
				test_flags(s, TextureStateFlag::shader_write_cs)) r |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			if((r & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == 0)
			{
				if (test_flags(s, TextureStateFlag::shader_read_vs) ||
					test_flags(s, TextureStateFlag::shader_read_cs)) r |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				if (test_flags(s, TextureStateFlag::shader_read_ps)) r |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}
			if (test_flags(s, TextureStateFlag::copy_dest)) r |= D3D12_RESOURCE_STATE_COPY_DEST;
			if (test_flags(s, TextureStateFlag::copy_source)) r |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			if (test_flags(s, TextureStateFlag::present)) r |= D3D12_RESOURCE_STATE_PRESENT;
			return r;
		}
		inline D3D12_FILTER encode_filter(Filter min_filter, Filter mag_filter, Filter mip_filter, bool anisotropic, bool comparison)
		{
			if(anisotropic) return comparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
			if(min_filter == Filter::nearest)
			{
				if(mag_filter == Filter::nearest)
				{
					if(mip_filter == Filter::nearest) return comparison ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
					else if(mip_filter == Filter::linear) return comparison ? D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				}
				else if(mag_filter == Filter::linear)
				{
					if(mip_filter == Filter::nearest) return comparison ? D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
					else if(mip_filter == Filter::linear) return comparison ? D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR : D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				}
			}
			else if(min_filter == Filter::linear)
			{
				if(mag_filter == Filter::nearest)
				{
					if(mip_filter == Filter::nearest) return comparison ? D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
					else if(mip_filter == Filter::linear) return comparison ? D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				}
				else if(mag_filter == Filter::linear)
				{
					if(mip_filter == Filter::nearest) return comparison ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
					else if(mip_filter == Filter::linear) return comparison ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				}
			}
			lupanic();
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
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
			default:
				lupanic();
				return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			}
		}
		inline D3D12_COMPARISON_FUNC encode_compare_function(CompareFunction c)
		{
			switch (c)
			{
			case CompareFunction::never:
				return D3D12_COMPARISON_FUNC_NEVER;
			case CompareFunction::less:
				return D3D12_COMPARISON_FUNC_LESS;
			case CompareFunction::equal:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case CompareFunction::less_equal:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case CompareFunction::greater:
				return D3D12_COMPARISON_FUNC_GREATER;
			case CompareFunction::not_equal:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case CompareFunction::greater_equal:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case CompareFunction::always:
			default:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			}
		}
		inline D3D12_RESOURCE_DESC encode_buffer_desc(const BufferDesc& desc)
		{
			D3D12_RESOURCE_DESC rd {};
			rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rd.Alignment = 0;
			rd.Width = desc.size;
			rd.Height = 1;
			rd.DepthOrArraySize = 1;
			rd.MipLevels = 1;
			rd.Format = DXGI_FORMAT_UNKNOWN;
			rd.SampleDesc.Count = 1;
			rd.SampleDesc.Quality = 0;
			rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			rd.Flags = D3D12_RESOURCE_FLAG_NONE;
			if ((desc.usages & BufferUsageFlag::read_write_buffer) != BufferUsageFlag::none)
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			return rd;
		}
		inline D3D12_RESOURCE_DESC encode_texture_desc(const TextureDesc& desc)
		{
			D3D12_RESOURCE_DESC rd;
			switch (desc.type)
			{
			case TextureType::tex1d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
				break;
			case TextureType::tex2d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				break;
			case TextureType::tex3d:
				rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
				break;
			default:
				lupanic();
				rd.Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
				break;
			}
			rd.Alignment = 0;
			rd.Width = desc.width;
			rd.Height = desc.height;
			rd.DepthOrArraySize = (desc.type == TextureType::tex3d) ? desc.depth : desc.array_size;
			rd.MipLevels = desc.mip_levels;
			rd.Format = encode_format(desc.format);
			rd.SampleDesc.Count = desc.sample_count;
			rd.SampleDesc.Quality = (desc.sample_count == 1) ? 0 : 1;
			rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			rd.Flags = D3D12_RESOURCE_FLAG_NONE;
			if (test_flags(desc.usages, TextureUsageFlag::color_attachment))
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			if (test_flags(desc.usages, TextureUsageFlag::depth_stencil_attachment))
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			if (test_flags(desc.usages, TextureUsageFlag::read_write_texture))
			{
				rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			return rd;
		}
		inline D3D12_HEAP_TYPE encode_memory_type(MemoryType memory_type)
		{
			switch (memory_type)
			{
			case MemoryType::local: return D3D12_HEAP_TYPE_DEFAULT;
			case MemoryType::upload: return D3D12_HEAP_TYPE_UPLOAD;
			case MemoryType::readback: return D3D12_HEAP_TYPE_READBACK;
			default: lupanic(); return D3D12_HEAP_TYPE_DEFAULT;
			}
		}
		inline D3D12_COMMAND_LIST_TYPE encode_command_queue_type(CommandQueueType t)
		{
			switch (t)
			{
			case CommandQueueType::graphics:
				return D3D12_COMMAND_LIST_TYPE_DIRECT;
				break;
			case CommandQueueType::compute:
				return D3D12_COMMAND_LIST_TYPE_COMPUTE;
				break;
			case CommandQueueType::copy:
				return D3D12_COMMAND_LIST_TYPE_COPY;
				break;
			default:
				lupanic();
				break;
			}
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
		inline void set_object_name(ID3D12Object* object, const c8* name)
		{
			usize len = utf8_to_utf16_len(name);
			wchar_t* buf = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
			utf8_to_utf16((c16*)buf, len + 1, name);
			object->SetName(buf);
		}
		inline RV encode_hresult(HRESULT code)
		{
			switch(code)
			{
			case S_OK: return ok;
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
		extern ComPtr<IDXGIFactory5> g_dxgi;
		extern Ref<IDevice> g_main_device;
	}
}