/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <cstdio>

using namespace Luna;

namespace Luna
{
    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUI::IGUIContext> gui;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        u32 selected_tab = 0;
        u32 click_count = 0;
        u32 double_click_count = 0;
        u32 right_click_count = 0;
        bool checkbox_a = true;
        bool checkbox_b = false;
        bool switch_a = true;
        bool switch_b = false;
        bool popup_open = false;
        bool floating_window_open = false;
        Float2U popup_position = Float2U(120.0f, 120.0f);
        String overview_quick_edit_text = "Overview quick edit";
        String widget_input_text = "Widget input";
        String widget_notes_text = "Editable widget notes";
        String layout_column_text = "Layout column input";
        String popup_text = "No popup action";
        String state_text = "Interact with the controls";
        i32 combo_index = 0;
        f32 slider_value = 0.35f;
        f32 drag_value = 42.0f;
        Float2 drag2_value = Float2(1.0f, -2.0f);
        Float3 drag3_value = Float3(0.0f, 1.0f, 2.0f);
        Float4 drag4_value = Float4(0.0f, 1.0f, 2.0f, 3.0f);
        Float3 color_value = Float3(0.20f, 0.55f, 0.90f);
        bool table_checks[4] = { true, false, true, false };
        f32 table_values[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
        bool clip_enabled = true;
        Float2U showcase_size = Float2U(920.0f, 700.0f);
        Float2U showcase_content_size = Float2U(880.0f, 590.0f);
    };

    struct FrameHandles
    {
        GUI::GUIItemHandle tabs[7];
        GUI::GUIItemHandle primary_button;
        GUI::GUIItemHandle double_click_item;
        GUI::GUIItemHandle right_click_item;
        GUI::GUIItemHandle open_window_button;
        GUI::GUIItemHandle popup_action;
        GUI::GUIItemHandle popup_close;
        GUI::GUIItemHandle canvas_hit;
    };

    constexpr const c8* DEMO_TABS[] =
    {
        "Overview",
        "Widgets",
        "Layout",
        "Tables",
        "Drawing",
        "Popups",
        "State"
    };

    void demo_section(const c8* title)
    {
        GUI::Text(title);
    }

    void demo_two_column_label(const c8* label)
    {
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fixed_width(150.0f));
        GUI::Text(label);
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
    }

    void draw_overview_tab(App& app)
    {
        demo_section("Showcase map");
        GUI::Text("This test is a compact interactive catalog for Luna GUI.");
        GUI::Text("Use the tabs above to exercise widgets, layout, tables, drawing, popups, and item state.");
        GUI::Text("The window title is updated from after-submit state queries every frame.");

        GUI::GUILayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;
        GUI::BeginHLayout("Overview Quick Actions", row);
        GUI::Text("Quick edit");
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
        GUI::InputText("Overview Text", app.overview_quick_edit_text);
        GUI::Checkbox("Enabled", &app.checkbox_a);
        GUI::EndHLayout();

        GUI::GUIItemHandle header = GUI::CollapsingHeader("What this page covers");
        if(GUI::GetItemState(header, GUI::GUIState::open()))
        {
            GUI::BeginVLayout("Coverage Details");
            GUI::Text("Immediate-style build API that produces a GUI description tree.");
            GUI::Text("Handle-based state queries before and after submit.");
            GUI::Text("Two-pass layout, tables, popups, absolute paint nodes, and clipping.");
            GUI::EndVLayout();
        }
    }

    void draw_widgets_tab(App& app, FrameHandles& handles)
    {
        demo_section("Basic widgets");
        GUI::GUILayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;

        GUI::BeginHLayout("Buttons", row);
        handles.primary_button = GUI::Button("Count Click");
        handles.double_click_item = GUI::Button("Double Click");
        handles.right_click_item = GUI::Selectable("Right Click Target");
        GUI::EndHLayout();

        c8 counters[160];
        snprintf(counters, 160, "Clicks: %u    Double clicks: %u    Right clicks: %u", app.click_count, app.double_click_count, app.right_click_count);
        GUI::Text(counters);

        GUI::BeginHLayout("Checks", row);
        GUI::Checkbox("Feature A", &app.checkbox_a);
        GUI::Checkbox("Feature B", &app.checkbox_b);
        GUI::EndHLayout();

        GUI::BeginHLayout("Switches", row);
        GUI::Switch("Realtime Preview", &app.switch_a);
        GUI::Switch("Network Sync", &app.switch_b);
        GUI::EndHLayout();

        GUI::BeginHLayout("Text Inputs", row);
        demo_two_column_label("InputText");
        GUI::InputText("InputText", app.widget_input_text);
        GUI::EndHLayout();

        GUI::BeginHLayout("Notes Input", row);
        demo_two_column_label("Notes");
        GUI::InputText("Notes", app.widget_notes_text);
        GUI::EndHLayout();

        const c8* combo_items[] = {"Alpha", "Beta", "Gamma", "Delta"};
        GUI::Combo("Combo cycles on click", &app.combo_index, Span<const c8*>(combo_items, 4));
        GUI::SliderFloat("SliderFloat", &app.slider_value, 0.0f, 1.0f);
        GUI::DragFloat("DragFloat", &app.drag_value, 0.25f, -100.0f, 100.0f);
        GUI::DragFloat2("DragFloat2", app.drag2_value.m, 0.05f, 0.0f, 0.0f);
        GUI::DragFloat3("DragFloat3", app.drag3_value.m, 0.05f, 0.0f, 0.0f);
        GUI::DragFloat4("DragFloat4", app.drag4_value.m, 0.05f, 0.0f, 0.0f);
        GUI::ColorEdit3("ColorEdit3", app.color_value.m);
    }

    void draw_layout_tab(App& app, FrameHandles& handles)
    {
        demo_section("Layout containers");
        GUI::Text("Rows and columns are measured first, then arranged.");

        GUI::GUILayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::stretch;
        GUI::BeginHLayout("Fill Row", row);
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fixed_width(110.0f));
        GUI::Button("Fixed");
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width(1.0f));
        GUI::Button("Fill 1");
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width(2.0f));
        GUI::Button("Fill 2");
        GUI::EndHLayout();

        GUI::GUILayoutDesc columns;
        columns.gap = 10.0f;
        columns.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::stretch;
        GUI::BeginHLayout("Columns", columns);
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
        GUI::BeginVLayout("Left Column");
        GUI::Text("Left column");
        GUI::Button("Action A");
        GUI::Button("Action B");
        GUI::EndVLayout();
        GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
        GUI::BeginVLayout("Right Column");
        GUI::Text("Right column");
        GUI::Checkbox("Check in column", &app.checkbox_a);
        GUI::InputText("Column input", app.layout_column_text);
        GUI::EndVLayout();
        GUI::EndHLayout();

        GUI::Text("ScrollView");
        GUI::BeginScrollView("Nested Scroll", GUI::GUISize::fixed(max(app.showcase_content_size.x - 16.0f, 240.0f), min(max(app.showcase_content_size.y * 0.30f, 120.0f), 220.0f)));
        for(u32 i = 0; i < 16; ++i)
        {
            c8 line[96];
            snprintf(line, 96, "Scrollable row %02u: mouse wheel should move this content.", i + 1);
            GUI::Text(line);
        }
        GUI::EndScrollView();

        handles.open_window_button = GUI::Button("Open Closeable Floating Window");
        if(app.floating_window_open)
        {
            GUI::BeginWindow("Floating Demo", &app.floating_window_open, GUI::GUISize::fixed(280.0f, 150.0f));
            GUI::Text("This is a closeable GUI window.");
            GUI::Checkbox("Window checkbox", &app.checkbox_b);
            GUI::EndWindow();
        }
    }

    void draw_tables_tab(App& app)
    {
        demo_section("TableLayout");
        GUI::Text("Columns are fixed and resizable; rows are hug-sized with alternating backgrounds.");

        GUI::GUITableDesc table;
        table.columns = 4;
        f32 table_width = max(app.showcase_content_size.x - 24.0f, 360.0f);
        f32 table_column = max((table_width - 180.0f) / 3.0f, 96.0f);
        table.column_sizes = {
            GUI::GUITableTrackSize::fixed(table_column),
            GUI::GUITableTrackSize::fixed(table_column),
            GUI::GUITableTrackSize::fixed(table_column),
            GUI::GUITableTrackSize::fixed(180.0f)
        };
        table.style.border_size = 1.0f;
        table.style.background_mode = GUI::GUITableBackgroundMode::alternate_rows;
        table.style.background_color = Float4U(0.10f, 0.12f, 0.15f, 0.92f);
        table.style.alternate_background_color = Float4U(0.14f, 0.16f, 0.20f, 0.92f);
        table.style.row_separators = true;
        table.style.column_separators = true;
        table.style.resize_fixed_columns = true;
        table.style.separator_size = 1.0f;
        GUI::BeginTableLayout("Table Showcase", table);
        GUI::Text("Name");
        GUI::Text("Enabled");
        GUI::Text("Value");
        GUI::Text("Action");
        for(u32 i = 0; i < 4; ++i)
        {
            c8 label[64];
            snprintf(label, 64, "Row %u", i + 1);
            GUI::Text(label);
            GUI::Checkbox("Enabled", &app.table_checks[i]);
            GUI::SliderFloat("Value", &app.table_values[i], 0.0f, 1.0f);
            GUI::Button("Run");
        }
        GUI::EndTableLayout();
    }

    void draw_drawing_tab(App& app, FrameHandles& handles)
    {
        demo_section("DrawList and absolute paint nodes");
        GUI::Text("The canvas below reserves layout space, then absolute draw commands paint into its last-frame rect.");
        f32 canvas_width = max(app.showcase_content_size.x - 16.0f, 260.0f);
        f32 canvas_height = min(max(app.showcase_content_size.y * 0.42f, 180.0f), 280.0f);
        GUI::GUIItemHandle canvas = GUI::Image(nullptr, GUI::GUISize::fixed(canvas_width, canvas_height));
        RectF rect = GUI::GetItemState(canvas, GUI::GUIState::rect());
        if(rect.width > 1.0f && rect.height > 1.0f)
        {
            GUI::PushClipRect(rect);
            GUI::DrawRect(rect, Float4U(0.06f, 0.08f, 0.10f, 1.0f), 6.0f);
            GUI::DrawRect(RectF(rect.offset_x + 20.0f, rect.offset_y + 20.0f, 180.0f, 76.0f), Float4U(0.22f, 0.34f, 0.55f, 1.0f), 8.0f);
            GUI::DrawCircle(Float2U(rect.offset_x + 290.0f, rect.offset_y + 58.0f), 38.0f, Float4U(app.color_value.x, app.color_value.y, app.color_value.z, 1.0f));
            GUI::DrawLine(Float2U(rect.offset_x + 360.0f, rect.offset_y + 30.0f), Float2U(rect.offset_x + 560.0f, rect.offset_y + 120.0f), Float4U(0.9f, 0.8f, 0.3f, 1.0f), 5.0f);
            GUI::DrawText(RectF(rect.offset_x + 24.0f, rect.offset_y + 118.0f, 420.0f, 32.0f), "DrawText: clipped to the canvas", Color::white(), 16.0f);
            handles.canvas_hit = GUI::HitBox("Canvas HitBox", RectF(rect.offset_x + 20.0f, rect.offset_y + 160.0f, 240.0f, 52.0f));
            GUI::DrawRect(RectF(rect.offset_x + 20.0f, rect.offset_y + 160.0f, 240.0f, 52.0f),
                GUI::IsItemHovered(handles.canvas_hit) ? Float4U(0.30f, 0.48f, 0.74f, 1.0f) : Float4U(0.16f, 0.24f, 0.34f, 1.0f), 6.0f);
            GUI::DrawText(RectF(rect.offset_x + 28.0f, rect.offset_y + 160.0f, 224.0f, 52.0f), "Absolute HitBox", Color::white(), 16.0f, GUI::GUITextAlignment::center);
            if(app.clip_enabled)
            {
                GUI::DrawCircle(Float2U(rect.offset_x + rect.width + 20.0f, rect.offset_y + 40.0f), 48.0f, Float4U(1.0f, 0.2f, 0.2f, 0.8f));
            }
            GUI::PopClipRect();
        }
        GUI::Checkbox("Draw an intentionally clipped red circle", &app.clip_enabled);
    }

    void draw_popups_tab(App& app, FrameHandles& handles)
    {
        demo_section("Popups and context menus");
        GUI::Text("Right-click the selectable below, or click the button to open a popup.");
        handles.right_click_item = GUI::Selectable("Right click me", app.popup_open);
        handles.primary_button = GUI::Button("Open Popup");
        GUI::Text(app.popup_text.c_str());
    }

    void draw_state_tab(App& app, FrameHandles& handles)
    {
        demo_section("State queries");
        GUI::Text("Widget APIs return handles. Query before submit for last-frame state and after submit for current-frame state.");
        handles.primary_button = GUI::Button("Inspect Me");
        bool hovered = GUI::IsItemHovered(handles.primary_button);
        bool active = GUI::IsItemActive(handles.primary_button);
        bool focused = GUI::IsItemFocused(handles.primary_button);
        c8 state[192];
        snprintf(state, 192, "Before submit query: hovered=%s active=%s focused=%s",
            hovered ? "true" : "false",
            active ? "true" : "false",
            focused ? "true" : "false");
        GUI::Text(state);
        GUI::Text(app.state_text.c_str());
    }

    void draw_showcase(App& app, FrameHandles& handles, const Float2U& surface_size)
    {
        app.showcase_size = Float2U(max(surface_size.x, 1.0f), max(surface_size.y, 1.0f));
        app.showcase_content_size = Float2U(max(app.showcase_size.x - 32.0f, 240.0f), max(app.showcase_size.y - 92.0f, 140.0f));
        GUI::BeginWindow("Luna GUI Showcase", GUI::GUISize::fixed(app.showcase_size.x, app.showcase_size.y));
        GUI::Text("Luna GUI Showcase");

        GUI::GUILayoutDesc tabs_layout;
        tabs_layout.gap = 4.0f;
        tabs_layout.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;
        GUI::BeginHLayout("Tab Bar", tabs_layout);
        f32 tab_width = clamp((app.showcase_content_size.x - tabs_layout.gap * 6.0f) / 7.0f, 44.0f, 112.0f);
        for(u32 i = 0; i < 7; ++i)
        {
            GUI::SetNextItemLayout(GUI::GUILayoutStyle::fixed_width(tab_width));
            handles.tabs[i] = GUI::Selectable(DEMO_TABS[i], app.selected_tab == i);
        }
        GUI::EndHLayout();

        GUI::BeginScrollView("Showcase Content", GUI::GUISize::fixed(app.showcase_content_size.x, app.showcase_content_size.y));
        GUI::PushID(app.selected_tab);
        switch(app.selected_tab)
        {
        case 0:
            draw_overview_tab(app);
            break;
        case 1:
            draw_widgets_tab(app, handles);
            break;
        case 2:
            draw_layout_tab(app, handles);
            break;
        case 3:
            draw_tables_tab(app);
            break;
        case 4:
            draw_drawing_tab(app, handles);
            break;
        case 5:
            draw_popups_tab(app, handles);
            break;
        case 6:
            draw_state_tab(app, handles);
            break;
        default:
            break;
        }
        GUI::PopID();
        GUI::EndScrollView();
        GUI::EndWindow();
    }

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg(), GUI::module_gui(), GUIWindow::module_gui_window()}));
            luexp(init_modules());
            set_log_to_platform_enabled(true);
            using namespace RHI;

            App app;
            luset(app.window, Window::new_window("Luna GUI Test"));

            Ref<IDevice> dev = get_main_device();
            u32 num_queues = dev->get_num_command_queues();
            for(u32 i = 0; i < num_queues; ++i)
            {
                auto desc = dev->get_command_queue_desc(i);
                if(desc.type == RHI::CommandQueueType::graphics)
                {
                    app.queue = i;
                    break;
                }
            }
            luset(app.swap_chain, dev->new_swap_chain(app.queue, app.window, SwapChainDesc({0, 0, 2, Format::bgra8_unorm, true})));
            luset(app.cmdbuf, dev->new_command_buffer(app.queue));
            app.gui = GUI::new_context(dev);
            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed()) break;
                if(app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }

                auto fb_sz = app.window->get_framebuffer_size();
                if(fb_sz.x && fb_sz.y && (fb_sz.x != app.width || fb_sz.y != app.height))
                {
                    luexp(app.swap_chain->reset({fb_sz.x, fb_sz.y, 2, Format::unknown, true}));
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }

                auto logical_sz = app.window->get_size();
                GUI::GUIFrameDesc frame;
                frame.surface_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);

                FrameHandles handles;
                u32 built_tab = app.selected_tab;
                draw_showcase(app, handles, frame.surface_size);
                if(app.popup_open)
                {
                    GUI::BeginPopup("Popup Test", app.popup_position, GUI::GUISize::fixed(180.0f, 72.0f));
                    handles.popup_action = GUI::Selectable("Popup action");
                    handles.popup_close = GUI::Selectable("Close");
                    GUI::EndPopup();
                }

                lulet(desc, app.gui->end_build());
                luexp(app.gui->submit(desc));
                luexp(GUIWindow::update_text_input(&input_adapter));

                for(u32 i = 0; i < 7; ++i)
                {
                    if(GUI::IsItemClicked(handles.tabs[i]))
                    {
                        app.selected_tab = i;
                    }
                }

                if(GUI::IsItemClicked(handles.popup_action))
                {
                    app.popup_text = "Popup action clicked";
                    app.popup_open = false;
                }
                if(GUI::IsItemClicked(handles.popup_close))
                {
                    app.popup_text = "Popup closed";
                    app.popup_open = false;
                }

                if(built_tab == 1)
                {
                    if(GUI::IsItemClicked(handles.primary_button))
                    {
                        ++app.click_count;
                    }
                    if(GUI::IsItemDoubleClicked(handles.double_click_item))
                    {
                        ++app.double_click_count;
                    }
                    if(GUI::IsItemRightClicked(handles.right_click_item))
                    {
                        ++app.right_click_count;
                        app.popup_open = true;
                        app.popup_position = GUI::GetPointerPosition();
                        app.popup_text = "Widget context popup";
                    }
                }
                else if(built_tab == 2)
                {
                    if(GUI::IsItemClicked(handles.open_window_button))
                    {
                        app.floating_window_open = true;
                    }
                }
                else if(built_tab == 4)
                {
                    if(GUI::IsItemClicked(handles.canvas_hit))
                    {
                        app.state_text = "Canvas HitBox clicked";
                    }
                }
                else if(built_tab == 5)
                {
                    if(GUI::IsItemClicked(handles.primary_button))
                    {
                        app.popup_open = true;
                        app.popup_position = GUI::GetPointerPosition();
                        app.popup_text = "Popup opened by button";
                    }
                    if(GUI::IsItemRightClicked(handles.right_click_item))
                    {
                        app.popup_open = true;
                        app.popup_position = GUI::GetPointerPosition();
                        app.popup_text = "Popup opened by right click";
                    }
                }
                else if(built_tab == 6)
                {
                    bool hovered = GUI::IsItemHovered(handles.primary_button);
                    bool active = GUI::IsItemActive(handles.primary_button);
                    bool focused = GUI::IsItemFocused(handles.primary_button);
                    c8 state[192];
                    snprintf(state, 192, "After submit query: hovered=%s active=%s focused=%s clicked=%s",
                        hovered ? "true" : "false",
                        active ? "true" : "false",
                        focused ? "true" : "false",
                        GUI::IsItemClicked(handles.primary_button) ? "true" : "false");
                    app.state_text = state;
                }

                c8 buf[128];
                snprintf(buf, 128, "Luna GUI Showcase - %s", DEMO_TABS[app.selected_tab]);
                luexp(app.window->set_title(buf));

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                RenderPassDesc render_pass;
                render_pass.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, Float4U(0.02f, 0.025f, 0.03f, 1.0f));
                app.cmdbuf->begin_render_pass(render_pass);
                app.cmdbuf->end_render_pass();
                luexp(app.gui->render(app.cmdbuf, back_buffer));
                app.cmdbuf->resource_barrier({}, {
                    {back_buffer, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::present, ResourceBarrierFlag::none}
                    });
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    auto r = Luna::run_app();
    if(failed(r))
    {
        Luna::log_error("GUITest", "%s", Luna::explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
