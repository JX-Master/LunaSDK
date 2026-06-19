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

#include <Luna/GUI/Editor.hpp>

#include "Assets/Texture.hpp"
#include "Assets/MeshAsset.hpp"
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
#include "Studio.meta.generated.hpp"

#include "SceneRenderer.hpp"
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Profiler.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    MainEditor* g_main_editor;

    namespace
    {
        GUICore::LayoutInput fixed_height_layout(f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::pixels;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutInput fill_layout()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }
    }

    void MainEditor::draw_main_menu_bar(GUICore::IContext* context, const RectF& rect)
    {
        luassert(context);
        context->push_data_scope(context->make_id("studio_menu_bar"));
        GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, context->make_id("bar"), "Main Menu Bar",
            fixed_height_layout(rect.height));
        GUICore::ElementHandle save_all_item;
        if(GUI::begin_menu(context, context->make_id("file"), "File"))
        {
            save_all_item = GUI::menu_item(context, context->make_id("save_all"), "Save All");
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 40.0f)));
        }

        GUICore::ElementHandle undo_item;
        GUICore::ElementHandle redo_item;
        if(GUI::begin_menu(context, context->make_id("edit"), "Edit"))
        {
            undo_item = GUI::menu_item(context, context->make_id("undo"), "Undo", "Ctrl+Z", false, can_undo());
            redo_item = GUI::menu_item(context, context->make_id("redo"), "Redo", "Ctrl+Shift+Z", false, can_redo());
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 230.0f, 70.0f)));
        }

        if(GUI::begin_menu(context, context->make_id("view"), "View"))
        {
            for(usize i = 0; i < 4; ++i)
            {
                c8 buf[32];
                snprintf(buf, 32, "Asset Browser %u", (u32)i);
                GUI::menu_item(context, context->make_id((GUICore::id_t)(100 + i)), buf, nullptr, &m_asset_browsers_enabled[i]);
            }
            GUI::menu_item(context, context->make_id("memory_profiler"), "Memory Profiler", nullptr, &m_memory_profiler_window_enabled);
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 230.0f, 160.0f)));
        }
        lupanic_if_failed(GUI::end_menu_bar(context, menu_bar, rect));

        if(GUI::is_item_clicked(context, save_all_item))
        {
            auto _ = save_all();
        }
        if(GUI::is_item_clicked(context, undo_item))
        {
            undo();
        }
        if(GUI::is_item_clicked(context, redo_item))
        {
            redo();
        }
        context->pop_data_scope();
    }

    bool MainEditor::draw_asset_editor(IAssetEditor* editor, GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        if(!editor || !context)
        {
            return false;
        }
        editor->on_render(context, layout);
        return true;
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
            m_gui = GUICore::new_context();
            GUI::register_editor_style_schemas(m_gui);
            luexp(m_gui->register_font(Name("default"), Font::get_default_font()));
            m_gui_draw_list = VG::new_shape_draw_list(g_env->device);
            m_gui_renderer = VG::new_fill_shape_renderer();

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

            Meta::register_Studio_types();
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

            register_enum_type<SceneRendererMode>();
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
            UInt2U framebuffer_size = m_window->get_framebuffer_size();
            GUICore::FrameDesc gui_frame;
            gui_frame.screen_size = Float2U((f32)sz.x, (f32)sz.y);
            gui_frame.framebuffer_size = framebuffer_size;
            gui_frame.dpi_scale = m_window->get_dpi_scale_factor();
            gui_frame.delta_time = 1.0f / 60.0f;
            m_gui->begin_frame(gui_frame);
            GUIWindow::update_input(m_window, m_gui);

            constexpr f32 menu_height = 34.0f;
            RectF screen_rect(0.0f, 0.0f, (f32)sz.x, (f32)sz.y);
            RectF menu_rect(0.0f, 0.0f, (f32)sz.x, min(menu_height, (f32)sz.y));
            RectF dock_rect(0.0f, menu_rect.height, (f32)sz.x, max((f32)sz.y - menu_rect.height, 0.0f));

            m_gui->push_layer(m_gui->make_id("studio_root_layer"), Float2U(0.0f), Name("Studio Root"));
            GUICore::ElementHandle root = GUI::begin_v_layout(m_gui, m_gui->make_id("studio_root"), "Studio Root");
            draw_main_menu_bar(m_gui, menu_rect);

            GUICore::ElementHandle dock_space = GUI::begin_dock_space(m_gui, m_gui->make_id("studio_dock_space"),
                "Studio DockSpace", fill_layout());
            for (usize i = 0; i < 4; ++i)
            {
                if (m_asset_browsers_enabled[i])
                {
                    m_asset_browsers[i]->render(m_gui, &m_asset_browsers_enabled[i]);
                }
            }

            if(m_memory_profiler_window_enabled)
            {
                m_memory_profiler.render(m_gui, fill_layout());
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
                    draw_asset_editor(iter->get(), m_gui, fill_layout());
                    ++iter;
                }
            }
            luexp(GUI::end_dock_space(m_gui, dock_space, dock_rect));
            luexp(GUI::end_v_layout(m_gui, root, screen_rect, GUICore::LinearLayoutDesc()));
            m_gui->pop_layer();
            m_gui->route_input();
            luexp(GUIWindow::update_text_input(m_window, m_gui));
            RHI::RenderPassDesc render_pass;
            lulet(back_buffer, m_swap_chain->get_current_back_buffer());
            render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store,
                { 0.0f, 0.0f, 0.0f, 1.0f });
            m_cmdbuf->begin_render_pass(render_pass);
            m_cmdbuf->end_render_pass();
            luexp(m_gui->compile_draw_commands(m_gui_draw_list));
            luexp(m_gui_draw_list->compile());
            Span<const VG::ShapeDrawCall> gui_draw_calls = m_gui_draw_list->get_draw_calls();
            if(!gui_draw_calls.empty())
            {
                luexp(m_gui_renderer->begin(back_buffer));
                m_gui_renderer->draw(m_gui_draw_list->get_vertex_buffer(),
                    m_gui_draw_list->get_index_buffer(),
                    gui_draw_calls,
                    nullptr);
                luexp(m_gui_renderer->end());
                m_gui_renderer->submit(m_cmdbuf);
            }
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
        register_enum_type<CameraType>();
        set_serializable<CameraType>();
        
        register_struct_type<Camera>();
        set_serializable<Camera>();
        g_env->component_types.insert(typeof<Camera>());
        set_property_attribute(typeof<Camera>(), "fov", "radian", true);
        set_property_attribute(typeof<Camera>(), "fov", "gui_min", (f64)deg_to_rad(60));
        set_property_attribute(typeof<Camera>(), "fov", "gui_max", (f64)deg_to_rad(160));
        set_property_attribute(typeof<Camera>(), "aspect_ratio", "hide", true);
        
        register_struct_type<ActorRef>();
        set_serializable<ActorRef>();

        register_struct_type<ActorInfo>();

        register_struct_type<Transform>();
        set_serializable<Transform>();

        register_enum_type<LightType>();

        set_serializable<LightType>();

        register_struct_type<Light>();
        set_serializable<Light>();
        g_env->component_types.insert(typeof<Light>());

        set_property_attribute(typeof<Light>(), "intensity", "color_gui", true);

        register_struct_type<ModelRenderer>();
        set_serializable<ModelRenderer>();
        g_env->component_types.insert(typeof<ModelRenderer>());

        register_struct_type<SceneSettings>();
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
        register_struct_type<Operation>();
        register_struct_type<AssetEditingOp>(typeof<Operation>());
        register_struct_type<DiffAssetEditingOp>(typeof<AssetEditingOp>());

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

    void draw_asset_tile(GUICore::IContext* context, Asset::asset_t asset, const RectF& draw_rect)
    {
        luassert(context);
        if(asset)
        {
            auto asset_type = Asset::get_asset_type(asset);
            auto iter = g_env->editor_types.find(asset_type);
            if(iter != g_env->editor_types.end())
            {
                if(iter->second.on_draw_tile_core)
                {
                    iter->second.on_draw_tile_core(context, iter->second.userdata.get(), asset, draw_rect);
                }
                else
                {
                    GUI::draw_text(context, context->make_id((GUICore::id_t)(usize)asset.handle), draw_rect,
                        asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
                }
                return;
            }
            GUI::draw_text(context, context->make_id((GUICore::id_t)(usize)asset.handle), draw_rect,
                asset_type.c_str(), Float4U(1.0f), 16.0f, GUI::TextAlignment::center, GUI::TextAlignment::center);
        }
    }

    namespace
    {
        void draw_relative_tile_text(GUICore::IContext* context, const RectF& relative_rect, const c8* text)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = relative_rect;
            command.color = Float4U(1.0f);
            command.font_size = 16.0f;
            command.horizontal_alignment = VG::TextAlignment::center;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }
    }

    void draw_asset_tile_preview(GUICore::IContext* context, Asset::asset_t asset, const RectF& relative_rect)
    {
        luassert(context);
        if(asset)
        {
            auto asset_type = Asset::get_asset_type(asset);
            auto iter = g_env->editor_types.find(asset_type);
            if(iter != g_env->editor_types.end())
            {
                if(iter->second.on_draw_tile_preview_core)
                {
                    iter->second.on_draw_tile_preview_core(context, iter->second.userdata.get(), asset, relative_rect);
                }
                else
                {
                    draw_relative_tile_text(context, relative_rect, asset_type.c_str());
                }
                return;
            }
            draw_relative_tile_text(context, relative_rect, asset_type.c_str());
        }
    }
}
