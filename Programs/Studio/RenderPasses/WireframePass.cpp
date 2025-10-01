/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WireframePass.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "WireframePass.hpp"
#include "../Mesh.hpp"
#include "../Model.hpp"
#include <Luna/Asset/Asset.hpp>
#include "../SceneRenderer.hpp"
#include <Luna/Runtime/File.hpp>
#include "../StudioHeader.hpp"

#include <WireframeVert.hpp>
#include <WireframePixel.hpp>

namespace Luna
{
    RV WireframePassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_debug_mesh_renderer_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::vertex),
                DescriptorSetLayoutBinding::read_buffer_view(1, 1, ShaderVisibilityFlag::vertex) })));

            auto dlayout = m_debug_mesh_renderer_dlayout.get();
            luset(m_debug_mesh_renderer_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dlayout, 1 },
                PipelineLayoutFlag::allow_input_assembler_input_layout)));

            GraphicsPipelineStateDesc ps_desc;
            ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
            ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::src_alpha,
                BlendFactor::one_minus_src_alpha, BlendOp::add, BlendFactor::one, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
            ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::none, 0.0f, 0.0f, 0.0f, false, true);
            ps_desc.depth_stencil_state = DepthStencilDesc(false, false, CompareFunction::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
            ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
            Vector<InputAttributeDesc> attributes;
            get_vertex_input_layout_desc(attributes);
            InputBindingDesc binding(0, sizeof(Vertex), InputRate::per_vertex);
            ps_desc.input_layout.bindings = { &binding, 1 };
            ps_desc.input_layout.attributes = { attributes.data(), attributes.size() };
            ps_desc.vs = LUNA_GET_SHADER_DATA(WireframeVert);
            ps_desc.ps = LUNA_GET_SHADER_DATA(WireframePixel);
            ps_desc.pipeline_layout = m_debug_mesh_renderer_playout;
            ps_desc.num_color_attachments = 1;
            ps_desc.color_formats[0] = Format::rgba8_unorm;
            luset(m_debug_mesh_renderer_pso, device->new_graphics_pipeline_state(ps_desc));
            u32 sb_alignment = device->check_feature(RHI::DeviceFeature::structured_buffer_offset_alignment).structured_buffer_offset_alignment;
            m_model_matrices_stride = (u32)align_upper(sizeof(MeshBuffer), sb_alignment);
        }
        lucatchret;
        return ok;
    }
    RV WireframePass::init(WireframePassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }
    RV WireframePass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            Ref<ITexture> output_tex = query_interface<ITexture>(ctx->get_output("scene_texture")->get_object());
            // Debug wireframe pass.
            RenderPassDesc render_pass;
            render_pass.color_attachments[0] = ColorAttachment(output_tex, LoadOp::clear, StoreOp::store, Float4U(0.0f));
            u32 time_query_begin, time_query_end;
            auto query_heap = ctx->get_timestamp_query_heap(&time_query_begin, &time_query_end);
            if(query_heap)
            {
                render_pass.timestamp_query_heap = query_heap;
                render_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                render_pass.timestamp_query_end_pass_write_index = time_query_end;
            }
            auto render_desc = output_tex->get_desc();
            cmdbuf->resource_barrier({}, { {output_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content} });
            cmdbuf->begin_render_pass(render_pass);
            cmdbuf->set_graphics_pipeline_layout(m_global_data->m_debug_mesh_renderer_playout);
            cmdbuf->set_graphics_pipeline_state(m_global_data->m_debug_mesh_renderer_pso);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width, (f32)render_desc.height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width, (i32)render_desc.height));
            // Draw Meshes.
            for (usize i = 0; i < mesh_render_params.size(); ++i)
            {
                auto vs = device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_debug_mesh_renderer_dlayout)).get();
                luexp(vs->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(camera_cb, 0, (u32)align_upper(sizeof(CameraCB), cb_align))),
                    WriteDescriptorSet::read_buffer_view(1, BufferViewDesc::structured_buffer(model_matrices, i, 1, m_global_data->m_model_matrices_stride))
                    }));
                IDescriptorSet* vs_d = vs.get();
                cmdbuf->set_graphics_descriptor_sets(0, { &vs_d, 1 });
                cmdbuf->attach_device_object(vs);

                // Draw pieces.
                auto mesh = get_asset_or_async_load_if_not_ready<Mesh>(get_asset_or_async_load_if_not_ready<Model>(mesh_render_params[i].renderer->model)->mesh);

                auto vb_view = VertexBufferView(mesh->vb, 0,
                    mesh->vb_count * sizeof(Vertex), sizeof(Vertex));

                cmdbuf->set_vertex_buffers(0, { &vb_view, 1 });
                cmdbuf->set_index_buffer({mesh->ib, 0, (u32)(mesh->ib_count * sizeof(u32)), Format::r32_uint});

                u32 num_pieces = (u32)mesh->pieces.size();
                for (u32 j = 0; j < num_pieces; ++j)
                {
                    cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
                }
            }
            cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }

    RV compile_wireframe_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            WireframePassGlobalData* data = (WireframePassGlobalData*)userdata;
            auto scene_texture = compiler->get_output_resource("scene_texture");
            if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "WireframePass: Output \"scene_texture\" is not specified.");
            RG::ResourceDesc desc = compiler->get_resource_desc(scene_texture);
            desc.texture.usages |= RHI::TextureUsageFlag::color_attachment;
            compiler->set_resource_desc(scene_texture, desc);
            Ref<WireframePass> pass = new_object<WireframePass>();
            luexp(pass->init(data));
            compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }

    RV register_wireframe_pass()
    {
        lutry
        {
            register_boxed_type<WireframePassGlobalData>();
            register_boxed_type<WireframePass>();
            impl_interface_for_type<WireframePass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Wireframe";
            desc.desc = "Draws wireframe of the scene.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.compile = compile_wireframe_pass;
            auto data = new_object<WireframePassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
