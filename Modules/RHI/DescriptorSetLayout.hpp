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
			//! Specifies uniform buffer view, which allows reading data from one uniform buffer.
			//! This descriptor is supported in all shaders.
			uniform_buffer_view,
			//! Specifies read buffer view, which allows reading data from one structured buffer.
			//! This descriptor is supported in all shaders.
			read_buffer_view,
			//! Speciifes read-write buffer view, which allows reading and writing data from one structured buffer.
			//! This descriptor is supported in compute shader only.
			read_write_buffer_view,
			//! Specifies sampled texture view, which allows sampling texture data from the texture
			//! using samplers.
			//! This descriptor is supported in all shaders.
			sampled_texture_view,
			//! Specifies read texture view, which allows reading texture data using pixel coordinates directly.
			//! This descriptor is supported in all shaders.
			read_texture_view,
			//! Specifies read-write texture view, which allows reading and writing texture data using pixel coordinates directly.
			//! This descriptor is supported in compute shader only.
			read_write_texture_view,
			//! Specifies one sampler.
			//! This descriptor is supported in all shaders.
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