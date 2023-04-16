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
		RV Resource::init_as_committed(const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			m_desc = validate_resource_desc(desc);
			D3D12_HEAP_PROPERTIES hp = encode_heap_properties(m_device.as<Device>(), m_desc.heap_type);
			D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
			D3D12_RESOURCE_DESC rd = encode_resource_desc(m_desc);

			D3D12_CLEAR_VALUE* pcv = nullptr;
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
			D3D12_RESOURCE_STATES s;
			if (desc.heap_type == ResourceHeapType::upload)
			{
				s = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (desc.heap_type == ResourceHeapType::readback)
			{
				s = D3D12_RESOURCE_STATE_COPY_DEST;
			}
			else
			{
				s = D3D12_RESOURCE_STATE_COMMON;
			}
			if (FAILED(m_device->m_device->CreateCommittedResource(&hp, flags, &rd, s,
				pcv, IID_PPV_ARGS(&m_res))))
			{
				return BasicError::bad_platform_call();
			}
			return post_init();
		}

		RV Resource::init_as_placed(ID3D12Heap* heap, UINT64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			m_desc = validate_resource_desc(desc);
			D3D12_RESOURCE_DESC rd = encode_resource_desc(m_desc);
			D3D12_CLEAR_VALUE* pcv = nullptr;
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
			D3D12_RESOURCE_STATES s;
			if (desc.heap_type == ResourceHeapType::upload)
			{
				s = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (desc.heap_type == ResourceHeapType::readback)
			{
				s = D3D12_RESOURCE_STATE_COPY_DEST;
			}
			else
			{
				s = D3D12_RESOURCE_STATE_COMMON;
			}
			if (FAILED(m_device->m_device->CreatePlacedResource(heap, heap_offset, &rd, s, pcv, IID_PPV_ARGS(&m_res))))
			{
				return BasicError::bad_platform_call();
			}
			return post_init();
		}

		RV Resource::post_init()
		{
			if (m_desc.type != ResourceType::buffer && ((m_desc.flags & ResourceFlag::simultaneous_access) == ResourceFlag::none))
			{
				m_states.resize(count_subresources(), ResourceState::common);
			}
			return ok;
		}

		RV Resource::map_subresource(u32 subresource, usize read_begin, usize read_end, void** out_data)
		{
			lutsassert();
			D3D12_RANGE range;
			range.Begin = read_begin;
			// Only buffers can be mapped, so we can read the resource size directly.
			range.End = min(read_end, (usize)m_desc.width_or_buffer_size);
			if (FAILED(m_res->Map(subresource, &range, out_data)))
			{
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		void Resource::unmap_subresource(u32 subresource, usize write_begin, usize write_end)
		{
			lutsassert();
			D3D12_RANGE range;
			range.Begin = write_begin;
			// Only buffers can be mapped, so we can read the resource size directly.
			range.End = min(write_end, (usize)m_desc.width_or_buffer_size);
			D3D12_RANGE* pRange = &range;
			m_res->Unmap(subresource, pRange);
		}
	}
}