/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorApp.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <Luna/Window/Event.hpp>
#if defined(LUNA_PLATFORM_MACOS)
#include <Luna/Window/ApplicationMenu.hpp>
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            RV EditorApp::run()
            {
                lutry
                {
                    i32 frame_index = 0;
                    while(true)
                    {
                        Window::poll_events();
#if defined(LUNA_PLATFORM_MACOS)
                        if(Window::is_application_quit_requested()) break;
                        luexp(update_application_menu_state());
#endif
                        if(window->is_closed()) break;
                        if(window->is_minimized())
                        {
                            sleep(100);
                            continue;
                        }
                        UInt2U framebuffer_size = window->get_framebuffer_size();
                        if(framebuffer_size.x && framebuffer_size.y &&
                            (framebuffer_size.x != width || framebuffer_size.y != height))
                        {
                            luexp(swap_chain->reset({framebuffer_size.x, framebuffer_size.y, 2,
                                RHI::Format::unknown, true}));
                            luexp(resize_target(framebuffer_size));
                            width = framebuffer_size.x;
                            height = framebuffer_size.y;
                        }
                        for(DocumentView& document : documents)
                            luexp(apply_preview_surface_size(document));
                        UInt2U logical_size = window->get_size();
                        GUI::FrameDesc frame;
                        frame.logical_size = Float2U((f32)logical_size.x, (f32)logical_size.y);
                        frame.render_size = framebuffer_size;
                        frame.delta_time = 1.0f / 60.0f;
                        gui->begin_frame(frame);
                        GUIWindow::update_input(&input_adapter);
                        gui->push_layer(gui->make_id("editor.root.layer"));
                        UIHandles handles;
                        GUI::ElementHandle root = build_editor(handles);
                        gui->pop_layer();
                        luexp(EditorGUI::layout_tree(gui, root,
                            RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                        gui->route_input();
                        EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(gui);
                        if(resolved.relayout_requested)
                        {
                            luexp(EditorGUI::layout_tree(gui, root,
                                RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                        }

                        PreviewInput preview_input;
                        RectF preview_rect = EditorGUI::get_item_rect(gui, handles.preview_host);
                        DocumentView* input_document = find_document(handles.document_id);
                        if(input_document)
                            collect_preview_input(*input_document, handles.preview_host,
                                preview_rect, frame, preview_input);
                        luexp(GUIWindow::update_text_input(&input_adapter));
                        luexp(gui->generate_draw_commands());

                        process_interactions(handles);
                        if(discard_smoke && frame_index == 0)
                        {
                            DocumentView* document = active_document();
                            if(document) request_close(*document, true);
                        }
                        DocumentView* preview_document = find_document(preview_input.document_id);
                        if(!preview_document) preview_document = active_document();
                        if(preview_document)
                        {
                            Span<const GUI::InputEvent> events;
                            if(preview_document->id == preview_input.document_id)
                                events = Span<const GUI::InputEvent>(preview_input.events.data(),
                                    preview_input.events.size());
                            luexp(build_preview(*preview_document, events));
                        }
                        luexp(render_frame(preview_document));
                        for(u64 id : deferred_document_removals) remove_document_view(id);
                        deferred_document_removals.clear();
                        ++frame_index;
                        if(max_frames >= 0 && frame_index >= max_frames) break;
                    }
#if defined(LUNA_PLATFORM_MACOS)
                    auto _ = Window::reset_application_menu();
#endif
                    GUIWindow::uninstall_window_event_handler(&input_adapter);
                    Window::set_event_handler(nullptr, nullptr);
                    service.reset();
                    documents.clear();
                    luexp(document_files.close());
                    luexp(VFS::unmount("/"));
                }
                lucatchret;
                return ok;
            }
            RV EditorApp::resize_target(const UInt2U& size)
            {
                lutry
                {
                    luset(gui_target, RHI::get_main_device()->new_texture(
                        RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                            swap_chain->get_desc().format,
                            RHI::TextureUsageFlag::color_attachment |
                                RHI::TextureUsageFlag::read_texture,
                            size.x, size.y, 1, 1)));
                }
                lucatchret;
                return ok;
            }

            RV EditorApp::initialize_dock_layout()
            {
                EditorGUI::DockSpaceLayoutDesc layout;
                layout.nodes.resize(9);
                layout.root_node = 0;
                layout.nodes[0].split = true;
                layout.nodes[0].split_axis = EditorGUI::DockSplitAxis::y;
                layout.nodes[0].split_ratio = 0.10f;
                layout.nodes[0].child0 = 1;
                layout.nodes[0].child1 = 2;
                layout.nodes[1].tabs.push_back(gui->make_id("panel.palette"));

                layout.nodes[2].split = true;
                layout.nodes[2].split_axis = EditorGUI::DockSplitAxis::x;
                layout.nodes[2].split_ratio = 0.22f;
                layout.nodes[2].child0 = 3;
                layout.nodes[2].child1 = 4;
                layout.nodes[3].tabs.push_back(gui->make_id("panel.hierarchy"));
                layout.nodes[4].split = true;
                layout.nodes[4].split_axis = EditorGUI::DockSplitAxis::x;
                layout.nodes[4].split_ratio = 0.76f;
                layout.nodes[4].child0 = 5;
                layout.nodes[4].child1 = 8;
                layout.nodes[5].split = true;
                layout.nodes[5].split_axis = EditorGUI::DockSplitAxis::y;
                layout.nodes[5].split_ratio = 0.78f;
                layout.nodes[5].child0 = 6;
                layout.nodes[5].child1 = 7;
                for(const DocumentView& document : documents)
                {
                    layout.nodes[6].tabs.push_back(document_panel_id(gui, document.id));
                }
                if(!documents.empty())
                {
                    usize selected = (usize)clamp(selected_document, 0, (i32)documents.size() - 1);
                    layout.nodes[6].selected_tab = document_panel_id(gui, documents[selected].id);
                }
                layout.nodes[7].tabs.push_back(gui->make_id("panel.diagnostics"));
                layout.nodes[8].tabs.push_back(gui->make_id("panel.inspector"));
                EditorGUI::set_dockspace_layout(gui, gui->make_id("editor.dock_space"), layout);
                dock_layout_initialized = true;
                return ok;
            }

            bool EditorApp::has_dirty_documents() const
            {
                for(const DocumentView& document : documents)
                {
                    if(document.dirty) return true;
                }
                return false;
            }

            RV EditorApp::init()
            {
                lutry
                {
                    Path current_dir;
                    if(workspace_path.empty())
                    {
                        const c8* process_path = get_process_path();
                        current_dir = process_path;
                        release_process_path(process_path);
                        current_dir.pop_back();
                    }
                    else current_dir = workspace_path.c_str();
                    luexp(set_current_dir(current_dir.encode().c_str()));
                    const c8* resolved_current_dir = get_current_dir();
                    workspace_root = resolved_current_dir;
                    release_current_dir(resolved_current_dir);
                    lulet(file_system, VFS::new_native_file_system(workspace_root.encode(PathSeparator::system_preferred).c_str()));
                    luexp(VFS::mount(file_system, "/"));
                    luexp(Asset::load_assets_meta("/", true));

                    luset(window, Window::new_window(APP_NAME,
                        Window::DEFAULT_POS, Window::DEFAULT_POS, 1440, 960));
                    luexp(window->set_foreground());
                    RHI::IDevice* device = RHI::get_main_device();
                    for(u32 i = 0; i < device->get_num_command_queues(); ++i)
                    {
                        if(device->get_command_queue_desc(i).type == RHI::CommandQueueType::graphics)
                        {
                            queue = i;
                            break;
                        }
                    }
                    if(queue == U32_MAX) luthrow(set_error(E_NOT_SUPPORTED,
                        "No graphics command queue is available."));
                    UInt2U size = window->get_framebuffer_size();
                    luset(swap_chain, device->new_swap_chain(queue, window,
                        RHI::SwapChainDesc({size.x, size.y, 2, RHI::Format::bgra8_unorm,
                            true, RHI::ColorSpace::srgb})));
                    luset(cmdbuf, device->new_command_buffer(queue));
                    luset(blit, RHIUtility::new_blit_context(device, swap_chain->get_desc().format));
                    luset(renderer, GUI::new_renderer(device));
                    luset(preview_renderer, GUI::new_renderer(device));
                    gui = GUI::new_context();
                    luexp(gui->register_font("default", Font::get_default_font()));
                    EditorGUI::register_style_schemas(gui);
                    EditorGUI::DefaultStyleDesc style;
                    style.input_mode = EditorGUI::InputMode::pointer;
                    style.color_theme = EditorGUI::ColorTheme::dark;
                    EditorGUI::set_default_style(gui, style);
                    luexp(resize_target(size));
                    width = size.x;
                    height = size.y;
                    lulet(created_service, GameGUIEditor::new_service());
                    service = move(created_service);
                    Variant types;
                    if(!invoke(GameGUIEditor::GET_NODE_TYPES_URL,
                        Variant(VariantType::object), types))
                    {
                        luthrow(E_FAILURE);
                    }
                    for(const Variant& value : types.values())
                    {
                        NodeTypeView type;
                        if(!decode_guid_string(value["type"], type.type)) continue;
                        type.name = value["name"].c_str();
                        type.display_name = value["display_name"].c_str();
                        type.category = value["category"].c_str();
                        if(!decode_editing_schema(value["property_schema"],
                            type.property_schema) ||
                            !decode_editing_schema(value["child_attachment_schema"],
                            type.child_attachment_schema))
                        {
                            luthrow(set_error(E_BAD_DATA,
                                "GameGUIEditor received an invalid editing schema."));
                        }
                        node_types.push_back(move(type));
                    }
                    if(!create_document()) luthrow(E_FAILURE);
                    luexp(initialize_dock_layout());

                    Window::set_event_handler([](object_t event, void* userdata)
                    {
                        EditorApp* app = (EditorApp*)userdata;
#if defined(LUNA_PLATFORM_MACOS)
                        if(auto menu_item = cast_object<Window::ApplicationMenuItemInvokedEvent>(event))
                        {
                            app->handle_application_menu_item(menu_item->item_id);
                        }
                        if(auto quit = cast_object<Window::ApplicationRequestQuitEvent>(event))
                        {
                            quit->do_quit = app->confirm_exit();
                        }
#endif
                        if(auto close = cast_object<Window::WindowRequestCloseEvent>(event))
                        {
                            if(close->window == app->window) close->do_close = app->confirm_exit();
                        }
                    }, this);
                    input_adapter.window = window;
                    input_adapter.gui = gui;
                    GUIWindow::install_window_event_handler(&input_adapter);
#if defined(LUNA_PLATFORM_MACOS)
                    luexp(install_application_menu());
#endif
                }
                lucatchret;
                return ok;
            }
        }
    }
}
