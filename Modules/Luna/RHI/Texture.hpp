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
        //! @addtogroup RHI
        //! @{

        //! Describes data formats for vertices and pixels in RHI.
        enum class Format : u16
        {
            unknown = 0,
            // Ordinary 8-Bit Formats

            //! Ordinary format with one component stored as 8-bit normalized unsigned integer.
            r8_unorm,
            //! Ordinary format with one component stored as 8-bit normalized signed integer.
            r8_snorm,
            //! Ordinary format with one component stored as 8-bit unsigned integer.
            r8_uint,
            //! Ordinary format with one component stored as 8-bit signed integer.
            r8_sint,

            // Ordinary 16-Bit Formats

            //! Ordinary format with one component stored as 16-bit normalized unsigned integer.
            r16_unorm,
            //! Ordinary format with one component stored as 16-bit normalized signed integer.
            r16_snorm,
            //! Ordinary format with one component stored as 16-bit unsigned integer.
            r16_uint,
            //! Ordinary format with one component stored as 16-bit signed integer.
            r16_sint,
            //! Ordinary format with one component stored as 16-bit (half-precision) floating-point number.
            r16_float,
            //! Ordinary format with two components stored as 8-bit normalized unsigned integers.
            rg8_unorm,
            //! Ordinary format with two components stored as 8-bit normalized signed integers.
            rg8_snorm,
            //! Ordinary format with two components stored as 8-bit unsigned integers.
            rg8_uint,
            //! Ordinary format with two components stored as 8-bit signed integers.
            rg8_sint,

            // Ordinary 32-Bit Formats

            //! Ordinary format with one component stored as 32-bit unsigned integer.
            r32_uint,
            //! Ordinary format with one component stored as 32-bit signed integer.
            r32_sint,
            //! Ordinary format with one component stored as 32-bit (single-precision) floating-point number.
            r32_float,
            //! Ordinary format with two components stored as 16-bit normalized unsigned integers.
            rg16_unorm,
            //! Ordinary format with two components stored as 16-bit normalized signed integers.
            rg16_snorm,
            //! Ordinary format with two components stored as 16-bit unsigned integers.
            rg16_uint,
            //! Ordinary format with two components stored as 16-bit signed integers.
            rg16_sint,
            //! Ordinary format with two components stored as 16-bit (half-precision) floating-point numbers.
            rg16_float,
            //! Ordinary format with four components stored as 8-bit normalized unsigned integers in RGBA order.
            rgba8_unorm,
            //! Ordinary format with four components stored as 8-bit normalized unsigned integers in RGBA order with conversion between sRGB and linear space.
            rgba8_unorm_srgb,
            //! Ordinary format with four components stored as 8-bit normalized signed integers in RGBA order.
            rgba8_snorm,
            //! Ordinary format with four components stored as 8-bit unsigned integers in RGBA order.
            rgba8_uint,
            //! Ordinary format with four components stored as 8-bit signed integers in RGBA order.
            rgba8_sint,
            //! Ordinary format with four components stored as 8-bit normalized unsigned integers in BGRA order.
            bgra8_unorm,
            //! Ordinary format with four components stored as 8-bit normalized unsigned integers in BGRA order with conversion between sRGB and linear space.
            bgra8_unorm_srgb,

            // Ordinary 64-Bit Formats

            //! Ordinary format with two components stored as 32-bit unsigned integers.
            rg32_uint,
            //! Ordinary format with two components stored as 32-bit signed integers.
            rg32_sint,
            //! Ordinary format with two components stored as 32-bit (single-precision) floating-point numbers.
            rg32_float,
            //! Ordinary format with four components stored as 16-bit normalized unsigned integers in RGBA order.
            rgba16_unorm,
            //! Ordinary format with four components stored as 16-bit normalized signed integers in RGBA order.
            rgba16_snorm,
            //! Ordinary format with four components stored as 16-bit unsigned integers in RGBA order.
            rgba16_uint,
            //! Ordinary format with four components stored as 16-bit signed integers in RGBA order.
            rgba16_sint,
            //! Ordinary format with four components stored as 16-bit (half-precision) floating-point numbers.
            rgba16_float,

            // Ordinary 96-Bit Formats

            //! Ordinary format with three components stored as 32-bit unsigned integers.
            rgb32_uint,
            //! Ordinary format with three components stored as 32-bit signed integers.
            rgb32_sint,
            //! Ordinary format with three components stored as 32-bit (single-precision) floating-point numbers.
            rgb32_float,

            // Ordinary 128-Bit Formats

            //! Ordinary format with four components stored as 32-bit unsigned integers in RGBA order.
            rgba32_uint,
            //! Ordinary format with four components stored as 32-bit signed integers in RGBA order.
            rgba32_sint,
            //! Ordinary format with four components stored as 32-bit (single-precision) floating-point numbers in RGBA order.
            rgba32_float,

            // Packed 16-Bit Formats

            //! Packed format with three components stored as 5-bit, 6-bit and 5-bit normalized unsigned integers in BGR order.
            b5g6r5_unorm,
            //! Packed format with four components stored as 5-bit, 5-bit, 5-bit and 1-bit normalized unsigned integers in BGRA order.
            bgr5a1_unorm,

            // Packed 32-Bit Formats

            //! Packed format with four components stored as 10-bit, 10-bit, 10-bit and 2-bit normalized unsigned integers in RGBA order.
            rgb10a2_unorm,
            //! Packed format with four components stored as 10-bit, 10-bit, 10-bit and 2-bit unsigned integers in RGBA order.
            rgb10a2_uint,
            //! Packed format with three components stored as 11-bit, 11-bit and 10-bit floating-point numbers in RGB order.
            //! The components have no sign bit. The 10-bit float has 5 bits of mantissa and 5 bits of exponent. The 11-bit floats have 6-bit mantissa and 5-bit exponent.
            rg11b10_float,
            //! Packed format with three components stored as floating-point numbers in RGB order. Each component has 9-bit mantissa, and a 5-bit exponent is shared by
            //! all three components.
            rgb9e5_float,

            // Depth-stencil Formats

            //! Depth stencil format with one depth component stored as 16-bit normalized unsigned integer.
            d16_unorm,
            //! Depth stencil format with one depth component stored as 32-bit (single-precision) floating-point number.
            d32_float,
            //! Depth stencil format with one depth component stored as 24-bit normalized unsigned integer and one stencil component stored as 8-bit unsigned integer.
            d24_unorm_s8_uint,
            //! Depth stencil format with one depth component stored as 32-bit (single-precision) floating-point number and one stencil component stored as 8-bit unsigned integer.
            d32_float_s8_uint_x24,

            // Compressed Formats

            //! BC1 (DXT1) compressed format.
            bc1_rgba_unorm,
            //! BC1 (DXT1) compressed format with conversion between sRGB and linear space.
            bc1_rgba_unorm_srgb,
            //! BC2 (DXT3) compressed format.
            bc2_rgba_unorm,
            //! BC2 (DXT3) compressed format with conversion between sRGB and linear space.
            bc2_rgba_unorm_srgb,
            //! BC3 (DXT5) compressed format.
            bc3_rgba_unorm,   
            //! BC3 (DXT5) compressed format with conversion between sRGB and linear space.
            bc3_rgba_unorm_srgb,
            //! BC4 compressed format with one component stored as normalized unsigned integer.
            bc4_r_unorm,
            //! BC4 compressed format with one component stored as normalized signed integer.
            bc4_r_snorm,
            //! BC5 compressed format with two components stored as normalized unsigned integer.
            bc5_rg_unorm,
            //! BC5 compressed format with two components stored as normalized signed integer.
            bc5_rg_snorm,
            //! BC6H compressed format with four floating-point components.
            bc6h_rgb_sfloat,
            //! BC6H compressed format with four unsigned floating-point components.
            bc6h_rgb_ufloat,
            //! BC7 compressed format.
            bc7_rgba_unorm,
            //! BC7 compressed format with conversion between sRGB and linear space.
            bc7_rgba_unorm_srgb,

            count
        };

        //! Gets the size of one pixel in the specified format, in bits.
        //! @param[in] format The format to check.
        //! @return Returns the number of bits used for one pixel of the specified format.
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

        //! Specifies the texture type.
        enum class TextureType : u8
        {
            //! Specify one-dimensional texture.
            tex1d,
            //! Specify two-dimensional texture.
            tex2d,
            //! Specify three-dimensional texture.
            tex3d,
        };

        //! Specifies possible usages of one texture resource.
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

        //! Describes one texture resource.
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

        //! Describes one pair of depth and stencil values used for clearing 
        //! depth stencil attachments.
        struct DepthStencilValue
        {
            f32 depth;
            u8 stencil;
        };

        //! Specifies the clear value type used.
        enum class ClearValueType : u32
        {
            color = 1,
            depth_stencil = 2
        };

        //! Describes one clear value used to specify optimized clear value for texture resources.
        struct ClearValue
        {
            //! The format of the texture.
            Format format;
            //! The type of the clear value.
            ClearValueType type;
            union
            {
                //! The clear color to use if `type` is @ref ClearValueType::color.
                f32 color[4];
                //! The depth stencil clear value to use if `type` is @ref ClearValueType::depth_stencil.
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

        //! @interface ITexture
        //! Represents one texture resource that can be used to contain 
        //! pixel data of certain format.
        struct ITexture : virtual IResource
        {
            luiid("{66189448-3914-4055-A4B3-AE3D6EF57F1A}");

            //! Gets the descriptor of the texture.
            virtual TextureDesc get_desc() = 0;
        };

        //! @}
    }
}