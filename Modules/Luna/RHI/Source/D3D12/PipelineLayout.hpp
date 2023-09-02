/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineLayout.hpp
* @author JXMaster
* @date 2020/3/14
*/
#pragma once
#include "../../PipelineLayout.hpp"
#include "Device.hpp"
#include <Luna/Runtime/Ref.hpp>
#include "D3D12Common.hpp"

namespace Luna
{
	namespace RHI
	{
		struct PipelineLayout : IPipelineLayout
		{
			lustruct("RHI::PipelineLayout", "{0a7ccb6d-bcf0-433a-af5b-ee454c37e5e2}");
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

			PipelineLayout() {}

			RV init(const PipelineLayoutDesc& desc);

			virtual IDevice* get_device() override
			{
				return m_device.as<IDevice>();
			}
			virtual void set_name(const c8* name) override { set_object_name(m_rs.Get(), name); }
		};
	}
}