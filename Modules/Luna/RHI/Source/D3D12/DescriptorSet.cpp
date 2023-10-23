/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2022/8/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		RV DescriptorSet::init(const DescriptorSetDesc& desc)
		{
			DescriptorSetLayout* layout = static_cast<DescriptorSetLayout*>(desc.layout->get_object());
			{
				m_view_heap_size = layout->m_view_heap.m_size;
				if(layout->m_view_heap.m_variable) m_view_heap_size += desc.num_variable_descriptors;
				if (m_view_heap_size) m_view_heap_offset = m_device->m_cbv_srv_uav_heap.allocate_descs(m_view_heap_size);
				else m_view_heap_offset = 0;
			}
			{
				m_sampler_heap_size = layout->m_sampler_heap.m_size;
				if (layout->m_sampler_heap.m_variable) m_sampler_heap_size += desc.num_variable_descriptors;
				if (m_sampler_heap_size) m_sampler_heap_offset = m_device->m_sampler_heap.allocate_descs(m_sampler_heap_size);
				else m_sampler_heap_offset = 0;
			}
			for (auto& i : layout->m_bindings)
			{
				m_bound_index_to_offset.insert(make_pair(i.desc.binding_slot, i.offset_in_heap));
			}
			return ok;
		}
		DescriptorSet::~DescriptorSet()
		{
			if (m_view_heap_size)
			{
				m_device->m_cbv_srv_uav_heap.free_descs(m_view_heap_offset, m_view_heap_size);
			}
			if (m_sampler_heap_size)
			{
				m_device->m_sampler_heap.free_descs(m_sampler_heap_offset, m_sampler_heap_size);
			}
		}
		RV DescriptorSet::update_descriptors(Span<const WriteDescriptorSet> writes)
		{
			for (auto& write : writes)
			{
				switch (write.type)
				{
				case DescriptorType::uniform_buffer_view:
					set_cbv_array(write.binding_slot, write.first_array_index, write.num_descs, write.buffer_views);
					break;
				case DescriptorType::read_buffer_view:
					set_buffer_srv_array(write.binding_slot, write.first_array_index, write.num_descs, write.buffer_views);
					break;
				case DescriptorType::read_write_buffer_view:
					set_buffer_uav_array(write.binding_slot, write.first_array_index, write.num_descs, write.buffer_views);
					break;
				case DescriptorType::read_texture_view:
					set_texture_srv_array(write.binding_slot, write.first_array_index, write.num_descs, write.texture_views);
					break;
				case DescriptorType::read_write_texture_view:
					set_texture_uav_array(write.binding_slot, write.first_array_index, write.num_descs, write.texture_views);
					break;
				case DescriptorType::sampler:
					set_sampler_array(write.binding_slot, write.first_array_index, write.num_descs, write.samplers);
					break;
				}
			}
			return ok;
		}
		void DescriptorSet::set_cbv_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				BufferResource* r = cast_object<BufferResource>(descs[i].buffer->get_object());
				D3D12_CONSTANT_BUFFER_VIEW_DESC d;
				d.BufferLocation = r->m_res->GetGPUVirtualAddress() + descs[i].first_element;
				d.SizeInBytes = descs[i].element_size == U32_MAX ? (u32)(r->m_desc.size - descs[i].first_element) : descs[i].element_size;
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				m_device->m_device->CreateConstantBufferView(&d, h);
			}
		}
		void DescriptorSet::set_buffer_srv_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				BufferResource* r = cast_object<BufferResource>(descs[i].buffer->get_object());
				lucheck(r);
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				const BufferViewDesc* srv = descs + i;
				D3D12_SHADER_RESOURCE_VIEW_DESC d;
				d.Format = DXGI_FORMAT_UNKNOWN;
				d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				d.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				d.Buffer.FirstElement = srv->first_element;
				d.Buffer.NumElements = srv->element_count;
				d.Buffer.StructureByteStride = srv->element_size;
				d.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				m_device->m_device->CreateShaderResourceView(r->m_res.Get(), &d, h);
			}
		}
		void DescriptorSet::set_texture_srv_array(u32 binding_slot, u32 offset, u32 num_descs, const TextureViewDesc* descs)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				TextureResource* res = cast_object<TextureResource>(descs[i].texture->get_object());
				lucheck(res);
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				TextureViewDesc srv = descs[i];
				validate_texture_view_desc(res->m_desc, srv);
				D3D12_SHADER_RESOURCE_VIEW_DESC d;
				d.Format = encode_format(srv.format);
				if (d.Format == DXGI_FORMAT_D16_UNORM) d.Format = DXGI_FORMAT_R16_UNORM;
				if (d.Format == DXGI_FORMAT_D32_FLOAT) d.Format = DXGI_FORMAT_R32_FLOAT;
				d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				switch (srv.type)
				{
				case TextureViewType::tex1d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					d.Texture1D.MipLevels = srv.mip_size;
					d.Texture1D.MostDetailedMip = srv.mip_slice;
					d.Texture1D.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::tex1darray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					d.Texture1DArray.ArraySize = srv.array_size;
					d.Texture1DArray.FirstArraySlice = srv.array_slice;
					d.Texture1DArray.MipLevels = srv.mip_size;
					d.Texture1DArray.MostDetailedMip = srv.mip_slice;
					d.Texture1DArray.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::tex2d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					d.Texture2D.MipLevels = srv.mip_size;
					d.Texture2D.MostDetailedMip = srv.mip_slice;
					d.Texture2D.PlaneSlice = 0;
					d.Texture2D.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::tex2dms:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					break;
				case TextureViewType::tex2darray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					d.Texture2DArray.ArraySize = srv.array_size;
					d.Texture2DArray.FirstArraySlice = srv.array_slice;
					d.Texture2DArray.MipLevels = srv.mip_size;
					d.Texture2DArray.MostDetailedMip = srv.mip_slice;
					d.Texture2DArray.PlaneSlice = 0;
					d.Texture2DArray.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::tex2dmsarray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					d.Texture2DMSArray.ArraySize = srv.array_size;
					d.Texture2DMSArray.FirstArraySlice = srv.array_slice;
					break;
				case TextureViewType::tex3d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					d.Texture3D.MipLevels = srv.mip_size;
					d.Texture3D.MostDetailedMip = srv.mip_slice;
					d.Texture3D.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::texcube:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					d.TextureCube.MipLevels = srv.mip_size;
					d.TextureCube.MostDetailedMip = srv.mip_slice;
					d.TextureCube.ResourceMinLODClamp = 0.0f;
					break;
				case TextureViewType::texcubearray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
					d.TextureCubeArray.First2DArrayFace = srv.array_slice;
					d.TextureCubeArray.MipLevels = srv.mip_size;
					d.TextureCubeArray.MostDetailedMip = srv.mip_slice;
					d.TextureCubeArray.NumCubes = srv.array_size / 6;
					d.TextureCubeArray.ResourceMinLODClamp = 0.0f;
					break;
				default:
					lupanic();
					break;
				}
				m_device->m_device->CreateShaderResourceView(res->m_res.Get(), &d, h);
			}
		}
		void DescriptorSet::set_buffer_uav_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				ID3D12Resource* r = cast_object<BufferResource>(descs[i].buffer->get_object())->m_res.Get();
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				const BufferViewDesc* uav = descs + i;
				D3D12_UNORDERED_ACCESS_VIEW_DESC d;
				d.Format = DXGI_FORMAT_UNKNOWN;
				d.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				d.Buffer.CounterOffsetInBytes = 0;
				d.Buffer.FirstElement = uav->first_element;
				d.Buffer.StructureByteStride = uav->element_size;
				d.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				d.Buffer.NumElements = uav->element_count;
				m_device->m_device->CreateUnorderedAccessView(r, NULL, &d, h);
			}
		}
		void DescriptorSet::set_texture_uav_array(u32 binding_slot, u32 offset, u32 num_descs, const TextureViewDesc* descs)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				TextureResource* res = cast_object<TextureResource>(descs[i].texture->get_object());
				lucheck(res);
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				TextureViewDesc uav = descs[i];
				validate_texture_view_desc(res->m_desc, uav);
				D3D12_UNORDERED_ACCESS_VIEW_DESC d;
				d.Format = encode_format(uav.format);
				if (d.Format == DXGI_FORMAT_D16_UNORM) d.Format = DXGI_FORMAT_R16_UNORM;
				if (d.Format == DXGI_FORMAT_D32_FLOAT) d.Format = DXGI_FORMAT_R32_FLOAT;
				switch (uav.type)
				{
				case TextureViewType::tex1d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					d.Texture1D.MipSlice = uav.mip_slice;
					break;
				case TextureViewType::tex1darray:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					d.Texture1DArray.ArraySize = uav.array_size;
					d.Texture1DArray.FirstArraySlice = uav.array_slice;
					d.Texture1DArray.MipSlice = uav.mip_slice;
					break;
				case TextureViewType::tex2d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					d.Texture2D.MipSlice = uav.mip_slice;
					d.Texture2D.PlaneSlice = 0;
					break;
				case TextureViewType::tex2darray:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					d.Texture2DArray.ArraySize = uav.array_size;
					d.Texture2DArray.FirstArraySlice = uav.array_slice;
					d.Texture2DArray.MipSlice = uav.mip_slice;
					d.Texture2DArray.PlaneSlice = 0;
					break;
				case TextureViewType::tex3d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					d.Texture3D.FirstWSlice = 0;
					d.Texture3D.MipSlice = uav.mip_slice;
					d.Texture3D.WSize = (UINT)-1;
					break;
				default:
					lupanic();
					break;
				}
				m_device->m_device->CreateUnorderedAccessView(res->m_res.Get(), NULL, &d, h);
			}
		}
		void DescriptorSet::set_sampler_array(u32 binding_slot, u32 offset, u32 num_samplers, const SamplerDesc* samplers)
		{
			lutsassert();
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_samplers; ++i)
			{
				const SamplerDesc& sampler = samplers[i];
				D3D12_SAMPLER_DESC d;
				d.AddressU = encode_address_mode(sampler.address_u);
				d.AddressV = encode_address_mode(sampler.address_v);
				d.AddressW = encode_address_mode(sampler.address_w);
				switch (sampler.border_color)
				{
				case BorderColor::float_0000:
				case BorderColor::int_0000:
					d.BorderColor[0] = 0.0f;
					d.BorderColor[1] = 0.0f;
					d.BorderColor[2] = 0.0f;
					d.BorderColor[3] = 0.0f;
					break;
				case BorderColor::float_0001:
				case BorderColor::int_0001:
					d.BorderColor[0] = 0.0f;
					d.BorderColor[1] = 0.0f;
					d.BorderColor[2] = 0.0f;
					d.BorderColor[3] = 1.0f;
					break;
				case BorderColor::float_1111:
				case BorderColor::int_1111:
					d.BorderColor[0] = 1.0f;
					d.BorderColor[1] = 1.0f;
					d.BorderColor[2] = 1.0f;
					d.BorderColor[3] = 1.0f;
					break;
				}
				d.ComparisonFunc = encode_compare_function(sampler.compare_function);
				d.Filter = encode_filter(sampler.min_filter, sampler.mag_filter, sampler.mip_filter, sampler.anisotropy_enable, sampler.compare_enable);
				d.MaxAnisotropy = sampler.max_anisotropy;
				d.MaxLOD = sampler.max_lod;
				d.MinLOD = sampler.min_lod;
				d.MipLODBias = 0;
				usize addr = m_device->m_sampler_heap.m_cpu_handle.ptr + (m_sampler_heap_offset + index + offset + i) * m_device->m_sampler_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				m_device->m_device->CreateSampler(&d, h);
			}
		}
	}
}