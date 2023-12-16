/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.cpp
* @author JXMaster
* @date 2019/8/10
* @brief D3D12 implementation of Resource Object
*/
#include "Resource.hpp"
#include "../../RHI.hpp"
#include "DescriptorSet.hpp"
#include <Luna/Runtime/Profiler.hpp>
namespace Luna
{
	namespace RHI
	{
		RV BufferResource::init_as_committed(MemoryType memory_type, const BufferDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				D3D12_RESOURCE_DESC rd = encode_buffer_desc(desc);
				D3D12MA::ALLOCATION_DESC allocation_desc{};
				if (test_flags(desc.flags, ResourceFlag::allow_aliasing))
				{
					allocation_desc.Flags |= D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
				}
				allocation_desc.HeapType = encode_memory_type(memory_type);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				if (memory_type == MemoryType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (memory_type == MemoryType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				m_memory = new_object<DeviceMemory>();
				m_memory->m_device = m_device;
				m_memory->m_memory_type = memory_type;
				luexp(encode_hresult(m_device->m_allocator->CreateResource(
					&allocation_desc,
					&rd, state, NULL, &m_memory->m_allocation, IID_PPV_ARGS(&m_res)
				)));
#ifdef LUNA_MEMORY_PROFILER_ENABLED
				memory_profiler_allocate(m_memory->m_allocation.Get(), m_memory->get_size());
				memory_profiler_set_memory_domain(m_memory->m_allocation.Get(), "GPU", 3);
				if(!test_flags(desc.flags, ResourceFlag::allow_aliasing))
				{
					memory_profiler_set_memory_type(m_memory->m_allocation.Get(), "Buffer", 6);
				}
				else
				{
					memory_profiler_set_memory_type(m_memory->m_allocation.Get(), "Aliasing Memory", 15);
				}
#endif
			}
			lucatchret;
			return ok;
		}
		RV BufferResource::init_as_aliasing(const BufferDesc& desc, DeviceMemory* memory)
		{
			lutry
			{
				m_desc = desc;
				D3D12_RESOURCE_DESC rd = encode_buffer_desc(desc);
				m_desc.flags |= ResourceFlag::allow_aliasing;
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				if (memory->m_memory_type == MemoryType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (memory->m_memory_type == MemoryType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				luexp(encode_hresult(m_device->m_allocator->CreateAliasingResource(
					memory->m_allocation.Get(), 0,
					&rd, state, NULL, IID_PPV_ARGS(&m_res))));
				m_memory = memory;
			}
			lucatchret;
			return ok;
		}
		RV BufferResource::map(usize read_begin, usize read_end, void** data)
		{
			lutsassert();
			lutry
			{
				D3D12_RANGE range;
				range.Begin = read_begin;
				// Only buffers can be mapped, so we can read the resource size directly.
				range.End = min(read_end, (usize)m_desc.size);
				luexp(encode_hresult(m_res->Map(0, &range, data)));
			}
			lucatchret;
			return ok;
		}
		void BufferResource::unmap(usize write_begin, usize write_end)
		{
			lutsassert();
			D3D12_RANGE range;
			range.Begin = write_begin;
			// Only buffers can be mapped, so we can read the resource size directly.
			range.End = min(write_end, (usize)m_desc.size);
			D3D12_RANGE* pRange = &range;
			m_res->Unmap(0, pRange);
		}
		bool compare_image_view_desc(const TextureViewDesc& lhs, const TextureViewDesc& rhs)
		{
			return
				lhs.texture == rhs.texture &&
				lhs.type == rhs.type &&
				lhs.format == rhs.format &&
				lhs.mip_slice == rhs.mip_slice &&
				lhs.mip_size == rhs.mip_size &&
				lhs.array_slice == rhs.array_slice &&
				lhs.array_size == rhs.array_size;
		}
		R<ID3D12DescriptorHeap*> TextureResource::get_rtv(const TextureViewDesc& desc)
		{
			auto validated_desc = desc;
			validate_texture_view_desc(m_desc, validated_desc);
			LockGuard guard(m_views_lock);
			for (auto& v : m_rtvs)
			{
				if (compare_image_view_desc(v.first, validated_desc))
				{
					return v.second.Get();
				}
			}
			// Create a new one.
			ComPtr<ID3D12DescriptorHeap> heap;
			lutry
			{
				luset(heap, m_device->m_rtv_heap.allocate_view());
				TextureResource* reso = cast_object<TextureResource>(desc.texture->get_object());
				ID3D12Resource* res = reso->m_res.Get();
				D3D12_RENDER_TARGET_VIEW_DESC rtv;
				switch (validated_desc.type)
				{
				case TextureViewType::tex1d:
					rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
					rtv.Texture1D.MipSlice = validated_desc.mip_slice;
					break;
				case TextureViewType::tex1darray:
					rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
					rtv.Texture1DArray.ArraySize = validated_desc.array_size;
					rtv.Texture1DArray.FirstArraySlice = validated_desc.array_slice;
					rtv.Texture1DArray.MipSlice = validated_desc.mip_slice;
					break;
				case TextureViewType::tex2d:
					if (reso->m_desc.sample_count == 1)
					{
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						rtv.Texture2D.MipSlice = validated_desc.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
					}
					else
					{
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
					}
					break;
				case TextureViewType::tex2darray:
					if (reso->m_desc.sample_count == 1)
					{
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						rtv.Texture2D.MipSlice = validated_desc.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
					}
					else
					{
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
						rtv.Texture2DMSArray.ArraySize = validated_desc.array_size;
						rtv.Texture2DMSArray.FirstArraySlice = validated_desc.array_slice;
					}
					break;
				case TextureViewType::tex3d:
					rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
					rtv.Texture3D.FirstWSlice = validated_desc.array_slice;
					rtv.Texture3D.MipSlice = validated_desc.mip_slice;
					rtv.Texture3D.WSize = validated_desc.array_size;
					break;
				default:
					lupanic();
				}
				rtv.Format = encode_format(validated_desc.format);
				m_device->m_device->CreateRenderTargetView(res, &rtv, heap->GetCPUDescriptorHandleForHeapStart());
				m_rtvs.push_back(make_pair(validated_desc, heap));
			}
			lucatchret;
			return heap.Get();
		}
		R<ID3D12DescriptorHeap*> TextureResource::get_dsv(const TextureViewDesc& desc)
		{
			auto validated_desc = desc;
			validate_texture_view_desc(m_desc, validated_desc);
			LockGuard guard(m_views_lock);
			for (auto& v : m_dsvs)
			{
				if (compare_image_view_desc(v.first, validated_desc))
				{
					return v.second.Get();
				}
			}
			// Create a new one.
			ComPtr<ID3D12DescriptorHeap> heap;
			lutry
			{
				luset(heap, m_device->m_dsv_heap.allocate_view());
				TextureResource* reso = cast_object<TextureResource>(desc.texture->get_object());
				ID3D12Resource* res = reso->m_res.Get();
				D3D12_DEPTH_STENCIL_VIEW_DESC dsv;
				dsv.Format = encode_format(validated_desc.format);
				dsv.Flags = D3D12_DSV_FLAG_NONE;
				switch (validated_desc.type)
				{
				case TextureViewType::tex1d:
					dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
					dsv.Texture1D.MipSlice = validated_desc.mip_slice;
					break;
				case TextureViewType::tex1darray:
					dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					dsv.Texture1DArray.ArraySize = validated_desc.array_size;
					dsv.Texture1DArray.FirstArraySlice = validated_desc.array_slice;
					dsv.Texture1DArray.MipSlice = validated_desc.mip_slice;
					break;
				case TextureViewType::tex2d:
					if (reso->m_desc.sample_count == 1)
					{
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						dsv.Texture2D.MipSlice = validated_desc.mip_slice;
					}
					else
					{
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
					}
					break;
				case TextureViewType::tex2darray:
					if (reso->m_desc.sample_count == 1)
					{
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
						dsv.Texture2DArray.ArraySize = validated_desc.array_size;
						dsv.Texture2DArray.FirstArraySlice = validated_desc.array_slice;
						dsv.Texture2DArray.MipSlice = validated_desc.mip_slice;
					}
					else
					{
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
						dsv.Texture2DMSArray.ArraySize = validated_desc.array_size;
						dsv.Texture2DMSArray.FirstArraySlice = validated_desc.array_slice;
					}
					break;
				default:
					lupanic();
				}
				m_device->m_device->CreateDepthStencilView(res, &dsv, heap->GetCPUDescriptorHandleForHeapStart());
				m_dsvs.push_back(make_pair(validated_desc, heap));
			}
			lucatchret;
			return heap.Get();
		}
		TextureResource::~TextureResource()
		{
			for (auto& rtv : m_rtvs)
			{
				m_device->m_rtv_heap.free_view(rtv.second.Get());
			}
			for (auto& dsv : m_dsvs)
			{
				m_device->m_dsv_heap.free_view(dsv.second.Get());
			}
		}
		RV TextureResource::init_as_committed(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			lutry
			{
				m_desc = desc;
				luexp(validate_texture_desc(m_desc));
				D3D12_RESOURCE_DESC rd = encode_texture_desc(m_desc);
				D3D12MA::ALLOCATION_DESC allocation_desc{};
				if (test_flags(m_desc.flags, ResourceFlag::allow_aliasing))
				{
					allocation_desc.Flags |= D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
				}
				allocation_desc.HeapType = encode_memory_type(memory_type);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				D3D12_CLEAR_VALUE* pcv = NULL;
				D3D12_CLEAR_VALUE cv;
				if (optimized_clear_value)
				{
					cv.Format = encode_format(optimized_clear_value->format);
					if (optimized_clear_value->type == ClearValueType::color)
					{
						cv.Color[0] = optimized_clear_value->color[0];
						cv.Color[1] = optimized_clear_value->color[1];
						cv.Color[2] = optimized_clear_value->color[2];
						cv.Color[3] = optimized_clear_value->color[3];
					}
					else
					{
						cv.DepthStencil.Depth = optimized_clear_value->depth_stencil.depth;
						cv.DepthStencil.Stencil = optimized_clear_value->depth_stencil.stencil;
					}
					pcv = &cv;
				}
				if (memory_type == MemoryType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (memory_type == MemoryType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				m_memory = new_object<DeviceMemory>();
				m_memory->m_device = m_device;
				m_memory->m_memory_type = memory_type;
				luexp(encode_hresult(m_device->m_allocator->CreateResource(
					&allocation_desc,
					&rd, state, pcv, &m_memory->m_allocation, IID_PPV_ARGS(&m_res)
				)));
				auto created_desc = m_res->GetDesc();
				m_desc.mip_levels = created_desc.MipLevels;
				post_init();
#ifdef LUNA_MEMORY_PROFILER_ENABLED
				memory_profiler_allocate(m_memory->m_allocation.Get(), m_memory->get_size());
				memory_profiler_set_memory_domain(m_memory->m_allocation.Get(), "GPU", 3);
				if(!test_flags(desc.flags, ResourceFlag::allow_aliasing))
				{
					memory_profiler_set_memory_type(m_memory->m_allocation.Get(), "Texture", 7);
				}
				else
				{
					memory_profiler_set_memory_type(m_memory->m_allocation.Get(), "Aliasing Memory", 15);
				}
#endif
			}
			lucatchret;
			return ok;
		}
		RV TextureResource::init_as_aliasing(const TextureDesc& desc, DeviceMemory* memory, const ClearValue* optimized_clear_value)
		{
			lutry
			{
				m_desc = desc;
				luexp(validate_texture_desc(m_desc));
				m_desc.flags |= ResourceFlag::allow_aliasing;
				D3D12_RESOURCE_DESC rd = encode_texture_desc(m_desc);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				D3D12_CLEAR_VALUE* pcv = NULL;
				D3D12_CLEAR_VALUE cv;
				if (optimized_clear_value)
				{
					cv.Format = encode_format(optimized_clear_value->format);
					if (optimized_clear_value->type == ClearValueType::color)
					{
						cv.Color[0] = optimized_clear_value->color[0];
						cv.Color[1] = optimized_clear_value->color[1];
						cv.Color[2] = optimized_clear_value->color[2];
						cv.Color[3] = optimized_clear_value->color[3];
					}
					else
					{
						cv.DepthStencil.Depth = optimized_clear_value->depth_stencil.depth;
						cv.DepthStencil.Stencil = optimized_clear_value->depth_stencil.stencil;
					}
					pcv = &cv;
				}
				if (memory->m_memory_type == MemoryType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (memory->m_memory_type == MemoryType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				luexp(encode_hresult(m_device->m_allocator->CreateAliasingResource(
					memory->m_allocation.Get(), 0,
					&rd, state, NULL, IID_PPV_ARGS(&m_res))));
				m_memory = memory;
				auto created_desc = m_res->GetDesc();
				m_desc.mip_levels = created_desc.MipLevels;
				post_init();
			}
			lucatchret;
			return ok;
		}
		void TextureResource::post_init()
		{
			m_states.resize(count_subresources(), D3D12_RESOURCE_STATE_COMMON);
		}
	}
}