/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureImporter.cpp
* @author JXMaster
* @date 2020/5/8
*/
#include "Texture.hpp"
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/RHIUtility/ResourceReadContext.hpp>
#include <Luna/Image/DDSImage.hpp>
#include <Luna/Image/RHIHelper.hpp>

#include <MipmapGenerationCS.hpp>
#include <PrecomputeEnvironmentMapMips.hpp>

namespace Luna
{
    enum class TexturePrefilerType : u8
    {
        normal = 0,
        environment_map = 1,
    };

    enum class TextureFileType : u8
    {
        image = 0,
        dds = 1,
    };

    struct TextureFile
    {
        Path m_path;
        String m_asset_name;
        Blob m_file_data;
        TextureFileType m_type;
        // For image files.
        Image::ImageDesc m_desc;
        TexturePrefilerType m_prefiler_type;
        // For DDS files.
        Image::DDSImageDesc m_dds_desc;
    };

    struct TextureImporter : public IAssetEditor
    {
        lustruct("TextureImporter", "{29488656-e1e3-4e7d-b772-2cf93308ba8b}");
        luiimpl();

        Path m_create_dir;

        Vector<TextureFile> m_files;

        Ref<RHI::IDescriptorSetLayout> m_mipmapping_dlayout;
        Ref<RHI::IPipelineLayout> m_mipmapping_playout;
        Ref<RHI::IPipelineState> m_mipmapping_pso;

        Ref<RHI::IDescriptorSetLayout> m_env_mipmapping_dlayout;
        Ref<RHI::IPipelineLayout> m_env_mipmapping_playout;
        Ref<RHI::IPipelineState> m_env_mipmapping_pso;

        static constexpr u32 ENV_MAP_MIPS = 5;

        RV init();
        void import_texture_asset(const Path& create_dir, const TextureFile& file);
        void import_dds_texture_asset(const Path& create_dir, const Image::DDSImage& file);
        RV generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
        R<Ref<RHI::ITexture>> generate_environment_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);

        bool m_open = true;

        TextureImporter() {}

        virtual void on_render() override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };

    RV TextureImporter::init()
    {
        using namespace RHI;
        lutry
        {
            {
                luset(m_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::all)
                    })));
                auto dlayout = m_mipmapping_dlayout.get();
                luset(m_mipmapping_playout, RHI::get_main_device()->new_pipeline_layout(PipelineLayoutDesc(
                    { &dlayout, 1 },
                    PipelineLayoutFlag::deny_vertex_shader_access |
                    
                    PipelineLayoutFlag::deny_pixel_shader_access)));

                ComputePipelineStateDesc ps_desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(ps_desc, MipmapGenerationCS);
                ps_desc.pipeline_layout = m_mipmapping_playout;
                luset(m_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
            }
            {
                luset(m_env_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::all)
                    })));
                auto dlayout = m_env_mipmapping_dlayout.get();
                luset(m_env_mipmapping_playout, RHI::get_main_device()->new_pipeline_layout(PipelineLayoutDesc(
                    { &dlayout, 1 },
                    PipelineLayoutFlag::deny_vertex_shader_access |
                    PipelineLayoutFlag::deny_pixel_shader_access)));
                ComputePipelineStateDesc ps_desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(ps_desc, PrecomputeEnvironmentMapMips);
                ps_desc.pipeline_layout = m_env_mipmapping_playout;
                luset(m_env_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
            }
        }
        lucatchret;
        return ok;
    }

    RV TextureImporter::generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
    {
        using namespace RHI;
        lutry
        {
            auto desc = resource_with_most_detailed_mip->get_desc();
            lucheck(desc.mip_levels);
            lucheck(desc.type == TextureType::tex2d);
            lucheck(desc.depth == 1);

            if (desc.mip_levels == 1)
            {
                return ok;
            }

            auto device = g_env->device;

            if (!m_mipmapping_playout)
            {
                luexp(init());
            }
            compute_cmdbuf->begin_compute_pass();
            compute_cmdbuf->set_compute_pipeline_layout(m_mipmapping_playout);
            compute_cmdbuf->set_compute_pipeline_state(m_mipmapping_pso);
            u32 cb_align = (u32)device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
            lulet(cb, device->new_buffer(MemoryType::upload,
                BufferDesc(BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

            void* mapped = nullptr;
            luexp(cb->map(0, 0, &mapped));
            for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
            {
                u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
                u32 height = max<u32>(desc.height >> (j + 1), 1);
                Float2U* dst = (Float2U*)((usize)mapped + cb_size * j);
                dst->x = 1.0f / (f32)width;
                dst->y = 1.0f / (f32)height;
            }
            cb->unmap(0, USIZE_MAX);

            u32 width = desc.width / 2;
            u32 height = desc.height / 2;

            for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
            {
                TextureBarrier barriers[] = {
                    {resource_with_most_detailed_mip, SubresourceIndex(j, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {resource_with_most_detailed_mip, SubresourceIndex(j + 1, 0),TextureStateFlag::automatic, TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
                };
                compute_cmdbuf->resource_barrier({}, { barriers, 2 });
                lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_mipmapping_dlayout)));
                luexp(vs->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb, cb_size * j, cb_size)),
                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(resource_with_most_detailed_mip, Format::unknown, j, 1)),
                    WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(resource_with_most_detailed_mip, Format::unknown, j + 1, 1)),
                    WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                    }));
                compute_cmdbuf->set_compute_descriptor_set(0, vs);
                compute_cmdbuf->attach_device_object(vs);
                compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
                width = max<u32>(width / 2, 1);
                height = max<u32>(height / 2, 1);
            }
            compute_cmdbuf->end_compute_pass();
            compute_cmdbuf->resource_barrier({}, { {resource_with_most_detailed_mip, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::none, ResourceBarrierFlag::none} });
            luexp(compute_cmdbuf->submit({}, {}, true));
            compute_cmdbuf->wait();
            luexp(compute_cmdbuf->reset());
        }
        lucatchret;
        return ok;
    }

    R<Ref<RHI::ITexture>> TextureImporter::generate_environment_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
    {
        using namespace RHI;
        Ref<RHI::ITexture> prefiltered;
        lutry
        {
            auto desc = resource_with_most_detailed_mip->get_desc();
            lucheck(desc.mip_levels);
            lucheck(desc.type == TextureType::tex2d);
            lucheck(desc.depth == 1);

            auto device = g_env->device;

            if (!m_mipmapping_playout)
            {
                luexp(init());
            }

            desc.mip_levels = ENV_MAP_MIPS;
            luset(prefiltered, device->new_texture(MemoryType::local, desc));
            desc = prefiltered->get_desc();

            compute_cmdbuf->begin_compute_pass();
            compute_cmdbuf->set_compute_pipeline_layout(m_env_mipmapping_playout);
            compute_cmdbuf->set_compute_pipeline_state(m_env_mipmapping_pso);
            struct CB
            {
                u32 tex_width;
                u32 tex_height;
                u32 mip_0_width;
                u32 mip_0_height;
                f32 roughness;
            };
            usize cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            u32 cb_size = (u32)align_upper(sizeof(CB), cb_align);
            lulet(cb, device->new_buffer(MemoryType::upload,
                BufferDesc(BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

            void* mapped = nullptr;
            luexp(cb->map(0, 0, &mapped));
            for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
            {
                u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
                u32 height = max<u32>(desc.height >> (j + 1), 1);
                CB* dst = (CB*)((usize)mapped + cb_size * j);
                dst->tex_width = width;
                dst->tex_height = height;
                dst->mip_0_width = (u32)desc.width;
                dst->mip_0_height = desc.height;
                dst->roughness = 1.0f / (desc.mip_levels - 1) * (j + 1);
            }
            cb->unmap(0, USIZE_MAX);

            compute_cmdbuf->resource_barrier({},
                {
                    {resource_with_most_detailed_mip, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_source, ResourceBarrierFlag::none},
                    {prefiltered, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::discard_content}
                });
            compute_cmdbuf->copy_texture(prefiltered, SubresourceIndex(0, 0), 0, 0, 0, resource_with_most_detailed_mip, SubresourceIndex(0, 0), 0, 0, 0, desc.width, desc.height, 1);

            compute_cmdbuf->resource_barrier({},
                {
                    {resource_with_most_detailed_mip, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {prefiltered, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic,  TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
                });

            for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
            {
                u32 dst_mip = j + 1;
                lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_env_mipmapping_dlayout)));
                luexp(vs->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb, cb_size * j, cb_size)),
                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(resource_with_most_detailed_mip)),
                    WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(prefiltered, desc.format, dst_mip, 1)),
                    WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                    }));
                compute_cmdbuf->set_compute_descriptor_set(0, vs);
                compute_cmdbuf->attach_device_object(vs);
                u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
                u32 height = max<u32>(desc.height >> (j + 1), 1);
                compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
            }
            compute_cmdbuf->end_compute_pass();
            luexp(compute_cmdbuf->submit({}, {}, true));
            compute_cmdbuf->wait();
            luexp(compute_cmdbuf->reset());
        }
        lucatchret;
        return prefiltered;
    }

    static Ref<IAssetEditor> new_static_texture_importer(const Path& create_dir)
    {
        auto dialog = new_object<TextureImporter>();
        dialog->m_create_dir = create_dir;
        return dialog;
    }
    void TextureImporter::import_texture_asset(const Path& create_dir, const TextureFile& file)
    {
        lutry
        {
            using namespace RHI;
            auto device = g_env->device;
            Asset::asset_t asset;
            Ref<ITexture> tex;
            if(file.m_type == TextureFileType::image)
            {
                // Create resource.
                luset(tex, device->new_texture(RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    Image::image_to_rhi_format(file.m_desc.format), 
                    RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest,
                    file.m_desc.width, file.m_desc.height)));
                // Upload data.
                {
                    Image::ImageDesc image_desc;
                    lulet(img_data, Image::read_image_file(file.m_file_data.data(), file.m_file_data.size(), Image::get_rhi_desired_format(file.m_desc.format), image_desc));
                    lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
                    auto writer = RHIUtility::new_resource_write_context(g_env->device);
                    u32 row_pitch, slice_pitch;
                    lulet(mapped, writer->write_texture(tex, RHI::SubresourceIndex(0, 0), 0, 0, 0, image_desc.width, image_desc.height, 1, row_pitch, slice_pitch));
                    memcpy_bitmap(mapped, img_data.data(), pixel_size(image_desc.format) * image_desc.width, image_desc.height, row_pitch, pixel_size(image_desc.format) * image_desc.width);
                    luexp(writer->commit(upload_cmdbuf, true));
                }
                // Generate mipmaps.
                {
                    lulet(cmd, device->new_command_buffer(g_env->async_compute_queue));
                    luexp(generate_mipmaps(tex, cmd));
                    if (file.m_prefiler_type == TexturePrefilerType::environment_map)
                    {
                        luset(tex, generate_environment_mipmaps(tex, cmd));
                    }
                }
                // Read data.
                //auto desc = tex->get_desc();
                //Vector<Image::ImageDesc> descs;
                //Vector<Blob> img_data;
                //Vector<Pair<u64, u64>> offsets;
                //for (u32 i = 0; i < desc.mip_levels; ++i)
                //{
                //    u32 mip_width = max<u32>(desc.width >> i, 1);
                //    u32 mip_height = max<u32>(desc.height >> i, 1);
                //    usize row_pitch = (usize)mip_width * bits_per_pixel(desc.format) / 8;
                //    usize data_sz = row_pitch * (usize)mip_height;
                //    Blob data(data_sz);
                //    lulet(readback_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
                //    luexp(copy_resource_data(readback_cmdbuf, { CopyResourceData::read_texture(data.data(), (u32)row_pitch, (u32)data_sz, tex, SubresourceIndex(i, 0), 0, 0, 0, mip_width, mip_height, 1) }));
                //    img_data.push_back(move(data));
                //}
                //u64 file_offset = 0;
                //Path file_path = create_dir;
                //file_path.push_back(file.m_asset_name);
                //luset(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
                //file_path.append_extension("tex");
                //lulet(f, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
                //luexp(f->write("LUNAMIPS", 8));
                //u64 num_mips = desc.mip_levels;
                //luexp(f->write(&num_mips, sizeof(u64)));
                //luexp(f->seek(sizeof(u64) * (2 * img_data.size() + 1), SeekMode::current));
                //// Convert data to file.
                //for (u32 i = 0; i < desc.mip_levels; ++i)
                //{
                //    u32 mip_width = max<u32>((u32)desc.width >> i, 1);
                //    u32 mip_height = max<u32>(desc.height >> i, 1);

                //    Image::ImageDesc img_desc;
                //    img_desc.width = mip_width;
                //    img_desc.height = mip_height;
                //    img_desc.format, Image::rhi_to_image_format(desc.format);
                //    if (img_desc.format == Image::ImageFormat::unknown)
                //    {
                //        luthrow(BasicError::not_supported());
                //    }
                //    Pair<u64, u64> offset;
                //    luset(offset.first, f->tell());
                //    if (img_desc.format == Image::ImageFormat::r32_float ||
                //        img_desc.format == Image::ImageFormat::rg32_float ||
                //        img_desc.format == Image::ImageFormat::rgba32_float)
                //    {
                //        luexp(Image::write_hdr_file(f, img_desc, img_data[i]));
                //    }
                //    else
                //    {
                //        luexp(Image::write_png_file(f, img_desc, img_data[i]));
                //    }
                //    luset(offset.second, f->tell());
                //    offset.second -= offset.first;
                //    offsets.push_back(offset);
                //}
                //// Write header.
                //luexp(f->seek(16, SeekMode::begin));
                //luexp(f->write(offsets.data(), offsets.size() * sizeof(Pair<u64, u64>)));
                //f.reset();
            }
            else if(file.m_type == TextureFileType::dds)
            {
                lulet(dds_image, Image::read_dds_image(file.m_file_data.data(), file.m_file_data.size()));
                RHI::TextureDesc desc;
                switch(dds_image.desc.dimension)
                {
                    case Image::DDSDimension::tex2d: desc.type = TextureType::tex2d; break;
                    case Image::DDSDimension::tex3d: desc.type = TextureType::tex3d; break;
                    case Image::DDSDimension::tex1d: desc.type = TextureType::tex1d; break;
                    default: lupanic();
                }
                desc.format = Image::dds_to_rhi_format(dds_image.desc.format);
                if(desc.format == RHI::Format::unknown) luthrow(set_error(BasicError::not_supported(), "Unsupported DDS formats."));
                desc.width = dds_image.desc.width;
                desc.height = dds_image.desc.height;
                desc.depth = dds_image.desc.depth;
                desc.array_size = dds_image.desc.array_size;
                desc.mip_levels = dds_image.desc.mip_levels;
                desc.sample_count = 1;
                desc.usages = RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest;
                if(test_flags(dds_image.desc.flags, Image::DDSFlag::texturecube))
                {
                    desc.usages |= RHI::TextureUsageFlag::cube;
                }
                desc.flags = RHI::ResourceFlag::none;
                // Create resource (only for checking whether this DDS file can be created).
                luset(tex, device->new_texture(RHI::MemoryType::local, desc));
                // Upload data.
                {
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
                }
            }
            else lupanic();
            // Write asset file (DDS).
            {
                auto desc = tex->get_desc();
                Image::DDSImageDesc image_desc;
                image_desc.width = desc.width;
                image_desc.height = desc.height;
                image_desc.depth = desc.depth;
                image_desc.array_size = desc.array_size;
                image_desc.mip_levels = desc.mip_levels;
                image_desc.format = Image::rhi_to_dds_format(desc.format);
                switch (desc.type)
                {
                case RHI::TextureType::tex1d: image_desc.dimension = Image::DDSDimension::tex1d; break;
                case RHI::TextureType::tex2d: image_desc.dimension = Image::DDSDimension::tex2d; break;
                case RHI::TextureType::tex3d: image_desc.dimension = Image::DDSDimension::tex3d; break;
                default: break;
                }
                if (test_flags(desc.usages, TextureUsageFlag::cube))
                {
                    set_flags(image_desc.flags, Image::DDSFlag::texturecube);
                }
                lulet(image, Image::new_dds_image(image_desc));
                auto reader = RHIUtility::new_resource_read_context(g_env->device);
                Vector<usize> read_ops;
                for (u32 item = 0; item < desc.array_size; ++item)
                {
                    for (u32 mip = 0; mip < desc.mip_levels; ++mip)
                    {
                        auto& dst = image.subresources[Image::calc_dds_subresoruce_index(mip, item, desc.mip_levels)];
                        read_ops.push_back(reader->read_texture(tex, RHI::SubresourceIndex(mip, item), 0, 0, 0, dst.width, dst.height, dst.depth));
                    }
                }
                lulet(readback_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
                luexp(reader->commit(readback_cmdbuf, true));
                usize read_i = 0;
                for (u32 item = 0; item < desc.array_size; ++item)
                {
                    for (u32 mip = 0; mip < desc.mip_levels; ++mip)
                    {
                        auto& dst = image.subresources[Image::calc_dds_subresoruce_index(mip, item, desc.mip_levels)];
                        usize read_op = read_ops[read_i];
                        ++read_i;
                        u32 row_pitch, slice_pitch;
                        lulet(mapped, reader->get_texture_data(read_op, row_pitch, slice_pitch));
                        memcpy_bitmap3d((u8*)image.data.data() + dst.data_offset, mapped, dst.row_pitch, dst.height, dst.depth, dst.row_pitch, row_pitch, dst.slice_pitch, slice_pitch);
                    }
                }
                Path file_path = create_dir;
                file_path.push_back(file.m_asset_name);
                luset(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
                file_path.append_extension("dds");
                lulet(f, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
                luexp(Image::write_dds_file(f, image));
                f.reset();
            }
            luexp(Asset::load_asset(asset));
        }
        lucatch
        {
            auto _ = Window::message_box(explain(luerr), "Failed to import texture asset",
                                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
        }
    }

    const c8* print_dds_format(Image::DDSFormat format)
    {
        switch(format)
        {
            case Image::DDSFormat::r32g32b32a32_typeless      : return "r32g32b32a32_typeless     ";
            case Image::DDSFormat::r32g32b32a32_float         : return "r32g32b32a32_float        ";
            case Image::DDSFormat::r32g32b32a32_uint          : return "r32g32b32a32_uint         ";
            case Image::DDSFormat::r32g32b32a32_sint          : return "r32g32b32a32_sint         ";
            case Image::DDSFormat::r32g32b32_typeless         : return "r32g32b32_typeless        ";
            case Image::DDSFormat::r32g32b32_float            : return "r32g32b32_float           ";
            case Image::DDSFormat::r32g32b32_uint             : return "r32g32b32_uint            ";
            case Image::DDSFormat::r32g32b32_sint             : return "r32g32b32_sint            ";
            case Image::DDSFormat::r16g16b16a16_typeless      : return "r16g16b16a16_typeless     ";
            case Image::DDSFormat::r16g16b16a16_float         : return "r16g16b16a16_float        ";
            case Image::DDSFormat::r16g16b16a16_unorm         : return "r16g16b16a16_unorm        ";
            case Image::DDSFormat::r16g16b16a16_uint          : return "r16g16b16a16_uint         ";
            case Image::DDSFormat::r16g16b16a16_snorm         : return "r16g16b16a16_snorm        ";
            case Image::DDSFormat::r16g16b16a16_sint          : return "r16g16b16a16_sint         ";
            case Image::DDSFormat::r32g32_typeless            : return "r32g32_typeless           ";
            case Image::DDSFormat::r32g32_float               : return "r32g32_float              ";
            case Image::DDSFormat::r32g32_uint                : return "r32g32_uint               ";
            case Image::DDSFormat::r32g32_sint                : return "r32g32_sint               ";
            case Image::DDSFormat::r32g8x24_typeless          : return "r32g8x24_typeless         ";
            case Image::DDSFormat::d32_float_s8x24_uint       : return "d32_float_s8x24_uint      ";
            case Image::DDSFormat::r32_float_x8x24_typeless   : return "r32_float_x8x24_typeless  ";
            case Image::DDSFormat::x32_typeless_g8x24_uint    : return "x32_typeless_g8x24_uint   ";
            case Image::DDSFormat::r10g10b10a2_typeless       : return "r10g10b10a2_typeless      ";
            case Image::DDSFormat::r10g10b10a2_unorm          : return "r10g10b10a2_unorm         ";
            case Image::DDSFormat::r10g10b10a2_uint           : return "r10g10b10a2_uint          ";
            case Image::DDSFormat::r11g11b10_float            : return "r11g11b10_float           ";
            case Image::DDSFormat::r8g8b8a8_typeless          : return "r8g8b8a8_typeless         ";
            case Image::DDSFormat::r8g8b8a8_unorm             : return "r8g8b8a8_unorm            ";
            case Image::DDSFormat::r8g8b8a8_unorm_srgb        : return "r8g8b8a8_unorm_srgb       ";
            case Image::DDSFormat::r8g8b8a8_uint              : return "r8g8b8a8_uint             ";
            case Image::DDSFormat::r8g8b8a8_snorm             : return "r8g8b8a8_snorm            ";
            case Image::DDSFormat::r8g8b8a8_sint              : return "r8g8b8a8_sint             ";
            case Image::DDSFormat::r16g16_typeless            : return "r16g16_typeless           ";
            case Image::DDSFormat::r16g16_float               : return "r16g16_float              ";
            case Image::DDSFormat::r16g16_unorm               : return "r16g16_unorm              ";
            case Image::DDSFormat::r16g16_uint                : return "r16g16_uint               ";
            case Image::DDSFormat::r16g16_snorm               : return "r16g16_snorm              ";
            case Image::DDSFormat::r16g16_sint                : return "r16g16_sint               ";
            case Image::DDSFormat::r32_typeless               : return "r32_typeless              ";
            case Image::DDSFormat::d32_float                  : return "d32_float                 ";
            case Image::DDSFormat::r32_float                  : return "r32_float                 ";
            case Image::DDSFormat::r32_uint                   : return "r32_uint                  ";
            case Image::DDSFormat::r32_sint                   : return "r32_sint                  ";
            case Image::DDSFormat::r24g8_typeless             : return "r24g8_typeless            ";
            case Image::DDSFormat::d24_unorm_s8_uint          : return "d24_unorm_s8_uint         ";
            case Image::DDSFormat::r24_unorm_x8_typeless      : return "r24_unorm_x8_typeless     ";
            case Image::DDSFormat::x24_typeless_g8_uint       : return "x24_typeless_g8_uint      ";
            case Image::DDSFormat::r8g8_typeless              : return "r8g8_typeless             ";
            case Image::DDSFormat::r8g8_unorm                 : return "r8g8_unorm                ";
            case Image::DDSFormat::r8g8_uint                  : return "r8g8_uint                 ";
            case Image::DDSFormat::r8g8_snorm                 : return "r8g8_snorm                ";
            case Image::DDSFormat::r8g8_sint                  : return "r8g8_sint                 ";
            case Image::DDSFormat::r16_typeless               : return "r16_typeless              ";
            case Image::DDSFormat::r16_float                  : return "r16_float                 ";
            case Image::DDSFormat::d16_unorm                  : return "d16_unorm                 ";
            case Image::DDSFormat::r16_unorm                  : return "r16_unorm                 ";
            case Image::DDSFormat::r16_uint                   : return "r16_uint                  ";
            case Image::DDSFormat::r16_snorm                  : return "r16_snorm                 ";
            case Image::DDSFormat::r16_sint                   : return "r16_sint                  ";
            case Image::DDSFormat::r8_typeless                : return "r8_typeless               ";
            case Image::DDSFormat::r8_unorm                   : return "r8_unorm                  ";
            case Image::DDSFormat::r8_uint                    : return "r8_uint                   ";
            case Image::DDSFormat::r8_snorm                   : return "r8_snorm                  ";
            case Image::DDSFormat::r8_sint                    : return "r8_sint                   ";
            case Image::DDSFormat::a8_unorm                   : return "a8_unorm                  ";
            case Image::DDSFormat::r1_unorm                   : return "r1_unorm                  ";
            case Image::DDSFormat::r9g9b9e5_sharedexp         : return "r9g9b9e5_sharedexp        ";
            case Image::DDSFormat::r8g8_b8g8_unorm            : return "r8g8_b8g8_unorm           ";
            case Image::DDSFormat::g8r8_g8b8_unorm            : return "g8r8_g8b8_unorm           ";
            case Image::DDSFormat::bc1_typeless               : return "bc1_typeless              ";
            case Image::DDSFormat::bc1_unorm                  : return "bc1_unorm                 ";
            case Image::DDSFormat::bc1_unorm_srgb             : return "bc1_unorm_srgb            ";
            case Image::DDSFormat::bc2_typeless               : return "bc2_typeless              ";
            case Image::DDSFormat::bc2_unorm                  : return "bc2_unorm                 ";
            case Image::DDSFormat::bc2_unorm_srgb             : return "bc2_unorm_srgb            ";
            case Image::DDSFormat::bc3_typeless               : return "bc3_typeless              ";
            case Image::DDSFormat::bc3_unorm                  : return "bc3_unorm                 ";
            case Image::DDSFormat::bc3_unorm_srgb             : return "bc3_unorm_srgb            ";
            case Image::DDSFormat::bc4_typeless               : return "bc4_typeless              ";
            case Image::DDSFormat::bc4_unorm                  : return "bc4_unorm                 ";
            case Image::DDSFormat::bc4_snorm                  : return "bc4_snorm                 ";
            case Image::DDSFormat::bc5_typeless               : return "bc5_typeless              ";
            case Image::DDSFormat::bc5_unorm                  : return "bc5_unorm                 ";
            case Image::DDSFormat::bc5_snorm                  : return "bc5_snorm                 ";
            case Image::DDSFormat::b5g6r5_unorm               : return "b5g6r5_unorm              ";
            case Image::DDSFormat::b5g5r5a1_unorm             : return "b5g5r5a1_unorm            ";
            case Image::DDSFormat::b8g8r8a8_unorm             : return "b8g8r8a8_unorm            ";
            case Image::DDSFormat::b8g8r8x8_unorm             : return "b8g8r8x8_unorm            ";
            case Image::DDSFormat::r10g10b10_xr_bias_a2_unorm : return "r10g10b10_xr_bias_a2_unorm";
            case Image::DDSFormat::b8g8r8a8_typeless          : return "b8g8r8a8_typeless         ";
            case Image::DDSFormat::b8g8r8a8_unorm_srgb        : return "b8g8r8a8_unorm_srgb       ";
            case Image::DDSFormat::b8g8r8x8_typeless          : return "b8g8r8x8_typeless         ";
            case Image::DDSFormat::b8g8r8x8_unorm_srgb        : return "b8g8r8x8_unorm_srgb       ";
            case Image::DDSFormat::bc6h_typeless              : return "bc6h_typeless             ";
            case Image::DDSFormat::bc6h_uf16                  : return "bc6h_uf16                 ";
            case Image::DDSFormat::bc6h_sf16                  : return "bc6h_sf16                 ";
            case Image::DDSFormat::bc7_typeless               : return "bc7_typeless              ";
            case Image::DDSFormat::bc7_unorm                  : return "bc7_unorm                 ";
            case Image::DDSFormat::bc7_unorm_srgb             : return "bc7_unorm_srgb            ";
            case Image::DDSFormat::b4g4r4a4_unorm             : return "b4g4r4a4_unorm            ";
            default: break;
        }
        return "unknown";
    }

    void TextureImporter::on_render()
    {
        char title[32];
        snprintf(title, 32, "Texture Importer###%d", (u32)(usize)this);

        ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Select Source File"))
        {
            lutry
            {
                m_files.clear();
                Window::FileDialogFilter filter;
                filter.name = "Image File";
                const c8* exts[] = {"jpg", "jpeg", "png", "tga", "bmp", "psd", "gif", "hdr", "pic", "dds"};
                filter.extensions = {exts, 10};
                lulet(img_paths, Window::open_file_dialog("Select Source File", {&filter, 1}, Path(), Window::FileDialogFlag::multi_select));
                for(auto& img_path : img_paths)
                {
                    // Open file.
                    TextureFile file;
                    lulet(img_file, open_file(img_path.encode(PathSeparator::system_preferred).c_str(),
                        FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
                    luset(file.m_file_data, load_file_data(img_file));
                    if(img_path.extension() == "dds")
                    {
                        file.m_type = TextureFileType::dds;
                        luset(file.m_dds_desc, Image::read_dds_image_file_desc(file.m_file_data.data(), file.m_file_data.size()));
                    }
                    else
                    {
                        file.m_type = TextureFileType::image;
                        luset(file.m_desc, Image::read_image_file_desc(file.m_file_data.data(), file.m_file_data.size()));
                        file.m_prefiler_type = TexturePrefilerType::normal;
                    }
                    file.m_path = img_path;
                    img_path.remove_extension();
                    file.m_asset_name = img_path.back().c_str();
                    m_files.push_back(move(file));
                }
            }
            lucatch
            {
                if (luerr != BasicError::interrupted())
                {
                    auto _ = Window::message_box(explain(luerr), "Failed to import texture",
                        Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                }
                m_files.clear();
            }
        }

        if(m_files.empty())
        {
            ImGui::Text("No image file selected.");
        }
        else
        {
            if(ImGui::Button("Import All"))
            {
                for(auto& file : m_files)
                {
                    if(!file.m_asset_name.empty())
                    {
                        import_texture_asset(m_create_dir, file);
                    }    
                }
            }
            for(auto& file : m_files)
            {
                ImGui::Text("%s", file.m_path.encode().c_str());
                ImGui::Text("Texture Information:");
                if(file.m_type == TextureFileType::image)
                {
                    ImGui::Text("Width: %u", file.m_desc.width);
                    ImGui::Text("Height: %u", file.m_desc.height);
                    const char* fmt = nullptr;
                    switch (file.m_desc.format)
                    {
                    case Image::ImageFormat::r8_unorm:
                        fmt = "R8 UNORM"; break;
                    case Image::ImageFormat::r16_unorm:
                        fmt = "R16 UNORM"; break;
                    case Image::ImageFormat::r32_float:
                        fmt = "R32 FLOAT"; break;
                    case Image::ImageFormat::rg8_unorm:
                        fmt = "RG8 UNORM"; break;
                    case Image::ImageFormat::rg16_unorm:
                        fmt = "RG16 UNORM"; break;
                    case Image::ImageFormat::rg32_float:
                        fmt = "RG32 FLOAT"; break;
                    case Image::ImageFormat::rgb8_unorm:
                        fmt = "RGB8 UNORM"; break;
                    case Image::ImageFormat::rgb16_unorm:
                        fmt = "RGB16 UNORM"; break;
                    case Image::ImageFormat::rgb32_float:
                        fmt = "RGB32 FLOAT"; break;
                    case Image::ImageFormat::rgba8_unorm:
                        fmt = "RGBA8 UNORM"; break;
                    case Image::ImageFormat::rgba16_unorm:
                        fmt = "RGBA16 UNORM"; break;
                    case Image::ImageFormat::rgba32_float:
                        fmt = "RGBA32 FLOAT"; break;
                    default:
                        lupanic();
                        break;
                    }
                    ImGui::Text("Format: %s", fmt);
                }
                else if(file.m_type == TextureFileType::dds)
                {
                    switch(file.m_dds_desc.dimension)
                    {
                        case Image::DDSDimension::tex1d:
                            ImGui::Text("1D Texture");
                            break;
                        case Image::DDSDimension::tex2d:
                            ImGui::Text("2D Texture");
                            break;
                        case Image::DDSDimension::tex3d:
                            ImGui::Text("3D Texture");
                            break;
                        default: lupanic();
                    }
                    ImGui::Text("Width: %u", file.m_dds_desc.width);
                    ImGui::Text("Height: %u", file.m_dds_desc.height);
                    ImGui::Text("Depth: %u", file.m_dds_desc.depth);
                    ImGui::Text("Mips: %u", file.m_dds_desc.mip_levels);
                    ImGui::Text("Array Size: %u", file.m_dds_desc.array_size);
                    const char* fmt = print_dds_format(file.m_dds_desc.format);
                    ImGui::Text("Format: %s", fmt);
                }
                ImGui::Text("Import Settings:");
                ImGui::InputText("Asset Name", file.m_asset_name);
                if(file.m_type == TextureFileType::image)
                {
                    int import_type = (int)file.m_prefiler_type;
                    ImGui::Combo("Import Type", &import_type, "Texture\0Environment Map\0\0");
                    file.m_prefiler_type = (TexturePrefilerType)import_type;
                }
                if (!file.m_asset_name.empty())
                {
                    ImGui::Text("The texture will be imported as: %s%s", m_create_dir.encode().c_str(), file.m_asset_name.c_str());
                    if (ImGui::Button("Import"))
                    {
                        import_texture_asset(m_create_dir, file);
                    }
                }
            }
        }
        ImGui::End();
    }
    void register_texture_importer()
    {
        register_boxed_type<TextureImporter>();
        impl_interface_for_type<TextureImporter, IAssetEditor>();
        AssetImporterDesc desc;
        desc.new_importer = new_static_texture_importer;
        g_env->register_asset_importer_type(get_static_texture_asset_type(), desc);
    }
}
