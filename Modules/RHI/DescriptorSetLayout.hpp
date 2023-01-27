/*
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
		enum class DescriptorType : u32
		{
			srv, // Shader resource view.
			uav, // Unordered access view.
			cbv, // Constant buffer view.
			sampler // Sampler view.
		};

		enum class ShaderVisibility : u32
		{
			all = 0,
			vertex = 1,
			hull = 2,
			domain = 3,
			geometry = 4,
			pixel = 5
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
			ShaderVisibility shader_visibility;

			DescriptorSetLayoutBinding() = default;
			DescriptorSetLayoutBinding(DescriptorType type, u32 binding_slot, u32 num_descs, ShaderVisibility shader_visibility) :
				type(type),
				binding_slot(binding_slot),
				num_descs(num_descs),
				shader_visibility(shader_visibility) {}
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