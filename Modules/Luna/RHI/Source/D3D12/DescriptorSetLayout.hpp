/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSetLayout.hpp
* @author JXMaster
* @date 2022/8/3
*/
#pragma once
#include "D3D12Common.hpp"
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DescriptorSetLayout : IDescriptorSetLayout
		{
			lustruct("RHI::DescriptorSetLayout", "{158C811E-AED1-4672-A395-0618DF0E29BF}");
			luiimpl();

			Ref<Device> m_device;
			DescriptorSetLayoutFlag m_flags;

			struct HeapInfo
			{
				//! The size of this heap.
				//! If this heap is variable-sized, this specifiy the size of the heap 
				//! excluding the variable-sized binding entry.
				u32 m_size = 0;
				//! Whether this heap is variable (the size is uncertain).
				bool m_variable = false;
			};

			struct RootParameterInfo
			{
				D3D12_DESCRIPTOR_HEAP_TYPE m_type;
				//! The shader visibility of this root parameter.
				D3D12_SHADER_VISIBILITY m_shader_visibility;
				Vector<D3D12_DESCRIPTOR_RANGE> m_ranges;
			};

			struct BindingInfo
			{
				DescriptorSetLayoutBinding desc;
				//! The target heap to allocate descriptors for this binding.
				//! We will create one heap for every type of descriptors (SRV/UAV/CBV heaps are merged into one heap).
				D3D12_DESCRIPTOR_HEAP_TYPE target_heap;
				//! The offset of the first descriptor for this binding in heap.
				u32 offset_in_heap;
				//! The target root parameter to place this binding.
				//! Multiple bindings may be merged to one root parameter if they have the compatible descriptor types and shader visibility.
				u32 root_parameter_index;
				//! The target range in the target root parameter.
				u32 range_index;
			};

			//! Describes how to allocate heaps for this descriptor set layout.
			HeapInfo m_view_heap;
			HeapInfo m_sampler_heap;
			//! Describes how to allocate root parameters in order to bind this descriptor set.
			//! (shader space is not initialized).
			Vector<RootParameterInfo> m_root_parameters;
			// Describes how every binding maps to root parameters and descriptor heaps.
			Vector<BindingInfo> m_bindings;

			void init(const DescriptorSetLayoutDesc& desc);

			HeapInfo* get_heap_by_type(D3D12_DESCRIPTOR_HEAP_TYPE heap);

			u32 get_root_parameter_index(DescriptorType type, ShaderVisibilityFlag shader_visibility);

			virtual IDevice* get_device() override
			{
				return m_device.as<IDevice>();
			}
			virtual void set_name(const c8* name) override {}
		};
	}
}