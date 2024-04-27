/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneRenderer.cpp
* @author JXMaster
* @date 2023/3/27
*/
#include "SceneRenderer.hpp"
#include "SceneSettings.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "ModelRenderer.hpp"
#include "Mesh.hpp"
#include "RenderPasses/SkyBoxPass.hpp"
#include "RenderPasses/ToneMappingPass.hpp"
#include "RenderPasses/WireframePass.hpp"
#include "RenderPasses/GeometryPass.hpp"
#include "RenderPasses/DeferredLightingPass.hpp"
#include "RenderPasses/BufferVisualizationPass.hpp"
#include "RenderPasses/BloomPass.hpp"
#include "StudioHeader.hpp"
#include "Material.hpp"

namespace Luna
{
    SceneRenderer::SceneRenderer(RHI::IDevice* device) :
        m_device(device)
    {
        m_render_graph = RG::new_render_graph(device);
        u32 sb_alignment = device->check_feature(RHI::DeviceFeature::structured_buffer_offset_alignment).structured_buffer_offset_alignment;
        m_model_matrices_stride = (u32)align_upper(sizeof(MeshBuffer), sb_alignment);
        m_material_parameter_stride = (u32)align_upper(sizeof(MaterialParameters), sb_alignment);
    }
    const SceneRendererSettings& SceneRenderer::get_settings()
    {
        return m_settings;
    }
    RV SceneRenderer::reset(const SceneRendererSettings& settings)
    {
        using namespace RHI;
        lutry
        { 
            m_settings = settings;
            usize cb_align = m_device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            luset(m_camera_cb, m_device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(CameraCB), cb_align))));
            
            // Build render graph.
            {
                using namespace RG;
                
                RenderGraphDesc desc;
                desc.passes.resize(7);
                desc.passes[WIREFRAME_PASS] = {"WireframePass", "Wireframe"};
                desc.passes[GEOMETRY_PASS] = {"GeometryPass", "Geometry"};
                desc.passes[BUFFER_VIS_PASS] = {"BufferVisualizationPass", "BufferVisualization"};
                desc.passes[SKYBOX_PASS] = {"SkyBoxPass", "SkyBox"};
                desc.passes[DEFERRED_LIGHTING_PASS] = {"DeferredLightingPass", "DeferredLighting"};
                desc.passes[BLOOM_PASS] = {"BloomPass", "Bloom"};
                desc.passes[TONE_MAPPING_PASS] = {"ToneMappingPass", "ToneMapping"};
                desc.resources.resize(9);
                desc.resources[LIGHTING_BUFFER] = { RenderGraphResourceType::transient,
                    RenderGraphResourceFlag::none,
                    "LightingBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba16_float, TextureUsageFlag::read_texture | TextureUsageFlag::read_write_texture,
                            (u32)settings.screen_size.x, (u32)settings.screen_size.y, 1, 1)) };
                desc.resources[DEPTH_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "DepthBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::d32_float, TextureUsageFlag::depth_stencil_attachment | TextureUsageFlag::read_texture, 
                            (u32)settings.screen_size.x, (u32)settings.screen_size.y, 1, 1)) };
                desc.resources[BACK_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "BackBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment | TextureUsageFlag::copy_source, 0, 0, 1, 1))};
                desc.resources[WIREFRAME_BACK_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "WireframeBackBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment | TextureUsageFlag::copy_source,
                            (u32)settings.screen_size.x, (u32)settings.screen_size.y, 1, 1))};
                desc.resources[GBUFFER_VIS_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "GBufferBackBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment | TextureUsageFlag::copy_source,
                            (u32)settings.screen_size.x, (u32)settings.screen_size.y, 1, 1))};
                desc.resources[BASE_COLOR_ROUGHNESS_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "BaseColorRoughnessBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment, 0, 0, 1, 1))};
                desc.resources[NORMAL_METALLIC_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "NormalMetallicBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment, 0, 0, 1, 1))};
                desc.resources[EMISSIVE_BUFFER] = {RenderGraphResourceType::transient, 
                    RenderGraphResourceFlag::none,
                    "EmissiveBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba16_float, TextureUsageFlag::read_texture | TextureUsageFlag::color_attachment, 0, 0, 1, 1))};
                desc.resources[BLOOM_BUFFER] = {RenderGraphResourceType::transient,
                    RenderGraphResourceFlag::none,
                    "BloomBuffer",
                    ResourceDesc::as_texture(MemoryType::local,
                        TextureDesc::tex2d(Format::rgba16_float, TextureUsageFlag::copy_dest | TextureUsageFlag::read_texture, 0, 0, 1, 1))};

                switch(settings.mode)
                {
                    case SceneRendererMode::wireframe:
                        desc.resources[WIREFRAME_BACK_BUFFER].type = RenderGraphResourceType::persistent;
                        desc.resources[WIREFRAME_BACK_BUFFER].flags |= RenderGraphResourceFlag::output;
                        break;
                    case SceneRendererMode::base_color:
                    case SceneRendererMode::normal:
                    case SceneRendererMode::roughness:
                    case SceneRendererMode::metallic:
                    case SceneRendererMode::depth:
                        desc.resources[GBUFFER_VIS_BUFFER].type = RenderGraphResourceType::persistent;
                        desc.resources[GBUFFER_VIS_BUFFER].flags |= RenderGraphResourceFlag::output;
                        break;
                    default:
                        desc.resources[BACK_BUFFER].type = RenderGraphResourceType::persistent;
                        desc.resources[BACK_BUFFER].flags |= RenderGraphResourceFlag::output;
                }
                
                desc.output_connections.push_back({WIREFRAME_PASS, "scene_texture", WIREFRAME_BACK_BUFFER});
                desc.output_connections.push_back({GEOMETRY_PASS, "depth_texture", DEPTH_BUFFER});
                desc.output_connections.push_back({GEOMETRY_PASS, "base_color_roughness_texture", BASE_COLOR_ROUGHNESS_BUFFER});
                desc.output_connections.push_back({GEOMETRY_PASS, "normal_metallic_texture", NORMAL_METALLIC_BUFFER});
                desc.output_connections.push_back({GEOMETRY_PASS, "emissive_texture", EMISSIVE_BUFFER});
                desc.input_connections.push_back({SKYBOX_PASS, "depth_texture", DEPTH_BUFFER});
                desc.output_connections.push_back({SKYBOX_PASS, "texture", LIGHTING_BUFFER});
                desc.input_connections.push_back({DEFERRED_LIGHTING_PASS, "depth_texture", DEPTH_BUFFER});
                desc.input_connections.push_back({DEFERRED_LIGHTING_PASS, "base_color_roughness_texture", BASE_COLOR_ROUGHNESS_BUFFER});
                desc.input_connections.push_back({DEFERRED_LIGHTING_PASS, "normal_metallic_texture", NORMAL_METALLIC_BUFFER});
                desc.input_connections.push_back({DEFERRED_LIGHTING_PASS, "emissive_texture", EMISSIVE_BUFFER});
                desc.output_connections.push_back({DEFERRED_LIGHTING_PASS, "scene_texture", LIGHTING_BUFFER});
                desc.input_connections.push_back({BUFFER_VIS_PASS, "depth_texture", DEPTH_BUFFER});
                desc.input_connections.push_back({BUFFER_VIS_PASS, "base_color_roughness_texture", BASE_COLOR_ROUGHNESS_BUFFER});
                desc.input_connections.push_back({BUFFER_VIS_PASS, "normal_metallic_texture", NORMAL_METALLIC_BUFFER});
                desc.output_connections.push_back({BUFFER_VIS_PASS, "scene_texture", GBUFFER_VIS_BUFFER});
                desc.input_connections.push_back({BLOOM_PASS, "scene_texture", LIGHTING_BUFFER});
                desc.output_connections.push_back({BLOOM_PASS, "bloom_texture", BLOOM_BUFFER});
                desc.input_connections.push_back({TONE_MAPPING_PASS, "hdr_texture", LIGHTING_BUFFER});
                desc.input_connections.push_back({TONE_MAPPING_PASS, "bloom_texture", BLOOM_BUFFER});
                desc.output_connections.push_back({TONE_MAPPING_PASS, "ldr_texture", BACK_BUFFER});

                m_render_graph->set_desc(desc);
                RG::RenderGraphCompileConfig config;
                config.enable_time_profiling = settings.frame_profiling;
                luexp(m_render_graph->compile(config));
            }
        }
        lucatchret;
        return ok;
    }
    RV SceneRenderer::render()
    {
        lutry
        {
            Scene* s = scene.get();
            if(!s) return set_error(BasicError::null_value(), "`scene` is `nullptr`.");

            auto scene_renderer = s->get_scene_component<SceneSettings>();
            if (!scene_renderer)
            {
                return set_error(BasicError::bad_data(), "The scene must have one `SceneSettings` global component attached.");
            }

            // Fetch camera component.
            auto camera_entity = s->find_entity(scene_renderer->camera_entity);
            if (!camera_entity)
            {
                return set_error(BasicError::bad_data(), "`camera_entity` of `SceneSettings` is null or refers to one invalid entity.");
            }

            auto camera_component = camera_entity->get_component<Camera>();
            if (!camera_component)
            {
                return set_error(BasicError::bad_data(), "`camera_entity` of `SceneSettings` must have one `Camera` component.");
            }

            camera_component->aspect_ratio = (f32)m_settings.screen_size.x / (f32)m_settings.screen_size.y;

            // Update and upload camera data.
            auto world_to_view = camera_entity->world_to_local_matrix();
            auto view_to_proj = camera_component->get_projection_matrix();
            auto world_to_proj = mul(world_to_view, view_to_proj);
            CameraCB camera_cb_data;
            camera_cb_data.world_to_view = world_to_view;
            camera_cb_data.view_to_proj = view_to_proj;
            camera_cb_data.world_to_proj = world_to_proj;
            camera_cb_data.proj_to_world = inverse(world_to_proj);
            camera_cb_data.view_to_world = camera_entity->local_to_world_matrix();
            Float3 env_color = scene_renderer->environment_color;
            camera_cb_data.screen_width = m_settings.screen_size.x;
            camera_cb_data.screen_height = m_settings.screen_size.y;
            void* mapped = nullptr;
            luexp(m_camera_cb->map(0, 0, &mapped));
            memcpy(mapped, &camera_cb_data, sizeof(CameraCB));
            m_camera_cb->unmap(0, sizeof(CameraCB));

            using namespace RHI;

            auto device = command_buffer->get_device();

            // Fetch meshes to draw.
            Vector<MeshRenderParams> render_params;
            Vector<MaterialParameters> materials;
            auto& entities = s->root_entities;
            for (auto& i : entities)
            {
                auto r = i->get_component<ModelRenderer>();
                if (r)
                {
                    auto model = get_asset_or_async_load_if_not_ready<Model>(r->model);
                    if (!model)
                    {
                        continue;
                    }
                    auto mesh = get_asset_or_async_load_if_not_ready<Mesh>(model->mesh);
                    if (!mesh)
                    {
                        continue;
                    }
                    MeshRenderParams params;
                    params.entity = i;
                    params.renderer = r;
                    u32 num_pieces = (u32)mesh->pieces.size();
                    for(u32 i = 0; i < num_pieces; ++i)
                    {
                        MaterialParameters mat_params;
                        mat_params.emissive_intensity = 1.0f;
                        if(i < model->materials.size())
                        {
                            auto mat = get_asset_or_async_load_if_not_ready<Material>(model->materials[i]);
                            if(mat)
                            {
                                mat_params.emissive_intensity = mat->emissive_intensity;
                            }
                        }
                        materials.push_back(mat_params);
                    }
                    render_params.push_back(params);
                }
            }

            // Upload mesh matrices.
            {
                if (m_num_model_matrices < render_params.size())
                {
                    luset(m_model_matrices, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::read_buffer, (u64)m_model_matrices_stride * (u64)render_params.size())));
                    m_num_model_matrices = render_params.size();
                }
                if (!render_params.empty())
                {
                    u8* mapped = nullptr;
                    luexp(m_model_matrices->map(0, 0, (void**)&mapped));
                    for (usize i = 0; i < render_params.size(); ++i)
                    {
                        MeshBuffer* dst = (MeshBuffer*)(mapped + i * m_model_matrices_stride);
                        dst->model_to_world = render_params[i].entity->local_to_world_matrix();
                        dst->world_to_model = render_params[i].entity->world_to_local_matrix();
                    }
                    m_model_matrices->unmap(0, render_params.size() * m_model_matrices_stride);
                }
            }
            // Upload material parameters.
            {
                if (m_num_materials < materials.size())
                {
                    luset(m_material_parameters, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::read_buffer, (u64)m_material_parameter_stride * (u64)materials.size())));
                    m_num_materials = materials.size();
                }
                if (!materials.empty())
                {
                    u8* mapped = nullptr;
                    luexp(m_material_parameters->map(0, 0, (void**)&mapped));
                    for(usize i = 0; i < materials.size(); ++i)
                    {
                        MaterialParameters* dst = (MaterialParameters*)(mapped + i * m_material_parameter_stride);
                        *dst = materials[i];
                    }
                    m_material_parameters->unmap(0, materials.size() * m_material_parameter_stride);
                }
            }

            // Fetches lights to draw.
            Vector<Ref<Entity>> light_ts;
            Vector<ObjRef> light_rs;
            for (auto& i : entities)
            {
                ObjRef r = ObjRef(i->get_component<DirectionalLight>());
                if (!r)
                {
                    r = i->get_component<PointLight>();
                    if (!r)
                    {
                        r = i->get_component<SpotLight>();
                    }
                }
                if (r)
                {
                    light_ts.push_back(i);
                    light_rs.push_back(r);
                }
            }

            // Upload lighting params.
            {
                usize light_size = max<usize>(light_ts.size(), 1);
                if (m_num_lights < light_size)
                {
                    luset(m_lighting_params, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::read_buffer, sizeof(LightingParams) * light_size)));
                    m_num_lights = light_size;
                }
                void* mapped = nullptr;
                luexp(m_lighting_params->map(0, 0, &mapped));
                for (usize i = 0; i < light_ts.size(); ++i)
                {
                    LightingParams p;
                    Ref<DirectionalLight> directional = light_rs[i];
                    if (directional)
                    {
                        p.strength = directional->intensity * directional->intensity_multiplier;
                        p.attenuation_power = 1.0f;
                        p.direction = AffineMatrix::forward(AffineMatrix::make_rotation(light_ts[i]->world_rotation()));
                        p.type = 0;
                        p.position = light_ts[i]->world_position();
                        p.spot_attenuation_power = 0.0f;
                    }
                    else
                    {
                        Ref<PointLight> point = light_rs[i];
                        if (point)
                        {
                            p.strength = point->intensity * point->intensity_multiplier;
                            p.attenuation_power = point->attenuation_power;
                            p.direction = Float3U(0.0f, 0.0f, 1.0f);
                            p.type = 1;
                            p.position = light_ts[i]->world_position();
                            p.spot_attenuation_power = 0.0f;
                        }
                        else
                        {
                            Ref<SpotLight> spot = light_rs[i];
                            if (spot)
                            {
                                p.strength = spot->intensity * spot->intensity_multiplier;
                                p.attenuation_power = spot->attenuation_power;
                                p.direction = AffineMatrix::forward(AffineMatrix::make_rotation(light_ts[i]->world_rotation()));
                                p.type = 2;
                                p.position = light_ts[i]->world_position();
                                p.spot_attenuation_power = spot->spot_power;
                            }
                            else
                            {
                                lupanic_always();
                            }
                        }
                    }
                    memcpy((LightingParams*)mapped + i, &p, sizeof(LightingParams));
                }
                // Adds one fake light if there is no light so the SRV is not empty (which is invalid).
                if (light_ts.empty())
                {
                    LightingParams p;
                    p.strength = Float3U{ 0.0f, 0.0f, 0.0f };
                    p.attenuation_power = 1.0f;
                    p.direction = Float3U{ 0.0f, 0.0f, 1.0f };
                    p.type = 0;
                    p.position = Float3U{ 0.0f, 0.0f, 0.0f };
                    p.spot_attenuation_power = 0.0f;
                    memcpy((LightingParams*)mapped, &p, sizeof(LightingParams));
                }
                m_lighting_params->unmap(0, light_size * sizeof(LightingParams));
            }

            {
                // Set parameters.
                if(m_settings.mode == SceneRendererMode::wireframe)
                {
                    WireframePass* wireframe = cast_object<WireframePass>(m_render_graph->get_render_pass(WIREFRAME_PASS)->get_object());
                    wireframe->model_matrices = m_model_matrices;
                    wireframe->camera_cb = m_camera_cb;
                    wireframe->mesh_render_params = {render_params.data(), render_params.size()};
                }
                else
                {
                    GeometryPass* geometry = cast_object<GeometryPass>(m_render_graph->get_render_pass(GEOMETRY_PASS)->get_object());
                    geometry->camera_cb = m_camera_cb;
                    geometry->mesh_render_params = {render_params.data(), render_params.size()};
                    geometry->model_matrices = m_model_matrices;
                    geometry->material_parameters = m_material_parameters;
                    if(m_settings.mode == SceneRendererMode::base_color ||
                        m_settings.mode == SceneRendererMode::normal ||
                        m_settings.mode == SceneRendererMode::roughness ||
                        m_settings.mode == SceneRendererMode::metallic ||
                        m_settings.mode == SceneRendererMode::depth)
                    {
                        BufferVisualizationPass* buffer_vis = cast_object<BufferVisualizationPass>(m_render_graph->get_render_pass(BUFFER_VIS_PASS)->get_object());
                        switch(m_settings.mode)
                        {
                            case SceneRendererMode::base_color: buffer_vis->vis_type = 0; break;
                            case SceneRendererMode::normal: buffer_vis->vis_type = 1; break;
                            case SceneRendererMode::roughness: buffer_vis->vis_type = 2; break;
                            case SceneRendererMode::metallic: buffer_vis->vis_type = 3; break;
                            case SceneRendererMode::depth: buffer_vis->vis_type = 4; break;
                            default: break;
                        }
                    }
                    else
                    {
                        SkyBoxPass* skybox = cast_object<SkyBoxPass>(m_render_graph->get_render_pass(SKYBOX_PASS)->get_object());
                        DeferredLightingPass* lighting = cast_object<DeferredLightingPass>(m_render_graph->get_render_pass(DEFERRED_LIGHTING_PASS)->get_object());
                        ToneMappingPass* tone_mapping = cast_object<ToneMappingPass>(m_render_graph->get_render_pass(TONE_MAPPING_PASS)->get_object());
                        BloomPass* bloom_pass = cast_object<BloomPass>(m_render_graph->get_render_pass(BLOOM_PASS)->get_object());
                        skybox->camera_fov = camera_component->fov;
                        skybox->camera_type = camera_component->type;
                        skybox->view_to_world = camera_entity->local_to_world_matrix();
                        auto skybox_tex = get_asset_or_async_load_if_not_ready<RHI::IResource>(scene_renderer->skybox);
                        skybox->skybox = skybox_tex;
                        lighting->skybox = skybox_tex;
                        lighting->camera_cb = m_camera_cb;
                        lighting->light_params = m_lighting_params;
                        lighting->light_ts = {light_ts.data(), light_ts.size()};
                        switch (m_settings.mode)
                        {
                            case SceneRendererMode::lit: lighting->lighting_mode = 0; break;
                            case SceneRendererMode::emissive: lighting->lighting_mode = 1; break;
                            case SceneRendererMode::diffuse_lighting: lighting->lighting_mode = 2; break;
                            case SceneRendererMode::specular_lighting: lighting->lighting_mode = 3; break;
                            case SceneRendererMode::ambient_diffuse_lighting: lighting->lighting_mode = 4; break;
                            case SceneRendererMode::ambient_specular_lighting: lighting->lighting_mode = 5; break;
                            default: break;
                        }
                        tone_mapping->exposure = scene_renderer->exposure;
                        tone_mapping->auto_exposure = scene_renderer->auto_exposure;
                        tone_mapping->bloom_intensity = scene_renderer->bloom_intensity;
                        bloom_pass->lum_threshold = scene_renderer->bloom_threshold;
                    }
                }
            }
            luexp(m_render_graph->execute(command_buffer));
            // Set render pass parameters.
            {
                switch(m_settings.mode)
                {
                    case SceneRendererMode::wireframe:
                        render_texture = m_render_graph->get_persistent_resource(WIREFRAME_BACK_BUFFER);
                        break;
                    case SceneRendererMode::base_color:
                    case SceneRendererMode::normal:
                    case SceneRendererMode::roughness:
                    case SceneRendererMode::metallic:
                    case SceneRendererMode::depth:
                        render_texture = m_render_graph->get_persistent_resource(GBUFFER_VIS_BUFFER);
                        break;
                    default:
                        render_texture = m_render_graph->get_persistent_resource(BACK_BUFFER);
                }
            }
        }
        lucatchret;
        return ok;
    }
    void SceneRenderer::collect_frame_profiling_data()
    {
        // Collect last frame profiling data.
        if(m_settings.frame_profiling)
        {
            auto queue_freq = m_device->get_command_queue_timestamp_frequency(g_env->graphics_queue).get();
            Vector<usize> render_passes;
            m_render_graph->get_enabled_render_passes(render_passes);
            if (!render_passes.empty())
            {
                enabled_passes.clear();
                auto& desc = m_render_graph->get_desc();
                for (usize i : render_passes)
                {
                    enabled_passes.push_back(desc.passes[i].name);
                }
                Vector<u64> times;
                auto r = m_render_graph->get_pass_time_intervals(times);
                pass_time_intervals.clear();
                if (succeeded(r))
                {
                    for (u64 t : times)
                    {
                        pass_time_intervals.push_back((f64)t / queue_freq);
                    }
                }
            }
        }
    }
}