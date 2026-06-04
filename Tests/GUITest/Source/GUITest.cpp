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
#include <Luna/VG/Shapes.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <cstdio>
#include <cstring>

using namespace Luna;

namespace Luna
{
    constexpr u32 MAX_DEMO_TABS = 16;

    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUI::IContext> gui;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        u32 selected_tab = 0;
        u32 tree_selected = 2;
        u32 click_count = 0;
        u32 double_click_count = 0;
        u32 right_click_count = 0;
        bool checkbox_a = true;
        bool checkbox_b = false;
        bool switch_a = true;
        bool switch_b = false;
        bool radio_manual = false;
        bool menu_show_grid = true;
        bool menu_snap_to_grid = false;
        bool dock_panel_a_open = true;
        bool dock_panel_b_open = true;
        bool tab_document_open[4] = { true, true, true, true };
        bool floating_window_open = false;
        i32 style_theme = 0;
        bool style_preview_checkbox = true;
        bool style_preview_switch = true;
        bool style_preview_bool = false;
        Float2U popup_position = Float2U(120.0f, 120.0f);
        String overview_quick_edit_text = "Overview quick edit";
        String widget_input_text = "Widget input";
        String widget_notes_text = "Editable widget notes";
        String layout_column_text = "Layout column input";
        String style_preview_text = "Theme preview input";
        String popup_text = "No popup action";
        String state_text = "Interact with the controls";
        String dropped_text = "Drop text payload here";
        String mixed_drop_text = "Drop either payload type here";
        i32 dropped_number = -1;
        i32 radio_choice = 1;
        i32 button_group_choice = 0;
        i32 style_preview_choice = 1;
        i32 style_preview_group = 0;
        i32 combo_index = 0;
        i32 slider_int_value = 4;
        i32 slider_int3_value[3] = { 2, 5, 8 };
        i32 slider_int_with_input_value = 64;
        i32 drag_int_value = 16;
        i32 drag_int2_value[2] = { -8, 8 };
        i32 drag_int3_value[3] = { -4, 0, 4 };
        i32 drag_int4_value[4] = { -6, -2, 2, 6 };
        f32 slider_value = 0.35f;
        Float3 slider3_value = Float3(0.15f, 0.45f, 0.75f);
        f32 slider_with_input_value = 0.72f;
        f32 style_preview_slider = 0.48f;
        f32 style_preview_drag = 32.0f;
        f32 progress_value = 0.62f;
        f32 drag_value = 42.0f;
        Float2 drag2_value = Float2(1.0f, -2.0f);
        Float3 drag3_value = Float3(0.0f, 1.0f, 2.0f);
        Float4 drag4_value = Float4(0.0f, 1.0f, 2.0f, 3.0f);
        Float3 color_value = Float3(0.20f, 0.55f, 0.90f);
        Float4 color4_value = Float4(0.85f, 0.36f, 0.18f, 0.72f);
        u8 color_u8_value[3] = { 48, 190, 126 };
        u8 color4_u8_value[4] = { 210, 80, 170, 200 };
        u32 color_rgba8_value = 0xff3366ccu;
        u32 color4_rgba8_value = 0xcc20c0ffu;
        bool table_checks[4] = { true, false, true, false };
        bool button_group_multi[3] = { true, false, true };
        f32 table_values[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
        bool clip_enabled = true;
        Float2U showcase_size = Float2U(920.0f, 700.0f);
        Float2U showcase_content_size = Float2U(880.0f, 590.0f);
#ifdef LUNA_GUI_ENABLE_DEBUG
        GUI::DebugInfo debug_info;
        bool has_debug_info = false;
        bool show_debug_panel = false;
#endif
    };

    struct FrameHandles
    {
        GUI::ItemHandle tabs[MAX_DEMO_TABS];
        GUI::ItemHandle tree_nodes[8];
        GUI::ItemHandle primary_button;
        GUI::ItemHandle double_click_item;
        GUI::ItemHandle right_click_item;
        GUI::ItemHandle open_window_button;
        GUI::ItemHandle managed_popup_button;
        GUI::ItemHandle managed_popup;
        GUI::ItemHandle managed_popup_action;
        GUI::ItemHandle nested_popup_button;
        GUI::ItemHandle nested_popup;
        GUI::ItemHandle nested_popup_close;
        GUI::ItemHandle menu_new;
        GUI::ItemHandle menu_save;
        GUI::ItemHandle menu_show_grid;
        GUI::ItemHandle menu_theme_dark;
        GUI::ItemHandle canvas_hit;
        GUI::ItemHandle drag_number_target;
        GUI::ItemHandle drag_text_target;
        GUI::ItemHandle drag_mixed_target;
    };

    GUI::RenderProxyDesc demo_custom_node_render_proxy();

    struct DemoCustomNode : GUI::Node
    {
        lustruct("GUITest::DemoCustomNode", "{A7A8030D-AAD4-4374-B967-74AF3DAD0A4D}");

        DemoCustomNode()
        {
            render_proxy = demo_custom_node_render_proxy();
        }

        virtual Guid type_guid() const override
        {
            return __guid;
        }

        virtual Ref<GUI::Node> clone() const override
        {
            return new_object<DemoCustomNode>(*this);
        }

        virtual GUI::LayoutMetrics measure() const override
        {
            GUI::LayoutMetrics metrics;
            metrics.min_size = Float2U(160.0f, 34.0f);
            metrics.preferred_size = Float2U(260.0f, 38.0f);
            metrics.max_size = Float2U(F32_MAX, 38.0f);
            return metrics;
        }
    };

    void draw_demo_custom_node(GUI::NodeRenderContext& ctx, const GUI::Node&, const RectF& rect, const RectF& clip_rect,
        const GUI::NodeRenderState& state, void*)
    {
        RectF r(rect.offset_x, state.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
        RectF c(clip_rect.offset_x, state.surface_size.y - clip_rect.offset_y - clip_rect.height, clip_rect.width, clip_rect.height);
        GUI::IDrawList* draw_list = ctx.draw_list();
        GUI::DrawListState draw_state = draw_list->get_state();
        draw_state.shape_buffer = draw_list->get_shape_buffer();
        draw_state.texture = nullptr;
        draw_state.clip_rect = c;
        u32 pop_id = draw_list->push_state(&draw_state);
        Vector<f32>& points = draw_list->get_shape_buffer()->get_shape_points(true);
        u32 begin = (u32)points.size();
        VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height, 6.0f);
        u32 end = (u32)points.size();
        Float4U color = state.active ? Float4U(0.22f, 0.38f, 0.64f, 1.0f) :
            (state.hovered ? Float4U(0.24f, 0.34f, 0.50f, 1.0f) : Float4U(0.14f, 0.20f, 0.30f, 1.0f));
        draw_list->add_shape(begin, end - begin,
            Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
            Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
            color);
        draw_list->pop_state(pop_id);
    }

    GUI::RenderProxyDesc demo_custom_node_render_proxy()
    {
        GUI::RenderProxyDesc desc;
        desc.draw = draw_demo_custom_node;
        return desc;
    }

    void draw_demo_proxy_button(GUI::NodeRenderContext& ctx, const GUI::Node& node, const RectF& rect, const RectF& clip_rect,
        const GUI::NodeRenderState& state, void*)
    {
        Float4U base = GUI::style_f32x4(ctx, node, Name("demo.proxy.background"), Float4U(0.10f, 0.15f, 0.20f, 1.0f));
        Float4U accent = GUI::style_f32x4(ctx, node, Name("demo.proxy.accent"), Float4U(0.30f, 0.66f, 0.92f, 1.0f));
        Float4U color = state.active ? Float4U(accent.x * 0.75f, accent.y * 0.75f, accent.z * 0.75f, accent.w) :
            (state.hovered ? Float4U(base.x + 0.06f, base.y + 0.08f, base.z + 0.10f, base.w) : base);
        ctx.draw_rect(rect, clip_rect, color, 6.0f);
        ctx.draw_line(Float2U(rect.offset_x + 8.0f, rect.offset_y + rect.height - 5.0f),
            Float2U(rect.offset_x + max(rect.width - 8.0f, 8.0f), rect.offset_y + rect.height - 5.0f),
            clip_rect, accent, state.active ? 4.0f : 2.0f);
        ctx.draw_text(RectF(rect.offset_x + 10.0f, rect.offset_y, max(rect.width - 20.0f, 1.0f), rect.height),
            clip_rect, node.text.c_str(), 16.0f, Color::white(), GUI::TextAlignment::center);
    }

    GUI::RenderProxyDesc demo_button_render_proxy()
    {
        GUI::RenderProxyDesc desc;
        desc.draw = draw_demo_proxy_button;
        return desc;
    }

    constexpr const c8* DEMO_TABS[] =
    {
        "Overview",
        "Widgets",
        "Layout",
        "Tables",
        "Drawing",
        "Tooltips",
        "Popups",
        "State",
        "Style",
        "Trees",
        "Tabs",
        "DragDrop",
        "Debug"
    };

    constexpr u32 DEMO_TAB_COUNT = (u32)(sizeof(DEMO_TABS) / sizeof(DEMO_TABS[0]));
    static_assert(DEMO_TAB_COUNT <= MAX_DEMO_TABS);
    enum DemoTab : u32
    {
        DEMO_TAB_OVERVIEW,
        DEMO_TAB_WIDGETS,
        DEMO_TAB_LAYOUT,
        DEMO_TAB_TABLES,
        DEMO_TAB_DRAWING,
        DEMO_TAB_TOOLTIPS,
        DEMO_TAB_POPUPS,
        DEMO_TAB_STATE,
        DEMO_TAB_STYLE,
        DEMO_TAB_TREES,
        DEMO_TAB_TABS,
        DEMO_TAB_DRAG_DROP,
        DEMO_TAB_DEBUG
    };
    static_assert((u32)DEMO_TAB_DEBUG + 1 == DEMO_TAB_COUNT);
    constexpr u32 TREE_NODE_COUNT = 8;

    void demo_section(App& app, const c8* title)
    {
        GUI::text(app.gui, title);
    }

    void demo_two_column_label(App& app, const c8* label)
    {
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(150.0f));
        GUI::text(app.gui, label);
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
    }

    GUI::ItemHandle demo_tree_node(App& app, FrameHandles& handles, u32 index, const c8* label, GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::none)
    {
        if(app.tree_selected == index)
        {
            flags |= GUI::TreeNodeFlag::selected;
        }
        handles.tree_nodes[index] = GUI::tree_node(app.gui, label, flags);
        return handles.tree_nodes[index];
    }

    Name demo_number_payload_type()
    {
        return Name("demo.number");
    }

    Name demo_text_payload_type()
    {
        return Name("demo.text");
    }

    void demo_number_drag_source(App& app, const c8* label, i32 value)
    {
        GUI::ItemHandle source = GUI::selectable(app.gui, label);
        Name type = demo_number_payload_type();
        if(GUI::begin_drag_drop_source(app.gui, source, type))
        {
            GUI::set_drag_drop_payload(app.gui, &value, sizeof(value));
            c8 preview[96];
            snprintf(preview, 96, "Dragging number %d", value);
            GUI::text(app.gui, preview);
            GUI::end_drag_drop_source(app.gui);
        }
    }

    void demo_text_drag_source(App& app, const c8* label, const c8* value)
    {
        GUI::ItemHandle source = GUI::selectable(app.gui, label);
        Name type = demo_text_payload_type();
        if(GUI::begin_drag_drop_source(app.gui, source, type))
        {
            GUI::set_drag_drop_payload(app.gui, value, strlen(value) + 1);
            c8 preview[128];
            snprintf(preview, 128, "Dragging text: %s", value);
            GUI::text(app.gui, preview);
            GUI::end_drag_drop_source(app.gui);
        }
    }

    void draw_overview_tab(App& app)
    {
        demo_section(app, "Showcase map");
        GUI::text(app.gui, "This test is a compact interactive catalog for Luna GUI.");
        GUI::text(app.gui, "Use the tabs above to exercise widgets, layout, tables, drawing, popups, and item state.");
        GUI::text(app.gui, "The window title is updated from after-submit state queries every frame.");

        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;
        GUI::begin_h_layout(app.gui, "Overview Quick Actions", row);
        GUI::text(app.gui, "Quick edit");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
        GUI::input_text(app.gui, "Overview Text", app.overview_quick_edit_text);
        GUI::checkbox(app.gui, "Enabled", &app.checkbox_a);
        GUI::end_h_layout(app.gui);

        GUI::ItemHandle header = GUI::collapsing_header(app.gui, "What this page covers");
        if(GUI::get_item_state(header, GUI::State::open()))
        {
            GUI::begin_v_layout(app.gui, "Coverage Details");
            GUI::text(app.gui, "Immediate-style build API that produces a GUI description tree.");
            GUI::text(app.gui, "Handle-based state queries before and after submit.");
            GUI::text(app.gui, "Two-pass layout, tables, popups, absolute paint nodes, and clipping.");
            GUI::end_v_layout(app.gui);
        }

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
        Ref<DemoCustomNode> custom = new_object<DemoCustomNode>();
        GUI::ItemHandle custom_handle = GUI::custom_node(app.gui, Ref<GUI::Node>(custom), "RTTI custom node", true);
        if(GUI::is_item_hovered(custom_handle))
        {
            GUI::text(app.gui, "The filled strip above is a user-defined Node registered outside the GUI module.");
        }
    }

    void draw_widgets_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "Basic widgets");
        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;

        GUI::begin_h_layout(app.gui, "Buttons", row);
        handles.primary_button = GUI::button(app.gui, "Count Click");
        handles.double_click_item = GUI::button(app.gui, "Double Click");
        handles.right_click_item = GUI::selectable(app.gui, "Right Click Target");
        GUI::end_h_layout(app.gui);

        c8 counters[160];
        snprintf(counters, 160, "Clicks: %u    Double clicks: %u    Right clicks: %u", app.click_count, app.double_click_count, app.right_click_count);
        GUI::text(app.gui, counters);

        GUI::begin_h_layout(app.gui, "Checks", row);
        GUI::checkbox(app.gui, "Feature A", &app.checkbox_a);
        GUI::checkbox(app.gui, "Feature B", &app.checkbox_b);
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Switches", row);
        GUI::toggle_switch(app.gui, "Realtime Preview", &app.switch_a);
        GUI::toggle_switch(app.gui, "Network Sync", &app.switch_b);
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Radio Buttons", row);
        GUI::radio_button(app.gui, "Low", &app.radio_choice, 0);
        GUI::radio_button(app.gui, "Medium", &app.radio_choice, 1);
        GUI::radio_button(app.gui, "High", &app.radio_choice, 2);
        GUI::radio_button(app.gui, "Manual Bool", &app.radio_manual);
        GUI::end_h_layout(app.gui);

        const c8* group_items[] = {"Title", "Settings", "Preview"};
        GUI::begin_h_layout(app.gui, "Button Group Single", row);
        demo_two_column_label(app, "ButtonGroup single");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(360.0f));
        GUI::button_group(app.gui, "Single Button Group", &app.button_group_choice, Span<const c8*>(group_items, 3));
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Button Group Multi", row);
        demo_two_column_label(app, "ButtonGroup multi");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(360.0f));
        GUI::button_group(app.gui, "Multi Button Group", Span<bool>(app.button_group_multi, 3), Span<const c8*>(group_items, 3));
        GUI::end_h_layout(app.gui);

        GUI::push_enabled(app.gui, false);
        GUI::begin_h_layout(app.gui, "Disabled Controls", row);
        GUI::button(app.gui, "Disabled Button");
        GUI::checkbox(app.gui, "Disabled Check", &app.checkbox_a);
        GUI::toggle_switch(app.gui, "Disabled Switch", &app.switch_a);
        GUI::radio_button(app.gui, "Disabled Radio", &app.radio_choice, 2);
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(280.0f));
        GUI::button_group(app.gui, "Disabled Button Group", &app.button_group_choice, Span<const c8*>(group_items, 3));
        GUI::selectable(app.gui, "Disabled Selectable", true);
        GUI::end_h_layout(app.gui);
        GUI::pop_enabled(app.gui);

        GUI::begin_h_layout(app.gui, "Progress Default", row);
        demo_two_column_label(app, "ProgressBar");
        GUI::progress_bar(app.gui, "ProgressBar", app.progress_value);
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Progress Custom", row);
        demo_two_column_label(app, "HP bar");
        GUI::progress_bar(app.gui, "HP ProgressBar", 0.76f, GUI::Size::fixed(320.0f, 24.0f), "HP 76 / 100");
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Text Inputs", row);
        demo_two_column_label(app, "InputText");
        GUI::input_text(app.gui, "InputText", app.widget_input_text);
        GUI::end_h_layout(app.gui);

        GUI::begin_h_layout(app.gui, "Notes Input", row);
        demo_two_column_label(app, "Notes");
        GUI::input_text(app.gui, "Notes", app.widget_notes_text);
        GUI::end_h_layout(app.gui);

        const c8* combo_items[] = {"Alpha", "Beta", "Gamma", "Delta"};
        GUI::combo(app.gui, "Combo dropdown", &app.combo_index, Span<const c8*>(combo_items, 4));
        GUI::slider_float(app.gui, "SliderFloat", &app.slider_value, 0.0f, 1.0f);
        GUI::slider_float3(app.gui, "SliderFloat3", app.slider3_value.m, 0.0f, 1.0f);
        GUI::slider_int(app.gui, "SliderInt", &app.slider_int_value, 0, 10);
        GUI::slider_int3(app.gui, "SliderInt3", app.slider_int3_value, 0, 10);
        GUI::slider_float_with_input(app.gui, "SliderFloatWithInput", &app.slider_with_input_value, 0.0f, 1.0f);
        GUI::slider_int_with_input(app.gui, "SliderIntWithInput", &app.slider_int_with_input_value, 0, 100);
        GUI::drag_float(app.gui, "DragFloat", &app.drag_value, 0.25f, -100.0f, 100.0f, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_float2(app.gui, "DragFloat2", app.drag2_value.m, 0.05f, 0.0f, 0.0f, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_float3(app.gui, "DragFloat3", app.drag3_value.m, 0.05f, 0.0f, 0.0f, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_float4(app.gui, "DragFloat4", app.drag4_value.m, 0.05f, 0.0f, 0.0f, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_int(app.gui, "DragInt", &app.drag_int_value, 1.0f, -100, 100, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_int2(app.gui, "DragInt2", app.drag_int2_value, 1.0f, -100, 100, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_int3(app.gui, "DragInt3", app.drag_int3_value, 1.0f, -100, 100, GUI::NumericEditFlag::input_on_double_click);
        GUI::drag_int4(app.gui, "DragInt4", app.drag_int4_value, 1.0f, -100, 100, GUI::NumericEditFlag::input_on_double_click);
        GUI::color_edit3(app.gui, "ColorEdit3", app.color_value.m);
        GUI::color_edit4(app.gui, "ColorEdit4", app.color4_value.m);
        GUI::color_edit3(app.gui, "ColorEdit3 u8", app.color_u8_value);
        GUI::color_edit4(app.gui, "ColorEdit4 u8", app.color4_u8_value);
        GUI::color_edit3(app.gui, "ColorEdit3 RGBA8", &app.color_rgba8_value);
        GUI::color_edit4(app.gui, "ColorEdit4 RGBA8", &app.color4_rgba8_value);
    }

    void draw_layout_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "Layout containers");
        GUI::text(app.gui, "Rows and columns are measured first, then arranged.");

        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
        GUI::begin_h_layout(app.gui, "Fill Row", row);
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(110.0f));
        GUI::button(app.gui, "Fixed");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width(1.0f));
        GUI::button(app.gui, "Fill 1");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width(2.0f));
        GUI::button(app.gui, "Fill 2");
        GUI::end_h_layout(app.gui);

        GUI::LayoutDesc columns;
        columns.gap = 10.0f;
        columns.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
        GUI::begin_h_layout(app.gui, "Columns", columns);
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
        GUI::begin_v_layout(app.gui, "Left Column");
        GUI::text(app.gui, "Left column");
        GUI::button(app.gui, "Action A");
        GUI::button(app.gui, "Action B");
        GUI::end_v_layout(app.gui);
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
        GUI::begin_v_layout(app.gui, "Right Column");
        GUI::text(app.gui, "Right column");
        GUI::checkbox(app.gui, "Check in column", &app.checkbox_a);
        GUI::input_text(app.gui, "Column input", app.layout_column_text);
        GUI::end_v_layout(app.gui);
        GUI::end_h_layout(app.gui);

        GUI::text(app.gui, "GridLayout");
        GUI::begin_scroll_view(app.gui, "Grid Scroll", GUI::Size::fixed(max(app.showcase_content_size.x - 16.0f, 260.0f), 190.0f));
        GUI::GridLayoutDesc fixed_cell_grid;
        fixed_cell_grid.sizing_mode = GUI::GridSizingMode::fixed_cell_size;
        fixed_cell_grid.cell_size = Float2U(112.0f, 58.0f);
        fixed_cell_grid.padding = GUI::EdgeInsets::all(8.0f);
        fixed_cell_grid.gap = Float2U(8.0f, 8.0f);
        GUI::begin_grid_layout(app.gui, "Fixed Cell Grid", fixed_cell_grid);
        for(u32 i = 0; i < 18; ++i)
        {
            c8 label[32];
            snprintf(label, 32, "Asset %02u", i + 1);
            GUI::push_id(app.gui, i);
            GUI::button(app.gui, label);
            GUI::pop_id(app.gui);
        }
        GUI::end_grid_layout(app.gui);

        GUI::GridLayoutDesc column_grid;
        column_grid.sizing_mode = GUI::GridSizingMode::fixed_columns;
        column_grid.columns = 4;
        column_grid.padding = GUI::EdgeInsets::xy(8.0f, 10.0f);
        column_grid.gap = Float2U(8.0f, 8.0f);
        GUI::begin_grid_layout(app.gui, "Four Column Grid", column_grid);
        for(u32 i = 0; i < 8; ++i)
        {
            c8 label[32];
            snprintf(label, 32, "Col %u", i + 1);
            GUI::push_id(app.gui, 100 + i);
            GUI::selectable(app.gui, label, (i % 3) == 0);
            GUI::pop_id(app.gui);
        }
        GUI::end_grid_layout(app.gui);
        GUI::end_scroll_view(app.gui);

        GUI::text(app.gui, "CanvasLayout");
        f32 canvas_width = max(app.showcase_content_size.x - 16.0f, 260.0f);
        f32 canvas_height = 190.0f;
        GUI::begin_canvas_layout(app.gui, "Viewport Canvas", GUI::Size::fixed(canvas_width, canvas_height));
        GUI::set_next_canvas_item_layout(app.gui, GUI::CanvasItemLayout::stretch(GUI::EdgeInsets::all(8.0f)));
        GUI::ItemHandle viewport = GUI::image(app.gui, nullptr, GUI::Size());
        RectF viewport_rect = GUI::get_item_state(viewport, GUI::State::rect());
        if(viewport_rect.width > 1.0f && viewport_rect.height > 1.0f)
        {
            GUI::draw_rect(app.gui, viewport_rect, Float4U(0.05f, 0.07f, 0.09f, 1.0f), 6.0f);
            GUI::draw_line(app.gui, Float2U(viewport_rect.offset_x + 20.0f, viewport_rect.offset_y + viewport_rect.height - 24.0f),
                Float2U(viewport_rect.offset_x + viewport_rect.width - 24.0f, viewport_rect.offset_y + 24.0f),
                Float4U(0.14f, 0.36f, 0.68f, 0.9f), 4.0f);
        }
        GUI::set_next_canvas_item_layout(app.gui, GUI::CanvasItemLayout::fixed(Float2U(18.0f, 14.0f), Float2U(230.0f, 28.0f)));
        GUI::text(app.gui, "Top-left anchored overlay");
        GUI::set_next_canvas_item_layout(app.gui, GUI::CanvasItemLayout::anchored(Float2U(1.0f, 0.0f), Float2U(-94.0f, 28.0f), Float2U(170.0f, 34.0f)));
        GUI::button(app.gui, "Top Right");
        GUI::set_next_canvas_item_layout(app.gui, GUI::CanvasItemLayout::anchored(Float2U(0.5f, 0.5f), Float2U(0.0f, 0.0f), Float2U(220.0f, 44.0f)));
        handles.canvas_hit = GUI::button(app.gui, "Center Hit Target");
        GUI::set_item_tooltip(app.gui, handles.canvas_hit, "Canvas children are positioned by anchor and offset, not cursor order.");
        GUI::CanvasItemLayout bottom_bar;
        bottom_bar.anchor_min = Float2U(0.0f, 1.0f);
        bottom_bar.anchor_max = Float2U(1.0f, 1.0f);
        bottom_bar.offset_min = Float2U(18.0f, -48.0f);
        bottom_bar.offset_max = Float2U(-18.0f, -12.0f);
        GUI::set_next_canvas_item_layout(app.gui, bottom_bar);
        GUI::begin_h_layout(app.gui, "Canvas Bottom Bar", row);
        GUI::text(app.gui, "Bottom stretch");
        GUI::button(app.gui, "Overlay");
        GUI::button(app.gui, "Controls");
        GUI::end_h_layout(app.gui);
        GUI::end_canvas_layout(app.gui);

        GUI::text(app.gui, "ScrollView");
        GUI::begin_scroll_view(app.gui, "Nested Scroll", GUI::Size::fixed(max(app.showcase_content_size.x - 16.0f, 240.0f), min(max(app.showcase_content_size.y * 0.30f, 120.0f), 220.0f)));
        for(u32 i = 0; i < 16; ++i)
        {
            c8 line[96];
            snprintf(line, 96, "Scrollable row %02u: mouse wheel should move this content.", i + 1);
            GUI::text(app.gui, line);
        }
        GUI::end_scroll_view(app.gui);

        GUI::text(app.gui, "DockSpace");
        GUI::begin_dock_space(app.gui, "Layout DockSpace", GUI::Size::fixed(max(app.showcase_content_size.x - 16.0f, 260.0f), 260.0f));
        if(app.dock_panel_a_open)
        {
            GUI::begin_dock_panel(app.gui, "Docked Panel", &app.dock_panel_a_open);
            GUI::text(app.gui, "This panel starts in docking mode.");
            GUI::button(app.gui, "Docked Action");
            GUI::end_dock_panel(app.gui);
        }
        if(app.dock_panel_b_open)
        {
            GUI::DockPanelStyle floating_style;
            floating_style.initial_mode = GUI::DockPanelMode::floating;
            floating_style.floating_position = Float2U(220.0f, 34.0f);
            floating_style.floating_size = Float2U(240.0f, 150.0f);
            GUI::begin_dock_panel(app.gui, "Floating Panel", &app.dock_panel_b_open, floating_style);
            GUI::text(app.gui, "Drag the title bar or resize from the corner.");
            GUI::toggle_switch(app.gui, "Live", &app.switch_a);
            GUI::end_dock_panel(app.gui);
        }
        GUI::end_dock_space(app.gui);

        handles.open_window_button = GUI::button(app.gui, "Open Closeable Floating Window");
        if(app.floating_window_open)
        {
            GUI::begin_window(app.gui, "Floating Demo", &app.floating_window_open, GUI::Size::fixed(280.0f, 150.0f));
            GUI::text(app.gui, "This is a closeable GUI window.");
            GUI::checkbox(app.gui, "Window checkbox", &app.checkbox_b);
            GUI::end_window(app.gui);
        }
    }

    void draw_tables_tab(App& app)
    {
        demo_section(app, "TableLayout");
        GUI::text(app.gui, "Columns are fixed and resizable; rows are hug-sized with alternating backgrounds.");

        GUI::TableDesc table;
        table.columns = 4;
        f32 table_width = max(app.showcase_content_size.x - 24.0f, 360.0f);
        f32 table_column = max((table_width - 180.0f) / 3.0f, 96.0f);
        table.column_sizes = {
            GUI::TableTrackSize::fixed(table_column),
            GUI::TableTrackSize::fixed(table_column),
            GUI::TableTrackSize::fixed(table_column),
            GUI::TableTrackSize::fixed(180.0f)
        };
        table.style.border_size = 1.0f;
        table.style.background_mode = GUI::TableBackgroundMode::alternate_rows;
        table.style.background_color = Float4U(0.10f, 0.12f, 0.15f, 0.92f);
        table.style.alternate_background_color = Float4U(0.14f, 0.16f, 0.20f, 0.92f);
        table.style.row_separators = true;
        table.style.column_separators = true;
        table.style.resize_fixed_columns = true;
        table.style.separator_size = 1.0f;
        GUI::begin_table_layout(app.gui, "Table Showcase", table);
        GUI::text(app.gui, "Name");
        GUI::text(app.gui, "Enabled");
        GUI::text(app.gui, "Value");
        GUI::text(app.gui, "Action");
        for(u32 i = 0; i < 4; ++i)
        {
            c8 label[64];
            snprintf(label, 64, "Row %u", i + 1);
            GUI::text(app.gui, label);
            GUI::checkbox(app.gui, "Enabled", &app.table_checks[i]);
            GUI::slider_float(app.gui, "Value", &app.table_values[i], 0.0f, 1.0f);
            GUI::button(app.gui, "Run");
        }
        GUI::end_table_layout(app.gui);
    }

    void draw_drawing_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "DrawList and absolute paint nodes");
        GUI::text(app.gui, "The canvas below reserves layout space, then absolute draw commands paint into its last-frame rect.");
        f32 canvas_width = max(app.showcase_content_size.x - 16.0f, 260.0f);
        f32 canvas_height = min(max(app.showcase_content_size.y * 0.42f, 180.0f), 280.0f);
        GUI::ItemHandle canvas = GUI::image(app.gui, nullptr, GUI::Size::fixed(canvas_width, canvas_height));
        RectF rect = GUI::get_item_state(canvas, GUI::State::rect());
        if(rect.width > 1.0f && rect.height > 1.0f)
        {
            GUI::push_clip_rect(app.gui, rect);
            GUI::draw_rect(app.gui, rect, Float4U(0.06f, 0.08f, 0.10f, 1.0f), 6.0f);
            GUI::draw_rect(app.gui, RectF(rect.offset_x + 20.0f, rect.offset_y + 20.0f, 180.0f, 76.0f), Float4U(0.22f, 0.34f, 0.55f, 1.0f), 8.0f);
            GUI::draw_circle(app.gui, Float2U(rect.offset_x + 290.0f, rect.offset_y + 58.0f), 38.0f, Float4U(app.color_value.x, app.color_value.y, app.color_value.z, 1.0f));
            GUI::draw_line(app.gui, Float2U(rect.offset_x + 360.0f, rect.offset_y + 30.0f), Float2U(rect.offset_x + 560.0f, rect.offset_y + 120.0f), Float4U(0.9f, 0.8f, 0.3f, 1.0f), 5.0f);
            GUI::draw_text(app.gui, RectF(rect.offset_x + 24.0f, rect.offset_y + 118.0f, 420.0f, 32.0f), "DrawText: clipped to the canvas", Color::white(), 16.0f);
            handles.canvas_hit = GUI::hit_box(app.gui, "Canvas HitBox", RectF(rect.offset_x + 20.0f, rect.offset_y + 160.0f, 240.0f, 52.0f));
            GUI::draw_rect(app.gui, RectF(rect.offset_x + 20.0f, rect.offset_y + 160.0f, 240.0f, 52.0f),
                GUI::is_item_hovered(handles.canvas_hit) ? Float4U(0.30f, 0.48f, 0.74f, 1.0f) : Float4U(0.16f, 0.24f, 0.34f, 1.0f), 6.0f);
            GUI::draw_text(app.gui, RectF(rect.offset_x + 28.0f, rect.offset_y + 160.0f, 224.0f, 52.0f), "Absolute HitBox", Color::white(), 16.0f, GUI::TextAlignment::center);
            if(app.clip_enabled)
            {
                GUI::draw_circle(app.gui, Float2U(rect.offset_x + rect.width + 20.0f, rect.offset_y + 40.0f), 48.0f, Float4U(1.0f, 0.2f, 0.2f, 0.8f));
            }
            GUI::pop_clip_rect(app.gui);
        }
        GUI::checkbox(app.gui, "Draw an intentionally clipped red circle", &app.clip_enabled);
    }

    void draw_popups_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "MenuBar and MenuItem");
        GUI::begin_menu_bar(app.gui, "Demo Menu Bar");
        if(GUI::begin_menu(app.gui, "File"))
        {
            handles.menu_new = GUI::menu_item(app.gui, "New Scene", "Ctrl+N");
            handles.menu_save = GUI::menu_item(app.gui, "Save", "Ctrl+S");
            GUI::menu_separator(app.gui);
            GUI::menu_item(app.gui, "Disabled Action", "Ctrl+D", false, false);
            GUI::end_menu(app.gui);
        }
        if(GUI::begin_menu(app.gui, "View"))
        {
            handles.menu_show_grid = GUI::menu_item(app.gui, "Show Grid", nullptr, &app.menu_show_grid);
            GUI::menu_item(app.gui, "Snap To Grid", nullptr, &app.menu_snap_to_grid);
            if(GUI::begin_menu(app.gui, "Theme"))
            {
                handles.menu_theme_dark = GUI::menu_item(app.gui, "Dark");
                GUI::menu_item(app.gui, "Light", nullptr, false, false);
                GUI::end_menu(app.gui);
            }
            GUI::end_menu(app.gui);
        }
        GUI::end_menu_bar(app.gui);

        demo_section(app, "Popups and context menus");
        GUI::text(app.gui, "Right-click the selectable below, or click the button to open a stack-managed popup.");
        handles.right_click_item = GUI::selectable(app.gui, "Right click me");
        handles.managed_popup_button = GUI::button(app.gui, "Open Stack Popup");
        GUI::text(app.gui, app.popup_text.c_str());

        GUI::PopupDesc popup_desc;
        popup_desc.position = app.popup_position;
        popup_desc.size = GUI::Size::fixed(220.0f, 100.0f);
        if(GUI::begin_popup(app.gui, "Managed Popup", popup_desc, &handles.managed_popup))
        {
            GUI::text(app.gui, "Stack-managed popup");
            handles.managed_popup_action = GUI::selectable(app.gui, "Action");
            handles.nested_popup_button = GUI::selectable(app.gui, "Open child popup");

            GUI::PopupDesc nested_desc;
            nested_desc.position = Float2U(app.popup_position.x + 180.0f, app.popup_position.y + 34.0f);
            nested_desc.size = GUI::Size::fixed(190.0f, 72.0f);
            if(GUI::begin_popup(app.gui, "Nested Managed Popup", nested_desc, &handles.nested_popup))
            {
                GUI::text(app.gui, "Nested overlay layer");
                handles.nested_popup_close = GUI::selectable(app.gui, "Close child");
                GUI::end_popup(app.gui);
            }
            GUI::end_popup(app.gui);
        }
    }

    void draw_tooltips_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "Tooltip");
        GUI::text(app.gui, "Hover these controls to show text-only and custom overlay tooltips.");

        GUI::ItemHandle button = GUI::button(app.gui, "Hover for tooltip");
        GUI::set_item_tooltip(app.gui, button, "Tooltip content is built every frame but only rendered after the hover delay.");

        GUI::TooltipDesc quick_desc;
        quick_desc.delay = 0.0f;
        GUI::ItemHandle instant = GUI::selectable(app.gui, "Instant tooltip");
        GUI::set_item_tooltip(app.gui, instant, "This tooltip has zero delay and follows the pointer.", quick_desc);

        GUI::TooltipDesc custom_desc;
        custom_desc.delay = 0.2f;
        custom_desc.max_width = 280.0f;
        GUI::ItemHandle custom = GUI::button(app.gui, "Hover for custom tooltip");
        GUI::begin_tooltip(app.gui, custom, "Custom Tooltip", custom_desc);
        GUI::text(app.gui, "Custom tooltip");
        GUI::text(app.gui, "Multiple nodes can be placed here.");
        c8 state[96];
        snprintf(state, 96, "Checkbox A: %s", app.checkbox_a ? "checked" : "unchecked");
        GUI::text(app.gui, state);
        GUI::end_tooltip(app.gui);

        GUI::text(app.gui, "Tooltip over an absolute canvas hit-box:");
        GUI::ItemHandle canvas = GUI::image(app.gui, nullptr, GUI::Size::fixed(260.0f, 72.0f));
        RectF rect = GUI::get_item_state(canvas, GUI::State::rect());
        if(rect.width > 1.0f && rect.height > 1.0f)
        {
            RectF hit_rect(rect.offset_x + 8.0f, rect.offset_y + 12.0f, 220.0f, 46.0f);
            handles.canvas_hit = GUI::hit_box(app.gui, "Tooltip Canvas Hit", hit_rect);
            GUI::draw_rect(app.gui, hit_rect, GUI::is_item_hovered(handles.canvas_hit) ? Float4U(0.25f, 0.43f, 0.68f, 1.0f) : Float4U(0.14f, 0.21f, 0.30f, 1.0f), 6.0f);
            GUI::draw_text(app.gui, hit_rect, "Hover canvas region", Color::white(), 16.0f, GUI::TextAlignment::center);
            GUI::set_item_tooltip(app.gui, handles.canvas_hit, "Tooltips can attach to regular widgets or explicit hit boxes.");
        }
    }

    void draw_state_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "State queries");
        GUI::text(app.gui, "Widget APIs return handles. Query before submit for last-frame state and after submit for current-frame state.");
        handles.primary_button = GUI::button(app.gui, "Inspect Me");
        bool hovered = GUI::is_item_hovered(handles.primary_button);
        bool active = GUI::is_item_active(handles.primary_button);
        bool focused = GUI::is_item_focused(handles.primary_button);
        c8 state[192];
        snprintf(state, 192, "Before submit query: hovered=%s active=%s focused=%s",
            hovered ? "true" : "false",
            active ? "true" : "false",
            focused ? "true" : "false");
        GUI::text(app.gui, state);
        GUI::text(app.gui, app.state_text.c_str());
    }

    void draw_trees_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "TreeNode");
        GUI::text(app.gui, "TreeNode supports open state, selection, leaf rows, and indentation.");
        GUI::text(app.gui, "The first node only toggles when the arrow is clicked.");

        GUI::ItemHandle scene = demo_tree_node(app, handles, 0, "Scene",
            GUI::TreeNodeFlag::default_open | GUI::TreeNodeFlag::open_on_arrow);
        if(GUI::get_item_state(scene, GUI::State::open()))
        {
            GUI::tree_push(app.gui);
            demo_tree_node(app, handles, 1, "Camera", GUI::TreeNodeFlag::leaf);
            demo_tree_node(app, handles, 2, "Directional Light", GUI::TreeNodeFlag::leaf);

            GUI::ItemHandle actor = demo_tree_node(app, handles, 3, "Sponza Actor", GUI::TreeNodeFlag::default_open);
            if(GUI::get_item_state(actor, GUI::State::open()))
            {
                GUI::tree_push(app.gui);
                demo_tree_node(app, handles, 4, "Transform", GUI::TreeNodeFlag::leaf);
                demo_tree_node(app, handles, 5, "Mesh Renderer", GUI::TreeNodeFlag::leaf);

                GUI::ItemHandle material = demo_tree_node(app, handles, 6, "Materials");
                if(GUI::get_item_state(material, GUI::State::open()))
                {
                    GUI::tree_push(app.gui);
                    demo_tree_node(app, handles, 7, "Material Slot 0", GUI::TreeNodeFlag::leaf);
                    GUI::tree_pop(app.gui);
                }
                GUI::tree_pop(app.gui);
            }
            GUI::tree_pop(app.gui);
        }

        c8 selected[96];
        snprintf(selected, 96, "Selected tree node index: %u", app.tree_selected);
        GUI::text(app.gui, selected);
    }

    void draw_drag_drop_tab(App& app, FrameHandles& handles)
    {
        demo_section(app, "Drag and Drop");
        GUI::text(app.gui, "Drag number payloads and text payloads to the matching targets.");
        GUI::text(app.gui, "Only targets that explicitly accept the payload type should be highlighted.");

        GUI::LayoutDesc columns;
        columns.gap = 18.0f;
        columns.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::begin;
        GUI::begin_h_layout(app.gui, "DragDrop Columns", columns);

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(220.0f));
        GUI::begin_v_layout(app.gui, "Drag Sources");
        GUI::text(app.gui, "Sources");
        demo_number_drag_source(app, "Number: 7", 7);
        demo_number_drag_source(app, "Number: 42", 42);
        demo_text_drag_source(app, "Text: Luna", "Luna");
        demo_text_drag_source(app, "Text: Studio", "Studio");
        GUI::end_v_layout(app.gui);

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(280.0f));
        GUI::begin_v_layout(app.gui, "Drop Targets");
        GUI::text(app.gui, "Targets");

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(240.0f));
        handles.drag_number_target = GUI::button(app.gui, "Accept Number");
        Name number_type = demo_number_payload_type();
        if(GUI::begin_drag_drop_target(app.gui, handles.drag_number_target, number_type))
        {
            (void)GUI::accept_drag_drop_payload(app.gui, number_type);
            GUI::end_drag_drop_target(app.gui);
        }
        c8 number_text[96];
        snprintf(number_text, 96, "Number target: %d", app.dropped_number);
        GUI::text(app.gui, number_text);

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(240.0f));
        handles.drag_text_target = GUI::button(app.gui, "Accept Text");
        Name text_type = demo_text_payload_type();
        if(GUI::begin_drag_drop_target(app.gui, handles.drag_text_target, text_type))
        {
            (void)GUI::accept_drag_drop_payload(app.gui, text_type);
            GUI::end_drag_drop_target(app.gui);
        }
        GUI::text(app.gui, app.dropped_text.c_str());

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(240.0f));
        handles.drag_mixed_target = GUI::button(app.gui, "Accept Both");
        if(GUI::begin_drag_drop_target(app.gui, handles.drag_mixed_target, number_type))
        {
            (void)GUI::accept_drag_drop_payload(app.gui, number_type);
            GUI::end_drag_drop_target(app.gui);
        }
        if(GUI::begin_drag_drop_target(app.gui, handles.drag_mixed_target, text_type))
        {
            (void)GUI::accept_drag_drop_payload(app.gui, text_type);
            GUI::end_drag_drop_target(app.gui);
        }
        GUI::text(app.gui, app.mixed_drop_text.c_str());
        GUI::end_v_layout(app.gui);

        GUI::end_h_layout(app.gui);
    }

    void draw_tabs_tab(App& app)
    {
        demo_section(app, "Basic TabBar");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_height(150.0f));
        GUI::begin_tab_bar(app.gui, "Basic Nested Tabs");
        if(GUI::begin_tab_item(app.gui, "Scene"))
        {
            GUI::text(app.gui, "Scene tab content");
            GUI::text(app.gui, "Only the selected tab builds its body.");
            GUI::end_tab_item(app.gui);
        }
        if(GUI::begin_tab_item(app.gui, "Inspector"))
        {
            GUI::text(app.gui, "Inspector tab content");
            GUI::checkbox(app.gui, "Visible", &app.checkbox_a);
            GUI::end_tab_item(app.gui);
        }
        if(GUI::begin_tab_item(app.gui, "Console"))
        {
            GUI::text(app.gui, "Console tab content");
            GUI::input_text(app.gui, "Console Filter", app.state_text);
            GUI::end_tab_item(app.gui);
        }
        GUI::end_tab_bar(app.gui);

        demo_section(app, "Closable Documents");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_height(170.0f));
        GUI::begin_tab_bar(app.gui, "Closable Document Tabs");
        bool any_open = false;
        for(u32 i = 0; i < 4; ++i)
        {
            if(!app.tab_document_open[i]) continue;
            any_open = true;
            c8 label[32];
            snprintf(label, 32, "Document %u", i + 1);
            GUI::TabItemFlag flags = i == 1 ? GUI::TabItemFlag::unsaved_document : GUI::TabItemFlag::none;
            if(GUI::begin_tab_item(app.gui, label, &app.tab_document_open[i], flags))
            {
                c8 text[96];
                snprintf(text, 96, "Document %u body. Close this tab with its X button.", i + 1);
                GUI::text(app.gui, text);
                GUI::end_tab_item(app.gui);
        }
    }

        if(!any_open)
        {
            GUI::tab_item_button(app.gui, "All documents closed");
        }
        GUI::end_tab_bar(app.gui);

        demo_section(app, "Scrollable Reorderable Tabs");
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_height(150.0f));
        GUI::TabBarFlag reorder_scroll_flags = (GUI::TabBarFlag)(
            (u32)GUI::TabBarFlag::fitting_scroll |
            (u32)GUI::TabBarFlag::reorderable |
            (u32)GUI::TabBarFlag::auto_select_new_tabs);
        GUI::begin_tab_bar(app.gui, "Overflow Reorder Tabs", reorder_scroll_flags);
        constexpr const c8* labels[] =
        {
            "Pinned",
            "Scene View",
            "Inspector",
            "Animation",
            "Profiler",
            "Console",
            "Material Graph",
            "Lighting",
            "Import Queue",
            "Build Output"
        };
        for(u32 i = 0; i < (u32)(sizeof(labels) / sizeof(labels[0])); ++i)
        {
            GUI::TabItemFlag flags = i == 0 ? GUI::TabItemFlag::no_reorder : GUI::TabItemFlag::none;
            if(GUI::begin_tab_item(app.gui, labels[i], nullptr, flags))
            {
                GUI::text(app.gui, "Use the arrow buttons or wheel over the tab strip to scroll.");
                GUI::text(app.gui, "Drag a non-pinned tab header horizontally to reorder it.");
                GUI::end_tab_item(app.gui);
            }
        }
        GUI::end_tab_bar(app.gui);
    }

    void draw_debug_tab(App& app)
    {
        demo_section(app, "Debug");
#ifdef LUNA_GUI_ENABLE_DEBUG
        GUI::checkbox(app.gui, "Show Debug Panel", &app.show_debug_panel);
        GUI::text(app.gui, app.show_debug_panel ?
            "Debug panel is visible. Hover and select nodes to inspect layout, style and input routing." :
            "Debug panel is hidden. No debug hit-test outlines are drawn.");
        GUI::text(app.gui, app.has_debug_info ? "Debug snapshot is ready." : "Debug snapshot will be available after the first submitted frame.");
#else
        GUI::text(app.gui, "GUI debug support is disabled for this build.");
#endif
    }

    struct DemoThemePalette
    {
        const c8* name;
        Float4U text;
        Float4U panel;
        Float4U title;
        Float4U surface;
        Float4U surface_focus;
        Float4U accent;
        Float4U accent_hover;
        Float4U accent_active;
        Float4U border;
        Float4U subtle;
        Float4U subtle_hover;
        Float4U selected;
        Float4U disabled;
        Float4U popup;
        Float4U tooltip;
    };

    void set_theme_color(App& app, const Name& style, const c8* entry, const Float4U& color)
    {
        GUI::set_style_f32x4(app.gui, style, Name(entry), color);
    }

    Name demo_font_id()
    {
        return Name("demo.font.default");
    }

    void define_demo_theme(App& app, const DemoThemePalette& theme)
    {
        Name style(theme.name);
        GUI::define_style(app.gui, style);
        GUI::set_style_name(app.gui, style, Name("gui.font"), demo_font_id());
        set_theme_color(app, style, "gui.text.color", theme.text);
        set_theme_color(app, style, "gui.text.disabled", theme.disabled);

        set_theme_color(app, style, "gui.window.background", theme.panel);
        set_theme_color(app, style, "gui.window.title_background", theme.title);
        set_theme_color(app, style, "gui.window.title_color", theme.text);
        set_theme_color(app, style, "gui.window.close_background", theme.subtle);
        set_theme_color(app, style, "gui.window.close_hovered", theme.accent_active);
        set_theme_color(app, style, "gui.scroll_view.background", theme.panel);
        set_theme_color(app, style, "gui.dock_space.background", theme.panel);

        set_theme_color(app, style, "gui.button.background", theme.accent);
        set_theme_color(app, style, "gui.button.background_hovered", theme.accent_hover);
        set_theme_color(app, style, "gui.button.background_active", theme.accent_active);
        set_theme_color(app, style, "gui.button.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.button.text_color", Float4U(1.0f));
        set_theme_color(app, style, "gui.button.text_disabled", theme.disabled);

        set_theme_color(app, style, "gui.selectable.background_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.selectable.background_selected", theme.selected);
        set_theme_color(app, style, "gui.selectable.background_active", theme.accent_active);
        set_theme_color(app, style, "gui.selectable.text_color", theme.text);
        set_theme_color(app, style, "gui.selectable.text_disabled", theme.disabled);

        set_theme_color(app, style, "gui.input_text.background", theme.surface);
        set_theme_color(app, style, "gui.input_text.background_focused", theme.surface_focus);
        set_theme_color(app, style, "gui.input_text.text_color", theme.text);
        set_theme_color(app, style, "gui.input_text.selection", Float4U(theme.accent.x, theme.accent.y, theme.accent.z, 0.72f));
        set_theme_color(app, style, "gui.input_text.cursor", theme.text);

        set_theme_color(app, style, "gui.numeric.background", theme.surface);
        set_theme_color(app, style, "gui.numeric.background_active", theme.surface_focus);
        set_theme_color(app, style, "gui.numeric.text_color", theme.text);
        set_theme_color(app, style, "gui.numeric.label_color", theme.text);
        set_theme_color(app, style, "gui.numeric.selection", Float4U(theme.accent.x, theme.accent.y, theme.accent.z, 0.72f));
        set_theme_color(app, style, "gui.numeric.cursor", theme.text);
        set_theme_color(app, style, "gui.numeric.slider_track", theme.subtle);
        set_theme_color(app, style, "gui.numeric.slider_track_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.numeric.slider_fill", theme.accent);
        set_theme_color(app, style, "gui.numeric.slider_fill_hovered", theme.accent_hover);
        set_theme_color(app, style, "gui.numeric.slider_fill_active", theme.accent_active);
        set_theme_color(app, style, "gui.numeric.drag_fill", theme.accent_hover);
        set_theme_color(app, style, "gui.progress_bar.background", theme.surface);
        set_theme_color(app, style, "gui.progress_bar.fill", theme.accent_active);
        set_theme_color(app, style, "gui.progress_bar.border", theme.border);
        set_theme_color(app, style, "gui.progress_bar.text_color", theme.text);

        set_theme_color(app, style, "gui.checkbox.background", theme.surface);
        set_theme_color(app, style, "gui.checkbox.background_checked", theme.accent_active);
        set_theme_color(app, style, "gui.checkbox.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.checkbox.background_checked_disabled", theme.selected);
        set_theme_color(app, style, "gui.checkbox.border", theme.border);
        set_theme_color(app, style, "gui.checkbox.border_hovered", theme.accent_hover);
        set_theme_color(app, style, "gui.checkbox.border_disabled", theme.border);
        set_theme_color(app, style, "gui.checkbox.text_color", theme.text);
        set_theme_color(app, style, "gui.checkbox.text_disabled", theme.disabled);
        set_theme_color(app, style, "gui.switch.off_track", theme.subtle);
        set_theme_color(app, style, "gui.switch.off_track_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.switch.on_track", theme.accent_active);
        set_theme_color(app, style, "gui.switch.on_track_hovered", theme.accent_hover);
        set_theme_color(app, style, "gui.switch.track_disabled", theme.subtle);
        set_theme_color(app, style, "gui.switch.track_checked_disabled", theme.selected);
        set_theme_color(app, style, "gui.switch.knob_disabled", theme.disabled);
        set_theme_color(app, style, "gui.switch.text_color", theme.text);
        set_theme_color(app, style, "gui.switch.text_disabled", theme.disabled);
        set_theme_color(app, style, "gui.radio_button.background", theme.surface);
        set_theme_color(app, style, "gui.radio_button.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.radio_button.ring", theme.border);
        set_theme_color(app, style, "gui.radio_button.ring_hovered", theme.accent_hover);
        set_theme_color(app, style, "gui.radio_button.ring_disabled", theme.border);
        set_theme_color(app, style, "gui.radio_button.selected_color", theme.accent_active);
        set_theme_color(app, style, "gui.radio_button.selected_disabled", theme.disabled);
        set_theme_color(app, style, "gui.radio_button.text_color", theme.text);
        set_theme_color(app, style, "gui.radio_button.text_disabled", theme.disabled);

        set_theme_color(app, style, "gui.button_group.background", theme.surface);
        set_theme_color(app, style, "gui.button_group.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.button_group.border", theme.border);
        set_theme_color(app, style, "gui.button_group.border_disabled", theme.border);
        set_theme_color(app, style, "gui.button_group.hover", theme.subtle_hover);
        set_theme_color(app, style, "gui.button_group.selected", theme.selected);
        set_theme_color(app, style, "gui.button_group.selected_hot", theme.accent_hover);
        set_theme_color(app, style, "gui.button_group.selected_disabled", theme.selected);
        set_theme_color(app, style, "gui.button_group.separator", theme.border);
        set_theme_color(app, style, "gui.button_group.separator_disabled", theme.border);
        set_theme_color(app, style, "gui.button_group.text", theme.text);
        set_theme_color(app, style, "gui.button_group.text_selected", theme.text);
        set_theme_color(app, style, "gui.button_group.text_disabled", theme.disabled);

        set_theme_color(app, style, "gui.collapsing_header.background", theme.subtle);
        set_theme_color(app, style, "gui.collapsing_header.background_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.collapsing_header.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.collapsing_header.text_color", theme.text);
        set_theme_color(app, style, "gui.collapsing_header.text_disabled", theme.disabled);
        set_theme_color(app, style, "gui.tree_node.background_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.tree_node.background_selected", theme.selected);
        set_theme_color(app, style, "gui.tree_node.background_active", theme.accent_active);
        set_theme_color(app, style, "gui.tree_node.background_disabled", theme.subtle);
        set_theme_color(app, style, "gui.tree_node.text_color", theme.text);
        set_theme_color(app, style, "gui.tree_node.text_disabled", theme.disabled);
        set_theme_color(app, style, "gui.tree_node.icon_color", theme.accent_active);
        set_theme_color(app, style, "gui.tree_node.leaf_icon_color", theme.border);
        set_theme_color(app, style, "gui.tree_node.icon_disabled", theme.disabled);

        set_theme_color(app, style, "gui.menu_bar.background", theme.title);
        set_theme_color(app, style, "gui.menu_bar.border", theme.border);
        set_theme_color(app, style, "gui.menu_item.background_hovered", theme.subtle_hover);
        set_theme_color(app, style, "gui.menu_item.background_active", theme.accent_active);
        set_theme_color(app, style, "gui.menu_item.text_color", theme.text);
        set_theme_color(app, style, "gui.menu_item.text_disabled", theme.disabled);
        set_theme_color(app, style, "gui.menu_separator.color", theme.border);
        set_theme_color(app, style, "gui.popup.background", theme.popup);
        set_theme_color(app, style, "gui.tooltip.background", theme.tooltip);
        set_theme_color(app, style, "gui.tooltip.border", theme.border);
        set_theme_color(app, style, "gui.tab_bar.background", theme.title);
        set_theme_color(app, style, "gui.tab_bar.header_line", theme.border);

        set_theme_color(app, style, "demo.proxy.background", theme.surface);
        set_theme_color(app, style, "demo.proxy.accent", theme.accent_hover);
    }

    void define_demo_theme_styles(App& app)
    {
        define_demo_theme(app, DemoThemePalette
        {
            "demo.theme.dark",
            Float4U(0.92f, 0.95f, 0.98f, 1.0f),
            Float4U(0.08f, 0.10f, 0.13f, 0.98f),
            Float4U(0.12f, 0.16f, 0.22f, 1.0f),
            Float4U(0.11f, 0.15f, 0.20f, 1.0f),
            Float4U(0.14f, 0.20f, 0.29f, 1.0f),
            Float4U(0.17f, 0.29f, 0.48f, 1.0f),
            Float4U(0.28f, 0.50f, 0.82f, 1.0f),
            Float4U(0.22f, 0.38f, 0.62f, 1.0f),
            Float4U(0.30f, 0.36f, 0.44f, 1.0f),
            Float4U(0.07f, 0.09f, 0.12f, 1.0f),
            Float4U(0.16f, 0.23f, 0.32f, 1.0f),
            Float4U(0.16f, 0.25f, 0.38f, 1.0f),
            Float4U(0.46f, 0.50f, 0.56f, 1.0f),
            Float4U(0.07f, 0.09f, 0.12f, 0.98f),
            Float4U(0.05f, 0.06f, 0.08f, 0.98f)
        });
        define_demo_theme(app, DemoThemePalette
        {
            "demo.theme.light",
            Float4U(0.12f, 0.16f, 0.22f, 1.0f),
            Float4U(0.94f, 0.96f, 0.98f, 0.98f),
            Float4U(0.84f, 0.89f, 0.96f, 1.0f),
            Float4U(1.0f, 1.0f, 1.0f, 1.0f),
            Float4U(0.88f, 0.94f, 1.0f, 1.0f),
            Float4U(0.24f, 0.45f, 0.78f, 1.0f),
            Float4U(0.34f, 0.60f, 0.94f, 1.0f),
            Float4U(0.18f, 0.36f, 0.66f, 1.0f),
            Float4U(0.62f, 0.68f, 0.76f, 1.0f),
            Float4U(0.88f, 0.91f, 0.95f, 1.0f),
            Float4U(0.78f, 0.86f, 0.96f, 1.0f),
            Float4U(0.70f, 0.82f, 0.96f, 1.0f),
            Float4U(0.50f, 0.56f, 0.64f, 1.0f),
            Float4U(0.96f, 0.98f, 1.0f, 0.98f),
            Float4U(0.98f, 0.99f, 1.0f, 0.98f)
        });
    }

    Name active_demo_theme_style(const App& app)
    {
        if(app.style_theme == 1) return Name("demo.theme.dark");
        if(app.style_theme == 2) return Name("demo.theme.light");
        return Name();
    }

    void draw_theme_preview(App& app)
    {
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_height(420.0f));
        GUI::begin_window(app.gui, "Theme Preview", GUI::Size::fixed(760.0f, 420.0f));

        GUI::begin_menu_bar(app.gui, "Theme Preview Menu");
        if(GUI::begin_menu(app.gui, "File"))
        {
            GUI::menu_item(app.gui, "New", "Ctrl+N");
            GUI::menu_item(app.gui, "Disabled", nullptr, false, false);
            GUI::menu_separator(app.gui);
            GUI::menu_item(app.gui, "Checkable", nullptr, &app.style_preview_bool);
            GUI::end_menu(app.gui);
        }
        GUI::end_menu_bar(app.gui);

        GUI::text(app.gui, "The preview below is rebuilt from ordinary widgets using the selected style.");
        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;
        GUI::begin_h_layout(app.gui, "Theme Preview Buttons", row);
        GUI::button(app.gui, "Primary Button");
        GUI::selectable(app.gui, "Selectable Row", true);
        GUI::end_h_layout(app.gui);

        GUI::checkbox(app.gui, "Checkbox", &app.style_preview_checkbox);
        GUI::toggle_switch(app.gui, "Switch", &app.style_preview_switch);
        GUI::begin_h_layout(app.gui, "Theme Preview Radio", row);
        GUI::radio_button(app.gui, "One", &app.style_preview_choice, 0);
        GUI::radio_button(app.gui, "Two", &app.style_preview_choice, 1);
        GUI::radio_button(app.gui, "Three", &app.style_preview_choice, 2);
        GUI::end_h_layout(app.gui);

        const c8* group_items[] = { "Alpha", "Beta", "Gamma" };
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(360.0f));
        GUI::button_group(app.gui, "Theme Button Group", &app.style_preview_group, Span<const c8*>(group_items, 3));
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill_width());
        GUI::input_text(app.gui, "Theme Input", app.style_preview_text);
        GUI::slider_float(app.gui, "Theme Slider", &app.style_preview_slider, 0.0f, 1.0f);
        GUI::progress_bar(app.gui, "Theme Progress", app.style_preview_slider);
        GUI::drag_float(app.gui, "Theme Drag", &app.style_preview_drag, 1.0f, 0.0f, 100.0f, GUI::NumericEditFlag::input_on_double_click);

        GUI::ItemHandle header = GUI::collapsing_header(app.gui, "Collapsing Header");
        if(GUI::get_item_state(header, GUI::State::open()))
        {
            GUI::tree_push(app.gui);
            GUI::tree_node(app.gui, "Tree Node", GUI::TreeNodeFlag::selected | GUI::TreeNodeFlag::leaf);
            GUI::tree_pop(app.gui);
        }

        GUI::set_next_item_render_proxy(app.gui, demo_button_render_proxy());
        GUI::button(app.gui, "Render Proxy Styled By Theme");
        GUI::end_window(app.gui);
    }

    void draw_style_tab(App& app)
    {
        demo_section(app, "Style inheritance");
        GUI::define_style(app.gui, Name("demo.button.base"));
        GUI::set_style_f32x4(app.gui, Name("demo.button.base"), Name("gui.button.background"), Float4U(0.16f, 0.24f, 0.34f, 1.0f));
        GUI::set_style_f32x4(app.gui, Name("demo.button.base"), Name("gui.button.background_hovered"), Float4U(0.22f, 0.34f, 0.50f, 1.0f));
        GUI::set_style_f32x4(app.gui, Name("demo.button.base"), Name("gui.button.background_active"), Float4U(0.26f, 0.42f, 0.64f, 1.0f));
        GUI::set_style_f32(app.gui, Name("demo.button.base"), Name("gui.button.radius"), 8.0f);

        GUI::define_style(app.gui, Name("demo.button.warning"), Name("demo.button.base"));
        GUI::set_style_f32x4(app.gui, Name("demo.button.warning"), Name("gui.button.background"), Float4U(0.42f, 0.25f, 0.12f, 1.0f));
        GUI::set_style_f32x4(app.gui, Name("demo.button.warning"), Name("gui.button.background_hovered"), Float4U(0.62f, 0.34f, 0.14f, 1.0f));

        GUI::define_style(app.gui, Name("demo.button.unset_hover"), Name("demo.button.warning"));
        GUI::unset_style_entry(app.gui, Name("demo.button.unset_hover"), Name("gui.button.background_hovered"));

        GUI::LayoutDesc row;
        row.gap = 8.0f;
        row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::center;
        GUI::begin_h_layout(app.gui, "Styled Buttons", row);
        GUI::push_style(app.gui, Name("demo.button.base"));
        GUI::button(app.gui, "Base Style");
        GUI::pop_style(app.gui);
        GUI::push_style(app.gui, Name("demo.button.warning"));
        GUI::button(app.gui, "Child Override");
        GUI::pop_style(app.gui);
        GUI::push_style(app.gui, Name("demo.button.unset_hover"));
        GUI::button(app.gui, "Unset Hover");
        GUI::pop_style(app.gui);
        GUI::button(app.gui, "Default Style");
        GUI::end_h_layout(app.gui);

        demo_section(app, "Render proxy override");
        GUI::define_style(app.gui, Name("demo.proxy.button"));
        GUI::set_style_f32x4(app.gui, Name("demo.proxy.button"), Name("demo.proxy.background"), Float4U(0.09f, 0.12f, 0.16f, 1.0f));
        GUI::set_style_f32x4(app.gui, Name("demo.proxy.button"), Name("demo.proxy.accent"), Float4U(0.38f, 0.74f, 0.98f, 1.0f));
        GUI::begin_h_layout(app.gui, "Proxy Buttons", row);
        GUI::push_style(app.gui, Name("demo.proxy.button"));
        GUI::set_next_item_render_proxy(app.gui, demo_button_render_proxy());
        GUI::button(app.gui, "Custom Proxy");
        GUI::set_next_item_render_proxy(app.gui, demo_button_render_proxy());
        GUI::button(app.gui, "Same Node, Different Draw");
        GUI::pop_style(app.gui);
        GUI::end_h_layout(app.gui);

        demo_section(app, "Theme switcher");
        define_demo_theme_styles(app);
        const c8* theme_items[] = { "Default", "Dark", "Light" };
        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fixed_width(360.0f));
        GUI::button_group(app.gui, "Theme Selector", &app.style_theme, Span<const c8*>(theme_items, 3));
        GUI::text(app.gui, app.style_theme == 0 ?
            "Default uses the GUI default colors and does not push a style." :
            (app.style_theme == 1 ? "Dark pushes demo.theme.dark around the preview." :
                "Light pushes demo.theme.light around the preview."));

        Name theme_style = active_demo_theme_style(app);
        if(theme_style)
        {
            GUI::push_style(app.gui, theme_style);
            draw_theme_preview(app);
            GUI::pop_style(app.gui);
        }
        else
        {
            draw_theme_preview(app);
        }
    }

    void draw_showcase_tab(App& app, FrameHandles& handles, u32 tab)
    {
        GUI::begin_scroll_view(app.gui, "Showcase Content", GUI::Size::fixed(app.showcase_content_size.x, app.showcase_content_size.y));
        GUI::push_id(app.gui, tab);
        switch(tab)
        {
        case DEMO_TAB_OVERVIEW:
            draw_overview_tab(app);
            break;
        case DEMO_TAB_WIDGETS:
            draw_widgets_tab(app, handles);
            break;
        case DEMO_TAB_LAYOUT:
            draw_layout_tab(app, handles);
            break;
        case DEMO_TAB_TABLES:
            draw_tables_tab(app);
            break;
        case DEMO_TAB_DRAWING:
            draw_drawing_tab(app, handles);
            break;
        case DEMO_TAB_TOOLTIPS:
            draw_tooltips_tab(app, handles);
            break;
        case DEMO_TAB_POPUPS:
            draw_popups_tab(app, handles);
            break;
        case DEMO_TAB_STATE:
            draw_state_tab(app, handles);
            break;
        case DEMO_TAB_STYLE:
            draw_style_tab(app);
            break;
        case DEMO_TAB_TREES:
            draw_trees_tab(app, handles);
            break;
        case DEMO_TAB_TABS:
            draw_tabs_tab(app);
            break;
        case DEMO_TAB_DRAG_DROP:
            draw_drag_drop_tab(app, handles);
            break;
        case DEMO_TAB_DEBUG:
            draw_debug_tab(app);
            break;
        default:
            break;
        }
        GUI::pop_id(app.gui);
        GUI::end_scroll_view(app.gui);
    }

    void draw_showcase(App& app, FrameHandles& handles, const Float2U& surface_size, u32& built_tab)
    {
        app.showcase_size = Float2U(max(surface_size.x, 1.0f), max(surface_size.y, 1.0f));
        app.showcase_content_size = Float2U(max(app.showcase_size.x - 32.0f, 240.0f), max(app.showcase_size.y - 92.0f, 140.0f));
        GUI::begin_window(app.gui, "Luna GUI Showcase", GUI::Size::fixed(app.showcase_size.x, app.showcase_size.y));
        GUI::text(app.gui, "Luna GUI Showcase");

        GUI::set_next_item_layout(app.gui, GUI::LayoutStyle::fill());
        GUI::begin_tab_bar(app.gui, "Showcase Tabs", GUI::TabBarFlag::fitting_shrink);
        for(u32 i = 0; i < DEMO_TAB_COUNT; ++i)
        {
            if(GUI::begin_tab_item(app.gui, DEMO_TABS[i]))
            {
                built_tab = i;
                app.selected_tab = i;
                draw_showcase_tab(app, handles, i);
                GUI::end_tab_item(app.gui);
            }
        }
        GUI::end_tab_bar(app.gui);
        GUI::end_window(app.gui);
    }

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg(), GUI::module_gui(), GUIWindow::module_gui_window()}));
            luexp(init_modules());
            register_struct_type<DemoCustomNode>({}, typeof<GUI::Node>());
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
            luexp(GUI::register_font(app.gui, demo_font_id(), Font::get_default_font()));
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
                GUI::FrameDesc frame;
                frame.surface_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);

                FrameHandles handles = {};
                u32 built_tab = app.selected_tab;
                draw_showcase(app, handles, frame.surface_size, built_tab);
#ifdef LUNA_GUI_ENABLE_DEBUG
                if(app.show_debug_panel && app.has_debug_info)
                {
                    GUI::show_debug_info(app.gui, app.debug_info);
                }
#endif

                lulet(desc, app.gui->end_build());
                luexp(app.gui->submit(desc));
#ifdef LUNA_GUI_ENABLE_DEBUG
                lulet(debug_info, app.gui->dump_debug_info());
                app.debug_info = move(debug_info);
                app.has_debug_info = true;
#endif
                luexp(GUIWindow::update_text_input(&input_adapter));

                if(built_tab == DEMO_TAB_WIDGETS)
                {
                    if(GUI::is_item_clicked(handles.primary_button))
                    {
                        ++app.click_count;
                    }
                    if(GUI::is_item_double_clicked(handles.double_click_item))
                    {
                        ++app.double_click_count;
                    }
                    if(GUI::is_item_right_clicked(handles.right_click_item))
                    {
                        ++app.right_click_count;
                    }
                }
                else if(built_tab == DEMO_TAB_LAYOUT)
                {
                    if(GUI::is_item_clicked(handles.open_window_button))
                    {
                        app.floating_window_open = true;
                    }
                }
                else if(built_tab == DEMO_TAB_DRAWING)
                {
                    if(GUI::is_item_clicked(handles.canvas_hit))
                    {
                        app.state_text = "Canvas HitBox clicked";
                    }
                }
                else if(built_tab == DEMO_TAB_POPUPS)
                {
                    if(GUI::is_item_clicked(handles.menu_new))
                    {
                        app.popup_text = "Menu: New Scene";
                    }
                    if(GUI::is_item_clicked(handles.menu_save))
                    {
                        app.popup_text = "Menu: Save";
                    }
                    if(GUI::is_item_clicked(handles.menu_show_grid))
                    {
                        app.popup_text = app.menu_show_grid ? "Menu: Show Grid on" : "Menu: Show Grid off";
                    }
                    if(GUI::is_item_clicked(handles.menu_theme_dark))
                    {
                        app.popup_text = "Menu: Dark theme";
                    }
                    if(GUI::is_item_clicked(handles.managed_popup_button))
                    {
                        app.popup_position = GUI::get_pointer_position(app.gui);
                        app.popup_text = "Stack popup opened";
                        GUI::open_popup(app.gui, handles.managed_popup);
                    }
                    if(GUI::is_item_clicked(handles.managed_popup_action))
                    {
                        app.popup_text = "Stack popup action clicked";
                        GUI::close_current_popup(app.gui);
                    }
                    if(GUI::is_item_clicked(handles.nested_popup_button))
                    {
                        app.popup_text = "Nested popup opened";
                        GUI::open_popup(app.gui, handles.nested_popup);
                    }
                    if(GUI::is_item_clicked(handles.nested_popup_close))
                    {
                        app.popup_text = "Nested popup closed";
                        GUI::close_popup(app.gui, handles.nested_popup);
                    }
                    if(GUI::is_item_right_clicked(handles.right_click_item))
                    {
                        app.popup_position = GUI::get_pointer_position(app.gui);
                        app.popup_text = "Stack popup opened by right click";
                        GUI::open_popup(app.gui, handles.managed_popup);
                    }
                }
                else if(built_tab == DEMO_TAB_STATE)
                {
                    bool hovered = GUI::is_item_hovered(handles.primary_button);
                    bool active = GUI::is_item_active(handles.primary_button);
                    bool focused = GUI::is_item_focused(handles.primary_button);
                    c8 state[192];
                    snprintf(state, 192, "After submit query: hovered=%s active=%s focused=%s clicked=%s",
                        hovered ? "true" : "false",
                        active ? "true" : "false",
                        focused ? "true" : "false",
                        GUI::is_item_clicked(handles.primary_button) ? "true" : "false");
                    app.state_text = state;
                }
                else if(built_tab == DEMO_TAB_TREES)
                {
                    for(u32 i = 0; i < TREE_NODE_COUNT; ++i)
                    {
                        if(GUI::is_item_clicked(handles.tree_nodes[i]))
                        {
                            app.tree_selected = i;
                            c8 state[96];
                            snprintf(state, 96, "Tree node %u clicked", i);
                            app.state_text = state;
                        }
                    }
                }
                else if(built_tab == DEMO_TAB_DRAG_DROP)
                {
                    Name number_type = demo_number_payload_type();
                    Name text_type = demo_text_payload_type();
                    if(const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, handles.drag_number_target, number_type))
                    {
                        if(const i32* value = payload->data_as<i32>())
                        {
                            app.dropped_number = *value;
                            c8 state[96];
                            snprintf(state, 96, "Number target received %d", *value);
                            app.state_text = state;
                        }
                    }
                    if(const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, handles.drag_text_target, text_type))
                    {
                        app.dropped_text = "Text target: ";
                        if(payload->data && payload->data_size)
                        {
                            app.dropped_text.append((const c8*)payload->data, payload->data_size - 1);
                        }
                        app.state_text = app.dropped_text;
                    }
                    if(const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, handles.drag_mixed_target, number_type))
                    {
                        if(const i32* value = payload->data_as<i32>())
                        {
                            c8 state[96];
                            snprintf(state, 96, "Mixed target: number %d", *value);
                            app.mixed_drop_text = state;
                            app.state_text = state;
                        }
                    }
                    if(const GUI::DragDropPayload* payload = GUI::accept_drag_drop_payload(app.gui, handles.drag_mixed_target, text_type))
                    {
                        app.mixed_drop_text = "Mixed target: text ";
                        if(payload->data && payload->data_size)
                        {
                            app.mixed_drop_text.append((const c8*)payload->data, payload->data_size - 1);
                        }
                        app.state_text = app.mixed_drop_text;
                    }
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
