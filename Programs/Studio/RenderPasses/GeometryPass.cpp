/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file GeometryPass.cpp
* @author JXMaster
* @date 2023/3/11
*/
#include "GeometryPass.hpp"
#include <Luna/Runtime/File.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../Material.hpp"
#include "../SceneRenderer.hpp"
#include "../StudioHeader.hpp"
#include <Luna/RHIUtility/ResourceWriteContext.hpp>

#include <GeometryVert.hpp>
#include <GeometryPixel.hpp>

namespace Luna
{
    RV GeometryPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_geometry_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::vertex | ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_buffer_view(1, 1, ShaderVisibilityFlag::vertex | ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 3, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 4, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 5, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 6, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::sampler(7, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::read_buffer_view(8, 1, ShaderVisibilityFlag::pixel)
                })));
            auto dl = m_geometry_pass_dlayout.get();
            luset(m_geometry_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dl, 1 },
                PipelineLayoutFlag::allow_input_assembler_input_layout)));

            GraphicsPipelineStateDesc ps_desc;
            ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
            ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::one, BlendFactor::zero, BlendOp::add, BlendFactor::one, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
            ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, false, true);
            ps_desc.depth_stencil_state = DepthStencilDesc(true, true, CompareFunction::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
            ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
            Vector<InputAttributeDesc> attributes;
            get_vertex_input_layout_desc(attributes);
            InputBindingDesc binding(0, sizeof(Vertex), InputRate::per_vertex);
            ps_desc.input_layout.attributes = { attributes.data(), attributes.size() };
            ps_desc.input_layout.bindings = { &binding, 1 };
            ps_desc.vs = LUNA_GET_SHADER_DATA(GeometryVert);
            ps_desc.ps = LUNA_GET_SHADER_DATA(GeometryPixel);
            ps_desc.pipeline_layout = m_geometry_pass_playout;
            ps_desc.num_color_attachments = 3;
            ps_desc.color_formats[0] = Format::rgba8_unorm;
            ps_desc.color_formats[1] = Format::rgba8_unorm;
            ps_desc.color_formats[2] = Format::rgba16_float;
            ps_desc.depth_stencil_format = Format::d32_float;
            luset(m_geometry_pass_pso, device->new_graphics_pipeline_state(ps_desc));

            luset(m_default_base_color, device->new_texture(MemoryType::local,
                TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
            luset(m_default_roughness, device->new_texture(MemoryType::local,
                TextureDesc::tex2d(Format::r8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
            luset(m_default_normal, device->new_texture(MemoryType::local,
                TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
            luset(m_default_metallic, device->new_texture(MemoryType::local,
                TextureDesc::tex2d(Format::r8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
            luset(m_default_emissive, device->new_texture(MemoryType::local,
                TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));

            // Upload default texture data.
            u8 base_color_data[4] = { 255, 255, 255, 255 };
            u8 roughness_data = 127;
            u8 normal_data[4] = { 127, 127, 255, 255 };
            u8 metallic_data = 0;
            u8 emissive_data[4] = { 0, 0, 0, 0 };
            lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
            auto writer = RHIUtility::new_resource_write_context(g_env->device);
            u32 row_pitch, slice_pitch;
            void* mapped;
            luset(mapped, writer->write_texture(m_default_base_color, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
            memcpy(mapped, base_color_data, sizeof(base_color_data));
            luset(mapped, writer->write_texture(m_default_roughness, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
            memcpy(mapped, &roughness_data, sizeof(roughness_data));
            luset(mapped, writer->write_texture(m_default_normal, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
            memcpy(mapped, normal_data, sizeof(normal_data));
            luset(mapped, writer->write_texture(m_default_metallic, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
            memcpy(mapped, &metallic_data, sizeof(metallic_data));
            luset(mapped, writer->write_texture(m_default_emissive, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
            memcpy(mapped, emissive_data, sizeof(emissive_data));
            luexp(writer->commit(upload_cmdbuf, true));
            u32 sb_alignment = device->check_feature(RHI::DeviceFeature::structured_buffer_offset_alignment).structured_buffer_offset_alignment;
            m_model_matrices_stride = (u32)align_upper(sizeof(MeshBuffer), sb_alignment);
            m_material_parameter_stride = (u32)align_upper(sizeof(MaterialParameters), sb_alignment);
        }
        lucatchret;
        return ok;
    }

    RV GeometryPass::init(GeometryPassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }

    RV GeometryPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            Ref<ITexture> base_color_roughness_tex = ctx->get_output("base_color_roughness_texture");
            Ref<ITexture> normal_metallic_tex = ctx->get_output("normal_metallic_texture");
            Ref<ITexture> emissive_tex = ctx->get_output("emissive_texture");
            Ref<ITexture> depth_tex = ctx->get_output("depth_texture");
            auto render_desc = base_color_roughness_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            cmdbuf->resource_barrier(
                {}, {
                    {base_color_roughness_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content},
                    {normal_metallic_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content},
                    {emissive_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content},
                    {depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::depth_stencil_attachment_write, ResourceBarrierFlag::discard_content} });
            
            for (usize i = 0; i < mesh_render_params.size(); ++i)
            {
                auto model = mesh_render_params[i].model;
                auto mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);

                u32 num_pieces = (u32)mesh->pieces.size();

                for (u32 j = 0; j < num_pieces; ++j)
                {
                    Ref<ITexture> base_color_tex = m_global_data->m_default_base_color;
                    Ref<ITexture> roughness_tex = m_global_data->m_default_roughness;
                    Ref<ITexture> normal_tex = m_global_data->m_default_normal;
                    Ref<ITexture> metallic_tex = m_global_data->m_default_metallic;
                    Ref<ITexture> emissive_tex = m_global_data->m_default_emissive;

                    if (j < model->materials.size())
                    {
                        auto mat = get_asset_or_async_load_if_not_ready<Material>(model->materials[j]);
                        if (mat)
                        {
                            // Set material for this piece.
                            Ref<ITexture> mat_base_color_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->base_color);
                            Ref<ITexture> mat_roughness_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->roughness);
                            Ref<ITexture> mat_normal_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->normal);
                            Ref<ITexture> mat_metallic_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->metallic);
                            Ref<ITexture> mat_emissive_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->emissive);
                            if (mat_base_color_tex)
                            {
                                base_color_tex = mat_base_color_tex;
                            }
                            if (mat_roughness_tex)
                            {
                                roughness_tex = mat_roughness_tex;
                            }
                            if (mat_normal_tex)
                            {
                                normal_tex = mat_normal_tex;
                            }
                            if (mat_metallic_tex)
                            {
                                metallic_tex = mat_metallic_tex;
                            }
                            if (mat_emissive_tex)
                            {
                                emissive_tex = mat_emissive_tex;
                            }
                            cmdbuf->resource_barrier(
                            {}, {
                                {base_color_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
                                {roughness_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
                                {normal_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
                                {metallic_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
                                {emissive_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none}});
                        }
                    }
                }
            }
            
            RenderPassDesc render_pass;
            render_pass.color_attachments[0] = ColorAttachment(base_color_roughness_tex, LoadOp::clear, StoreOp::store, Float4U(0.0f));
            render_pass.color_attachments[1] = ColorAttachment(normal_metallic_tex, LoadOp::clear, StoreOp::store, Float4U(0.0f));
            render_pass.color_attachments[2] = ColorAttachment(emissive_tex, LoadOp::clear, StoreOp::store, Float4U(0.0f));
            render_pass.depth_stencil_attachment = DepthStencilAttachment(depth_tex, false, LoadOp::clear, StoreOp::store, 1.0F);
            u32 time_query_begin, time_query_end;
            auto query_heap = ctx->get_timestamp_query_heap(&time_query_begin, &time_query_end);
            if(query_heap)
            {
                render_pass.timestamp_query_heap = query_heap;
                render_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                render_pass.timestamp_query_end_pass_write_index = time_query_end;
            }
            cmdbuf->begin_render_pass(render_pass);
            cmdbuf->set_graphics_pipeline_layout(m_global_data->m_geometry_pass_playout);
            cmdbuf->set_graphics_pipeline_state(m_global_data->m_geometry_pass_pso);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width, (f32)render_desc.height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width, (i32)render_desc.height));

            // Draw Meshes.
            u32 first_material_element = 0;
            for (usize i = 0; i < mesh_render_params.size(); ++i)
            {
                auto model = mesh_render_params[i].model;
                auto mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);
                cmdbuf->set_vertex_buffers(0, { VertexBufferView(mesh->vb, 0,
                    mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
                cmdbuf->set_index_buffer({mesh->ib, 0, (u32)(mesh->ib_count * sizeof(u32)), Format::r32_uint});

                u32 num_pieces = (u32)mesh->pieces.size();
                for (u32 j = 0; j < num_pieces; ++j)
                {
                    Ref<ITexture> base_color_tex = m_global_data->m_default_base_color;
                    Ref<ITexture> roughness_tex = m_global_data->m_default_roughness;
                    Ref<ITexture> normal_tex = m_global_data->m_default_normal;
                    Ref<ITexture> metallic_tex = m_global_data->m_default_metallic;
                    Ref<ITexture> emissive_tex = m_global_data->m_default_emissive;

                    if (j < model->materials.size())
                    {
                        auto mat = get_asset_or_async_load_if_not_ready<Material>(model->materials[j]);
                        if (mat)
                        {
                            // Set material for this piece.
                            Ref<ITexture> mat_base_color_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->base_color);
                            Ref<ITexture> mat_roughness_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->roughness);
                            Ref<ITexture> mat_normal_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->normal);
                            Ref<ITexture> mat_metallic_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->metallic);
                            Ref<ITexture> mat_emissive_tex = get_asset_or_async_load_if_not_ready<ITexture>(mat->emissive);
                            if (mat_base_color_tex)
                            {
                                base_color_tex = mat_base_color_tex;
                            }
                            if (mat_roughness_tex)
                            {
                                roughness_tex = mat_roughness_tex;
                            }
                            if (mat_normal_tex)
                            {
                                normal_tex = mat_normal_tex;
                            }
                            if (mat_metallic_tex)
                            {
                                metallic_tex = mat_metallic_tex;
                            }
                            if (mat_emissive_tex)
                            {
                                emissive_tex = mat_emissive_tex;
                            }
                        }
                    }
                    lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_geometry_pass_dlayout)));
                    luexp(vs->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(camera_cb, 0, (u32)align_upper(sizeof(CameraCB), cb_align))),
                        WriteDescriptorSet::read_buffer_view(1, BufferViewDesc::structured_buffer(model_matrices, i, 1, m_global_data->m_model_matrices_stride)),
                        WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(base_color_tex)),
                        WriteDescriptorSet::read_texture_view(3, TextureViewDesc::tex2d(roughness_tex)),
                        WriteDescriptorSet::read_texture_view(4, TextureViewDesc::tex2d(normal_tex)),
                        WriteDescriptorSet::read_texture_view(5, TextureViewDesc::tex2d(metallic_tex)),
                        WriteDescriptorSet::read_texture_view(6, TextureViewDesc::tex2d(emissive_tex)),
                        WriteDescriptorSet::sampler(7, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat)),
                        WriteDescriptorSet::read_buffer_view(8, BufferViewDesc::structured_buffer(material_parameters, first_material_element + j, 1, m_global_data->m_material_parameter_stride))
                        }));
                    cmdbuf->set_graphics_descriptor_set(0, vs);
                    cmdbuf->attach_device_object(vs);
                    cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
                }
                first_material_element += num_pieces;
            }
            cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }
    RV compile_geometry_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            GeometryPassGlobalData* data = (GeometryPassGlobalData*)userdata;
            auto depth_texture = compiler->get_output_resource("depth_texture");
            if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"depth_texture\" is not specified.");
            auto base_color_roughness_tex = compiler->get_output_resource("base_color_roughness_texture");
            auto normal_metallic_tex = compiler->get_output_resource("normal_metallic_texture");
            auto emissive_tex = compiler->get_output_resource("emissive_texture");
            if(base_color_roughness_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"base_color_roughness_texture\" is not specified.");
            if(normal_metallic_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"normal_metallic_tex\" is not specified.");
            if(emissive_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"emissive_tex\" is not specified.");
            RG::ResourceDesc desc = compiler->get_resource_desc(depth_texture);
            if(desc.type == RG::ResourceType::texture && desc.texture.type == RHI::TextureType::tex2d && desc.texture.format == RHI::Format::unknown)
            {
                desc.texture.format = RHI::Format::d32_float;
            }
            if (desc.type != RG::ResourceType::texture || desc.texture.type != RHI::TextureType::tex2d || desc.texture.format != RHI::Format::d32_float)
            {
                return set_error(BasicError::bad_arguments(), "GeometryPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be 2D texture with Format::d32_float.");
            }
            desc.texture.usages |= RHI::TextureUsageFlag::depth_stencil_attachment;
            compiler->set_resource_desc(depth_texture, desc);

            auto desc2 = compiler->get_resource_desc(base_color_roughness_tex);
            desc2.texture.type = RHI::TextureType::tex2d;
            if(!desc2.texture.width) desc2.texture.width = desc.texture.width;
            if(!desc2.texture.height) desc2.texture.height = desc.texture.height;
            if(desc2.texture.format == RHI::Format::unknown) desc2.texture.format = RHI::Format::rgba8_unorm;
            desc2.texture.usages |= RHI::TextureUsageFlag::color_attachment;
            compiler->set_resource_desc(base_color_roughness_tex, desc2);

            desc2 = compiler->get_resource_desc(normal_metallic_tex);
            desc2.texture.type = RHI::TextureType::tex2d;
            if(!desc2.texture.width) desc2.texture.width = desc.texture.width;
            if(!desc2.texture.height) desc2.texture.height = desc.texture.height;
            if(desc2.texture.format == RHI::Format::unknown) desc2.texture.format = RHI::Format::rgba8_unorm;
            desc2.texture.usages |= RHI::TextureUsageFlag::color_attachment;
            compiler->set_resource_desc(normal_metallic_tex, desc2);

            desc2 = compiler->get_resource_desc(emissive_tex);
            desc2.texture.type = RHI::TextureType::tex2d;
            if(!desc2.texture.width) desc2.texture.width = desc.texture.width;
            if(!desc2.texture.height) desc2.texture.height = desc.texture.height;
            if(desc2.texture.format == RHI::Format::unknown) desc2.texture.format = RHI::Format::rgba16_float;
            desc2.texture.usages |= RHI::TextureUsageFlag::color_attachment;
            compiler->set_resource_desc(emissive_tex, desc2);

            Ref<GeometryPass> pass = new_object<GeometryPass>();
            luexp(pass->init(data));
            compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_geometry_pass()
    {
        lutry
        {
            register_boxed_type<GeometryPassGlobalData>();
            register_boxed_type<GeometryPass>();
            impl_interface_for_type<GeometryPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Geometry";
            desc.desc = "Writes scene geometry information to the geometry buffer (G-buffer).";
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture with pre-rendered depth information."});
            desc.output_parameters.push_back({"base_color_roughness_texture", "The base color (RGB) and roughness (A) G-buffer."});
            desc.output_parameters.push_back({"normal_metallic_texture", "The normal (RGB) and metallic (A) G-buffer."});
            desc.output_parameters.push_back({"emissive_texture", "The emissive (RGB) G-buffer."});
            desc.compile = compile_geometry_pass;
            auto data = new_object<GeometryPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
