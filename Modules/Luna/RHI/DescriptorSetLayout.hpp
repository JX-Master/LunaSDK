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
		enum class DescriptorType : u8
		{
			//! Specifies uniform buffer view, which allows reading data from one uniform buffer.
			//! This descriptor is supported in all shaders.
			//! 
			//! To represent one uniform buffer view, use the following parameter types in shader source code with register type `b`:
			//! * cbuffer
			//! * ConstantBuffer
			uniform_buffer_view,
			//! Specifies read buffer view, which allows reading data from one structured buffer.
			//! This descriptor is supported in all shaders.
			//! 
			//! To specify one read buffer view, use the following parameter types in shader source code with register type `t`:
			//! * StructuredBuffer
			read_buffer_view,
			//! Speciifes read-write buffer view, which allows reading and writing data from one structured buffer.
			//! This descriptor is supported in compute shader only.
			//! 
			//! To specify one read write buffer view, use the following parameter types in shader source code with register type `u`:
			//! * RWStructuredBuffer
			read_write_buffer_view,
			//! Specifies read texture view, which allows reading texture data using pixel coordinates directly 
			//! or sampling texture data from the texture using samplers.
			//! This descriptor is supported in all shaders.
			//! 
			//! To represent one read texture view, use the following parameter types in shader source code with register type `t`:
			//! * Texture1D/Texture2D/Texture3D/TextureCube
			//! * Texture1DArray/Texture2DArray/Texture2DMS/Texture2DMSArray/TextureCubeArray
			read_texture_view,
			//! Specifies read-write texture view, which allows reading and writing texture data using pixel coordinates directly.
			//! This descriptor is supported in compute shader only.
			//! 
			//! To represent one read write texture view, use the following parameter types in shader source code with register type `u`:
			//! * RWTexture1D/RWTexture2D/RWTexture3D
			//! * RWTexture1DArray/RWTexture2DArray
			read_write_texture_view,
			//! Specifies one sampler.
			//! This descriptor is supported in all shaders.
			sampler,
		};

		enum class TextureViewType : u8
		{
			//! Use the image type as the image view type.
			unspecified = 0,
			tex1d,
			tex2d,
			tex2dms,
			tex3d,
			texcube,
			tex1darray,
			tex2darray,
			tex2dmsarray,
			texcubearray
		};

		enum class ShaderVisibilityFlag : u8
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
			//! The type of descriptors.
			DescriptorType type;
			//! The type of the texture view if this binding represents one texture, otherwise @ref TextureViewType::unspecified.
			TextureViewType texture_view_type;
			//! Specify which pipeline shader can access a resource for this binding.
			ShaderVisibilityFlag shader_visibility_flags;

			DescriptorSetLayoutBinding() = default;
			DescriptorSetLayoutBinding(DescriptorType type, TextureViewType texture_view_type, u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags) :
				type(type),
				texture_view_type(texture_view_type),
				binding_slot(binding_slot),
				num_descs(num_descs),
				shader_visibility_flags(shader_visibility_flags) {}

			static DescriptorSetLayoutBinding uniform_buffer_view(u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::uniform_buffer_view;
				r.texture_view_type = TextureViewType::unspecified;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
			static DescriptorSetLayoutBinding read_buffer_view(u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::read_buffer_view;
				r.texture_view_type = TextureViewType::unspecified;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
			static DescriptorSetLayoutBinding read_write_buffer_view(u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::read_write_buffer_view;
				r.texture_view_type = TextureViewType::unspecified;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
			static DescriptorSetLayoutBinding read_texture_view(TextureViewType texture_view_type, u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::read_texture_view;
				r.texture_view_type = texture_view_type;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
			static DescriptorSetLayoutBinding read_write_texture_view(TextureViewType texture_view_type, u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::read_write_texture_view;
				r.texture_view_type = texture_view_type;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
			static DescriptorSetLayoutBinding sampler(u32 binding_slot, u32 num_descs, ShaderVisibilityFlag shader_visibility_flags)
			{
				DescriptorSetLayoutBinding r;
				r.binding_slot = binding_slot;
				r.num_descs = num_descs;
				r.type = DescriptorType::sampler;
				r.texture_view_type = TextureViewType::unspecified;
				r.shader_visibility_flags = shader_visibility_flags;
				return r;
			}
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
			Span<const DescriptorSetLayoutBinding> bindings;
			DescriptorSetLayoutFlag flags = DescriptorSetLayoutFlag::none;

			DescriptorSetLayoutDesc() {}
			DescriptorSetLayoutDesc(Span<const DescriptorSetLayoutBinding> bindings,
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
