/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DDSImage.hpp
* @author JXMaster
* @date 2023/11/5
*/
#pragma once
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Stream.hpp>

#ifndef LUNA_IMAGE_API
#define LUNA_IMAGE_API
#endif

namespace Luna
{
    namespace Image
    {
        enum class DDSFlag : u32
        {
            none = 0,
            texturecube = 0x4,
        };

        // Maps to DXGI_FORMAT
        enum class DDSFormat : u32
        {
            unknown	                                = 0,
            r32g32b32a32_typeless                   = 1,
            r32g32b32a32_float                      = 2,
            r32g32b32a32_uint                       = 3,
            r32g32b32a32_sint                       = 4,
            r32g32b32_typeless                      = 5,
            r32g32b32_float                         = 6,
            r32g32b32_uint                          = 7,
            r32g32b32_sint                          = 8,
            r16g16b16a16_typeless                   = 9,
            r16g16b16a16_float                      = 10,
            r16g16b16a16_unorm                      = 11,
            r16g16b16a16_uint                       = 12,
            r16g16b16a16_snorm                      = 13,
            r16g16b16a16_sint                       = 14,
            r32g32_typeless                         = 15,
            r32g32_float                            = 16,
            r32g32_uint                             = 17,
            r32g32_sint                             = 18,
            r32g8x24_typeless                       = 19,
            d32_float_s8x24_uint                    = 20,
            r32_float_x8x24_typeless                = 21,
            x32_typeless_g8x24_uint                 = 22,
            r10g10b10a2_typeless                    = 23,
            r10g10b10a2_unorm                       = 24,
            r10g10b10a2_uint                        = 25,
            r11g11b10_float                         = 26,
            r8g8b8a8_typeless                       = 27,
            r8g8b8a8_unorm                          = 28,
            r8g8b8a8_unorm_srgb                     = 29,
            r8g8b8a8_uint                           = 30,
            r8g8b8a8_snorm                          = 31,
            r8g8b8a8_sint                           = 32,
            r16g16_typeless                         = 33,
            r16g16_float                            = 34,
            r16g16_unorm                            = 35,
            r16g16_uint                             = 36,
            r16g16_snorm                            = 37,
            r16g16_sint                             = 38,
            r32_typeless                            = 39,
            d32_float                               = 40,
            r32_float                               = 41,
            r32_uint                                = 42,
            r32_sint                                = 43,
            r24g8_typeless                          = 44,
            d24_unorm_s8_uint                       = 45,
            r24_unorm_x8_typeless                   = 46,
            x24_typeless_g8_uint                    = 47,
            r8g8_typeless                           = 48,
            r8g8_unorm                              = 49,
            r8g8_uint                               = 50,
            r8g8_snorm                              = 51,
            r8g8_sint                               = 52,
            r16_typeless                            = 53,
            r16_float                               = 54,
            d16_unorm                               = 55,
            r16_unorm                               = 56,
            r16_uint                                = 57,
            r16_snorm                               = 58,
            r16_sint                                = 59,
            r8_typeless                             = 60,
            r8_unorm                                = 61,
            r8_uint                                 = 62,
            r8_snorm                                = 63,
            r8_sint                                 = 64,
            a8_unorm                                = 65,
            r1_unorm                                = 66,
            r9g9b9e5_sharedexp                      = 67,
            r8g8_b8g8_unorm                         = 68,
            g8r8_g8b8_unorm                         = 69,
            bc1_typeless                            = 70,
            bc1_unorm                               = 71,
            bc1_unorm_srgb                          = 72,
            bc2_typeless                            = 73,
            bc2_unorm                               = 74,
            bc2_unorm_srgb                          = 75,
            bc3_typeless                            = 76,
            bc3_unorm                               = 77,
            bc3_unorm_srgb                          = 78,
            bc4_typeless                            = 79,
            bc4_unorm                               = 80,
            bc4_snorm                               = 81,
            bc5_typeless                            = 82,
            bc5_unorm                               = 83,
            bc5_snorm                               = 84,
            b5g6r5_unorm                            = 85,
            b5g5r5a1_unorm                          = 86,
            b8g8r8a8_unorm                          = 87,
            b8g8r8x8_unorm                          = 88,
            r10g10b10_xr_bias_a2_unorm              = 89,
            b8g8r8a8_typeless                       = 90,
            b8g8r8a8_unorm_srgb                     = 91,
            b8g8r8x8_typeless                       = 92,
            b8g8r8x8_unorm_srgb                     = 93,
            bc6h_typeless                           = 94,
            bc6h_uf16                               = 95,
            bc6h_sf16                               = 96,
            bc7_typeless                            = 97,
            bc7_unorm                               = 98,
            bc7_unorm_srgb                          = 99,
            b4g4r4a4_unorm                          = 115
        };

        inline bool is_valid(DDSFormat fmt)
        {
            return ((u32)fmt) < 99 || ((u32)fmt) == 115;
        }

        inline bool is_compressed(DDSFormat fmt)
        {
            switch (fmt)
            {
            case DDSFormat::bc1_typeless:
            case DDSFormat::bc1_unorm:
            case DDSFormat::bc1_unorm_srgb:
            case DDSFormat::bc2_typeless:
            case DDSFormat::bc2_unorm:
            case DDSFormat::bc2_unorm_srgb:
            case DDSFormat::bc3_typeless:
            case DDSFormat::bc3_unorm:
            case DDSFormat::bc3_unorm_srgb:
            case DDSFormat::bc4_typeless:
            case DDSFormat::bc4_unorm:
            case DDSFormat::bc4_snorm:
            case DDSFormat::bc5_typeless:
            case DDSFormat::bc5_unorm:
            case DDSFormat::bc5_snorm:
            case DDSFormat::bc6h_typeless:
            case DDSFormat::bc6h_uf16:
            case DDSFormat::bc6h_sf16:
            case DDSFormat::bc7_typeless:
            case DDSFormat::bc7_unorm:
            case DDSFormat::bc7_unorm_srgb:
                return true;

            default:
                return false;
            }
        }

        inline bool is_packed(DDSFormat fmt)
        {
            switch (fmt)
            {
            case DDSFormat::r8g8_b8g8_unorm:
            case DDSFormat::g8r8_g8b8_unorm:
                return true;
            default:
                return false;
            }
        }

        inline usize bits_per_pixel(DDSFormat fmt)
        {
            switch (fmt)
            {
            case DDSFormat::r32g32b32a32_typeless:
            case DDSFormat::r32g32b32a32_float:
            case DDSFormat::r32g32b32a32_uint:
            case DDSFormat::r32g32b32a32_sint:
                return 128;

            case DDSFormat::r32g32b32_typeless:
            case DDSFormat::r32g32b32_float:
            case DDSFormat::r32g32b32_uint:
            case DDSFormat::r32g32b32_sint:
                return 96;

            case DDSFormat::r16g16b16a16_typeless:
            case DDSFormat::r16g16b16a16_float:
            case DDSFormat::r16g16b16a16_unorm:
            case DDSFormat::r16g16b16a16_uint:
            case DDSFormat::r16g16b16a16_snorm:
            case DDSFormat::r16g16b16a16_sint:
            case DDSFormat::r32g32_typeless:
            case DDSFormat::r32g32_float:
            case DDSFormat::r32g32_uint:
            case DDSFormat::r32g32_sint:
            case DDSFormat::r32g8x24_typeless:
            case DDSFormat::d32_float_s8x24_uint:
            case DDSFormat::r32_float_x8x24_typeless:
            case DDSFormat::x32_typeless_g8x24_uint:
                return 64;

            case DDSFormat::r10g10b10a2_typeless:
            case DDSFormat::r10g10b10a2_unorm:
            case DDSFormat::r10g10b10a2_uint:
            case DDSFormat::r11g11b10_float:
            case DDSFormat::r8g8b8a8_typeless:
            case DDSFormat::r8g8b8a8_unorm:
            case DDSFormat::r8g8b8a8_unorm_srgb:
            case DDSFormat::r8g8b8a8_uint:
            case DDSFormat::r8g8b8a8_snorm:
            case DDSFormat::r8g8b8a8_sint:
            case DDSFormat::r16g16_typeless:
            case DDSFormat::r16g16_float:
            case DDSFormat::r16g16_unorm:
            case DDSFormat::r16g16_uint:
            case DDSFormat::r16g16_snorm:
            case DDSFormat::r16g16_sint:
            case DDSFormat::r32_typeless:
            case DDSFormat::d32_float:
            case DDSFormat::r32_float:
            case DDSFormat::r32_uint:
            case DDSFormat::r32_sint:
            case DDSFormat::r24g8_typeless:
            case DDSFormat::d24_unorm_s8_uint:
            case DDSFormat::r24_unorm_x8_typeless:
            case DDSFormat::x24_typeless_g8_uint:
            case DDSFormat::r9g9b9e5_sharedexp:
            case DDSFormat::r8g8_b8g8_unorm:
            case DDSFormat::g8r8_g8b8_unorm:
            case DDSFormat::b8g8r8a8_unorm:
            case DDSFormat::b8g8r8x8_unorm:
            case DDSFormat::r10g10b10_xr_bias_a2_unorm:
            case DDSFormat::b8g8r8a8_typeless:
            case DDSFormat::b8g8r8a8_unorm_srgb:
            case DDSFormat::b8g8r8x8_typeless:
            case DDSFormat::b8g8r8x8_unorm_srgb:
                return 32;

            case DDSFormat::r8g8_typeless:
            case DDSFormat::r8g8_unorm:
            case DDSFormat::r8g8_uint:
            case DDSFormat::r8g8_snorm:
            case DDSFormat::r8g8_sint:
            case DDSFormat::r16_typeless:
            case DDSFormat::r16_float:
            case DDSFormat::d16_unorm:
            case DDSFormat::r16_unorm:
            case DDSFormat::r16_uint:
            case DDSFormat::r16_snorm:
            case DDSFormat::r16_sint:
            case DDSFormat::b5g6r5_unorm:
            case DDSFormat::b5g5r5a1_unorm:
            case DDSFormat::b4g4r4a4_unorm:
                return 16;

            case DDSFormat::r8_typeless:
            case DDSFormat::r8_unorm:
            case DDSFormat::r8_uint:
            case DDSFormat::r8_snorm:
            case DDSFormat::r8_sint:
            case DDSFormat::a8_unorm:
            case DDSFormat::bc2_typeless:
            case DDSFormat::bc2_unorm:
            case DDSFormat::bc2_unorm_srgb:
            case DDSFormat::bc3_typeless:
            case DDSFormat::bc3_unorm:
            case DDSFormat::bc3_unorm_srgb:
            case DDSFormat::bc5_typeless:
            case DDSFormat::bc5_unorm:
            case DDSFormat::bc5_snorm:
            case DDSFormat::bc6h_typeless:
            case DDSFormat::bc6h_uf16:
            case DDSFormat::bc6h_sf16:
            case DDSFormat::bc7_typeless:
            case DDSFormat::bc7_unorm:
            case DDSFormat::bc7_unorm_srgb:
                return 8;

            case DDSFormat::r1_unorm:
                return 1;

            case DDSFormat::bc1_typeless:
            case DDSFormat::bc1_unorm:
            case DDSFormat::bc1_unorm_srgb:
            case DDSFormat::bc4_typeless:
            case DDSFormat::bc4_unorm:
            case DDSFormat::bc4_snorm:
                return 4;

            default:
                return 0;
            }
        }

        enum class DDSDimension : u32
        {
            tex1d = 2,
            tex2d = 3,
            tex3d = 4,
        };

        struct DDSImageDesc
        {
            u32 width;
            u32 height;
            u32 depth;
            u32 array_size;
            u32 mip_levels;
            DDSFormat format;
            DDSDimension dimension;
            DDSFlag flags;
        };

        struct DDSSubresource
        {
            u32 width;
            u32 height;
            u32 depth;
            usize row_pitch;
            usize slice_pitch;
            usize data_offset;
        };

        inline constexpr u32 calc_dds_subresoruce_index(u32 mip_slice, u32 array_slice, u32 mip_levels)
        {
            return array_slice * mip_levels + mip_slice;
        }

        struct DDSImage
        {
            DDSImageDesc desc;
            Blob data;
            Vector<DDSSubresource> subresources;

            DDSImage() :
                desc({}) {}
        };

        LUNA_IMAGE_API R<DDSImage> new_dds_image(const DDSImageDesc& desc);
        LUNA_IMAGE_API R<DDSImageDesc> read_dds_image_file_desc(const void* data, usize data_size);
        LUNA_IMAGE_API R<DDSImage> read_dds_image(const void* data, usize data_size);
        LUNA_IMAGE_API RV write_dds_file(ISeekableStream* stream, const DDSImage& image);
    }
}