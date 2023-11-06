/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DDSImage.cpp
* @author JXMaster
* @date 2023/11/5
*/
#include "Image.hpp"
#include "../DDSImage.hpp"

namespace Luna
{
    namespace Image
    {
        constexpr u32 DDS_FOURCC = 0x00000004;
        constexpr u32 DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L;
        // Mapped to file structure directly.
        struct DDSPixelFormat
        {
            u32    size;
            u32    flags;
            u32    four_cc;
            u32    rgb_bit_count;
            u32    r_bit_mask;
            u32    g_bit_mask;
            u32    b_bit_mask;
            u32    a_bit_mask;
        };
        constexpr u32 DDS_HEIGHT = 0x02;
        constexpr u32 DDS_HEADER_FLAGS_VOLUME = 0x00800000;
        struct DDSHeader
        {
            u32        size;
            u32        flags;
            u32        height;
            u32        width;
            u32        pitch_or_linear_size;
            u32        depth;
            u32        mip_map_count;
            u32        reserved1[11];
            DDSPixelFormat ddspf;
            u32        caps;
            u32        caps2;
            u32        caps3;
            u32        caps4;
            u32        reserved2;
        };
        static_assert(sizeof(DDSHeader) == 124, "DDS header size check failed!");

        struct DDSHeaderDXT10
        {
            DDSFormat      format;
            DDSDimension   resource_dimension;
            u32                 misc_flag; // see D3D11_RESOURCE_MISC_FLAG
            u32                 array_size;
            u32                 misc_flags2; // see DDS_MISC_FLAGS2
        };

        inline void memcpy_le(void* dst, const void* src, usize size)
        {
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
            memcpy(dst, src, size);
#else
            for(usize i = 0; i < size; ++i)
            {
                ((c8*)dst)[i] = ((const c8*)src)[size - i - 1];
            }
#endif
        }

        LUNA_IMAGE_API R<DDSImageDesc> read_dds_image_file_desc(const void* data, usize data_size)
        {
            lucheck(data && data_size);
            DDSImageDesc desc;
            memzero(&desc);
            if(data_size < (sizeof(DDSHeader) + sizeof(u32)))
            {
                return set_error(BasicError::bad_data(), "Invalid DDS file data.");
            }
            // Check header magic number.
            {
                if(memcmp(data, "DDS ", 4))
                {
                    return set_error(BasicError::bad_data(), "DDS file magic number check failed.");
                }
            }
            // Encode header.
            DDSHeader header;
            {
                const u32* src = (const u32*)((const c8*)data + 4);
                for(usize i = 0; i < sizeof(DDSHeader) / sizeof(u32); ++i)
                {
                    memcpy_le(((u32*)&header) + i, src + i, sizeof(u32));
                }
            }
            // Verify header to validate DDS file
            if(header.size != sizeof(DDSHeader))
            {
                return BasicError::not_supported();
            }
            if(header.ddspf.size != sizeof(DDSPixelFormat))
            {
                return BasicError::not_supported();
            }
            desc.mip_levels = header.mip_map_count;
            if(desc.mip_levels == 0) desc.mip_levels = 1;
            // Check for DX10 extension
            if((header.ddspf.flags & DDS_FOURCC)
                && (header.ddspf.four_cc == 'DX10'))
            {
                if(header.size != sizeof(DDSHeader)
                    || header.ddspf.size != sizeof(DDSPixelFormat))
                {
                    // We do not accept legacy DX9 'known variants' for modern "DX10" extension header files.
                    return BasicError::not_supported();
                }
                // Buffer must be big enough for both headers and magic value.
                if(data_size < (sizeof(DDSHeader) + sizeof(u32) + sizeof(DDSHeaderDXT10)))
                {
                    return BasicError::not_supported();
                }
                const u32* src = (const u32*)((const u8*)data + sizeof(u32) + sizeof(DDSHeader));
                DDSHeaderDXT10 d3d10ext;
                for(usize i = 0; i < sizeof(DDSHeaderDXT10) / sizeof(u32); ++i)
                {
                    memcpy_le(((u32*)&d3d10ext) + i, src + i, sizeof(u32));
                }
                desc.array_size = d3d10ext.array_size;
                if(desc.array_size == 0)
                {
                    return BasicError::bad_data();
                }
                desc.format = d3d10ext.format;
                if(!((u32)desc.format >= 1 && (u32)desc.format <= 191) || 
                    (u32)desc.format >= 111 && (u32)desc.format <= 114)
                {
                    return BasicError::not_supported();
                }
                if(d3d10ext.misc_flag & DDS_RESOURCE_MISC_TEXTURECUBE)
                {
                    set_flags(desc.flags, DDSFlag::texturecube);
                }
                switch(d3d10ext.resource_dimension)
                {
                    case DDSDimension::tex1d:
                        // D3DX writes 1D textures with a fixed Height of 1
                        if((header.flags & DDS_HEIGHT) && header.height != 1)
                        {
                            return BasicError::bad_data();
                        }
                        desc.width = header.width;
                        desc.height = 1;
                        desc.depth = 1;
                        desc.dimension = DDSDimension::tex1d;
                        break;
                    case DDSDimension::tex2d:
                        if(d3d10ext.misc_flag & DDS_RESOURCE_MISC_TEXTURECUBE)
                        {
                            set_flags(desc.flags, DDSFlag::texturecube);
                            desc.array_size *= 6;
                        }
                        desc.width = header.width;
                        desc.height = header.height;
                        desc.depth = 1;
                        desc.dimension = DDSDimension::tex2d;
                        break;
                    case DDSDimension::tex3d:
                        if(!(header.flags & DDS_HEADER_FLAGS_VOLUME))
                        {
                            return BasicError::bad_data();
                        }
                        if(desc.array_size > 1) return BasicError::not_supported();
                        desc.width = header.width;
                        desc.height = header.height;
                        desc.depth = header.depth;
                        desc.dimension = DDSDimension::tex3d;
                        break;
                    default: return BasicError::bad_data();
                }
                desc.flags2 = d3d10ext.misc_flags2;
            }
            else
            {
                // We do not support legacy DX9 DDS files.
                return set_error(BasicError::not_supported(), "Legacy DX9 DDS formats are not supported.");
            }
            return desc;
        }
        u32 count_mips(u32 width, u32 height)
        {
            u32 mip_levels = 1;
            while (height > 1 || width > 1)
            {
                if (height > 1)
                    height >>= 1;

                if (width > 1)
                    width >>= 1;

                ++mip_levels;
            }
            return mip_levels;
        }
        u32 count_mips_3d(u32 width, u32 height, u32 depth)
        {
            usize mip_levels = 1;
            while (height > 1 || width > 1 || depth > 1)
            {
                if (height > 1)
                    height >>= 1;

                if (width > 1)
                    width >>= 1;

                if (depth > 1)
                    depth >>= 1;

                ++mip_levels;
            }
            return mip_levels;
        }
        bool calc_mip_levels(u32 width, u32 height, u32& mip_levels)
        {
            if (mip_levels > 1)
            {
                const usize max_mips = count_mips(width, height);
                if (mip_levels > max_mips)
                    return false;
            }
            else if (mip_levels == 0)
            {
                mip_levels = count_mips(width, height);
            }
            else
            {
                mip_levels = 1;
            }
            return true;
        }
        bool calc_mip_levels_3d(u32 width, u32 height, u32 depth, u32& mip_levels)
        {
            if (mip_levels > 1)
            {
                const usize max_mips = count_mips_3d(width, height, depth);
                if (mip_levels > max_mips)
                    return false;
            }
            else if (mip_levels == 0)
            {
                mip_levels = count_mips_3d(width, height, depth);
            }
            else
            {
                mip_levels = 1;
            }
            return true;
        }
        RV compute_pitch(DDSFormat format, usize width, usize height, usize& row_pitch, usize& slice_pitch)
        {
            u64 pitch = 0;
            u64 slice = 0;

            switch (format)
            {
            case DDSFormat::bc1_typeless:
            case DDSFormat::bc1_unorm:
            case DDSFormat::bc1_unorm_srgb:
            case DDSFormat::bc4_typeless:
            case DDSFormat::bc4_unorm:
            case DDSFormat::bc4_snorm:
                luassert(is_compressed(format));
                {
                    const u64 nbw = max<u64>(1u, (u64(width) + 3u) / 4u);
                    const u64 nbh = max<u64>(1u, (u64(height) + 3u) / 4u);
                    pitch = nbw * 8u;
                    slice = pitch * nbh;
                }
                break;
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
                luassert(is_compressed(format));
                {
                    const u64 nbw = max<u64>(1u, (u64(width) + 3u) / 4u);
                    const u64 nbh = max<u64>(1u, (u64(height) + 3u) / 4u);
                    pitch = nbw * 16u;
                    slice = pitch * nbh;
                }
                break;

            case DDSFormat::r8g8_b8g8_unorm:
            case DDSFormat::g8r8_g8b8_unorm:
                luassert(is_packed(format));
                pitch = ((u64(width) + 1u) >> 1) * 4u;
                slice = pitch * u64(height);
                break;
            default:
                luassert(!is_compressed(format) && !is_packed(format));
                {
                    usize bpp = bpp = bits_per_pixel(format);
                    if (!bpp)
                        return BasicError::bad_arguments();
                    pitch = (u64(width) * bpp + 7u) / 8u;
                    slice = pitch * u64(height);
                }
                break;
            }
            row_pitch = static_cast<usize>(pitch);
            slice_pitch = static_cast<usize>(slice);
            return ok;
        }
        RV determine_image_array(const DDSImageDesc& desc, usize& out_num_images, usize& out_pixel_size)
        {
            luassert(desc.width > 0 && desc.height > 0 && desc.depth > 0);
            luassert(desc.array_size > 0);
            luassert(desc.mip_levels > 0);
            u64 total_pixel_size = 0;
            usize num_images = 0;
            switch(desc.dimension)
            {
                case DDSDimension::tex1d:
                case DDSDimension::tex2d:
                    for (usize item = 0; item < desc.array_size; ++item)
                    {
                        usize w = desc.width;
                        usize h = desc.height;
                        for (usize level = 0; level < desc.mip_levels; ++level)
                        {
                            usize row_pitch, slice_pitch;
                            auto r = compute_pitch(desc.format, w, h, row_pitch, slice_pitch);
                            if (failed(r))
                            {
                                out_num_images = out_pixel_size = 0;
                                return r;
                            }

                            total_pixel_size += u64(slice_pitch);
                            ++num_images;

                            if (h > 1)
                                h >>= 1;

                            if (w > 1)
                                w >>= 1;
                        }
                    }
                    break;
            }
        }
        bool setup_image_array(DDSImage& image)
        {
            usize index = 0;
            byte_t* pixels = image.data.data();
            const byte_t* end_bits = image.data.data() + image.data.size();

            switch (image.desc.dimension)
            {
            case DDSDimension::tex1d:
            case DDSDimension::tex2d:
                if (image.desc.array_size == 0 || image.desc.mip_levels == 0)
                {
                    return false;
                }
                for (usize item = 0; item < image.desc.array_size; ++item)
                {
                    usize w = image.desc.width;
                    usize h = image.desc.height;

                    for (usize level = 0; level < image.desc.mip_levels; ++level)
                    {
                        if (index >= image.subresources.size())
                        {
                            return false;
                        }

                        usize row_pitch, slice_pitch;
                        if (failed(compute_pitch(image.desc.format, w, h, row_pitch, slice_pitch)))
                            return false;

                        image.subresources[index].width = w;
                        image.subresources[index].height = h;
                        image.subresources[index].row_pitch = row_pitch;
                        image.subresources[index].slice_pitch = slice_pitch;
                        image.subresources[index].data_offset = pixels - image.data.data();
                        ++index;

                        pixels += slice_pitch;
                        if (pixels > end_bits)
                        {
                            return false;
                        }

                        if (h > 1)
                            h >>= 1;

                        if (w > 1)
                            w >>= 1;
                    }
                }
                return true;

            case DDSDimension::tex3d:
                {
                    if (image.desc.mip_levels == 0 || image.desc.depth == 0)
                    {
                        return false;
                    }

                    usize w = image.desc.width;
                    usize h = image.desc.height;
                    usize d = image.desc.depth;

                    for (usize level = 0; level < image.desc.mip_levels; ++level)
                    {
                        usize row_pitch, slice_pitch;
                        if (failed(compute_pitch(image.desc.format, w, h, row_pitch, slice_pitch)))
                            return false;

                        for (usize slice = 0; slice < d; ++slice)
                        {
                            if (index >= image.subresources.size())
                            {
                                return false;
                            }

                            // We use the same memory organization that Direct3D 11 needs for D3D11_SUBRESOURCE_DATA
                            // with all slices of a given miplevel being continuous in memory
                            image.subresources[index].width = w;
                            image.subresources[index].height = h;
                            image.subresources[index].row_pitch = row_pitch;
                            image.subresources[index].slice_pitch = slice_pitch;
                            image.subresources[index].data_offset = pixels - image.data.data();
                            ++index;

                            pixels += slice_pitch;
                            if (pixels > end_bits)
                            {
                                return false;
                            }
                        }

                        if (h > 1)
                            h >>= 1;

                        if (w > 1)
                            w >>= 1;

                        if (d > 1)
                            d >>= 1;
                    }
                }
                return true;

            default:
                return false;
            }
        }
        RV init_dds_image(DDSImage& image)
        {
            auto& desc = image.desc;
            u32 mip_levels = desc.mip_levels;
            switch(desc.dimension)
            {
                case DDSDimension::tex1d:
                    if (!desc.width || desc.height != 1 || desc.depth != 1 || !desc.array_size)
                        return BasicError::bad_arguments();
                    if (!calc_mip_levels(desc.width, 1, mip_levels))
                        return BasicError::bad_arguments();
                    break;
                case DDSDimension::tex2d:
                    if (!desc.width || !desc.height || desc.depth != 1 || !desc.array_size)
                        return BasicError::bad_arguments();
                    if(test_flags(desc.flags, DDSFlag::texturecube))
                    {
                        if((desc.array_size % 6) != 0)
                            return BasicError::bad_arguments();
                    }
                    if (!calc_mip_levels(desc.width, desc.height, mip_levels))
                        return BasicError::bad_arguments();
                    break;
                case DDSDimension::tex3d:
                    if (!desc.width || !desc.height || !desc.depth || desc.array_size != 1)
                        return BasicError::bad_arguments();
                    if(!calc_mip_levels_3d(desc.width, desc.height, desc.depth, mip_levels))
                        return BasicError::bad_arguments();
                    break;
                default:
                    return BasicError::not_supported();
            }
            image.subresources.clear();
            image.subresources.shrink_to_fit();
            image.data.clear();
            usize pixel_size, num_images;
            auto r = determine_image_array(desc, num_images, pixel_size);
            if(failed(r)) return r;
            image.subresources.resize(num_images);
            image.data = Blob(pixel_size, 16);
            if(!setup_image_array(image))
            {
                image.subresources.clear();
                image.subresources.shrink_to_fit();
                image.data.clear();
                return BasicError::failure();
            }
            return ok;
        }

        LUNA_IMAGE_API R<DDSImage> read_dds_image(const void* data, usize data_size)
        {
            DDSImage r;
            lutry
            {
                luset(r.desc, read_dds_image_file_desc(data, data_size));
                usize offset = sizeof(u32) + sizeof(DDSHeader) + sizeof(DDSHeaderDXT10);
                // Resize images.
                luexp(init_dds_image(r));
                const void* pixels = (const u8*)data + offset;
                
            }
            lucatchret;
            return r;
        }
    }
}