/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderInputLayout.cpp
* @author JXMaster
* @date 2020/3/14
*/
#include "ShaderInputLayout.hpp"

#ifdef LUNA_RHI_D3D12

#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		RV ShaderInputLayout::init(const ShaderInputLayoutDesc& desc)
		{
			D3D12_ROOT_SIGNATURE_DESC d;
			d.NumStaticSamplers = 0;
			d.pStaticSamplers = nullptr;

			Vector<D3D12_ROOT_PARAMETER> parameters;

			for (u32 i = 0; i < desc.descriptor_set_layouts.size(); ++i)
			{
				DescriptorSetLayout* layout = static_cast<DescriptorSetLayout*>(desc.descriptor_set_layouts[i]->get_object());
				DescriptorSetLayoutInfo info;
				info.m_root_parameter_offset = (u32)parameters.size();
				for (auto& root : layout->m_root_parameters)
				{
					D3D12_ROOT_PARAMETER param;
					param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					param.ShaderVisibility = root.m_shader_visibility;
					param.DescriptorTable.NumDescriptorRanges = (UINT)root.m_ranges.size();
					D3D12_DESCRIPTOR_RANGE* ranges = (D3D12_DESCRIPTOR_RANGE*)alloca(sizeof(D3D12_DESCRIPTOR_RANGE) * root.m_ranges.size());
					param.DescriptorTable.pDescriptorRanges = ranges;
					for (usize j = 0; j < root.m_ranges.size(); ++j)
					{
						D3D12_DESCRIPTOR_RANGE* dest = ranges + j;
						const D3D12_DESCRIPTOR_RANGE* src = &root.m_ranges[j];
						dest->NumDescriptors = src->NumDescriptors;
						dest->BaseShaderRegister = src->BaseShaderRegister;
						dest->OffsetInDescriptorsFromTableStart = src->OffsetInDescriptorsFromTableStart;
						dest->RangeType = src->RangeType;
						dest->RegisterSpace = i;
					}
					parameters.push_back(param);
					info.m_heap_types.push_back(root.m_type);
				}
				m_descriptor_set_layouts.push_back(move(info));
			}
			d.NumParameters = (UINT)parameters.size();
			d.pParameters = parameters.data();
			d.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
			if ((desc.flags & ShaderInputLayoutFlag::allow_input_assembler_input_layout) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			}
			if ((desc.flags & ShaderInputLayoutFlag::allow_stream_output) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
			}
			if ((desc.flags & ShaderInputLayoutFlag::deny_domain_shader_access) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
			}
			if ((desc.flags & ShaderInputLayoutFlag::deny_geometry_shader_access) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			}
			if ((desc.flags & ShaderInputLayoutFlag::deny_hull_shader_access) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
			}
			if ((desc.flags & ShaderInputLayoutFlag::deny_pixel_shader_access) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			}
			if ((desc.flags & ShaderInputLayoutFlag::deny_vertex_shader_access) != ShaderInputLayoutFlag::none)
			{
				d.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
			}
			ComPtr<ID3DBlob> b;
			ComPtr<ID3DBlob> err;
			if (FAILED(D3D12SerializeRootSignature(&d, D3D_ROOT_SIGNATURE_VERSION_1_0, b.GetAddressOf(), err.GetAddressOf())))
			{
				return set_error(BasicError::bad_platform_call(), "Failed to create D3D12 root signature: %s", err->GetBufferPointer());
			}
			if (FAILED(m_device->m_device->CreateRootSignature(0, b->GetBufferPointer(), b->GetBufferSize(), IID_PPV_ARGS(&m_rs))))
			{
				return set_error(BasicError::bad_platform_call(), "Failed to create D3D12 root signature.");
			}
			return ok;
		}
	}
}

#endif