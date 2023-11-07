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
        constexpr u32 DDS_HEADER_FLAGS_TEXTURE = 0x00001007;  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
        constexpr u32 DDS_HEADER_FLAGS_MIPMAP = 0x00020000;   // DDSD_MIPMAPCOUNT
        constexpr u32 DDS_HEADER_FLAGS_PITCH = 0x00000008;  // DDSD_PITCH
        constexpr u32 DDS_HEADER_FLAGS_LINEARSIZE = 0x00080000; // DDSD_LINEARSIZE
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

        inline constexpr u32 make_four_cc(u8 ch0, u8 ch1, u8 ch2, u8 ch3)
        {
            return (static_cast<u32>(ch0)
                | (static_cast<u32>(ch1) << 8)
                | (static_cast<u32>(ch2) << 16)
                | (static_cast<u32>(ch3) << 24));
        }

        constexpr u32 DX10_FOURCC = make_four_cc('D', 'X', '1', '0');

        LUNA_IMAGE_API R<DDSImageDesc> read_dds_image_file_desc(const void* data, usize data_size)
        {
            lucheck(data && data_size);
#ifndef LUNA_PLATFORM_LITTLE_ENDIAN
            return set_error(BasicError::not_supported(), "read_dds_image_file_desc is not implemented on big endian platforms");
#else
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
            const DDSHeader* header = (const DDSHeader*)((const byte_t*)data + sizeof(u32));
            // Verify header to validate DDS file
            if(header->size != sizeof(DDSHeader))
            {
                return BasicError::not_supported();
            }
            if(header->ddspf.size != sizeof(DDSPixelFormat))
            {
                return BasicError::not_supported();
            }
            desc.mip_levels = header->mip_map_count;
            if(desc.mip_levels == 0) desc.mip_levels = 1;
            // Check for DX10 extension
            if((header->ddspf.flags & DDS_FOURCC)
                && (header->ddspf.four_cc == DX10_FOURCC))
            {
                if(header->size != sizeof(DDSHeader)
                    || header->ddspf.size != sizeof(DDSPixelFormat))
                {
                    return set_error(BasicError::not_supported(), "Legacy DDS formats (without DX10 extension) are not suuported.");
                }
                // Buffer must be big enough for both headers and magic value.
                if(data_size < (sizeof(DDSHeader) + sizeof(u32) + sizeof(DDSHeaderDXT10)))
                {
                    return BasicError::not_supported();
                }
                const DDSHeaderDXT10* d3d10ext = (const DDSHeaderDXT10*)((const byte_t*)data + sizeof(u32) + sizeof(DDSHeader));;
                desc.array_size = d3d10ext->array_size;
                if(desc.array_size == 0)
                {
                    return BasicError::bad_data();
                }
                desc.format = d3d10ext->format;
                if(!((u32)desc.format >= 1 && (u32)desc.format <= 191) || 
                    (u32)desc.format >= 111 && (u32)desc.format <= 114)
                {
                    return BasicError::not_supported();
                }
                if(d3d10ext->misc_flag & DDS_RESOURCE_MISC_TEXTURECUBE)
                {
                    set_flags(desc.flags, DDSFlag::texturecube);
                }
                switch(d3d10ext->resource_dimension)
                {
                    case DDSDimension::tex1d:
                        // D3DX writes 1D textures with a fixed Height of 1
                        if((header->flags & DDS_HEIGHT) && header->height != 1)
                        {
                            return BasicError::bad_data();
                        }
                        desc.width = header->width;
                        desc.height = 1;
                        desc.depth = 1;
                        desc.dimension = DDSDimension::tex1d;
                        break;
                    case DDSDimension::tex2d:
                        if(d3d10ext->misc_flag & DDS_RESOURCE_MISC_TEXTURECUBE)
                        {
                            set_flags(desc.flags, DDSFlag::texturecube);
                            desc.array_size *= 6;
                        }
                        desc.width = header->width;
                        desc.height = header->height;
                        desc.depth = 1;
                        desc.dimension = DDSDimension::tex2d;
                        break;
                    case DDSDimension::tex3d:
                        if(!(header->flags & DDS_HEADER_FLAGS_VOLUME))
                        {
                            return BasicError::bad_data();
                        }
                        if(desc.array_size > 1) return BasicError::not_supported();
                        desc.width = header->width;
                        desc.height = header->height;
                        desc.depth = header->depth;
                        desc.dimension = DDSDimension::tex3d;
                        break;
                    default: return BasicError::bad_data();
                }
            }
            else
            {
                // We do not support legacy DX9 DDS files.
                return set_error(BasicError::not_supported(), "Legacy DX9 DDS formats are not supported.");
            }
            return desc;
#endif
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
            usize total_pixel_size = 0;
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

                        total_pixel_size += slice_pitch;
                        ++num_images;

                        if (h > 1)
                            h >>= 1;

                        if (w > 1)
                            w >>= 1;
                    }
                }
                break;
                case DDSDimension::tex3d:
                {
                    usize w = desc.width;
                    usize h = desc.height;
                    usize d = desc.depth;

                    for (usize level = 0; level < desc.mip_levels; ++level)
                    {
                        usize row_pitch, slice_pitch;
                        auto r = compute_pitch(desc.format, w, h, row_pitch, slice_pitch);
                        if (failed(r))
                        {
                            num_images = out_pixel_size = 0;
                            return r;
                        }
                        total_pixel_size += slice_pitch * d;
                        ++num_images;
                        if (h > 1)
                            h >>= 1;

                        if (w > 1)
                            w >>= 1;

                        if (d > 1)
                            d >>= 1;
                    }
                }
                break;
            }
            out_num_images = num_images;
            out_pixel_size = total_pixel_size;
            return ok;
        }
        bool setup_image_array(const u8* pixels, usize pixel_size, DDSImageDesc& desc, Vector<DDSSubresource>& subresources)
        {
            usize index = 0;
            const byte_t* start_bits = pixels;
            const byte_t* end_bits = pixels + pixel_size;
            switch (desc.dimension)
            {
            case DDSDimension::tex1d:
            case DDSDimension::tex2d:
                if (desc.array_size == 0 || desc.mip_levels == 0)
                {
                    return false;
                }
                for (usize item = 0; item < desc.array_size; ++item)
                {
                    usize w = desc.width;
                    usize h = desc.height;

                    for (usize level = 0; level < desc.mip_levels; ++level)
                    {
                        if (index >= subresources.size())
                        {
                            return false;
                        }

                        usize row_pitch, slice_pitch;
                        if (failed(compute_pitch(desc.format, w, h, row_pitch, slice_pitch)))
                            return false;

                        subresources[index].width = w;
                        subresources[index].height = h;
                        subresources[index].depth = 1;
                        subresources[index].row_pitch = row_pitch;
                        subresources[index].slice_pitch = slice_pitch;
                        subresources[index].data_offset = pixels - start_bits;
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
                    if (desc.mip_levels == 0 || desc.depth == 0)
                    {
                        return false;
                    }

                    usize w = desc.width;
                    usize h = desc.height;
                    usize d = desc.depth;

                    for (usize level = 0; level < desc.mip_levels; ++level)
                    {
                        usize row_pitch, slice_pitch;
                        if (failed(compute_pitch(desc.format, w, h, row_pitch, slice_pitch)))
                            return false;

                        if (index >= subresources.size())
                        {
                            return false;
                        }

                        // We use the same memory organization that Direct3D 11 needs for D3D11_SUBRESOURCE_DATA
                        // with all slices of a given miplevel being continuous in memory
                        subresources[index].width = w;
                        subresources[index].height = h;
                        subresources[index].depth = d;
                        subresources[index].row_pitch = row_pitch;
                        subresources[index].slice_pitch = slice_pitch;
                        subresources[index].data_offset = pixels - start_bits;
                        ++index;

                        pixels += slice_pitch * d;
                        if (pixels > end_bits)
                        {
                            return false;
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
            if(!setup_image_array(image.data.data(), image.data.size(), image.desc, image.subresources))
            {
                image.subresources.clear();
                image.subresources.shrink_to_fit();
                image.data.clear();
                return BasicError::failure();
            }
            return ok;
        }
        RV copy_image(const byte_t* pixels, usize size, DDSImage& image)
        {
            usize pixel_size = image.data.size();
            if(pixel_size > size)
            {
                return BasicError::end_of_file();
            }
            Vector<DDSSubresource> subresources(image.subresources.size());
            if(!setup_image_array(pixels, size, image.desc, subresources))
            {
                return BasicError::failure();
            }
            switch(image.desc.dimension)
            {
                case DDSDimension::tex1d:
                case DDSDimension::tex2d:
                {
                    usize index = 0;
                    for (usize item = 0; item < image.desc.array_size; ++item)
                    {
                        usize lastgood = 0;
                        for (usize level = 0; level < image.desc.mip_levels; ++level, ++index)
                        {
                            luassert(image.subresources[index].row_pitch == subresources[index].row_pitch);
                            usize row_pitch = image.subresources[index].row_pitch;

                            const byte_t *src = pixels + subresources[index].data_offset;
                            byte_t* dst = image.data.data() + image.subresources[index].data_offset;

                            if (is_compressed(image.desc.format))
                            {
                                luassert(image.subresources[index].slice_pitch == subresources[index].slice_pitch);
                                usize slice_pitch = image.subresources[index].slice_pitch;
                                memcpy(dst, src, slice_pitch);
                            }
                            else
                            {
                                memcpy_bitmap(dst, src, row_pitch, image.subresources[index].height, row_pitch, row_pitch);
                                src += row_pitch * image.subresources[index].height;
                                dst += row_pitch * image.subresources[index].height;
                            }
                        }
                    }
                }
                break;
                case DDSDimension::tex3d:
                {
                    usize index = 0;
                    usize d = image.desc.depth;
                    usize lastgood = 0;
                    for (usize level = 0; level < image.desc.mip_levels; ++level)
                    {
                        for (usize slice = 0; slice < d; ++slice, ++index)
                        {
                            luassert(image.subresources[index].row_pitch == subresources[index].row_pitch);
                            usize row_pitch = image.subresources[index].row_pitch;

                            const byte_t* src = pixels + subresources[index].data_offset;
                            byte_t* dst = image.data.data() + image.subresources[index].data_offset;

                            if (is_compressed(image.desc.format))
                            {
                                luassert(image.subresources[index].slice_pitch == subresources[index].slice_pitch);
                                usize slice_pitch = image.subresources[index].slice_pitch;
                                memcpy(dst, src, slice_pitch);
                            }
                            else
                            {
                                memcpy_bitmap(dst, src, row_pitch, image.subresources[index].height, row_pitch, row_pitch);
                                src += row_pitch * image.subresources[index].height;
                                dst += row_pitch * image.subresources[index].height;
                            }
                        }
                        if (d > 1) d >>= 1;
                    }
                }
                break;
            }
            return ok;
        }
        LUNA_IMAGE_API R<DDSImage> new_dds_image(const DDSImageDesc& desc)
        {
            DDSImage image;
            lutry
            {
                image.desc = desc;
                usize pixel_size = 0;
                if(image.desc.mip_levels == 0)
                {
                    image.desc.mip_levels = 1;
                    u32 width = image.desc.width;
                    u32 height = image.desc.height;
                    u32 depth = image.desc.depth;
                    while(width > 1 || height > 1 || depth > 1)
                    {
                        ++(image.desc.mip_levels);
                        if(width > 1) width >>= 1;
                        if(height > 1) height >>= 1;
                        if(depth > 1) depth >>= 1;
                    }
                }
                image.subresources.resize(image.desc.array_size * image.desc.mip_levels);
                usize data_offset = 0;
                for(u32 item = 0; item < image.desc.array_size; ++item)
                {
                    u32 width = image.desc.width;
                    u32 height = image.desc.height;
                    u32 depth = image.desc.depth;
                    for (u32 mip = 0; mip < image.desc.mip_levels; ++mip)
                    {
                        auto& dst = image.subresources[calc_dds_subresoruce_index(mip, item, image.desc.mip_levels)];
                        dst.width = width;
                        dst.height = height;
                        dst.depth = depth;
                        luexp(compute_pitch(image.desc.format, dst.width, dst.height, dst.row_pitch, dst.slice_pitch));
                        dst.data_offset = data_offset;
                        data_offset += dst.slice_pitch * dst.depth;
                        if(width > 1) width >>= 1;
                        if(height > 1) height >>= 1;
                        if(depth > 1) depth >>= 1;
                    }
                }
                image.data = Blob(data_offset);
            }
            lucatchret;
            return image;
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
                luexp(copy_image((const byte_t*)pixels, data_size - offset, r));
            }
            lucatchret;
            return r;
        }
        constexpr u32 DDS_SURFACE_FLAGS_TEXTURE = 0x00001000; // DDSCAPS_TEXTURE
        constexpr u32 DDS_SURFACE_FLAGS_MIPMAP = 0x00400008; // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
        constexpr u32 DDS_SURFACE_FLAGS_CUBEMAP = 0x00000008;// DDSCAPS_COMPLEX
        constexpr u32 DDS_CUBEMAP_POSITIVEX = 0x00000600; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
        constexpr u32 DDS_CUBEMAP_NEGATIVEX = 0x00000a00; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
        constexpr u32 DDS_CUBEMAP_POSITIVEY = 0x00001200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
        constexpr u32 DDS_CUBEMAP_NEGATIVEY = 0x00002200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
        constexpr u32 DDS_CUBEMAP_POSITIVEZ = 0x00004200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
        constexpr u32 DDS_CUBEMAP_NEGATIVEZ = 0x00008200; // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

        constexpr u32 DDS_CUBEMAP_ALLFACES = ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |
                                    DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |
                                    DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ );

        constexpr u32 DDS_FLAGS_VOLUME = 0x00200000; // DDSCAPS2_VOLUME
        RV encode_dds_header(ISeekableStream* stream, const DDSImageDesc& desc)
        {
            lutry
            {
                usize header_size = sizeof(u32) + sizeof(DDSHeader) + sizeof(DDSHeaderDXT10);
                // write dds magic number.
                luexp(stream->write("DDS ", 4));
                DDSHeader header;
                memzero(&header);
                header.size = sizeof(DDSHeader);
                header.ddspf.size = sizeof(DDSPixelFormat);
                header.flags = DDS_HEADER_FLAGS_TEXTURE;
                header.caps = DDS_SURFACE_FLAGS_TEXTURE;
                if(desc.mip_levels > 0)
                {
                    header.flags |= DDS_HEADER_FLAGS_MIPMAP;
                    if(desc.mip_levels > UINT16_MAX) return BasicError::bad_arguments();
                    header.mip_map_count = desc.mip_levels;
                    if(header.mip_map_count > 1) header.caps |= DDS_SURFACE_FLAGS_MIPMAP;
                }
                switch(desc.dimension)
                {
                    case DDSDimension::tex1d:
                        header.width = desc.width;
                        header.height = header.depth = 1;
                        break;
                    case DDSDimension::tex2d:
                        header.width = desc.width;
                        header.height = desc.height;
                        header.depth = 1;
                        if(test_flags(desc.flags, DDSFlag::texturecube))
                        {
                            header.caps |= DDS_SURFACE_FLAGS_CUBEMAP;
                            header.caps2 |= DDS_CUBEMAP_ALLFACES;
                        }
                        break;
                    case DDSDimension::tex3d:
                        header.flags |= DDS_HEADER_FLAGS_VOLUME;
                        header.caps2 |= DDS_FLAGS_VOLUME;
                        header.width = desc.width;
                        header.height = desc.height;
                        header.depth = desc.depth;
                        break;
                    default: lupanic();
                }
                usize row_pitch, slice_pitch;
                luexp(compute_pitch(desc.format, desc.width, desc.height, row_pitch, slice_pitch));
                if(row_pitch > (usize)U32_MAX || slice_pitch > (usize)U32_MAX) return BasicError::not_supported();
                if(is_compressed(desc.format))
                {
                    header.flags |= DDS_HEADER_FLAGS_LINEARSIZE;
                    header.pitch_or_linear_size = slice_pitch;
                }
                else
                {
                    header.flags |= DDS_HEADER_FLAGS_PITCH;
                    header.pitch_or_linear_size = row_pitch;
                }
                // Fill DX10 externsion info.
                header.ddspf.flags |= DDS_FOURCC;
                header.ddspf.four_cc = DX10_FOURCC;
                DDSHeaderDXT10 ext;
                memzero(&ext);
                ext.format = desc.format;
                ext.resource_dimension = desc.dimension;
                if(desc.array_size > UINT16_MAX)
                {
                    return BasicError::bad_arguments();
                }
                if(test_flags(desc.flags, DDSFlag::texturecube))
                {
                    ext.misc_flag |= DDS_RESOURCE_MISC_TEXTURECUBE;
                    if((desc.array_size % 6) != 0) return BasicError::bad_arguments();
                    ext.array_size = desc.array_size / 6;
                }
                else
                {
                    ext.array_size = desc.array_size;
                }
                luexp(stream->write(&header, sizeof(DDSHeader)));
                luexp(stream->write(&ext, sizeof(DDSHeaderDXT10)));
            }
            lucatchret;
            return ok;
        }
        usize compute_scanlines(DDSFormat fmt, usize height)
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
                luassert(is_compressed(fmt));
                return max<usize>(1, (height + 3) / 4);
            default:
                luassert(is_valid(fmt));
                luassert(!is_compressed(fmt));
                return height;
            }
        }
        LUNA_IMAGE_API RV write_dds_file(ISeekableStream* stream, const DDSImage& image)
        {
            lutry
            {
                luexp(encode_dds_header(stream, image.desc));
                switch(image.desc.dimension)
                {
                    case DDSDimension::tex1d:
                    case DDSDimension::tex2d:
                    {
                        usize index = 0;
                        for (usize item = 0; item < image.desc.array_size; ++item)
                        {
                            for (usize level = 0; level < image.desc.mip_levels; ++level, ++index)
                            {
                                if (index >= image.subresources.size())
                                    return BasicError::bad_arguments();

                                if(image.subresources[index].data_offset >= image.data.size())
                                    return BasicError::bad_arguments();

                                luassert(image.subresources[index].row_pitch > 0);
                                luassert(image.subresources[index].slice_pitch > 0);

                                usize dds_row_pitch, dds_slice_pitch;
                                luexp(compute_pitch(image.desc.format, image.subresources[index].width, image.subresources[index].height, dds_row_pitch, dds_slice_pitch));

                                if ((image.subresources[index].slice_pitch == dds_slice_pitch) && (dds_slice_pitch <= U32_MAX))
                                {
                                    luexp(stream->write(image.data.data() + image.subresources[index].data_offset, dds_slice_pitch));
                                }
                                else
                                {
                                    const usize row_pitch = image.subresources[index].row_pitch;
                                    if (row_pitch < dds_row_pitch)
                                    {
                                        // DDS uses 1-byte alignment, so if this is happening then the input pitch isn't actually a full line of data
                                        return BasicError::failure();
                                    }
                                    if (dds_row_pitch > U32_MAX)
                                        return BasicError::out_of_range();

                                    const u8* src = image.data.data() + image.subresources[index].data_offset;
                                    const usize lines = compute_scanlines(image.desc.format, image.subresources[index].height);
                                    for (usize j = 0; j < lines; ++j)
                                    {
                                        luexp(stream->write(src, dds_row_pitch));
                                        src += row_pitch;
                                    }
                                }
                            }
                        }
                    }
                    break;
                    case DDSDimension::tex3d:
                    {
                        if (image.desc.array_size != 1)
                            return BasicError::not_supported();

                        usize d = image.desc.depth;

                        usize index = 0;
                        for (usize level = 0; level < image.desc.mip_levels; ++level)
                        {
                            for (usize slice = 0; slice < d; ++slice, ++index)
                            {
                                if (index >= image.subresources.size())
                                    return BasicError::bad_arguments();

                                if(image.subresources[index].data_offset >= image.data.size())
                                    return BasicError::bad_arguments();

                                luassert(image.subresources[index].row_pitch > 0);
                                luassert(image.subresources[index].slice_pitch > 0);

                                usize dds_row_pitch, dds_slice_pitch;
                                luexp(compute_pitch(image.desc.format, image.subresources[index].width, image.subresources[index].height, dds_row_pitch, dds_slice_pitch));

                                if ((image.subresources[index].slice_pitch == dds_slice_pitch) && (dds_slice_pitch <= U32_MAX))
                                {
                                    luexp(stream->write(image.data.data() + image.subresources[index].data_offset, dds_slice_pitch));
                                }
                                else
                                {
                                    const usize row_pitch = image.subresources[index].row_pitch;
                                    if (row_pitch < dds_row_pitch)
                                    {
                                        // DDS uses 1-byte alignment, so if this is happening then the input pitch isn't actually a full line of data
                                        return BasicError::failure();
                                    }
                                    if (dds_row_pitch > U32_MAX) return BasicError::out_of_range();

                                    const u8* src = image.data.data() + image.subresources[index].data_offset;
                                    const usize lines = compute_scanlines(image.desc.format, image.subresources[index].height);
                                    for (usize j = 0; j < lines; ++j)
                                    {
                                        luexp(stream->write(src, dds_row_pitch));
                                        src += row_pitch;
                                    }
                                }
                            }
                            if (d > 1) d >>= 1;
                        }
                    }
                }
            }
            lucatchret;
            return ok;
        }
    }
}