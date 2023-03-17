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
		namespace srv
		{
			struct Buffer
			{
				//! The index of the first element to be accessed by the view.
				u64 offset;
				//! The number of elements in the resource.
				u32 count;
				//! The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer.
				//! This can be ignored for typed 
				u32 element_size;
				//! Indicates the buffer is a byte address buffer ( raw view buffer ).
				bool raw_view;
			};
			struct Tex1D
			{
				//! The most detailed mip level to use.
				u32  most_detailed_mip;
				//! The mip levels to use. Set to -1 to indicate all the mipmap levels from `most_detailed_mip` on down to least detailed. 
				//! Available mip levels are [most_detailed, most_detailed + mip_levels - 1].
				u32  mip_levels;
				//! A value to clamp sample LOD values to. 
				//! For example, if you specify 2.0f for the clamp value, you ensure that no individual sample accesses a mip level 
				//! less than 2.0f.
				f32 resource_min_lod_clamp;
			};
			struct Tex1DArray
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				//! The index of the first texture to use in an array of textures.
				u32  first_array_slice;
				//! Number of textures in the array.
				u32  array_size;
				f32 resource_min_lod_clamp;
			};
			struct Tex2D
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				f32 resource_min_lod_clamp;
			};
			struct Tex2DArray
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				u32  first_array_slice;
				u32  array_size;
				f32 resource_min_lod_clamp;
			};
			struct Tex2DMS {};
			struct Tex2DMSArray
			{
				u32  first_array_slice;
				u32  array_size;
			};
			struct Tex3D
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				f32 resource_min_lod_clamp;
			};
			struct TexCube
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				f32 resource_min_lod_clamp;
			};
			struct TexCubeArray
			{
				u32  most_detailed_mip;
				u32  mip_levels;
				u32  first_2darray_face;
				u32  num_cubes;
				f32 resource_min_lod_clamp;
			};
		}

		enum class ShaderResourceViewType
		{
			unknown = 0,
			buffer,
			tex1d,
			tex1darray,
			tex2d,
			tex2darray,
			tex2dms,
			tex2dmsarray,
			tex3d,
			texcube,
			texcubearray
		};

		struct ShaderResourceViewDesc
		{
			Format format;
			ShaderResourceViewType type;
			union
			{
				srv::Buffer buffer;
				srv::Tex1D tex1d;
				srv::Tex1DArray tex1darray;
				srv::Tex2D tex2d;
				srv::Tex2DArray tex2darray;
				srv::Tex2DMS tex2dms;
				srv::Tex2DMSArray tex2dmsarray;
				srv::Tex3D tex3d;
				srv::TexCube texcube;
				srv::TexCubeArray texcubearray;
			};

			static ShaderResourceViewDesc as_buffer(Format format, u64 offset, u32 count, u32 element_size, bool raw_view = false)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::buffer;
				desc.buffer.offset = offset;
				desc.buffer.count = count;
				desc.buffer.raw_view = raw_view;
				desc.buffer.element_size = element_size;
				return desc;
			}

			static ShaderResourceViewDesc as_tex1d(Format format, u32 most_detailed_mip, u32 mip_levels, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex1d;
				desc.tex1d.mip_levels = mip_levels;
				desc.tex1d.most_detailed_mip = most_detailed_mip;
				desc.tex1d.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_tex1darray(Format format, u32 most_detailed_mip, u32 mip_levels, u32 first_array_slice, u32 array_size, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex1darray;
				desc.tex1darray.most_detailed_mip = most_detailed_mip;
				desc.tex1darray.mip_levels = mip_levels;
				desc.tex1darray.array_size = array_size;
				desc.tex1darray.first_array_slice = first_array_slice;
				desc.tex1darray.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_tex2d(Format format, u32  most_detailed_mip, u32  mip_levels, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex2d;
				desc.tex2d.most_detailed_mip = most_detailed_mip;
				desc.tex2d.mip_levels = mip_levels;
				desc.tex2d.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_tex2darray(Format format, u32 most_detailed_mip, u32 mip_levels, u32 first_array_slice, u32 array_size, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex2darray;
				desc.tex2darray.most_detailed_mip = most_detailed_mip;
				desc.tex2darray.mip_levels = mip_levels;
				desc.tex2darray.first_array_slice = first_array_slice;
				desc.tex2darray.array_size = array_size;
				desc.tex2darray.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_tex2dms(Format format)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex2dms;
				return desc;
			}

			static ShaderResourceViewDesc as_tex2dmsarray(Format format, u32 first_array_slice, u32 array_size)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex2dmsarray;
				desc.tex2dmsarray.first_array_slice = first_array_slice;
				desc.tex2dmsarray.array_size = array_size;
				return desc;
			}

			static ShaderResourceViewDesc as_tex3d(Format format, u32 most_detailed_mip, u32 mip_levels, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::tex3d;
				desc.tex3d.most_detailed_mip = most_detailed_mip;
				desc.tex3d.mip_levels = mip_levels;
				desc.tex3d.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_texcube(Format format, u32 most_detailed_mip, u32 mip_levels, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::texcube;
				desc.texcube.mip_levels = mip_levels;
				desc.texcube.most_detailed_mip = most_detailed_mip;
				desc.texcube.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}

			static ShaderResourceViewDesc as_texcubearray(Format format, u32 most_detailed_mip, u32 mip_levels, u32 first_2d_array_face, u32 num_cubes, f32 resource_min_lod_clamp)
			{
				ShaderResourceViewDesc desc;
				desc.format = format;
				desc.type = ShaderResourceViewType::texcubearray;
				desc.texcubearray.first_2darray_face = first_2d_array_face;
				desc.texcubearray.mip_levels = mip_levels;
				desc.texcubearray.most_detailed_mip = most_detailed_mip;
				desc.texcubearray.num_cubes = num_cubes;
				desc.texcubearray.resource_min_lod_clamp = resource_min_lod_clamp;
				return desc;
			}
		};

		namespace uav
		{
			struct Buffer
			{
				u64 offset;
				u32 count;
				u32 element_size;
				u64 counter_offset_in_bytes;
				bool raw_view;
			};
			struct Tex1D
			{
				u32 mip_slice;
			};
			struct Tex1DArray
			{
				u32 mip_slice;
				u32 first_array_slice;
				u32 array_size;
			};
			struct Tex2D
			{
				u32 mip_slice;
			};
			struct Tex2DArray
			{
				u32 mip_slice;
				u32 first_array_slice;
				u32 array_size;
			};
			struct Tex3D
			{
				u32 mip_slice;
				u32 first_layer_slice;
				u32 layer_size;
			};
		}

		enum class UnorderedAccessViewType
		{
			unknown = 0,
			buffer,
			tex1d,
			tex1darray,
			tex2d,
			tex2darray,
			tex3d
		};

		struct UnorderedAccessViewDesc
		{
			Format format;
			UnorderedAccessViewType type;
			union
			{
				uav::Buffer buffer;
				uav::Tex1D tex1d;
				uav::Tex1DArray tex1darray;
				uav::Tex2D tex2d;
				uav::Tex2DArray tex2darray;
				uav::Tex3D tex3d;
			};

			static UnorderedAccessViewDesc as_buffer(Format format, u64 offset, u32 count, u32 element_size, u64 counter_offset_in_bytes, bool raw_view)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::buffer;
				desc.buffer.offset = offset;
				desc.buffer.count = count;
				desc.buffer.element_size = element_size;
				desc.buffer.counter_offset_in_bytes = counter_offset_in_bytes;
				desc.buffer.raw_view = raw_view;
				return desc;
			}

			static UnorderedAccessViewDesc as_tex1d(Format format, u32 mip_slice)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::tex1d;
				desc.tex1d.mip_slice = mip_slice;
				return desc;
			}

			static UnorderedAccessViewDesc as_tex1darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::tex1darray;
				desc.tex1darray.array_size = array_size;
				desc.tex1darray.first_array_slice = first_array_slice;
				desc.tex1darray.mip_slice = mip_slice;
				return desc;
			}

			static UnorderedAccessViewDesc as_tex2d(Format format, u32 mip_slice)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::tex2d;
				desc.tex2d.mip_slice = mip_slice;
				return desc;
			}

			static UnorderedAccessViewDesc as_tex2darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::tex2darray;
				desc.tex2darray.array_size = array_size;
				desc.tex2darray.first_array_slice = first_array_slice;
				desc.tex2darray.mip_slice = mip_slice;
				return desc;
			}

			static UnorderedAccessViewDesc as_tex3d(Format format, u32 mip_slice, u32 first_layer_slice, u32 layer_size)
			{
				UnorderedAccessViewDesc desc;
				desc.format = format;
				desc.type = UnorderedAccessViewType::tex3d;
				desc.tex3d.first_layer_slice = first_layer_slice;
				desc.tex3d.layer_size = layer_size;
				desc.tex3d.mip_slice = mip_slice;
				return desc;
			}
		};

		struct ConstantBufferViewDesc
		{
			u64 offset;
			u32 size;

			ConstantBufferViewDesc() = default;
			ConstantBufferViewDesc(u64 offset, u32 size) :
				offset(offset), size(size) {}
		};

		enum class FilterMode : u32
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
			comparison_anisotropic,
			minimum_min_mag_mip_point,
			minimum_min_mag_point_mip_linear,
			minimum_min_point_mag_linear_mip_point,
			minimum_min_point_mag_mip_linear,
			minimum_min_linear_mag_mip_point,
			minimum_min_linear_mag_point_mip_linear,
			minimum_min_mag_linear_mip_point,
			minimum_min_mag_mip_linear,
			minimum_anisotropic,
			maximum_min_mag_mip_point,
			maximum_min_mag_point_mip_linear,
			maximum_min_point_mag_linear_mip_point,
			maximum_min_point_mag_mip_linear,
			maximum_min_linear_mag_mip_point,
			maximum_min_linear_mag_point_mip_linear,
			maximum_min_mag_linear_mip_point,
			maximum_min_mag_mip_linear,
			maximum_anisotropic
		};

		enum class TextureAddressMode : u32
		{
			repeat,
			mirror,
			clamp,
			border,
			mirror_once
		};

		struct SamplerDesc
		{
			FilterMode filter;
			TextureAddressMode address_u;
			TextureAddressMode address_v;
			TextureAddressMode address_w;
			f32 mip_lod_bias;
			u32 max_anisotropy;
			ComparisonFunc comparison_func;
			f32 border_color[4];
			f32 min_lod;
			f32 max_lod;

			SamplerDesc() = default;
			SamplerDesc(FilterMode filter,
				TextureAddressMode address_u,
				TextureAddressMode address_v,
				TextureAddressMode address_w,
				f32 mip_lod_bias = 0.0f,
				u32 max_anisotropy = 1,
				ComparisonFunc comparison_func = ComparisonFunc::always,
				const Float4U& border_color = Float4U(0, 0, 0, 0),
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
				max_lod(max_lod)
			{
				this->border_color[0] = border_color.x;
				this->border_color[1] = border_color.y;
				this->border_color[2] = border_color.z;
				this->border_color[3] = border_color.w;
			}
			bool operator==(const SamplerDesc& rhs)
			{
				return !memcmp(this, &rhs, sizeof(SamplerDesc));
			}
			bool operator!= (const SamplerDesc& rhs)
			{
				return !(*this == rhs);
			}
		};

		struct StreamOutputBufferView
		{
			IResource* soresource;
			u64 offset_in_bytes;
			u64 size_in_bytes;
			IResource* buffer_filled_size_resource;
			u64 buffer_filled_size_offset;
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

		LUNA_RHI_API ShaderResourceViewDesc get_default_srv_from_resource(IResource* resource);
		LUNA_RHI_API UnorderedAccessViewDesc get_default_uav_from_resource(IResource* resource);

		//! @interface IDescriptorSet
		//! Describes which views and samples are bound to the pipeline.
		//! This can be set at any time before the draw call or dispatch call
		//! is submitted.
		struct IDescriptorSet : virtual IDeviceChild
		{
			luiid("{f12bc4b0-2aad-42bb-8b8c-237ed0593aa3}");

			virtual void set_cbv(u32 binding_slot, IResource* res, const ConstantBufferViewDesc& cbv) = 0;
			virtual void set_cbv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ConstantBufferViewDesc* descs) = 0;
			virtual void set_srv(u32 binding_slot, IResource* res, const ShaderResourceViewDesc* srv = nullptr) = 0;
			virtual void set_srv_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, const ShaderResourceViewDesc* descs) = 0;
			virtual void set_uav(u32 binding_slot, IResource* res, IResource* counter_resource = nullptr, const UnorderedAccessViewDesc* uav = nullptr) = 0;
			virtual void set_uav_array(u32 binding_slot, u32 offset, u32 num_descs, IResource** resources, IResource** counter_resources, const UnorderedAccessViewDesc* descs) = 0;
			virtual void set_sampler(u32 binding_slot, const SamplerDesc& sampler) = 0;
			virtual void set_sampler_array(u32 binding_slot, u32 offset, u32 num_samplers, const SamplerDesc* samplers) = 0;
		};
	}
}