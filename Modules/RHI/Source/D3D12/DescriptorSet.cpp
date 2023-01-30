/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2022/8/3
*/
#ifdef LUNA_RHI_D3D12
#include <Runtime/PlatformDefines.hpp>
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
		void DescriptorSet::set_cbv(u32 binding_slot, IResource* res, const ConstantBufferViewDesc& cbv)
		{
			set_cbv_array(binding_slot, 0, 1, &res, &cbv);
		}
		void DescriptorSet::set_cbv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ConstantBufferViewDesc* descs)
		{
			lutsassert(this);
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				Resource* r = static_cast<Resource*>(resources[i]->get_object());
				D3D12_CONSTANT_BUFFER_VIEW_DESC d;
				d.BufferLocation = r->m_res->GetGPUVirtualAddress() + descs[i].offset;
				d.SizeInBytes = descs[i].size;
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				m_device->m_device->CreateConstantBufferView(&d, h);
			}
		}
		LUNA_RHI_API ShaderResourceViewDesc get_default_srv_from_resource(IResource* resource)
		{
			ResourceDesc d = resource->get_desc();
			switch (d.type)
			{
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					ShaderResourceViewDesc::as_tex1d(d.pixel_format, 0, d.mip_levels, 0.0f) :
					ShaderResourceViewDesc::as_tex1darray(d.pixel_format, 0, d.mip_levels, 0, d.depth_or_array_size, 0.0f);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					((d.sample_count == 1) ?
						ShaderResourceViewDesc::as_tex2d(d.pixel_format, 0, d.mip_levels, 0.0f) :
						ShaderResourceViewDesc::as_tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						ShaderResourceViewDesc::as_tex2darray(d.pixel_format, 0, d.mip_levels, 0, d.depth_or_array_size, 0.0f) :
						ShaderResourceViewDesc::as_tex2dmsarray(d.pixel_format, 0, d.depth_or_array_size)
						);
			case ResourceType::texture_3d:
				return ShaderResourceViewDesc::as_tex3d(d.pixel_format, 0, d.mip_levels, 0.0f);
			case ResourceType::buffer:
				return ShaderResourceViewDesc::as_buffer(0, (u32)d.width_or_buffer_size, 1, false);
			default:
				break;
			}
			lupanic();
			return ShaderResourceViewDesc();
		}
		void DescriptorSet::set_srv(u32 binding_slot, IResource* res, const ShaderResourceViewDesc* srv)
		{
			set_srv_array(binding_slot, 0, 1, &res, srv ? srv : &get_default_srv_from_resource(res));
		}
		void DescriptorSet::set_srv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ShaderResourceViewDesc* descs)
		{
			lutsassert(this);
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				Resource* r = static_cast<Resource*>(resources[i]->get_object());
				lucheck(r);
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				const ShaderResourceViewDesc* srv = descs + i;
				D3D12_SHADER_RESOURCE_VIEW_DESC d;
				d.Format = encode_pixel_format(srv->format);
				d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				switch (srv->type)
				{
				case ShaderResourceViewType::buffer:
					d.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					d.Buffer.FirstElement = srv->buffer.offset;
					d.Buffer.NumElements = srv->buffer.count;
					d.Buffer.StructureByteStride = srv->buffer.element_size;
					d.Buffer.Flags = srv->buffer.raw_view ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
					break;
				case ShaderResourceViewType::tex1d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					d.Texture1D.MipLevels = srv->tex1d.mip_levels;
					d.Texture1D.MostDetailedMip = srv->tex1d.most_detailed_mip;
					d.Texture1D.ResourceMinLODClamp = srv->tex1d.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::tex1darray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					d.Texture1DArray.ArraySize = srv->tex1darray.array_size;
					d.Texture1DArray.FirstArraySlice = srv->tex1darray.first_array_slice;
					d.Texture1DArray.MipLevels = srv->tex1darray.mip_levels;
					d.Texture1DArray.MostDetailedMip = srv->tex1darray.most_detailed_mip;
					d.Texture1DArray.ResourceMinLODClamp = srv->tex1darray.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::tex2d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					d.Texture2D.MipLevels = srv->tex2d.mip_levels;
					d.Texture2D.MostDetailedMip = srv->tex2d.most_detailed_mip;
					d.Texture2D.PlaneSlice = 0;
					d.Texture2D.ResourceMinLODClamp = srv->tex2d.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::tex2darray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					d.Texture2DArray.ArraySize = srv->tex2darray.array_size;
					d.Texture2DArray.FirstArraySlice = srv->tex2darray.first_array_slice;
					d.Texture2DArray.MipLevels = srv->tex2darray.mip_levels;
					d.Texture2DArray.MostDetailedMip = srv->tex2darray.most_detailed_mip;
					d.Texture2DArray.PlaneSlice = 0;
					d.Texture2DArray.ResourceMinLODClamp = srv->tex2darray.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::tex2dms:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					break;
				case ShaderResourceViewType::tex2dmsarray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					d.Texture2DMSArray.ArraySize = srv->tex2dmsarray.array_size;
					d.Texture2DMSArray.FirstArraySlice = srv->tex2dmsarray.first_array_slice;
					break;
				case ShaderResourceViewType::tex3d:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					d.Texture3D.MipLevels = srv->tex3d.mip_levels;
					d.Texture3D.MostDetailedMip = srv->tex3d.most_detailed_mip;
					d.Texture3D.ResourceMinLODClamp = srv->tex3d.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::texcube:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					d.TextureCube.MipLevels = srv->texcube.mip_levels;
					d.TextureCube.MostDetailedMip = srv->texcube.most_detailed_mip;
					d.TextureCube.ResourceMinLODClamp = srv->texcube.resource_min_lod_clamp;
					break;
				case ShaderResourceViewType::texcubearray:
					d.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
					d.TextureCubeArray.First2DArrayFace = srv->texcubearray.first_2darray_face;
					d.TextureCubeArray.MipLevels = srv->texcubearray.mip_levels;
					d.TextureCubeArray.MostDetailedMip = srv->texcubearray.most_detailed_mip;
					d.TextureCubeArray.NumCubes = srv->texcubearray.num_cubes;
					d.TextureCubeArray.ResourceMinLODClamp = srv->texcubearray.resource_min_lod_clamp;
					break;
				default:
					lupanic();
					break;
				}
				m_device->m_device->CreateShaderResourceView(r->m_res.Get(), &d, h);
			}
		}
		LUNA_RHI_API UnorderedAccessViewDesc get_default_uav_from_resource(IResource* resource)
		{
			ResourceDesc d = resource->get_desc();
			switch (d.type)
			{
			case ResourceType::buffer:
				return UnorderedAccessViewDesc::as_buffer(Format::unknown, 0, (u32)d.width_or_buffer_size, 1, 0, false);
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					UnorderedAccessViewDesc::as_tex1d(d.pixel_format, 0) :
					UnorderedAccessViewDesc::as_tex1darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					UnorderedAccessViewDesc::as_tex2d(d.pixel_format, 0) :
					UnorderedAccessViewDesc::as_tex2darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_3d:
				return UnorderedAccessViewDesc::as_tex3d(d.pixel_format, 0, 0, d.depth_or_array_size);
			default:
				break;
			}
			lupanic();
			return UnorderedAccessViewDesc();
		}

		void DescriptorSet::set_uav(u32 binding_slot, IResource* res, IResource* counter_resource, const UnorderedAccessViewDesc* uav)
		{
			set_uav_array(binding_slot, 0, 1, &res, &counter_resource, uav ? uav : &get_default_uav_from_resource(res));
		}
		void DescriptorSet::set_uav_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, IResource** counter_resources, const UnorderedAccessViewDesc* descs)
		{
			lutsassert(this);
			auto iter = m_bound_index_to_offset.find(binding_slot);
			lucheck_msg(iter != m_bound_index_to_offset.end(), "Invalid binding slot");
			u32 index = iter->second;
			for (usize i = 0; i < num_descs; ++i)
			{
				ID3D12Resource* r = static_cast<Resource*>(resources[i]->get_object())->m_res.Get();
				ID3D12Resource* cr = counter_resources[i] ? static_cast<Resource*>(counter_resources[i]->get_object())->m_res.Get() : nullptr;
				usize addr = m_device->m_cbv_srv_uav_heap.m_cpu_handle.ptr + (m_view_heap_offset + index + offset + i) * m_device->m_cbv_srv_uav_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				const UnorderedAccessViewDesc* uav = descs + i;
				D3D12_UNORDERED_ACCESS_VIEW_DESC d;
				d.Format = encode_pixel_format(uav->format);
				switch (uav->type)
				{
				case UnorderedAccessViewType::buffer:
					d.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
					d.Buffer.CounterOffsetInBytes = uav->buffer.counter_offset_in_bytes;
					d.Buffer.FirstElement = uav->buffer.offset;
					d.Buffer.Flags = uav->buffer.raw_view ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
					d.Buffer.NumElements = uav->buffer.count;
					d.Buffer.StructureByteStride = uav->buffer.element_size;
					break;
				case UnorderedAccessViewType::tex1d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					d.Texture1D.MipSlice = uav->tex1d.mip_slice;
					break;
				case UnorderedAccessViewType::tex1darray:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					d.Texture1DArray.ArraySize = uav->tex1darray.array_size;
					d.Texture1DArray.FirstArraySlice = uav->tex1darray.first_array_slice;
					d.Texture1DArray.MipSlice = uav->tex1darray.mip_slice;
					break;
				case UnorderedAccessViewType::tex2d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					d.Texture2D.MipSlice = uav->tex2d.mip_slice;
					d.Texture2D.PlaneSlice = 0;
					break;
				case UnorderedAccessViewType::tex2darray:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					d.Texture2DArray.ArraySize = uav->tex2darray.array_size;
					d.Texture2DArray.FirstArraySlice = uav->tex2darray.first_array_slice;
					d.Texture2DArray.MipSlice = uav->tex2darray.mip_slice;
					d.Texture2DArray.PlaneSlice = 0;
					break;
				case UnorderedAccessViewType::tex3d:
					d.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					d.Texture3D.FirstWSlice = uav->tex3d.first_layer_slice;
					d.Texture3D.MipSlice = uav->tex3d.mip_slice;
					d.Texture3D.WSize = uav->tex3d.layer_size;
					break;
				default:
					lupanic();
					break;
				}
				m_device->m_device->CreateUnorderedAccessView(r, cr, &d, h);
			}
		}
		void DescriptorSet::set_sampler(u32 binding_slot, const SamplerDesc& sampler)
		{
			set_sampler_array(binding_slot, 0, 1, &sampler);
		}
		void DescriptorSet::set_sampler_array(u32 binding_slot, u32 offset, u32 num_samplers, const SamplerDesc* samplers)
		{
			lutsassert(this);
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
				d.BorderColor[0] = sampler.border_color[0];
				d.BorderColor[1] = sampler.border_color[1];
				d.BorderColor[2] = sampler.border_color[2];
				d.BorderColor[3] = sampler.border_color[3];
				d.ComparisonFunc = encode_comparison_func(sampler.comparison_func);
				d.Filter = encode_filter(sampler.filter);
				d.MaxAnisotropy = sampler.max_anisotropy;
				d.MaxLOD = sampler.max_lod;
				d.MinLOD = sampler.min_lod;
				d.MipLODBias = sampler.mip_lod_bias;
				usize addr = m_device->m_sampler_heap.m_cpu_handle.ptr + (m_sampler_heap_offset + index + offset + i) * m_device->m_sampler_heap.m_descriptor_size;
				D3D12_CPU_DESCRIPTOR_HANDLE h;
				h.ptr = addr;
				m_device->m_device->CreateSampler(&d, h);
			}
		}
	}
}

#endif