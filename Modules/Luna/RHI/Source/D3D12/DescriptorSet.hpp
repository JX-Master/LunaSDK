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
#include "D3D12Common.hpp"
#include "Device.hpp"
#include "Resource.hpp"
#include "PipelineLayout.hpp"

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

			virtual IDevice* get_device() override
			{
				return m_device.as<IDevice>();
			}
			virtual void set_name(const c8* name) override {}
			virtual RV update_descriptors(Span<const WriteDescriptorSet> writes) override;
			
			void set_cbv_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs);
			void set_buffer_srv_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs);
			void set_texture_srv_array(u32 binding_slot, u32 offset, u32 num_descs, const TextureViewDesc* descs);
			void set_buffer_uav_array(u32 binding_slot, u32 offset, u32 num_descs, const BufferViewDesc* descs);
			void set_texture_uav_array(u32 binding_slot, u32 offset, u32 num_descs, const TextureViewDesc* descs);
			void set_sampler_array(u32 binding_slot, u32 offset, u32 num_samplers, const SamplerDesc* samplers);
		};
	}
}