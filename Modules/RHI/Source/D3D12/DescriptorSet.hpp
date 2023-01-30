/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2022/8/3
*/
#pragma once

#ifdef LUNA_RHI_D3D12
#include "D3D12Common.hpp"
#include "Device.hpp"
#include "Resource.hpp"
#include "ShaderInputLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DescriptorSet : IDescriptorSet
		{
			lustruct("RHI::DescriptorSet", "{6937b6b1-ce6d-4211-a5d5-5af9a6eba60a}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;

			u32 m_view_heap_offset;
			u32 m_sampler_heap_offset;

			u32 m_view_heap_size;
			u32 m_sampler_heap_size;

			HashMap<u32, u32> m_bound_index_to_offset;

			RV init(const DescriptorSetDesc& desc);
			~DescriptorSet();

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_cbv(u32 binding_slot, IResource* res, const ConstantBufferViewDesc& cbv);
			void set_cbv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ConstantBufferViewDesc* descs);
			void set_srv(u32 binding_slot, IResource* res, const ShaderResourceViewDesc* srv = nullptr);
			void set_srv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ShaderResourceViewDesc* descs);
			void set_uav(u32 binding_slot, IResource* res, IResource* counter_resource = nullptr, const UnorderedAccessViewDesc* uav = nullptr);
			void set_uav_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, IResource** counter_resources, const UnorderedAccessViewDesc* descs);
			void set_sampler(u32 binding_slot, const SamplerDesc& sampler);
			void set_sampler_array(u32 binding_slot, u32 offset, u32 num_samplers, const SamplerDesc* samplers);
		};
	}
}

#endif