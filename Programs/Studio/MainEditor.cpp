/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MainEditor.cpp
* @author JXMaster
* @date 2020/4/27
*/
#include "MainEditor.hpp"

#include "Assets/Texture.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/Scene.hpp"
#include "Assets/Model.hpp"
#include "Assets/Material.hpp"

#include <Luna/VFS/VFS.hpp>
#include <Luna/Window/MessageBox.hpp>

#include "Camera.hpp"
#include "Transform.hpp"
#include "Light.hpp"
#include "SceneSettings.hpp"
#include "ModelRenderer.hpp"
#include <Luna/Runtime/Serialization.hpp>
#include "World.hpp"
#include "Scene.hpp"

#include "RenderPasses/SkyBoxPass.hpp"
#include "RenderPasses/ToneMappingPass.hpp"
#include "RenderPasses/WireframePass.hpp"
#include "RenderPasses/GeometryPass.hpp"
#include "RenderPasses/DeferredLightingPass.hpp"
#include "RenderPasses/BufferVisualizationPass.hpp"
#include "RenderPasses/BloomPass.hpp"

#include "SceneRenderer.hpp"
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Profiler.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>

namespace Luna
{
    MainEditor* g_main_editor;

    void MainEditor::draw_main_menu_bar()
    {
        GUI::set_next_item_layout(m_gui, GUI::LayoutStyle::fixed_height(34.0f));
        GUI::begin_menu_bar(m_gui, "Main Menu Bar");
        GUI::begin_menu(m_gui, "File");
        GUI::ItemHandle save_all_item = GUI::menu_item(m_gui, "Save All");
        GUI::end_menu(m_gui);

        GUI::begin_menu(m_gui, "Edit");
        GUI::ItemHandle undo_item = GUI::menu_item(m_gui, "Undo", "Ctrl+Z", false, can_undo());
        GUI::ItemHandle redo_item = GUI::menu_item(m_gui, "Redo", "Ctrl+Shift+Z", false, can_redo());
        GUI::end_menu(m_gui);

        GUI::begin_menu(m_gui, "View");
        for(usize i = 0; i < 4; ++i)
        {
            c8 buf[32];
            snprintf(buf, 32, "Asset Browser %u", (u32)i);
            GUI::menu_item(m_gui, buf, nullptr, &m_asset_browsers_enabled[i]);
        }
        GUI::menu_item(m_gui, "Memory Profiler", nullptr, &m_memory_profiler_window_enabled);
        GUI::end_menu(m_gui);
        GUI::end_menu_bar(m_gui);

        if(GUI::is_item_clicked(save_all_item))
        {
            auto _ = save_all();
        }
        if(GUI::is_item_clicked(undo_item))
        {
            undo();
        }
        if(GUI::is_item_clicked(redo_item))
        {
            redo();
        }
    }

    RV MainEditor::init(const Path& project_path)
    {
        lutry
        {
            set_log_to_platform_enabled(true);

            MemoryProfilerCallback memory_profiler_callback;
            memory_profiler_callback.m_profiler = &m_memory_profiler;
            m_memory_profiler_callback_handle = register_profiler_callback(memory_profiler_callback);

            char title[256];
            auto name = project_path.filename();

            luset(m_job_scheduler, JobSystem::new_job_scheduler());

            // Mount Data folder.
            auto mount_path = project_path;
            mount_path.push_back("Data");
            luexp(VFS::mount(VFS::get_platform_filesystem_driver(), mount_path.encode(PathSeparator::system_preferred).c_str(), "/"));

            // Load all asset metadata.
            luexp(Asset::load_assets_meta("/"));

            // Create window and render objects.
            snprintf(title, 256, "%s - %s", name.c_str(), APP_NAME);
            luset(m_window, Window::new_window(title));

            Window::set_event_handler([](object_t event, void* userdata){
                MainEditor* editor = (MainEditor*)userdata;
                GUIWindow::handle_window_event(event, editor->m_window, editor->m_gui);
                if(auto e = cast_object<Window::WindowRequestCloseEvent>(event))
                {
                    if(e->window == editor->m_window)
                    {
                        bool should_close = true;
                        if(editor->has_any_unsaved_changes())
                        {
                            auto r = Window::message_box("Save changes before closing the current project?", APP_NAME, Window::MessageBoxType::yes_no_cancel, Window::MessageBoxIcon::question);
                            luassert_always(succeeded(r));
                            if(r.get() == Window::MessageBoxButton::cancel)
                            {
                                should_close = false;
                            }
                            else if(r.get() == Window::MessageBoxButton::yes)
                            {
                                // Save document.
                                RV ret = editor->save_all();
                                if(failed(ret))
                                {
                                    should_close = false;
                                }
                            }
                        }
                        e->do_close = should_close;
                    }
                }
            }, this);

            luset(m_swap_chain, g_env->device->new_swap_chain(g_env->graphics_queue, m_window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
            luset(m_cmdbuf, g_env->device->new_command_buffer(g_env->graphics_queue));
            m_gui = GUI::new_context(g_env->device);

            // Create asset browser instance.
            for (usize i = 0; i < 4; ++i)
            {
                Ref<AssetBrowser> browser = new_object<AssetBrowser>();
                browser->m_editor = this;
                //browser->m_index = m_next_asset_browser_index;
                //++m_next_asset_browser_index;
                browser->m_path = "/";
                auto his_path = browser->m_path;
                browser->m_histroy_paths.push_back(his_path);
                m_asset_browsers[i] = browser;
            }

            // Register types.

            register_components();

            luexp(register_static_texture_asset_type());
            register_texture_editor();
            register_texture_importer();
            register_static_mesh_asset_type();
            register_static_mesh_importer();
            
            register_material_asset_type();
            register_material_editor();
            register_model_asset_type();
            register_model_editor();

            register_scene_asset_type();
            luexp(register_scene_editor());

            g_env->new_asset_types.insert(get_material_asset_type());
            g_env->new_asset_types.insert(get_model_asset_type());
            g_env->new_asset_types.insert(get_scene_asset_type());

            luexp(register_sky_box_pass());
            luexp(register_wireframe_pass());
            luexp(register_geometry_pass());
            luexp(register_deferred_lighting_pass());
            luexp(register_bloom_pass());
            luexp(register_tone_mapping_pass());
            luexp(register_buffer_visualization_pass());

            register_enum_type<SceneRendererMode>({
                luoption(SceneRendererMode, lit),
                luoption(SceneRendererMode, wireframe),
                luoption(SceneRendererMode, base_color),
                luoption(SceneRendererMode, normal),
                luoption(SceneRendererMode, roughness),
                luoption(SceneRendererMode, metallic),
                luoption(SceneRendererMode, depth),
                luoption(SceneRendererMode, emissive),
                luoption(SceneRendererMode, diffuse_lighting),
                luoption(SceneRendererMode, specular_lighting),
                luoption(SceneRendererMode, ambient_diffuse_lighting),
                luoption(SceneRendererMode, ambient_specular_lighting)
            });
        }
        lucatchret;
        return ok;
    }

    RV MainEditor::update()
    {
        Window::poll_events();

        if (m_window->is_closed())
        {
            m_exiting = true;
            return ok;
        }
        if (m_window->is_minimized())
        {
            sleep(100);
            return ok;
        }

        lutry
        {
            // Recreate the back buffer if needed.
            auto sz = m_window->get_framebuffer_size();
            if (sz.x && sz.y && (sz.x != m_main_window_width || sz.y != m_main_window_height))
            {
                luexp(m_swap_chain->reset({sz.x, sz.y, 2, RHI::Format::unknown, true}));
                m_main_window_width = sz.x;
                m_main_window_height = sz.y;
            }

            sz = m_window->get_size();
            GUI::FrameDesc gui_frame;
            gui_frame.surface_size = Float2U((f32)sz.x, (f32)sz.y);
            gui_frame.framebuffer_size = m_window->get_framebuffer_size();
            gui_frame.dpi_scale = m_window->get_dpi_scale_factor();
            gui_frame.delta_time = 1.0f / 60.0f;
            m_gui->begin_frame(gui_frame);

            // Main window GUI code goes here.

            //m_ctx->show_demo_window();

            GUI::LayoutDesc root_layout;
            root_layout.padding = GUI::EdgeInsets::all(0.0f);
            root_layout.gap = 0.0f;
            GUI::begin_v_layout(m_gui, "Studio Root", RectF(0.0f, 0.0f, (f32)sz.x, (f32)sz.y), root_layout);
            draw_main_menu_bar();

            GUI::set_next_item_layout(m_gui, GUI::LayoutStyle::fill());
            GUI::begin_dock_space(m_gui, "Studio DockSpace");
            for (usize i = 0; i < 4; ++i)
            {
                if (m_asset_browsers_enabled[i])
                {
                    m_asset_browsers[i]->render(m_gui, &m_asset_browsers_enabled[i]);
                }
            }

            if(m_memory_profiler_window_enabled)
            {
                m_memory_profiler.render(m_gui);
            }

            // Draw Editors.
            auto iter = m_editors.begin();
            while (iter != m_editors.end())
            {
                if ((*iter)->closed())
                {
                    iter = m_editors.erase(iter);
                }
                else
                {
                    (*iter)->on_render(m_gui);
                    ++iter;
                }
            }
            GUI::end_dock_space(m_gui);
            GUI::end_v_layout(m_gui);

            lulet(gui_desc, m_gui->end_build());
            luexp(m_gui->submit(gui_desc));
            luexp(GUIWindow::update_text_input(m_window, m_gui));
            RHI::RenderPassDesc render_pass;
            lulet(back_buffer, m_swap_chain->get_current_back_buffer());
            render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store,
                { 0.0f, 0.0f, 0.0f, 1.0f });
            m_cmdbuf->begin_render_pass(render_pass);
            m_cmdbuf->end_render_pass();
            luexp(m_gui->render(m_cmdbuf, back_buffer));
            m_cmdbuf->resource_barrier({}, {
                    {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                });
            luexp(m_cmdbuf->submit({}, {}, true));
            m_cmdbuf->wait();
            luexp(m_cmdbuf->reset());
            luexp(m_swap_chain->present());
        }
        lucatchret;
        return ok;
    }
    void MainEditor::close()
    {
        unregister_profiler_callback(m_memory_profiler_callback_handle);
    }
    RV MainEditor::save_all()
    {
        RV ret = ok;
        for(auto& asset : m_assets_version)
        {
            if(asset.second.edit_version != asset.second.save_version)
            {
                auto r = save_asset(asset.first);
                if(failed(r))
                {
                    ret = r;
                    String errmsg;
                    strprintf(errmsg, "Failed to save asset %s: %s", Asset::get_asset_path(asset.first).encode().c_str(), explain(r.errcode()));
                    auto _ = Window::message_box(errmsg.c_str(), APP_NAME, Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                }
            }
        }
        return ret;
    }
    void MainEditor::execute(Operation* op)
    {
        luassert(op);
        op->execute();
        while(m_operations_stack_top < m_operations_stack.size())
        {
            m_operations_stack.pop_back();
        }
        // Keep memory usage reasonable for undo history.
        while(m_operations_stack.size() > 256)
        {
            m_operations_stack.pop_front();
        }
        m_operations_stack.push_back(op);
        m_operations_stack_top = m_operations_stack.size();
    }
    void MainEditor::undo()
    {
        luassert(can_undo());
        m_operations_stack[m_operations_stack_top - 1]->revert();
        m_operations_stack_top -= 1;
    }
    void MainEditor::redo()
    {
        luassert(can_redo());
        m_operations_stack[m_operations_stack_top]->execute();
        m_operations_stack_top += 1;
    }
    RV MainEditor::save_asset(Asset::asset_t asset)
    {
        lutry
        {
            luexp(Asset::save_asset(asset));
            mark_asset_as_saved(asset);
        }
        lucatchret;
        return ok;
    }

    void register_components()
    {
        register_enum_type<CameraType>({
                luoption(CameraType, perspective),
                luoption(CameraType, orthographic)
            });
        set_serializable<CameraType>();
        
        register_struct_type<Camera>({
            luproperty(Camera, CameraType, type),
            luproperty(Camera, f32, fov),
            luproperty(Camera, f32, size),
            luproperty(Camera, f32, near_clipping_plane),
            luproperty(Camera, f32, far_clipping_plane),
            luproperty(Camera, f32, aspect_ratio)
            });
        set_serializable<Camera>();
        g_env->component_types.insert(typeof<Camera>());
        set_property_attribute(typeof<Camera>(), "fov", "radian", true);
        set_property_attribute(typeof<Camera>(), "fov", "gui_min", (f64)deg_to_rad(60));
        set_property_attribute(typeof<Camera>(), "fov", "gui_max", (f64)deg_to_rad(160));
        set_property_attribute(typeof<Camera>(), "aspect_ratio", "hide", true);
        
        register_struct_type<ActorRef>({
            luproperty(ActorRef, Guid, guid)
        });
        set_serializable<ActorRef>();

        register_struct_type<ActorInfo>({});

        register_struct_type<Transform>({
            luproperty(Transform, Float3, position),
            luproperty(Transform, Quaternion, rotation),
            luproperty(Transform, Float3, scale)
        });
        set_serializable<Transform>();

        register_enum_type<LightType>({
            luoption(LightType, directional),
            luoption(LightType, point),
            luoption(LightType, spot)
        });

        set_serializable<LightType>();

        register_struct_type<Light>({
            luproperty(Light, LightType, type),
            luproperty(Light, Float3, intensity),
            luproperty(Light, f32, intensity_multiplier),
            luproperty(Light, f32, attenuation_power),
            luproperty(Light, f32, spot_power)
            });
        set_serializable<Light>();
        g_env->component_types.insert(typeof<Light>());

        set_property_attribute(typeof<Light>(), "intensity", "color_gui", true);

        register_struct_type<ModelRenderer>({
            luproperty(ModelRenderer, Asset::asset_t, model)
            });
        set_serializable<ModelRenderer>();
        g_env->component_types.insert(typeof<ModelRenderer>());

        register_struct_type<SceneSettings>({
            luproperty(SceneSettings, ActorRef, camera_actor),
            luproperty(SceneSettings, Asset::asset_t, skybox),
            luproperty(SceneSettings, Float3, environment_color),
            luproperty(SceneSettings, f32, skybox_rotation),
            luproperty(SceneSettings, f32, exposure),
            luproperty(SceneSettings, bool, auto_exposure),
            luproperty(SceneSettings, f32, bloom_threshold),
            luproperty(SceneSettings, f32, bloom_intensity)
            });
        set_serializable<SceneSettings>();
        g_env->scene_component_types.insert(typeof<SceneSettings>());
        set_property_attribute(typeof<SceneSettings>(), "environment_color", "color_gui", true);
        set_property_attribute(typeof<SceneSettings>(), "exposure", "gui_min", (f64)0.00001f);
        set_property_attribute(typeof<SceneSettings>(), "exposure", "gui_max", (f64)1.0f);
        set_property_attribute(typeof<SceneSettings>(), "bloom_threshold", "gui_min", (f64)0.0f);
        set_property_attribute(typeof<SceneSettings>(), "bloom_threshold", "gui_max", (f64)10.0f);
        set_property_attribute(typeof<SceneSettings>(), "bloom_intensity", "gui_min", (f64)0.0f);
        set_property_attribute(typeof<SceneSettings>(), "bloom_intensity", "gui_max", (f64)2.0f);
    }

    void run_main_editor(const Path& project_path)
    {
        register_boxed_type<MainEditor>();
        register_boxed_type<AssetBrowser>();
        register_struct_type<Operation>({});
        register_struct_type<AssetEditingOp>({}, typeof<Operation>());
        register_struct_type<DiffAssetEditingOp>({}, typeof<AssetEditingOp>());

        Ref<MainEditor> main_editor = new_object<MainEditor>();
        g_main_editor = main_editor;
        if (!main_editor)
        {
            return;
        }
        lutry
        {
            luexp(main_editor->init(project_path));
            while (!main_editor->m_exiting)
            {
                luexp(main_editor->update());
            }
            main_editor->close();
        }
        lucatch
        {
            auto _ = Window::message_box(explain(luerr), "Editor Crashed.", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
            return;
        }
        Asset::close();
    }

    void draw_asset_tile(GUI::IContext* context, Asset::asset_t asset, const RectF& draw_rect)
    {
        if (asset)
        {
            auto asset_type = Asset::get_asset_type(asset);
            auto iter = g_env->editor_types.find(asset_type);
            if (iter != g_env->editor_types.end())
            {
                if (iter->second.on_draw_tile)
                {
                    iter->second.on_draw_tile(context, iter->second.userdata.get(), asset, draw_rect);
                }
                else
                {
                    GUI::draw_text(context, draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
                }
                return;
            }
            GUI::draw_text(context, draw_rect, asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
        }
    }
}
