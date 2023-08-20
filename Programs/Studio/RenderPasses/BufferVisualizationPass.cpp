/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BufferVisualizationPass.cpp
* @author JXMaster
* @date 2023/3/18
*/
#include "BufferVisualizationPass.hpp"
#include <Luna/Runtime/File.hpp>
#include "../SceneRenderer.hpp"
#include "../StudioHeader.hpp"

namespace Luna
{
    RV BufferVisualizationPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_buffer_visualization_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::compute),
						DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 2, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 3, 1, ShaderVisibilityFlag::compute),
						DescriptorSetLayoutBinding(DescriptorType::read_write_texture_view, 4, 1, ShaderVisibilityFlag::compute)
						})));
            auto dlayout = m_buffer_visualization_pass_dlayout.get();
			luset(m_buffer_visualization_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dlayout, 1 },
				PipelineLayoutFlag::deny_vertex_shader_access |
				PipelineLayoutFlag::deny_pixel_shader_access)));

            lulet(cs_blob, compile_shader("Shaders/BufferVisualization.hlsl", ShaderCompiler::ShaderType::compute));

			ComputePipelineStateDesc ps_desc;
			ps_desc.cs = cs_blob.cspan();
			ps_desc.pipeline_layout = m_buffer_visualization_pass_playout;
			luset(m_buffer_visualization_pass_pso, device->new_compute_pipeline_state(ps_desc));
        }
        lucatchret;
        return ok;
    }
    RV BufferVisualizationPass::init(BufferVisualizationPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = m_global_data->m_buffer_visualization_pass_dlayout->get_device();
            luset(m_ds, device->new_descriptor_set(
                DescriptorSetDesc(global_data->m_buffer_visualization_pass_dlayout)));
            luset(m_vis_params, device->new_buffer(MemoryType::upload,
                BufferDesc(BufferUsageFlag::uniform_buffer, 
                    align_upper(sizeof(u32), device->get_uniform_buffer_data_alignment()))));
        }
        lucatchret;
        return ok;
    }
    RV BufferVisualizationPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            u32* mapped = nullptr;
            luexp(m_vis_params->map(0, 0, (void**)&mapped));
            *mapped = vis_type;
            m_vis_params->unmap(0, sizeof(u32));
            Ref<ITexture> scene_tex = ctx->get_output("scene_texture");
            Ref<ITexture> depth_tex = ctx->get_input("depth_texture");
            Ref<ITexture> base_color_roughness_tex = ctx->get_input("base_color_roughness_texture");
            Ref<ITexture> normal_metallic_tex = ctx->get_input("normal_metallic_texture");
            auto cmdbuf = ctx->get_command_buffer();
            ComputePassDesc compute_pass;
            u32 time_query_begin, time_query_end;
            auto query_heap = ctx->get_timestamp_query_heap(&time_query_begin, &time_query_end);
            if(query_heap)
            {
                compute_pass.timestamp_query_heap = query_heap;
                compute_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                compute_pass.timestamp_query_end_pass_write_index = time_query_end;
            }
            cmdbuf->begin_compute_pass(compute_pass);
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_uniform_buffer_data_alignment();
            cmdbuf->resource_barrier(
                { {m_vis_params, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs, ResourceBarrierFlag::none} },
                {
                    {scene_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs, ResourceBarrierFlag::discard_content},
                    {depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {base_color_roughness_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {normal_metallic_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                });
            luexp(m_ds->update_descriptors({
                WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_vis_params, 0, (u32)align_upper(sizeof(u32), cb_align))),
                WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(base_color_roughness_tex)),
                WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(normal_metallic_tex)),
                WriteDescriptorSet::read_texture_view(3, TextureViewDesc::tex2d(depth_tex)),
                WriteDescriptorSet::read_write_texture_view(4, TextureViewDesc::tex2d(scene_tex))
                }));
            auto scene_desc = scene_tex->get_desc();
            cmdbuf->set_compute_pipeline_layout(m_global_data->m_buffer_visualization_pass_playout);
            cmdbuf->set_compute_pipeline_state(m_global_data->m_buffer_visualization_pass_pso);
            cmdbuf->set_compute_descriptor_set(0, m_ds);
            cmdbuf->dispatch((u32)align_upper(scene_desc.width, 8) / 8,
                (u32)align_upper(scene_desc.height, 8) / 8, 1);
            cmdbuf->end_compute_pass();
        }
        lucatchret;
        return ok;
    }

    RV compile_buffer_visualization_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            BufferVisualizationPassGlobalData* data = (BufferVisualizationPassGlobalData*)userdata;
			auto scene_texture = compiler->get_output_resource("scene_texture");
			auto depth_texture = compiler->get_input_resource("depth_texture");
            auto base_color_roughness_texture = compiler->get_input_resource("base_color_roughness_texture");
            auto normal_metallic_texture = compiler->get_input_resource("normal_metallic_texture");
			if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Output \"scene_texture\" is not specified.");
			if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Input \"depth_texture\" is not specified.");
            if(base_color_roughness_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Input \"base_color_roughness_texture\" is not specified.");
            if(normal_metallic_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Input \"normal_metallic_texture\" is not specified.");
            RG::ResourceDesc desc = compiler->get_resource_desc(scene_texture);
			if (desc.texture.format != RHI::Format::rgba8_unorm)
			{
				return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Invalid format for \"scene_texture\" is specified. \"scene_texture\" must be Format::rgba8_unorm.");
			}
			desc.texture.usages |= RHI::TextureUsageFlag::read_write_texture;
			compiler->set_resource_desc(scene_texture, desc);

			desc = compiler->get_resource_desc(depth_texture);
			if (desc.texture.format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "BufferVisualizationPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be Format::d32_float.");
			}
			desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
			compiler->set_resource_desc(depth_texture, desc);

            desc = compiler->get_resource_desc(base_color_roughness_texture);
			desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
			compiler->set_resource_desc(base_color_roughness_texture, desc);

            desc = compiler->get_resource_desc(normal_metallic_texture);
			desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
			compiler->set_resource_desc(normal_metallic_texture, desc);

			Ref<BufferVisualizationPass> pass = new_object<BufferVisualizationPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }

    RV register_buffer_visualization_pass()
    {
        lutry
        {
            register_boxed_type<BufferVisualizationPassGlobalData>();
            register_boxed_type<BufferVisualizationPass>();
            impl_interface_for_type<BufferVisualizationPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "BufferVisualization";
            desc.desc = "Visualize geometry buffer data.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture"});
            desc.input_parameters.push_back({"base_color_roughness_texture", "The base color and roughness texture from geometry pass."});
            desc.input_parameters.push_back({"normal_metallic_texture", "The normal and metallic texture from geometry pass."});
            desc.compile = compile_buffer_visualization_pass;
            auto data = new_object<BufferVisualizationPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
