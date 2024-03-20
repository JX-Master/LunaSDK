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
        //! @addtogroup Image
        //! @{
        
        //! Specifies additional flags of one DDS image.
        enum class DDSFlag : u32
        {
            none = 0,
            //! This file stores one or more 2D cube textures.
            texturecube = 0x4,
        };

        //! Specifies pixel formats of one DDS image.
        //! Maps to DXGI_FORMAT
        enum class DDSFormat : u32
        {
            //! DXGI_FORMAT_UNKNOWN
            unknown                                 = 0,
            //! DXGI_FORMAT_R32G32B32A32_TYPELESS
            r32g32b32a32_typeless                   = 1,
            //! DXGI_FORMAT_R32G32B32A32_FLOAT
            r32g32b32a32_float                      = 2,
            //! DXGI_FORMAT_R32G32B32A32_UINT
            r32g32b32a32_uint                       = 3,
            //! DXGI_FORMAT_R32G32B32A32_UINT
            r32g32b32a32_sint                       = 4,
            //! DXGI_FORMAT_R32G32B32_TYPELESS
            r32g32b32_typeless                      = 5,
            //! DXGI_FORMAT_R32G32B32_FLOAT
            r32g32b32_float                         = 6,
            //! DXGI_FORMAT_R32G32B32_UINT
            r32g32b32_uint                          = 7,
            //! DXGI_FORMAT_R32G32B32_SINT
            r32g32b32_sint                          = 8,
            //! DXGI_FORMAT_R16G16B16A16_TYPELESS
            r16g16b16a16_typeless                   = 9,
            //! DXGI_FORMAT_R16G16B16A16_FLOAT
            r16g16b16a16_float                      = 10,
            //! DXGI_FORMAT_R16G16B16A16_UNORM
            r16g16b16a16_unorm                      = 11,
            //! DXGI_FORMAT_R16G16B16A16_UINT
            r16g16b16a16_uint                       = 12,
            //! DXGI_FORMAT_R16G16B16A16_SNORM
            r16g16b16a16_snorm                      = 13,
            //! DXGI_FORMAT_R16G16B16A16_SINT
            r16g16b16a16_sint                       = 14,
            //! DXGI_FORMAT_R32G32_TYPELESS
            r32g32_typeless                         = 15,
            //! DXGI_FORMAT_R32G32_FLOAT
            r32g32_float                            = 16,
            //! DXGI_FORMAT_R32G32_UINT
            r32g32_uint                             = 17,
            //! DXGI_FORMAT_R32G32_SINT
            r32g32_sint                             = 18,
            //! DXGI_FORMAT_R32G8X24_TYPELESS
            r32g8x24_typeless                       = 19,
            //! DXGI_FORMAT_D32_FLOAT_S8X24_UINT
            d32_float_s8x24_uint                    = 20,
            //! DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
            r32_float_x8x24_typeless                = 21,
            //! DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
            x32_typeless_g8x24_uint                 = 22,
            //! DXGI_FORMAT_R10G10B10A2_TYPELESS
            r10g10b10a2_typeless                    = 23,
            //! DXGI_FORMAT_R10G10B10A2_UNORM
            r10g10b10a2_unorm                       = 24,
            //! DXGI_FORMAT_R10G10B10A2_UINT
            r10g10b10a2_uint                        = 25,
            //! DXGI_FORMAT_R11G11B10_FLOAT
            r11g11b10_float                         = 26,
            //! DXGI_FORMAT_R8G8B8A8_TYPELESS
            r8g8b8a8_typeless                       = 27,
            //! DXGI_FORMAT_R8G8B8A8_UNORM
            r8g8b8a8_unorm                          = 28,
            //! DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
            r8g8b8a8_unorm_srgb                     = 29,
            //! DXGI_FORMAT_R8G8B8A8_UINT
            r8g8b8a8_uint                           = 30,
            //! DXGI_FORMAT_R8G8B8A8_SNORM
            r8g8b8a8_snorm                          = 31,
            //! DXGI_FORMAT_R8G8B8A8_SINT
            r8g8b8a8_sint                           = 32,
            //! DXGI_FORMAT_R16G16_TYPELESS
            r16g16_typeless                         = 33,
            //! DXGI_FORMAT_R16G16_FLOAT
            r16g16_float                            = 34,
            //! DXGI_FORMAT_R16G16_UNORM
            r16g16_unorm                            = 35,
            //! DXGI_FORMAT_R16G16_UINT
            r16g16_uint                             = 36,
            //! DXGI_FORMAT_R16G16_SNORM
            r16g16_snorm                            = 37,
            //! DXGI_FORMAT_R16G16_SINT
            r16g16_sint                             = 38,
            //! DXGI_FORMAT_R32_TYPELESS
            r32_typeless                            = 39,
            //! DXGI_FORMAT_D32_FLOAT
            d32_float                               = 40,
            //! DXGI_FORMAT_R32_FLOAT
            r32_float                               = 41,
            //! DXGI_FORMAT_R32_UINT
            r32_uint                                = 42,
            //! DXGI_FORMAT_R32_SINT
            r32_sint                                = 43,
            //! DXGI_FORMAT_R24G8_TYPELESS
            r24g8_typeless                          = 44,
            //! DXGI_FORMAT_D24_UNORM_S8_UINT
            d24_unorm_s8_uint                       = 45,
            //! DXGI_FORMAT_R24_UNORM_X8_TYPELESS
            r24_unorm_x8_typeless                   = 46,
            //! DXGI_FORMAT_X24_TYPELESS_G8_UINT
            x24_typeless_g8_uint                    = 47,
            //! DXGI_FORMAT_R8G8_TYPELESS
            r8g8_typeless                           = 48,
            //! DXGI_FORMAT_R8G8_UNORM
            r8g8_unorm                              = 49,
            //! DXGI_FORMAT_R8G8_UINT
            r8g8_uint                               = 50,
            //! DXGI_FORMAT_R8G8_SNORM
            r8g8_snorm                              = 51,
            //! DXGI_FORMAT_R8G8_SINT
            r8g8_sint                               = 52,
            //! DXGI_FORMAT_R16_TYPELESS
            r16_typeless                            = 53,
            //! DXGI_FORMAT_R16_FLOAT
            r16_float                               = 54,
            //! DXGI_FORMAT_D16_UNORM
            d16_unorm                               = 55,
            //! DXGI_FORMAT_R16_UNORM
            r16_unorm                               = 56,
            //! DXGI_FORMAT_R16_UINT
            r16_uint                                = 57,
            //! DXGI_FORMAT_R16_SNORM
            r16_snorm                               = 58,
            //! DXGI_FORMAT_R16_SINT
            r16_sint                                = 59,
            //! DXGI_FORMAT_R8_TYPELESS
            r8_typeless                             = 60,
            //! DXGI_FORMAT_R8_UNORM
            r8_unorm                                = 61,
            //! DXGI_FORMAT_R8_UINT
            r8_uint                                 = 62,
            //! DXGI_FORMAT_R8_SNORM
            r8_snorm                                = 63,
            //! DXGI_FORMAT_R8_SINT
            r8_sint                                 = 64,
            //! DXGI_FORMAT_A8_UNORM
            a8_unorm                                = 65,
            //! DXGI_FORMAT_R1_UNORM
            r1_unorm                                = 66,
            //! DXGI_FORMAT_R9G9B9E5_SHAREDEXP
            r9g9b9e5_sharedexp                      = 67,
            //! DXGI_FORMAT_R8G8_B8G8_UNORM
            r8g8_b8g8_unorm                         = 68,
            //! DXGI_FORMAT_G8R8_G8B8_UNORM
            g8r8_g8b8_unorm                         = 69,
            //! DXGI_FORMAT_BC1_TYPELESS
            bc1_typeless                            = 70,
            //! DXGI_FORMAT_BC1_UNORM
            bc1_unorm                               = 71,
            //! DXGI_FORMAT_BC1_UNORM_SRGB
            bc1_unorm_srgb                          = 72,
            //! DXGI_FORMAT_BC2_TYPELESS
            bc2_typeless                            = 73,
            //! DXGI_FORMAT_BC2_UNORM
            bc2_unorm                               = 74,
            //! DXGI_FORMAT_BC2_UNORM_SRGB
            bc2_unorm_srgb                          = 75,
            //! DXGI_FORMAT_BC3_TYPELESS
            bc3_typeless                            = 76,
            //! DXGI_FORMAT_BC3_UNORM
            bc3_unorm                               = 77,
            //! DXGI_FORMAT_BC3_UNORM_SRGB
            bc3_unorm_srgb                          = 78,
            //! DXGI_FORMAT_BC4_TYPELESS
            bc4_typeless                            = 79,
            //! DXGI_FORMAT_BC4_UNORM
            bc4_unorm                               = 80,
            //! DXGI_FORMAT_BC4_SNORM
            bc4_snorm                               = 81,
            //! DXGI_FORMAT_BC5_TYPELESS
            bc5_typeless                            = 82,
            //! DXGI_FORMAT_BC5_UNORM
            bc5_unorm                               = 83,
            //! DXGI_FORMAT_BC5_SNORM
            bc5_snorm                               = 84,
            //! DXGI_FORMAT_B5G6R5_UNORM
            b5g6r5_unorm                            = 85,
            //! DXGI_FORMAT_B5G5R5A1_UNORM
            b5g5r5a1_unorm                          = 86,
            //! DXGI_FORMAT_B8G8R8A8_UNORM
            b8g8r8a8_unorm                          = 87,
            //! DXGI_FORMAT_B8G8R8X8_UNORM
            b8g8r8x8_unorm                          = 88,
            //! DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
            r10g10b10_xr_bias_a2_unorm              = 89,
            //! DXGI_FORMAT_B8G8R8A8_TYPELESS
            b8g8r8a8_typeless                       = 90,
            //! DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
            b8g8r8a8_unorm_srgb                     = 91,
            //! DXGI_FORMAT_B8G8R8X8_TYPELESS
            b8g8r8x8_typeless                       = 92,
            //! DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
            b8g8r8x8_unorm_srgb                     = 93,
            //! DXGI_FORMAT_BC6H_TYPELESS
            bc6h_typeless                           = 94,
            //! DXGI_FORMAT_BC6H_UF16
            bc6h_uf16                               = 95,
            //! DXGI_FORMAT_BC6H_SF16
            bc6h_sf16                               = 96,
            //! DXGI_FORMAT_BC7_TYPELESS
            bc7_typeless                            = 97,
            //! DXGI_FORMAT_BC7_UNORM
            bc7_unorm                               = 98,
            //! DXGI_FORMAT_BC7_UNORM_SRGB
            bc7_unorm_srgb                          = 99,
            //! DXGI_FORMAT_B4G4R4A4_UNORM
            b4g4r4a4_unorm                          = 115
        };

        //! Checks whether one DDSFormat is a valid format for DDS files.
        //! @param[in] fmt The format to check.
        //! @return Returns `true` if the format is valid, returns `false` otherwise.
        inline bool is_valid(DDSFormat fmt)
        {
            return ((u32)fmt) < 99 || ((u32)fmt) == 115;
        }

        //! Checks whether one DDSFormat is a compressed format.
        //! @param[in] fmt The format to check.
        //! @return Returns `true` if the format is a compressed format, returns `false` otherwise.
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

        //! Checks whether one DDSFormat is a packed format.
        //! @param[in] fmt The format to check.
        //! @return Returns `true` if the format is a packed format, returns `false` otherwise.
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

        //! Gets the number of bits used to represent one pixel in the specified format.
        //! @param[in] fmt The format to check.
        //! @return Returns the number of bits used to represent one pixel in the specified format.
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

        //! Specifies the dimension of one DDS texture.
        enum class DDSDimension : u32
        {
            //! 1D texture.
            tex1d = 2,
            //! 2D texture.
            tex2d = 3,
            //! 3D texture.
            tex3d = 4,
        };

        //! Describes one DDS image.
        struct DDSImageDesc
        {
            //! The width of the image in pixels.
            u32 width;
            //! The height of the image in pixels.
            u32 height;
            //! The depth of the image in pixels.
            u32 depth;
            //! The array size of the image.
            u32 array_size;
            //! The number of mips for every element of the image.
            u32 mip_levels;
            //! The pixel format of the image.
            DDSFormat format;
            //! The image dimension.
            DDSDimension dimension;
            //! Additional flags of the image.
            DDSFlag flags;
        };
        
        //! Describes one subresource in one DDS image.
        struct DDSSubresource
        {
            //! The width of this subresource in pixels.
            u32 width;
            //! The height of this subresource in pixels.
            u32 height;
            //! The depth of this subresource in pixels.
            u32 depth;
            //! The number of bytes to advance between every two rows of data of this subresource.
            usize row_pitch;
            //! The number of bytes to advance between every two slices (rows * columns) of data of this subresource.
            usize slice_pitch;
            //! The offset, in bytes, of the beginning data of this subresource from the beginning of the image pixel data.
            usize data_offset;
        };

        //! Calculates subresource index for the specified subresource in DDS image.
        //! @param[in] mip_slice The mip index of the subresource.
        //! @param[in] array_slice The array index of the subresource.
        //! @param[in] mip_levels The number of mips of the subresource.
        //! @return Returns the calculated subresource index.
        inline constexpr u32 calc_dds_subresoruce_index(u32 mip_slice, u32 array_slice, u32 mip_levels)
        {
            return array_slice * mip_levels + mip_slice;
        }

        //! Represents one loaded DDS image.
        //! @details Do not initialize this object by yourself, instead, creates one empty DDS image object by calling @ref new_dds_image,
        //! which will allocate pixel memory and initialize subresource descriptors
        struct DDSImage
        {
            //! The image descriptor.
            DDSImageDesc desc;
            //! The image pixel data.
            Blob data;
            //! An array of subresource descriptors.
            Array<DDSSubresource> subresources;

            DDSImage() :
                desc({}) {}
        };

        //! Creates one new DDS image object that can be saved later.
        //! @param[in] desc The DDS image descriptor.
        //! @return Returns the created DDS image. The pixel memory of the returned DDS image is allocated but uninitialized, 
        //! the user should fill the pixel data by itself.
        LUNA_IMAGE_API R<DDSImage> new_dds_image(const DDSImageDesc& desc);
        //! Reads DDS image description from DDS image file data.
        //! @param[in] data The image file data.
        //! @param[in] data_size The size of the image file data in bytes.
        //! @return Returns the read image description.
        LUNA_IMAGE_API R<DDSImageDesc> read_dds_image_file_desc(const void* data, usize data_size);
        //! Reads DDS image data from DDS image file data.
        //! @param[in] data The image file data.
        //! @param[in] data_size The size of the image file data in bytes.
        //! @return Returns the read DDS image.
        LUNA_IMAGE_API R<DDSImage> read_dds_image(const void* data, usize data_size);
        //! Writes the DDS image to one DDS file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] image The DDS image to write.
        LUNA_IMAGE_API RV write_dds_file(ISeekableStream* stream, const DDSImage& image);

        //! @}
    }
}