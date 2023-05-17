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

namespace Luna
{
	namespace RHI
	{
		RV BufferResource::init_as_committed(const BufferDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				D3D12_RESOURCE_DESC rd = encode_buffer_desc(desc);
				D3D12MA::ALLOCATION_DESC allocation_desc{};
				allocation_desc.HeapType = encode_heap_type(desc.heap_type);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				if (desc.heap_type == ResourceHeapType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (desc.heap_type == ResourceHeapType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				m_memory = new_object<DeviceMemory>();
				m_memory->m_device = m_device;
				luexp(encode_hresult(m_device->m_allocator->CreateResource(
					&allocation_desc,
					&rd, state, NULL, &m_memory->m_allocation, IID_PPV_ARGS(&m_res)
				)));
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
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				if (desc.heap_type == ResourceHeapType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (desc.heap_type == ResourceHeapType::readback)
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
		R<void*> BufferResource::map(usize read_begin, usize read_end)
		{
			lutsassert();
			void* out_data = nullptr;
			lutry
			{
				D3D12_RANGE range;
				range.Begin = read_begin;
				// Only buffers can be mapped, so we can read the resource size directly.
				range.End = min(read_end, (usize)m_desc.size);
				luexp(encode_hresult(m_res->Map(0, &range, &out_data)));
			}
			lucatchret;
			return out_data;
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
		RV TextureResource::init_as_committed(const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			lutry
			{
				m_desc = desc;
				D3D12_RESOURCE_DESC rd = encode_texture_desc(desc);
				D3D12MA::ALLOCATION_DESC allocation_desc{};
				allocation_desc.HeapType = encode_heap_type(desc.heap_type);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				D3D12_CLEAR_VALUE* pcv = NULL;
				D3D12_CLEAR_VALUE cv;
				if (optimized_clear_value)
				{
					cv.Format = encode_pixel_format(optimized_clear_value->pixel_format);
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
				if (desc.heap_type == ResourceHeapType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (desc.heap_type == ResourceHeapType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				m_memory = new_object<DeviceMemory>();
				m_memory->m_device = m_device;
				luexp(encode_hresult(m_device->m_allocator->CreateResource(
					&allocation_desc,
					&rd, state, pcv, &m_memory->m_allocation, IID_PPV_ARGS(&m_res)
				)));
				post_init();
			}
			lucatchret;
			return ok;
		}
		RV TextureResource::init_as_aliasing(const TextureDesc& desc, DeviceMemory* memory, const ClearValue* optimized_clear_value)
		{
			lutry
			{
				m_desc = desc;
				D3D12_RESOURCE_DESC rd = encode_texture_desc(desc);
				D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
				D3D12_CLEAR_VALUE* pcv = NULL;
				D3D12_CLEAR_VALUE cv;
				if (optimized_clear_value)
				{
					cv.Format = encode_pixel_format(optimized_clear_value->pixel_format);
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
				if (desc.heap_type == ResourceHeapType::upload)
				{
					state = D3D12_RESOURCE_STATE_GENERIC_READ;
				}
				else if (desc.heap_type == ResourceHeapType::readback)
				{
					state = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				luexp(encode_hresult(m_device->m_allocator->CreateAliasingResource(
					memory->m_allocation.Get(), 0,
					&rd, state, NULL, IID_PPV_ARGS(&m_res))));
				m_memory = memory;
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