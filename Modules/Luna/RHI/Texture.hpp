/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Texture.hpp
* @author JXMaster
* @date 2023/9/20
*/
#pragma once
#include "Resource.hpp"

namespace Luna
{
    namespace RHI
    {
        enum class Format : u16
		{
			unknown = 0,
			// Ordinary 8-Bit Formats
			r8_unorm,
			r8_snorm,
			r8_uint,
			r8_sint,
			// Ordinary 16-Bit Formats
			r16_unorm,
			r16_snorm,
			r16_uint,
			r16_sint,
			r16_float,
			rg8_unorm,
			rg8_snorm,
			rg8_uint,
			rg8_sint,
			// Ordinary 32-Bit Formats
			r32_uint,
			r32_sint,
			r32_float,
			rg16_unorm,
			rg16_snorm,
			rg16_uint,
			rg16_sint,
			rg16_float,
			rgba8_unorm,
			rgba8_unorm_srgb,
			rgba8_snorm,
			rgba8_uint,
			rgba8_sint,
			bgra8_unorm,
			bgra8_unorm_srgb,
			// Ordinary 64-Bit Formats
			rg32_uint,
			rg32_sint,
			rg32_float,
			rgba16_unorm,
			rgba16_snorm,
			rgba16_uint,
			rgba16_sint,
			rgba16_float,
			// Ordinary 96-Bit Formats
			rgb32_uint,
			rgb32_sint,
			rgb32_float,
			// Ordinary 128-Bit Formats
			rgba32_uint,
			rgba32_sint,
			rgba32_float,
			// Packed 16-Bit Formats
			b5g6r5_unorm,
			bgr5a1_unorm,
			// Packed 32-Bit Formats
			rgb10a2_unorm,
			rgb10a2_uint,
			rg11b10_float,
			rgb9e5_float,
			// Depth-stencil
			d16_unorm,
			d32_float,
			d24_unorm_s8_uint,
			d32_float_s8_uint_x24,
			// Compressed formats
			bc1_rgba_unorm,   // DXT1
			bc1_rgba_unorm_srgb,
			bc2_rgba_unorm,   // DXT3
			bc2_rgba_unorm_srgb,
			bc3_rgba_unorm,   // DXT5
			bc3_rgba_unorm_srgb,
			bc4_r_unorm,   // RGTC Unsigned Red
			bc4_r_snorm,   // RGTC Signed Red
			bc5_rg_unorm,   // RGTC Unsigned RG
			bc5_rg_snorm,   // RGTC Signed RG
			bc6h_rgb_sfloat,
			bc6h_rgb_ufloat,
			bc7_rgba_unorm,
			bc7_rgba_unorm_srgb,

			count
		};

		//! Returns the size of one pixel in the specified format, in bits.
		inline usize bits_per_pixel(Format format)
		{
			switch (format)
			{
			case Format::r8_unorm:
			case Format::r8_snorm:
			case Format::r8_uint:
			case Format::r8_sint:
				return 8;
			case Format::r16_unorm:
			case Format::r16_snorm:
			case Format::r16_uint:
			case Format::r16_sint:
			case Format::r16_float:
			case Format::rg8_unorm:
			case Format::rg8_snorm:
			case Format::rg8_uint:
			case Format::rg8_sint:
			case Format::b5g6r5_unorm:
			case Format::bgr5a1_unorm:
			case Format::d16_unorm:
				return 16;
			case Format::r32_uint:
			case Format::r32_sint:
			case Format::r32_float:
			case Format::rg16_uint:
			case Format::rg16_sint:
			case Format::rg16_unorm:
			case Format::rg16_snorm:
			case Format::rg16_float:
			case Format::rgba8_unorm:
			case Format::rgba8_unorm_srgb:
			case Format::rgba8_snorm:
			case Format::rgba8_uint:
			case Format::rgba8_sint:
			case Format::bgra8_unorm:
			case Format::bgra8_unorm_srgb:
			case Format::rgb10a2_unorm:
			case Format::rgb10a2_uint:
			case Format::rg11b10_float:
			case Format::rgb9e5_float:
			case Format::d32_float:
			case Format::d24_unorm_s8_uint:
				return 32;
			case Format::rg32_uint:
			case Format::rg32_sint:
			case Format::rg32_float:
			case Format::rgba16_unorm:
			case Format::rgba16_snorm:
			case Format::rgba16_uint:
			case Format::rgba16_sint:
			case Format::rgba16_float:
			case Format::d32_float_s8_uint_x24:
				return 64;
			case Format::rgb32_uint:
			case Format::rgb32_sint:
			case Format::rgb32_float:
				return 96;
			case Format::rgba32_uint:
			case Format::rgba32_sint:
			case Format::rgba32_float:
				return 128;
			case Format::bc1_rgba_unorm:
			case Format::bc1_rgba_unorm_srgb:
			case Format::bc4_r_snorm:
			case Format::bc4_r_unorm:
				return 4;
			case Format::bc2_rgba_unorm:
			case Format::bc2_rgba_unorm_srgb:
			case Format::bc3_rgba_unorm:
			case Format::bc3_rgba_unorm_srgb:
			case Format::bc5_rg_snorm:
			case Format::bc5_rg_unorm:
			case Format::bc6h_rgb_sfloat:
			case Format::bc6h_rgb_ufloat:
			case Format::bc7_rgba_unorm:
			case Format::bc7_rgba_unorm_srgb:
				return 8;
			default:
				lupanic();
				return 0;
			}
		}

        enum class TextureType : u8
		{
			//! Specify one-dimensional texture.
			tex1d,
			//! Specify two-dimensional texture.
			tex2d,
			//! Specify three-dimensional texture.
			tex3d,
		};

		enum class TextureUsageFlag : u16
		{
			none = 0x00,
			//! Allows this texture to be bound as copy source.
			copy_source = 0x01,
			//! Allows this texture to be bound as copy destination.
			copy_dest = 0x02,
			//! Allows this texture to be bound to a read texture view.
			read_texture = 0x04,
			//! Allows this texture to be bound to a read-write texture view.
			read_write_texture = 0x08,
			//! Allows this texture to be bound as color attachment.
			color_attachment = 0x10,
			//! Allows this texture to be bound as depth stencil attachment.
			depth_stencil_attachment = 0x20,
			//! Allows this texture to be bound to a resolve attachment.
			resolve_attachment = 0x40,
			//! Allows this texture to be bound as a texture cube view.
			cube = 0x80,
		};

		struct TextureDesc
		{
			//! The type of the texture.
			TextureType type;
			//! The pixel format of the texture.
			Format format;
			//! The width of the texture.
			u32 width;
			//! The height of the texture.
			//! This should always be 1 for 1D textures.
			u32 height;
			//! The depth of the texture.
			//! This should always be 1 for 1D and 2D textures.
			u32 depth;
			//! The texture array size, specify 1 if this is not a texture array.
			//! This should always be 1 for 3D textures.
			//! This should be times of 6 of `usages` contains `TextureUsageFlag::cube`.
			u32 array_size;
			//! The number of mip-map slices. 
			//! Specify 0 tells the system to create full mip-map chain for the resource.
			u32 mip_levels;
			//! The sample count per pixel for multi-sample texture resource, specify 1 if the 
			//! multi-sample is disabled for this texture.
			//! This should always be 1 for 1D and 3D textures.
			u32 sample_count;
			//! A combination of `TextureUsageFlag` flags to indicate all possible 
			//! usages of this texture.
			TextureUsageFlag usages;
			//! The resource flags.
			ResourceFlag flags;

			static inline TextureDesc tex1d(Format format, TextureUsageFlag usages, u32 width, u32 array_size = 1, u32 mip_levels = 0, ResourceFlag flags = ResourceFlag::none)
			{
				TextureDesc d;
				d.type = TextureType::tex1d;
				d.format = format;
				d.width = width;
				d.height = 1;
				d.depth = 1;
				d.array_size = array_size;
				d.mip_levels = mip_levels;
				d.sample_count = 1;
				d.usages = usages;
				d.flags = flags;
				return d;
			}
			static inline TextureDesc tex2d(Format format, TextureUsageFlag usages, u32 width, u32 height, u32 array_size = 1, u32 mip_levels = 0,
				u32 sample_count = 1, ResourceFlag flags = ResourceFlag::none)
			{
				TextureDesc d;
				d.type = TextureType::tex2d;
				d.format = format;
				d.width = width;
				d.height = height;
				d.depth = 1;
				d.array_size = array_size;
				d.mip_levels = mip_levels;
				d.sample_count = sample_count;
				d.usages = usages;
				d.flags = flags;
				return d;
			}
			static inline TextureDesc texcube(Format format, TextureUsageFlag usages, u32 width, u32 height, u32 num_cubes = 1, u32 mip_levels = 0,
				u32 sample_count = 1, ResourceFlag flags = ResourceFlag::none)
			{
				TextureDesc d;
				d.type = TextureType::tex2d;
				d.format = format;
				d.width = width;
				d.height = height;
				d.depth = 1;
				d.array_size = num_cubes * 6;
				d.mip_levels = mip_levels;
				d.sample_count = sample_count;
				d.usages = usages;
				d.usages |= TextureUsageFlag::cube;
				d.flags = flags;
				return d;
			}
			static inline TextureDesc tex3d(Format format, TextureUsageFlag usages, u32 width, u32 height, u32 depth, u32 mip_levels = 0, ResourceFlag flags = ResourceFlag::none)
			{
				TextureDesc d;
				d.type = TextureType::tex3d;
				d.format = format;
				d.width = width;
				d.height = height;
				d.depth = depth;
				d.array_size = 1;
				d.mip_levels = mip_levels;
				d.sample_count = 1;
				d.usages = usages;
				d.flags = flags;
				return d;
			}
		};

        struct DepthStencilValue
		{
			f32 depth;
			u8 stencil;
		};

		enum class ClearValueType : u32
		{
			color = 1,
			depth_stencil = 2
		};

		struct ClearValue
		{
			Format format;
			ClearValueType type;
			union
			{
				f32 color[4];
				DepthStencilValue depth_stencil;
			};
			static ClearValue as_color(Format format, f32 color[4])
			{
				ClearValue r;
				r.format = format;
				r.type = ClearValueType::color;
				r.color[0] = color[0];
				r.color[1] = color[1];
				r.color[2] = color[2];
				r.color[3] = color[3];
				return r;
			}
			static ClearValue as_depth_stencil(Format format, f32 depth, u8 stencil)
			{
				ClearValue r;
				r.type = ClearValueType::depth_stencil;
				r.format = format;
				r.depth_stencil.depth = depth;
				r.depth_stencil.stencil = stencil;
				return r;
			}
		};

        struct ITexture : virtual IResource
		{
			luiid("{66189448-3914-4055-A4B3-AE3D6EF57F1A}");

			virtual TextureDesc get_desc() = 0;
		};
    }
}