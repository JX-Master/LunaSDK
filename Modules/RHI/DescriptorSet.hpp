/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include "Resource.hpp"
#include "PipelineState.hpp"
#include <Runtime/Math/Vector.hpp>
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif

namespace Luna
{
	namespace RHI
	{
		struct BufferViewDesc
		{
			//! The offset, in bytes, from the beginning of the buffer to
			//! the first element to be accessed by the view.
			u64 offset;
			//! The buffer to set.
			IBuffer* buffer;
			//! The number of elements in this view.
			u32 element_count;
			//! The size, in bytes, of each element in the buffer structure when the buffer represents a structured buffer.
			//! This is ignored if `format` is not `Format::unknown`.
			u32 element_size;
			//! The format of the buffer element in this buffer represents one typed buffer.
			//! If this buffer represents one structured buffer, this must be set to `Format::unknown`.
			Format format;
			static BufferViewDesc uniform_buffer(IBuffer* buffer, u64 offset = 0, u32 size = U32_MAX)
			{
				BufferViewDesc ret;
				ret.buffer = buffer;
				ret.offset = offset;
				ret.element_count = 1;
				ret.element_size = size;
				ret.format = Format::unknown;
				return ret;
			}
			static BufferViewDesc typed_buffer(IBuffer* buffer, u64 offset, u32 element_count, Format element_format)
			{
				BufferViewDesc ret;
				ret.buffer = buffer;
				ret.offset = offset;
				ret.element_count = element_count;
				ret.format = element_format;
				ret.element_size = 0;
				return ret;
			}
			static BufferViewDesc structured_buffer(IBuffer* buffer, u64 offset, u32 element_count, u32 element_size)
			{
				BufferViewDesc ret;
				ret.buffer = buffer;
				ret.offset = offset;
				ret.element_count = element_count;
				ret.format = Format::unknown;
				ret.element_size = element_size;
				return ret;
			}
		};

		enum class TextureViewType : u8
		{
			tex1d,
			tex2d,
			tex3d,
			texcube,
			tex1darray,
			tex2darray,
			texcubearray
		};

		struct TextureViewDesc
		{
			ITexture* texture;
			TextureViewType type;
			Format format;
			u32 mip_slice;
			u32 mip_size;
			u32 array_slice;
			u32 array_size;
			static TextureViewDesc tex1d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.type = TextureViewType::tex1d;
				desc.format = format;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static TextureViewDesc tex1darray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.type = TextureViewType::tex1darray;
				desc.format = format;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = array_slice;
				desc.array_size = array_size;
				return desc;
			}
			static TextureViewDesc tex2d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.format = format;
				desc.type = TextureViewType::tex2d;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static TextureViewDesc tex2darray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.format = format;
				desc.type = TextureViewType::tex2darray;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = array_slice;
				desc.array_size = array_size;
				return desc;
			}
			static TextureViewDesc tex3d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.format = format;
				desc.type = TextureViewType::tex3d;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static TextureViewDesc texcube(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.format = format;
				desc.type = TextureViewType::texcube;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = 0;
				desc.array_size = 6;
				return desc;
			}
			static TextureViewDesc texcubearray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
			{
				TextureViewDesc desc;
				desc.texture = texture;
				desc.format = format;
				desc.type = TextureViewType::texcubearray;
				desc.mip_slice = mip_slice;
				desc.mip_size = mip_size;
				desc.array_slice = array_slice;
				desc.array_size = array_size;
				return desc;
			}
		};

		enum class Filter : u8
		{
			min_mag_mip_point,
			min_mag_point_mip_linear,
			min_point_mag_linear_mip_point,
			min_point_mag_mip_linear,
			min_linear_mag_mip_point,
			min_linear_mag_point_mip_linear,
			min_mag_linear_mip_point,
			min_mag_mip_linear,
			anisotropic,
			comparison_min_mag_mip_point,
			comparison_min_mag_point_mip_linear,
			comparison_min_point_mag_linear_mip_point,
			comparison_min_point_mag_mip_linear,
			comparison_min_linear_mag_mip_point,
			comparison_min_linear_mag_point_mip_linear,
			comparison_min_mag_linear_mip_point,
			comparison_min_mag_mip_linear,
			comparison_anisotropic
		};

		enum class TextureAddressMode : u8
		{
			repeat,
			mirror,
			clamp,
			border
		};

		enum class BorderColor : u8
		{
			float_0000,
			int_0000,
			float_0001,
			int_0001,
			float_1111,
			int_1111,
		};

		struct SamplerDesc
		{
			Filter filter;
			TextureAddressMode address_u;
			TextureAddressMode address_v;
			TextureAddressMode address_w;
			ComparisonFunc comparison_func;
			BorderColor border_color;
			f32 mip_lod_bias;
			u32 max_anisotropy;
			f32 min_lod;
			f32 max_lod;

			SamplerDesc() = default;
			SamplerDesc(Filter filter,
				TextureAddressMode address_u,
				TextureAddressMode address_v,
				TextureAddressMode address_w,
				f32 mip_lod_bias = 0.0f,
				u32 max_anisotropy = 1,
				ComparisonFunc comparison_func = ComparisonFunc::always,
				BorderColor border_color = BorderColor::float_0000,
				f32 min_lod = 0.0f,
				f32 max_lod = FLT_MAX) :
				filter(filter),
				address_u(address_u),
				address_v(address_v),
				address_w(address_w),
				mip_lod_bias(mip_lod_bias),
				max_anisotropy(max_anisotropy),
				comparison_func(comparison_func),
				min_lod(min_lod),
				max_lod(max_lod) {}
			bool operator==(const SamplerDesc& rhs)
			{
				return !memcmp(this, &rhs, sizeof(SamplerDesc));
			}
			bool operator!= (const SamplerDesc& rhs)
			{
				return !(*this == rhs);
			}
		};

		struct DescriptorSetDesc
		{
			//! The descriptor layout for this descriptor set.
			IDescriptorSetLayout* layout;
			//! If the descriptor layout has `DescriptorSetLayoutFlag::variable_descriptors` 
			//! set, this is the number of variable descriptors being allocated for this else.
			//! Otherwise, this should always be 0.
			u32 num_variable_descriptors = 0;

			DescriptorSetDesc() {}
			DescriptorSetDesc(IDescriptorSetLayout* layout, u32 num_variable_descriptors = 0) :
				layout(layout),
				num_variable_descriptors(num_variable_descriptors) {}
		};

		struct DescriptorSetWrite
		{
			u32 binding_slot;
			u32 first_array_index;
			DescriptorType type;
			//! Used if `type` is `uniform_buffer_view`, `read_buffer_view` or `read_write_buffer_view`. 
			Span<const BufferViewDesc> buffer_views;
			//! Used if `type` is `sampled_texture_view`, `read_texture_view` or `read_write_texture_view`.
			Span<const TextureViewDesc> texture_views;
			//! Used if `type` is `sampler`.
			Span<const SamplerDesc> samplers;

			static DescriptorSetWrite uniform_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::uniform_buffer_view;
				ret.buffer_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite uniform_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::uniform_buffer_view;
				ret.buffer_views = descs;
				return ret;
			}
			static DescriptorSetWrite read_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::read_buffer_view;
				ret.buffer_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite read_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::read_buffer_view;
				ret.buffer_views = descs;
				return ret;
			}
			static DescriptorSetWrite read_write_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::read_write_buffer_view;
				ret.buffer_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite read_write_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::read_write_buffer_view;
				ret.buffer_views = descs;
				return ret;
			}
			static DescriptorSetWrite sampled_texture_view(u32 binding_slot, const TextureViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::sampled_texture_view;
				ret.texture_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite sampled_texture_view_array(u32 binding_slot, u32 first_array_index, Span<const TextureViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::sampled_texture_view;
				ret.texture_views = descs;
				return ret;
			}
			static DescriptorSetWrite read_texture_view(u32 binding_slot, const TextureViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::read_texture_view;
				ret.texture_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite read_texture_view_array(u32 binding_slot, u32 first_array_index, Span<const TextureViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::read_texture_view;
				ret.texture_views = descs;
				return ret;
			}
			static DescriptorSetWrite read_write_texture_view(u32 binding_slot, const TextureViewDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::read_write_texture_view;
				ret.texture_views = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite read_write_texture_view_array(u32 binding_slot, u32 first_array_index, Span<const TextureViewDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::read_write_texture_view;
				ret.texture_views = descs;
				return ret;
			}
			static DescriptorSetWrite sampler(u32 binding_slot, const SamplerDesc& desc)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = 0;
				ret.type = DescriptorType::sampler;
				ret.samplers = { &desc, 1 };
				return ret;
			}
			static DescriptorSetWrite sampler_array(u32 binding_slot, u32 first_array_index, Span<const SamplerDesc> descs)
			{
				DescriptorSetWrite ret;
				ret.binding_slot = binding_slot;
				ret.first_array_index = first_array_index;
				ret.type = DescriptorType::sampler;
				ret.samplers = descs;
				return ret;
			}
		};

		//! @interface IDescriptorSet
		//! Describes which views and samples are bound to the pipeline.
		//! This can be set at any time before the draw call or dispatch call
		//! is submitted.
		struct IDescriptorSet : virtual IDeviceChild
		{
			luiid("{f12bc4b0-2aad-42bb-8b8c-237ed0593aa3}");

			virtual RV update_descriptors(Span<const DescriptorSetWrite> writes) = 0;
		};
	}
}