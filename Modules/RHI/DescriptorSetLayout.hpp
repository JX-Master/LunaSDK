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
#include "DeviceChild.hpp"

namespace Luna
{
	namespace RHI
	{
		//! Specify the type of descriptors that can be placed in a descriptor set.
		enum class DescriptorType : u32
		{
			//! Specifies texture shader resource view (SRV).
			//! Such descriptor allows sampling the specified texture using samplers.
			texture_srv,
			//! Specifies buffer shader resource view (SRV).
			//! Such descriptor allows reading buffer data, but not writing to it.
			buffer_srv,
			//! Specifies texture unordered access view (UAV). 
			//! Such descriptor allows reading, writing and performing atomic operations on the texture pixels.
			texture_uav,
			//! Specifies buffer unordered access view (UAV).
			//! Such descriptor allows reading, writing and performing atomic operations on the buffer data.
			buffer_uav,
			//! Specifies constant buffer view (CBV). 
			//! Such descriptor allows reading data from one uniform buffer.
			cbv,
			//! Specifies one sampler.
			sampler,
		};

		enum class ShaderVisibilityFlag : u32
		{
			none = 0x00,
			vertex = 0x01,
			pixel = 0x02,
			compute = 0x04,
			all = vertex | pixel | compute
		};

		//! Describes one binding in one descriptor set.
		struct DescriptorSetLayoutBinding
		{
			//! The type of descriptors.
			DescriptorType type;
			//! The slot to bind this descriptor.
			//! If `num_descs` is greater than 1, slots [binding_slot, binding_slot + num_descs)
			//! will be occupied and cannot be used in another bindings.
			//! The binding slot does not need to be continuous.
			u32 binding_slot;
			//! The number of descriptors for this binding. 
			//! If this number is greater than 1, this binding will be interprated as one
			//! array of descriptors.
			//! If this is a variable-sized descriptors array, this is the maximum number 
			//! of descriptors that may be bound. This value may be used for hardware validation when needed.
			u32 num_descs;
			//! Specify which pipeline shader can access a resource for this binding.
			ShaderVisibilityFlag shader_visibility_flags;

			DescriptorSetLayoutBinding() = default;
			DescriptorSetLayoutBinding(DescriptorType type, u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags) :
				type(type),
				binding_slot(binding_slot),
				num_descs(num_descs),
				shader_visibility_flags(shader_visibility_flags) {}
		};

		enum class DescriptorSetLayoutFlag : u32
		{
			none = 0,
			//! Enable variable-sized descriptors array for the last binding (the binding with 
			//! the largest `binding_slot` value).
			variable_descriptors = 1,
		};

		struct DescriptorSetLayoutDesc
		{
			Vector<DescriptorSetLayoutBinding> bindings;
			DescriptorSetLayoutFlag flags = DescriptorSetLayoutFlag::none;

			DescriptorSetLayoutDesc() {}
			DescriptorSetLayoutDesc(InitializerList<DescriptorSetLayoutBinding> bindings, 
				DescriptorSetLayoutFlag flags = DescriptorSetLayoutFlag::none) :
				bindings(bindings),
				flags(flags) {}
		};

		struct IDescriptorSetLayout : virtual IDeviceChild
		{
			luiid("{68D6929B-D94F-48B1-A19E-B89E0CF0D008}");


		};
	}
}