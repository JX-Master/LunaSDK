/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureAsset.cpp
* @author JXMaster
* @date 2020/5/9
*/
#include "Texture.hpp"
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Image/RHIHelper.hpp>
#include <Luna/RHIUtility/MipmapGenerationContext.hpp>
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    struct TextureAssetUserdata
    {
        lustruct("TextureAssetUserdata", "{816CDA20-AB1C-4E24-A7CE-59E2EFE9BE1E}");

        Ref<RHIUtility::IMipmapGenerationContext> m_mipmap_generation_context;
        SpinLock m_lock;

        RV init();

        RV generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
    };
    RV TextureAssetUserdata::init()
    {
        using namespace RHI;
        lutry
        {
            luset(m_mipmap_generation_context, RHIUtility::new_mipmap_generation_context(g_env->device));
        }
        lucatchret;
        return ok;
    }
    RV TextureAssetUserdata::generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
    {
        using namespace RHI;
        lutry
        {
            LockGuard guard(m_lock);
            m_mipmap_generation_context->generate_mipmaps(resource_with_most_detailed_mip);
            luexp(m_mipmap_generation_context->commit(compute_cmdbuf, true));
            m_mipmap_generation_context->reset();
        }
        lucatchret;
        return ok;
    }
    static R<ObjRef> load_texture_asset(object_t userdata, Asset::asset_t asset, const Path& path)
    {
        ObjRef ret;
        lutry
        {
            // Open image file.
            Path file_path = path;
            file_path.append_extension("tex");
            auto file = VFS::open_file(file_path, FileOpenFlag::read, FileCreationMode::open_existing);
            if (failed(file))
            {
                file_path.replace_extension("dds");
                file = VFS::open_file(file_path, FileOpenFlag::read, FileCreationMode::open_existing);
            }
            if (failed(file))
            {
                return file.errcode();
            }
            lulet(file_data, load_file_data(file.get()));
            if (file_data.size() >= 4 && !memcmp((const c8*)file_data.data(), "DDS ", 4))
            {
                lulet(dds_image, Image::read_dds_image(file_data.data(), file_data.size()));
                RHI::TextureDesc desc;
                switch (dds_image.desc.dimension)
                {
                case Image::DDSDimension::tex2d: desc.type = RHI::TextureType::tex2d; break;
                case Image::DDSDimension::tex3d: desc.type = RHI::TextureType::tex3d; break;
                case Image::DDSDimension::tex1d: desc.type = RHI::TextureType::tex1d; break;
                default: lupanic();
                }
                desc.format = Image::dds_to_rhi_format(dds_image.desc.format);
                if (desc.format == RHI::Format::unknown) luthrow(set_error(BasicError::not_supported(), "Unsupported DDS formats."));
                desc.width = dds_image.desc.width;
                desc.height = dds_image.desc.height;
                desc.depth = dds_image.desc.depth;
                desc.array_size = dds_image.desc.array_size;
                desc.mip_levels = dds_image.desc.mip_levels;
                desc.sample_count = 1;
                desc.usages = RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest;
                if (test_flags(dds_image.desc.flags, Image::DDSFlag::texturecube))
                {
                    desc.usages |= RHI::TextureUsageFlag::cube;
                }
                desc.flags = RHI::ResourceFlag::none;
                // Create resource.
                lulet(tex, g_env->device->new_texture(RHI::MemoryType::local, desc));
                // Upload data.
                lulet(upload_cmdbuf, g_env->device->new_command_buffer(g_env->async_copy_queue));
                auto writer = RHIUtility::new_resource_write_context(g_env->device);
                for (u32 item = 0; item < desc.array_size; ++item)
                {
                    u32 d = desc.depth;
                    for (u32 mip = 0; mip < desc.mip_levels; ++mip)
                    {
                        auto& subresource = dds_image.subresources[Image::calc_dds_subresoruce_index(mip, item, desc.mip_levels)];
                        u32 row_pitch, slice_pitch;
                        lulet(mapped, writer->write_texture(tex, RHI::SubresourceIndex(mip, item), 0, 0, 0, subresource.width, subresource.height, d, row_pitch, slice_pitch));
                        memcpy_bitmap3d(mapped, (const u8*)dds_image.data.data() + subresource.data_offset, subresource.row_pitch, subresource.height, d, row_pitch, subresource.row_pitch, slice_pitch, subresource.slice_pitch);
                    }
                    if (d > 1) d >>= 1;
                }
                luexp(writer->commit(upload_cmdbuf, true));
                tex->set_name(path.encode().c_str());
                ret = tex;
            }
            else if (file_data.size() >= 8 && !memcmp((const c8*)file_data.data(), "LUNAMIPS", 8))
            {
                u64* dp = (u64*)((u8*)file_data.data() + 8);
                u32 num_mips = (u32)*dp;
                ++dp;
                Vector<Pair<u64, u64>> mip_descs;
                mip_descs.reserve(num_mips);
                for (u64 i = 0; i < num_mips; ++i)
                {
                    Pair<u64, u64> p;
                    p.first = dp[i * 2];
                    p.second = dp[i * 2 + 1];
                    mip_descs.push_back(p);
                }
                // Load texture from file.
                lulet(desc, Image::read_image_file_desc((const u8*)file_data.data() + mip_descs[0].first, mip_descs[0].second));
                auto desired_format = Image::get_rhi_desired_format(desc.format);
                // Create resource.
                lulet(tex, g_env->device->new_texture(RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    Image::image_to_rhi_format(desc.format), 
                    RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest, 
                    desc.width, desc.height)));
                // Upload data
                auto writer = RHIUtility::new_resource_write_context(g_env->device);
                for(u32 i = 0; i < num_mips; ++i)
                {
                    Image::ImageDesc desc;
                    lulet(image_data, Image::read_image_file((const u8*)file_data.data() + mip_descs[i].first, mip_descs[i].second, desired_format, desc));
                    u32 row_pitch, slice_pitch;
                    lulet(mapped, writer->write_texture(tex, RHI::SubresourceIndex(i, 0), 0, 0, 0, desc.width, desc.height, 1, row_pitch, slice_pitch));
                    memcpy_bitmap(mapped, image_data.data(), pixel_size(desc.format) * desc.width, desc.height, row_pitch, pixel_size(desc.format) * desc.width);
                }
                lulet(upload_cmdbuf, g_env->device->new_command_buffer(g_env->async_copy_queue));
                luexp(writer->commit(upload_cmdbuf, true));
                tex->set_name(path.encode().c_str());
                ret = tex;
            }
            else
            {
                // Load texture from file.
                lulet(desc, Image::read_image_file_desc(file_data.data(), file_data.size()));
                auto desired_format = Image::get_rhi_desired_format(desc.format);
                lulet(image_data, Image::read_image_file(file_data.data(), file_data.size(), desired_format, desc));
                // Create resource.
                lulet(tex, RHI::get_main_device()->new_texture(RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    Image::image_to_rhi_format(desc.format),
                    RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest,
                    desc.width, desc.height)));
                // Upload data.
                lulet(upload_cmdbuf, g_env->device->new_command_buffer(g_env->async_copy_queue));
                auto writer = RHIUtility::new_resource_write_context(g_env->device);
                u32 row_pitch, slice_pitch;
                lulet(mapped, writer->write_texture(tex, RHI::SubresourceIndex(0,0), 0, 0, 0, desc.width, desc.height, 1, row_pitch, slice_pitch));
                memcpy_bitmap(mapped, image_data.data(), pixel_size(desc.format) * desc.width, desc.height, row_pitch, pixel_size(desc.format) * desc.width);
                luexp(writer->commit(upload_cmdbuf, true));
                // Generate mipmaps.
                Ref<TextureAssetUserdata> ctx = ObjRef(userdata);
                lulet(cmdbuf, g_env->device->new_command_buffer(g_env->async_compute_queue));
                cmdbuf->set_name("MipmapGeneration");
                luexp(ctx->generate_mipmaps(tex, cmdbuf));
                tex->set_name(path.encode().c_str());
                ret = tex;
            }
        }
        lucatchret;
        return ret;
    }
    RV register_static_texture_asset_type()
    {
        lutry
        {
            register_boxed_type<TextureAssetUserdata>();
            Ref<TextureAssetUserdata> userdata = new_object<TextureAssetUserdata>();
            luexp(userdata->init());
            Asset::AssetTypeDesc desc;
            desc.name = get_static_texture_asset_type();
            desc.on_load_asset = load_texture_asset;
            desc.on_save_asset = nullptr;
            desc.on_set_asset_data = nullptr;
            desc.userdata = userdata;
            Asset::register_asset_type(desc);
        }
        lucatchret;
        return ok;
    }
    Name get_static_texture_asset_type()
    {
        return "Static Texture";
    }
}
