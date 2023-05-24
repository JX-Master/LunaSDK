/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderInputLayout.hpp
* @author JXMaster
* @date 2020/3/14
*/
#pragma once
#include "../../ShaderInputLayout.hpp"
#include "Device.hpp"
#include <Runtime/Ref.hpp>
#include "D3D12Common.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ShaderInputLayout : IShaderInputLayout
		{
			lustruct("RHI::ShaderInputLayout", "{0a7ccb6d-bcf0-433a-af5b-ee454c37e5e2}");
			luiimpl();

			Ref<Device> m_device;
			ComPtr<ID3D12RootSignature> m_rs;

			struct DescriptorSetLayoutInfo
			{
				//! The first root parameter.
				u32 m_root_parameter_offset;
				//! The heap type to bind for every root parameter.
				Vector<D3D12_DESCRIPTOR_HEAP_TYPE> m_memory_types;
			};
			Vector<DescriptorSetLayoutInfo> m_descriptor_set_layouts;

			ShaderInputLayout() {}

			RV init(const ShaderInputLayoutDesc& desc);

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) { set_object_name(m_rs.Get(), name); }
		};
	}
}