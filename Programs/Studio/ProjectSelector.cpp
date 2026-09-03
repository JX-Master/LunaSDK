/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ProjectSelector.cpp
* @author JXMaster
* @date 2020/4/20
*/
#include "ProjectSelector.hpp"
#include "StudioEnv.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Window/Event.hpp>

namespace Luna
{
    //! Creates project file at the specified directory.
    static R<Path> create_project_dir(const Path& dir_path, const String& project_name, bool should_create_dir)
    {
        if (project_name.size() == 0)
        {
            return set_error(E_BAD_ARGUMENTS, "Project name is empty.");
        }
        Path ret_path;
        lutry
        {
            auto project_path = Path();
            project_path.assign(dir_path);
            if (should_create_dir)
            {
                project_path.push_back(Name(project_name.c_str()));
                luexp(create_dir(project_path.encode().c_str()));
            }

            // Create Data folder.
            project_path.push_back("Data");
            luexp(create_dir(project_path.encode().c_str()));
            project_path.pop_back();
            project_path.push_back(project_name);
            project_path.append_extension("lunaproj");
            lulet(f, open_file(project_path.encode().c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            f.reset();
            project_path.pop_back();
            ret_path = project_path;
        }
        lucatchret;
        return ret_path;
    }

    struct RecentFileRecord
    {
        u64 m_last_use_time;
        Path m_path;
    };

    void read_recents(Vector<RecentFileRecord>& recents)
    {
        lutry
        {
            lulet(f, open_file("RecentProjects.json", FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
            lulet(blob, load_file_data(f));
            lulet(data, VariantUtils::read_json((c8*)blob.data(), blob.size()));
            for (auto& item : data.values())
            {
                RecentFileRecord rec;
                rec.m_path = item["path"].c_str();
                rec.m_last_use_time = item["last_use_time"].unum();
                auto attr = get_file_attribute(item["path"].c_str());
                if (succeeded(attr))
                {
                    recents.push_back(rec);
                }
            }
        }
        lucatch
        {
            return;
        }
    }

    void write_recents(Vector<RecentFileRecord>& recents, const Path& opened)
    {
        auto iter = recents.begin();
        if (!opened.empty())
        {
            bool insert = true;
            while (iter != recents.end())
            {
                if (iter->m_path.equal_to(opened))
                {
                    RecentFileRecord rec = *iter;
                    rec.m_last_use_time = get_utc_timestamp();
                    recents.erase(iter);
                    iter = recents.begin();
                    recents.insert(iter, rec);
                    insert = false;
                    break;
                }
                ++iter;
            }
            if (insert)
            {
                RecentFileRecord rec;
                rec.m_last_use_time = get_utc_timestamp();
                rec.m_path = opened;
                recents.insert(recents.begin(), rec);
            }
        }
        lutry
        {
            Variant var(VariantType::array);
            for (auto& i : recents)
            {
                Variant item(VariantType::object);
                item["path"] = i.m_path.encode();
                item["last_use_time"] = i.m_last_use_time;
                var.push_back(move(item));
            }
            String data = VariantUtils::write_json(var);
            lulet(f, open_file("RecentProjects.json", FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
            luexp(f->write(data.data(), data.size()));
        }
        lucatch
        {
            return;
        }
    }

    constexpr GUI::id_t PROJECT_SELECTOR_LAYER_ID = 1;
    constexpr GUI::id_t PROJECT_SELECTOR_ROOT_ID = 2;
    constexpr GUI::id_t PROJECT_SELECTOR_BACKGROUND_ID = 3;
    constexpr GUI::id_t PROJECT_SELECTOR_NEW_HEADER_ID = 10;
    constexpr GUI::id_t PROJECT_SELECTOR_NAME_INPUT_ID = 11;
    constexpr GUI::id_t PROJECT_SELECTOR_CREATE_DIR_ID = 12;
    constexpr GUI::id_t PROJECT_SELECTOR_CREATE_BUTTON_ID = 13;
    constexpr GUI::id_t PROJECT_SELECTOR_OPEN_HEADER_ID = 20;
    constexpr GUI::id_t PROJECT_SELECTOR_BROWSE_BUTTON_ID = 21;
    constexpr GUI::id_t PROJECT_SELECTOR_RECENT_VIEW_ID = 22;
    constexpr GUI::id_t PROJECT_SELECTOR_RECENT_CLIP_PUSH_ID = 23;
    constexpr GUI::id_t PROJECT_SELECTOR_RECENT_CLIP_POP_ID = 24;
    constexpr GUI::id_t PROJECT_SELECTOR_FIRST_TEXT_ID = 1000;
    constexpr GUI::id_t PROJECT_SELECTOR_FIRST_RECENT_ID = 2000;
    constexpr GUI::id_t PROJECT_SELECTOR_RECENT_STRIDE = 8;

    static GUI::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUI::LayoutConfig layout;
        layout.width.kind = GUI::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUI::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    static void set_element_rect(GUI::IContext* context, const GUI::ElementHandle& element, const RectF& rect)
    {
        GUI::LayoutResult layout;
        layout.rect = rect;
        layout.clip_rect = rect;
        layout.content_size = Float2U(rect.width, rect.height);
        context->set_layout_result(element, layout);
    }

    static bool clicked(GUI::IContext* context, GUI::id_t id)
    {
        return context->get_interaction_state(id).clicked;
    }

    static Float4U style_color(GUI::IContext* context, const c8* entry, const Float4U& fallback)
    {
        return context->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name(entry),
            GUI::style_f32x4(fallback)).number;
    }

    struct ProjectSelectorDrawData
    {
        GUI::DrawCommand command;
    };

    using ProjectSelectorDrawStorage = Vector<UniquePtr<ProjectSelectorDrawData>>;

    static R<GUI::paint_order_id_t> draw_project_selector_command(GUI::IContext* context,
        const GUI::ElementHandle& element, GUI::DrawPhase phase,
        GUI::paint_order_id_t paint_order_id, void* userdata)
    {
        ProjectSelectorDrawData& data = *(ProjectSelectorDrawData*)userdata;
        context->draw(data.command, paint_order_id);
        return paint_order_id;
    }

    static void submit_draw_command(GUI::IContext* context, GUI::id_t id,
        GUI::DrawCommand&& command, ProjectSelectorDrawStorage& draw_data)
    {
        UniquePtr<ProjectSelectorDrawData> data(memnew<ProjectSelectorDrawData>());
        data->command = move(command);
        GUI::ElementHandle element = context->begin_element(id);
        context->set_element_debug_name(element, Name("Project Selector Draw Command"));
        GUI::DrawConfig draw;
        draw.name = Name("studio.project_selector.command");
        draw.callback = draw_project_selector_command;
        draw.userdata = data.get();
        context->set_draw_config(element, draw);
        context->end_element();
        draw_data.push_back(move(data));
    }

    static void draw_label(GUI::IContext* context, GUI::id_t id, const RectF& rect,
        const c8* text, ProjectSelectorDrawStorage& draw_data, f32 font_size = 16.0f)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::text;
        command.rect = rect;
        command.color = style_color(context, "gui.text.color", Float4U(0.15f, 0.16f, 0.17f, 1.0f));
        command.font_size = font_size;
        command.text = text ? text : "";
        submit_draw_command(context, id, move(command), draw_data);
    }

    static void draw_rect(GUI::IContext* context, GUI::id_t id, const RectF& rect,
        const Float4U& color, ProjectSelectorDrawStorage& draw_data, f32 radius = 0.0f)
    {
        GUI::DrawCommand command;
        command.type = radius > 0.0f ? GUI::DrawCommandType::rounded_rect : GUI::DrawCommandType::rect;
        command.rect = rect;
        command.color = color;
        command.radius = radius;
        submit_draw_command(context, id, move(command), draw_data);
    }

    static void push_clip(GUI::IContext* context, GUI::id_t id, const RectF& rect,
        ProjectSelectorDrawStorage& draw_data)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::push_clip;
        command.rect = rect;
        command.rect_reference = GUI::DrawCommandRectReference::layer;
        submit_draw_command(context, id, move(command), draw_data);
    }

    static void pop_clip(GUI::IContext* context, GUI::id_t id,
        ProjectSelectorDrawStorage& draw_data)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::pop_clip;
        submit_draw_command(context, id, move(command), draw_data);
    }

    static GUI::ElementHandle begin_scroll_region(GUI::IContext* context, GUI::id_t id, const RectF& rect)
    {
        GUI::ElementHandle element = context->begin_element(id);
        context->set_layout_config(element, fixed_layout(rect.width, rect.height));
        GUI::Interactable interactable;
        interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
        set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
        set_flags(interactable.flags, GUI::InteractableFlag::scrollable);
        context->set_interactable(element, interactable);
        set_element_rect(context, element, rect);
        return element;
    }

    static GUI::ElementHandle build_project_selector_gui(GUI::IContext* context,
        const Float2U& surface_size, String& project_name, bool& create_dir,
        const Vector<RecentFileRecord>& recents, f32 recent_scroll,
        ProjectSelectorDrawStorage& draw_data)
    {
        GUI::ElementHandle root = context->begin_element(PROJECT_SELECTOR_ROOT_ID);
        set_element_rect(context, root, RectF(0.0f, 0.0f, surface_size.x, surface_size.y));
        draw_rect(context, PROJECT_SELECTOR_BACKGROUND_ID, RectF(0.0f, 0.0f, surface_size.x, surface_size.y),
            style_color(context, "gui.canvas", Float4U(0.92f, 0.93f, 0.92f, 1.0f)), draw_data, 0.0f);

        f32 content_w = max(surface_size.x - 32.0f, 320.0f);
        f32 y = 16.0f;
        draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID,
            RectF(16.0f, y, 360.0f, 30.0f), "Luna Studio Project Selector", draw_data, 20.0f);
        y += 46.0f;

        GUI::ElementHandle new_header;
        EditorGUI::DisclosureDesc header_desc;
        header_desc.default_open = true;
        bool show_new_project = EditorGUI::collapsing_header(context, PROJECT_SELECTOR_NEW_HEADER_ID, "New Project",
            fixed_layout(content_w, 30.0f), header_desc, &new_header);
        set_element_rect(context, new_header, RectF(16.0f, y, content_w, 30.0f));
        y += 40.0f;
        if(show_new_project)
        {
            draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 1,
                RectF(32.0f, y, 160.0f, 30.0f), "Project Name", draw_data);
            GUI::ElementHandle input = EditorGUI::input_text(context, PROJECT_SELECTOR_NAME_INPUT_ID, project_name,
                fixed_layout(max(content_w - 190.0f, 160.0f), 30.0f));
            set_element_rect(context, input, RectF(190.0f, y, max(content_w - 190.0f, 160.0f), 30.0f));
            y += 40.0f;

            GUI::ElementHandle checkbox = EditorGUI::checkbox(context, PROJECT_SELECTOR_CREATE_DIR_ID, "Create Project Folder", &create_dir,
                fixed_layout(260.0f, 30.0f));
            set_element_rect(context, checkbox, RectF(32.0f, y, 260.0f, 30.0f));
            y += 42.0f;

            GUI::ElementHandle create_button = EditorGUI::text_button(context, PROJECT_SELECTOR_CREATE_BUTTON_ID, "Create New Project",
                fixed_layout(190.0f, 32.0f));
            set_element_rect(context, create_button, RectF(32.0f, y, 190.0f, 32.0f));
            y += 46.0f;
        }

        GUI::ElementHandle open_header;
        bool show_open_project = EditorGUI::collapsing_header(context, PROJECT_SELECTOR_OPEN_HEADER_ID, "Open Existing Project",
            fixed_layout(content_w, 30.0f), header_desc, &open_header);
        set_element_rect(context, open_header, RectF(16.0f, y, content_w, 30.0f));
        y += 40.0f;
        if(show_open_project)
        {
            GUI::ElementHandle browse_button = EditorGUI::text_button(context, PROJECT_SELECTOR_BROWSE_BUTTON_ID, "Browse Project File",
                fixed_layout(190.0f, 32.0f));
            set_element_rect(context, browse_button, RectF(32.0f, y, 190.0f, 32.0f));
            y += 48.0f;

            if(recents.empty())
            {
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 2,
                    RectF(32.0f, y, 260.0f, 28.0f), "No recent projects.", draw_data);
            }
            else
            {
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 2,
                    RectF(32.0f, y, 260.0f, 28.0f), "Recent Projects", draw_data);
                y += 34.0f;

                f32 recent_h = max(surface_size.y - y - 24.0f, 120.0f);
                RectF view_rect(32.0f, y, max(content_w - 32.0f, 260.0f), recent_h);
                GUI::ElementHandle scroll = begin_scroll_region(context, PROJECT_SELECTOR_RECENT_VIEW_ID, view_rect);
                draw_rect(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 3, view_rect,
                    style_color(context, "gui.surface.0", Float4U(0.95f, 0.95f, 0.94f, 1.0f)), draw_data, 4.0f);
                push_clip(context, PROJECT_SELECTOR_RECENT_CLIP_PUSH_ID, view_rect, draw_data);

                f32 row_h = 34.0f;
                f32 header_h = 30.0f;
                f32 path_w = max(view_rect.width - 284.0f, 160.0f);
                f32 time_w = 120.0f;
                f32 open_w = 72.0f;
                f32 remove_w = 88.0f;
                f32 x = view_rect.offset_x;
                f32 header_y = view_rect.offset_y - recent_scroll;
                draw_rect(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 4, RectF(x, header_y, view_rect.width, header_h),
                    style_color(context, "gui.surface.2", Float4U(0.93f, 0.93f, 0.92f, 1.0f)), draw_data, 0.0f);
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 5,
                    RectF(x + 8.0f, header_y + 2.0f, path_w - 16.0f, 26.0f), "Project", draw_data);
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 6,
                    RectF(x + path_w + 8.0f, header_y + 2.0f, time_w - 16.0f, 26.0f), "Last Used", draw_data);
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 7,
                    RectF(x + path_w + time_w + 8.0f, header_y + 2.0f, open_w - 16.0f, 26.0f), "Open", draw_data);
                draw_label(context, PROJECT_SELECTOR_FIRST_TEXT_ID + 8,
                    RectF(x + path_w + time_w + open_w + 8.0f, header_y + 2.0f, remove_w - 16.0f, 26.0f),
                    "Remove", draw_data);

                for(usize i = 0; i < recents.size(); ++i)
                {
                    f32 row_y = view_rect.offset_y + header_h + (f32)i * row_h - recent_scroll;
                    if(row_y + row_h < view_rect.offset_y || row_y > view_rect.offset_y + view_rect.height)
                    {
                        continue;
                    }
                    GUI::id_t row_base = PROJECT_SELECTOR_FIRST_RECENT_ID + (GUI::id_t)i * PROJECT_SELECTOR_RECENT_STRIDE;
                    Float4U row_color = (i % 2) ?
                        style_color(context, "gui.surface.1", Float4U(0.97f, 0.97f, 0.96f, 1.0f)) :
                        style_color(context, "gui.surface.0", Float4U(0.95f, 0.95f, 0.94f, 1.0f));
                    draw_rect(context, row_base, RectF(x, row_y, view_rect.width, row_h),
                        row_color, draw_data, 0.0f);
                    DateTime dt = timestamp_to_datetime(utc_timestamp_to_local_timestamp(recents[i].m_last_use_time));
                    String time_text;
                    strprintf(time_text, "%hu/%hu/%hu %02hu:%02hu", dt.year, dt.month, dt.day, dt.hour, dt.minute);
                    draw_label(context, row_base + 1,
                        RectF(x + 8.0f, row_y + 3.0f, path_w - 16.0f, 28.0f),
                        recents[i].m_path.encode().c_str(), draw_data);
                    draw_label(context, row_base + 2,
                        RectF(x + path_w + 8.0f, row_y + 3.0f, time_w - 16.0f, 28.0f),
                        time_text.c_str(), draw_data);
                    GUI::ElementHandle open = EditorGUI::text_button(context, row_base + 3, "Open",
                        fixed_layout(open_w - 8.0f, 28.0f));
                    set_element_rect(context, open,
                        RectF(x + path_w + time_w + 4.0f, row_y + 3.0f, open_w - 8.0f, 28.0f));
                    GUI::ElementHandle remove = EditorGUI::text_button(context, row_base + 4, "Remove",
                        fixed_layout(remove_w - 8.0f, 28.0f));
                    set_element_rect(context, remove,
                        RectF(x + path_w + time_w + open_w + 4.0f, row_y + 3.0f, remove_w - 8.0f, 28.0f));
                }
                pop_clip(context, PROJECT_SELECTOR_RECENT_CLIP_POP_ID, draw_data);
                context->end_element();
            }
        }

        context->end_element();
        return root;
    }

    R<Path> select_project()
    {
        Path path;
        lutry
        {
            lulet(window, Window::new_window("Luna Studio - Open Project", Window::DEFAULT_POS, Window::DEFAULT_POS, 1000, 500));
            lulet(swap_chain, g_env->device->new_swap_chain(g_env->graphics_queue, window,
                RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true, RHI::ColorSpace::srgb})));
            lulet(cmdbuf, g_env->device->new_command_buffer(g_env->graphics_queue));
            Ref<GUI::IContext> gui = GUI::new_context();
            lulet(gui_renderer, GUI::new_renderer(g_env->device));
            EditorGUI::register_style_schemas(gui);
            luexp(gui->register_font(Name("default"), Font::get_default_font()));
            EditorGUI::DefaultStyleDesc style_desc;
            style_desc.input_mode = EditorGUI::InputMode::pointer;
            EditorGUI::set_default_style(gui, style_desc);

            // Create back buffer.
            u32 w = 0, h = 0;

            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = window;
            input_adapter.gui = gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            auto new_solution_name = String();

            Vector<RecentFileRecord> recents;
            read_recents(recents);

            bool create_dir = true;
            f32 recent_scroll = 0.0f;
            ProjectSelectorDrawStorage draw_data;

            while (path.empty())
            {
                Window::poll_events();

#if defined(LUNA_PLATFORM_MACOS)
                if(Window::is_application_quit_requested())
                {
                    break;
                }
#endif
                if (window->is_closed())
                {
                    break;
                }
                if (window->is_minimized())
                {
                    sleep(100);
                    continue;
                }

                // Recreate the back buffer if needed.
                auto fb_sz = window->get_framebuffer_size();
                if (fb_sz.x && fb_sz.y && (fb_sz.x != w || fb_sz.y != h))
                {
                    luexp(swap_chain->reset({fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true}));
                    w = fb_sz.x;
                    h = fb_sz.y;
                }
                auto sz = window->get_size();

                GUI::FrameDesc frame;
                frame.logical_size = Float2U((f32)sz.x, (f32)sz.y);
                frame.render_size = fb_sz;
                frame.delta_time = 1.0f / 60.0f;
                gui->begin_frame(frame);
                draw_data.clear();
                GUIWindow::update_input(&input_adapter);

                gui->push_layer(PROJECT_SELECTOR_LAYER_ID, Float2U(0.0f));
                GUI::ElementHandle root = build_project_selector_gui(gui, frame.logical_size, new_solution_name, create_dir,
                    recents, recent_scroll, draw_data);
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
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(gui->generate_draw_commands());

                if (clicked(gui, PROJECT_SELECTOR_CREATE_BUTTON_ID))
                {
                    auto rpath = Window::open_dir_dialog("Select Project Folder");
                    if (succeeded(rpath))
                    {
                        auto res2 = create_project_dir(rpath.get(), new_solution_name, create_dir);
                        if (succeeded(res2))
                        {
                            path = res2.get();
                        }
                        else
                        {
                            auto _ = Window::message_box(explain(res2.errcode()), "Project Creation Failed", {"OK"}, Window::MessageBoxIcon::error);
                        }
                    }
                }

                if (clicked(gui, PROJECT_SELECTOR_BROWSE_BUTTON_ID))
                {
                    Window::FileDialogFilter filter;
                    filter.name = "Luna Project File";
                    const c8* extension = "lunaproj";
                    filter.extensions = {&extension, 1};
                    auto rpath = Window::open_file_dialog("Select Project File", {&filter, 1});
                    if (succeeded(rpath) && !rpath.get().empty())
                    {
                        path = rpath.get()[0];
                        path.pop_back();
                    }
                }

                usize remove_recent_index = USIZE_MAX;
                for(usize i = 0; i < recents.size(); ++i)
                {
                    GUI::id_t row_base = PROJECT_SELECTOR_FIRST_RECENT_ID + (GUI::id_t)i * PROJECT_SELECTOR_RECENT_STRIDE;
                    if (clicked(gui, row_base + 3))
                    {
                        path = recents[i].m_path;
                    }
                    if (clicked(gui, row_base + 4))
                    {
                        remove_recent_index = i;
                    }
                }
                if(remove_recent_index != USIZE_MAX)
                {
                    recents.erase(recents.begin() + remove_recent_index);
                    write_recents(recents, Path());
                }
                Span<const GUI::RoutedInputEvent> recent_events = gui->get_routed_input_events(PROJECT_SELECTOR_RECENT_VIEW_ID);
                for(const GUI::RoutedInputEvent& routed : recent_events)
                {
                    if(routed.event.type == GUI::InputEventType::pointer_wheel)
                    {
                        recent_scroll = max(recent_scroll - routed.event.wheel_delta.y * 32.0f, 0.0f);
                    }
                }
                f32 visible_rows_height = max((f32)sz.y - 260.0f, 120.0f);
                f32 content_rows_height = 30.0f + (f32)recents.size() * 34.0f;
                recent_scroll = clamp(recent_scroll, 0.0f, max(content_rows_height - visible_rows_height, 0.0f));

                Float4U clear_color = style_color(gui, "gui.canvas",
                    Float4U(0.92f, 0.93f, 0.92f, 1.0f));

                lulet(back_buffer, swap_chain->get_current_back_buffer());
                GUI::RenderTargetDesc target(back_buffer);
                target.color_load_op = RHI::LoadOp::clear;
                target.color_clear_value = clear_color;
                target.color_final_state = RHI::TextureStateFlag::present;
                luexp(gui_renderer->render(gui, cmdbuf, target));
                luexp(cmdbuf->submit({}, {}, true));
                cmdbuf->wait();
                luexp(cmdbuf->reset());
                luexp(swap_chain->present());
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
            if(window->is_text_input_active())
            {
                luexp(window->end_text_input());
            }
            if (path.empty())
            {
                return E_FAILURE;
            }

            // Write to the recents.
            write_recents(recents, path);
        }
        lucatchret;
        return path;
    }
}
