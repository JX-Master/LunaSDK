/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeferredLightingPass.cpp
* @author JXMaster
* @date 2023/3/18
*/
#include "DeferredLightingPass.hpp"
#include <Luna/Runtime/File.hpp>
#include "../SceneRenderer.hpp"
#include "../StudioHeader.hpp"
#include <Luna/RHI/Utility.hpp>

#include <DeferredLighting.hpp>
#include <PrecomputeIntegrateBRDF.hpp>

namespace Luna
{
    RV DeferredLightingPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_deferred_lighting_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                        DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::uniform_buffer_view(1, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_buffer_view(2, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 3, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 4, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 5, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 6, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 7, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 8, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 9, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::sampler(10, 1, ShaderVisibilityFlag::compute)
                        })));
            auto dlayout = m_deferred_lighting_pass_dlayout.get();
            luset(m_deferred_lighting_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dlayout, 1 },
                PipelineLayoutFlag::deny_vertex_shader_access |
                PipelineLayoutFlag::deny_pixel_shader_access)));

            ComputePipelineStateDesc ps_desc;
            LUNA_FILL_COMPUTE_SHADER_DATA(ps_desc, DeferredLighting);
            ps_desc.pipeline_layout = m_deferred_lighting_pass_playout;
            luset(m_deferred_lighting_pass_pso, device->new_compute_pipeline_state(ps_desc));

            luset(m_default_skybox, device->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm, 
                TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
            u8 skybox_data[] = {0, 0, 0, 0};
            lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
            luexp(copy_resource_data(upload_cmdbuf, {CopyResourceData::write_texture(m_default_skybox, SubresourceIndex(0, 0), 0, 0, 0, skybox_data, 4, 4, 1, 1, 1)}));

            // Generate integrate brdf.
            constexpr usize INTEGEATE_BRDF_SIZE = 256;
            {
                luset(m_integrate_brdf, device->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm,
                    TextureUsageFlag::read_texture | TextureUsageFlag::read_write_texture, INTEGEATE_BRDF_SIZE, INTEGEATE_BRDF_SIZE, 1, 1)));
                lulet(dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                        DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute) })));
                auto dl = dlayout.get();
                lulet(playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dl, 1 },
                    PipelineLayoutFlag::deny_vertex_shader_access |
                    PipelineLayoutFlag::deny_pixel_shader_access)));
                ComputePipelineStateDesc ps_desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(ps_desc, PrecomputeIntegrateBRDF);
                ps_desc.pipeline_layout = playout;
                lulet(pso, device->new_compute_pipeline_state(ps_desc));
                lulet(compute_cmdbuf, device->new_command_buffer(g_env->async_compute_queue));
                u32 cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
                u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
                lulet(cb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, cb_size)));
                Float2U* mapped = nullptr;
                luexp(cb->map(0, 0, (void**)&mapped));
                *mapped = Float2U(1.0f / (f32)INTEGEATE_BRDF_SIZE, 1.0f / (f32)INTEGEATE_BRDF_SIZE);
                cb->unmap(0, sizeof(Float2U));
                compute_cmdbuf->resource_barrier(
                    { {cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs, ResourceBarrierFlag::none} },
                    {{m_integrate_brdf, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs, ResourceBarrierFlag::discard_content}});
                lulet(vs, device->new_descriptor_set(DescriptorSetDesc(dlayout)));
                luexp(vs->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb, 0, cb_size)),
                    WriteDescriptorSet::read_write_texture_view(1, TextureViewDesc::tex2d(m_integrate_brdf))
                    }));
                compute_cmdbuf->begin_compute_pass();
                compute_cmdbuf->set_compute_pipeline_layout(playout);
                compute_cmdbuf->set_compute_pipeline_state(pso);
                compute_cmdbuf->set_compute_descriptor_set(0, vs);
                compute_cmdbuf->dispatch(align_upper(INTEGEATE_BRDF_SIZE, 8) / 8, align_upper(INTEGEATE_BRDF_SIZE, 8) / 8, 1);
                compute_cmdbuf->end_compute_pass();
                luexp(compute_cmdbuf->submit({}, {}, true));
                compute_cmdbuf->wait();
            }
        }
        lucatchret;
        return ok;
    }
    struct LightingParamsCB
    {
        u32 lighting_mode;
        u32 num_lights;
    };
    RV DeferredLightingPass::init(DeferredLightingPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = m_global_data->m_deferred_lighting_pass_dlayout->get_device();
            luset(m_ds, device->new_descriptor_set(
                DescriptorSetDesc(global_data->m_deferred_lighting_pass_dlayout)));
            luset(m_lighting_params_cb, device->new_buffer(MemoryType::upload,
                BufferDesc(BufferUsageFlag::uniform_buffer,
                    align_upper(sizeof(LightingParamsCB), device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment))));
        }
        lucatchret;
        return ok;
    }
    RV DeferredLightingPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            u32 num_lights = light_ts.empty() ? 1 : (u32)light_ts.size();
            LightingParamsCB* mapped = nullptr;
            luexp(m_lighting_params_cb->map(0, 0, (void**)&mapped));
            mapped->lighting_mode = lighting_mode;
            mapped->num_lights = num_lights;
            m_lighting_params_cb->unmap(0, sizeof(LightingParamsCB));
            Ref<ITexture> scene_tex = ctx->get_output("scene_texture");
            Ref<ITexture> depth_tex = ctx->get_input("depth_texture");
            Ref<ITexture> base_color_roughness_tex = ctx->get_input("base_color_roughness_texture");
            Ref<ITexture> normal_metallic_tex = ctx->get_input("normal_metallic_texture");
            Ref<ITexture> emissive_tex = ctx->get_input("emissive_texture");
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            auto sky_box = skybox ? skybox : m_global_data->m_default_skybox;
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
            cmdbuf->resource_barrier(
                { 
                    {camera_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs, ResourceBarrierFlag::none},
                    {m_lighting_params_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs, ResourceBarrierFlag::none},
                    {light_params, BufferStateFlag::automatic, BufferStateFlag::shader_read_cs, ResourceBarrierFlag::none}
                },
                {
                    {scene_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none},
                    {depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {base_color_roughness_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {normal_metallic_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {emissive_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {sky_box, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                    {m_global_data->m_integrate_brdf, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                });
            luexp(m_ds->update_descriptors({
                WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(camera_cb, 0, (u32)align_upper(sizeof(CameraCB), cb_align))),
                WriteDescriptorSet::uniform_buffer_view(1, BufferViewDesc::uniform_buffer(m_lighting_params_cb, 0, (u32)align_upper(sizeof(LightingParamsCB), cb_align))),
                WriteDescriptorSet::read_buffer_view(2, BufferViewDesc::structured_buffer(light_params, 0, num_lights, sizeof(LightingParams))),
                WriteDescriptorSet::read_texture_view(3, TextureViewDesc::tex2d(base_color_roughness_tex)),
                WriteDescriptorSet::read_texture_view(4, TextureViewDesc::tex2d(normal_metallic_tex)),
                WriteDescriptorSet::read_texture_view(5, TextureViewDesc::tex2d(emissive_tex)),
                WriteDescriptorSet::read_texture_view(6, TextureViewDesc::tex2d(depth_tex, Format::d32_float, 0, 1)),
                WriteDescriptorSet::read_texture_view(7, TextureViewDesc::tex2d(sky_box)),
                WriteDescriptorSet::read_texture_view(8, TextureViewDesc::tex2d(m_global_data->m_integrate_brdf)),
                WriteDescriptorSet::read_write_texture_view(9, TextureViewDesc::tex2d(scene_tex)),
                WriteDescriptorSet::sampler(10, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp)),
                }));
            auto scene_desc = scene_tex->get_desc();
            cmdbuf->set_compute_pipeline_layout(m_global_data->m_deferred_lighting_pass_playout);
            cmdbuf->set_compute_pipeline_state(m_global_data->m_deferred_lighting_pass_pso);
            cmdbuf->set_compute_descriptor_set(0, m_ds);
            cmdbuf->dispatch((u32)align_upper(scene_desc.width, 8) / 8,
                (u32)align_upper(scene_desc.height, 8) / 8, 1);
            cmdbuf->end_compute_pass();
        }
        lucatchret;
        return ok;
    }

    RV compile_deferred_lighting_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            DeferredLightingPassGlobalData* data = (DeferredLightingPassGlobalData*)userdata;
            auto scene_texture = compiler->get_output_resource("scene_texture");
            auto depth_texture = compiler->get_input_resource("depth_texture");
            auto base_color_roughness_texture = compiler->get_input_resource("base_color_roughness_texture");
            auto normal_metallic_texture = compiler->get_input_resource("normal_metallic_texture");
            auto emissive_texture = compiler->get_input_resource("emissive_texture");
            if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Output \"scene_texture\" is not specified.");
            if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"depth_texture\" is not specified.");
            if(base_color_roughness_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"base_color_roughness_texture\" is not specified.");
            if(normal_metallic_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"normal_metallic_texture\" is not specified.");
            if(emissive_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"emissive_texture\" is not specified.");
            RG::ResourceDesc desc = compiler->get_resource_desc(scene_texture);
            if (desc.texture.format != RHI::Format::rgba32_float)
            {
                return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Invalid format for \"scene_texture\" is specified. \"scene_texture\" must be Format::rgba32_float.");
            }
            desc.texture.usages |= RHI::TextureUsageFlag::read_write_texture;
            compiler->set_resource_desc(scene_texture, desc);

            desc = compiler->get_resource_desc(depth_texture);
            if (desc.texture.format != RHI::Format::d32_float)
            {
                return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be Format::d32_float.");
            }
            desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
            compiler->set_resource_desc(depth_texture, desc);

            desc = compiler->get_resource_desc(base_color_roughness_texture);
            desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
            compiler->set_resource_desc(base_color_roughness_texture, desc);

            desc = compiler->get_resource_desc(normal_metallic_texture);
            desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
            compiler->set_resource_desc(normal_metallic_texture, desc);

            desc = compiler->get_resource_desc(emissive_texture);
            desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
            compiler->set_resource_desc(emissive_texture, desc);

            Ref<DeferredLightingPass> pass = new_object<DeferredLightingPass>();
            luexp(pass->init(data));
            compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }

    RV register_deferred_lighting_pass()
    {
        lutry
        {
            register_boxed_type<DeferredLightingPassGlobalData>();
            register_boxed_type<DeferredLightingPass>();
            impl_interface_for_type<DeferredLightingPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "DeferredLighting";
            desc.desc = "Illuminate the scene.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture"});
            desc.input_parameters.push_back({"base_color_roughness_texture", "The base color and roughness texture from geometry pass."});
            desc.input_parameters.push_back({"normal_metallic_texture", "The normal and metallic texture from geometry pass."});
            desc.input_parameters.push_back({"emissive_texture", "The emissive texture from geometry pass."});
            desc.compile = compile_deferred_lighting_pass;
            auto data = new_object<DeferredLightingPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
