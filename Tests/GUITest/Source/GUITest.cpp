/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/MemoryUtils.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeBuffer.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>
#include <cstdio>

using namespace Luna;

namespace
{
    struct DemoState
    {
        i32 selected_tab = 0;
        i32 selected_group = 1;
        bool selected_group_multi[4] = { true, false, true, false };
        bool selectable_selected = false;
        bool checkbox_value = true;
        bool switch_value = false;
        i32 radio_value = 1;
        i32 button_clicks = 0;
        String input_value = "Editable text";
        String notes = "The new GUI package is built directly on GUI Core.";
        String read_only_value = "Read-only value";
        f32 slider_value = 0.42f;
        i32 slider_integer = 64;
        f32 slider_float3[3] = { 0.2f, 0.5f, 0.8f };
        i32 slider_int3[3] = { 20, 50, 80 };
        f32 drag_float_value = 12.5f;
        i32 drag_int_value = 24;
        f32 drag_float3[3] = { 1.0f, 2.0f, 3.0f };
        i32 drag_int3[3] = { 4, 8, 12 };
        i32 combo_item = 0;
        bool menu_grid = true;
        u32 table_submitted_rows = 0;
        Float2U popup_position = Float2U(120.0f, 180.0f);
        Float2U child_popup_position = Float2U(380.0f, 220.0f);
        Ref<RHI::ITexture> image_texture;
        Ref<VG::IShapeBuffer> icon_shape_buffer;
        GUICore::ShapeDesc icon_shape;
    };

    struct FrameHandles
    {
        GUICore::ElementHandle container_button;
        GUICore::ElementHandle selectable;
        GUICore::ElementHandle hit_box;
    };

    struct DemoApp
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUICore::IContext> gui;
        Ref<VG::IShapeDrawList> draw_list;
        Ref<VG::IShapeRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        DemoState state;
    };

    struct DemoOptions
    {
        i32 selected_tab = 0;
        i32 max_frames = -1;
    };

    GUICore::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUICore::LayoutConfig layout;
        layout.width.kind = GUICore::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUICore::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    GUICore::LayoutConfig fill_layout()
    {
        GUICore::LayoutConfig layout;
        layout.width.kind = GUICore::SizeKind::percent;
        layout.width.value = 1.0f;
        layout.height.kind = GUICore::SizeKind::percent;
        layout.height.value = 1.0f;
        return layout;
    }

    GUICore::LayoutConfig fill_width_layout(f32 height, f32 margin = 4.0f)
    {
        GUICore::LayoutConfig layout;
        layout.width.kind = GUICore::SizeKind::percent;
        layout.width.value = 1.0f;
        layout.height.kind = GUICore::SizeKind::fixed;
        layout.height.value = height;
        layout.margin = Float4U(8.0f, margin, 8.0f, margin);
        return layout;
    }

    GUICore::LayoutConfig growing_layout()
    {
        GUICore::LayoutConfig layout;
        layout.width.kind = GUICore::SizeKind::fit;
        layout.height.kind = GUICore::SizeKind::fit;
        layout.flex_grow = 1.0f;
        return layout;
    }

    GUI::id_t make_id(GUICore::IContext* context, const c8* name)
    {
        return context->make_id(name);
    }

    GUI::id_t make_child_id(GUI::id_t parent, u64 index)
    {
        return GUICore::make_scoped_id(parent, index + 1);
    }

    GUI::TextDesc heading_desc(f32 size)
    {
        GUI::TextDesc desc;
        desc.font_size = size;
        return desc;
    }

    void section_heading(GUICore::IContext* context, const c8* id, const c8* value)
    {
        GUI::text(context, make_id(context, id), value, fill_width_layout(30.0f, 8.0f), heading_desc(19.0f));
    }

    GUICore::ElementHandle begin_page(GUICore::IContext* context, const c8* id)
    {
        GUI::ScrollViewDesc desc;
        desc.horizontal = false;
        return GUI::begin_scroll_view(context, make_id(context, id), id, fill_layout(), desc);
    }

    void build_primitives_page(GUICore::IContext* context, DemoState& state)
    {
        begin_page(context, "demo.primitives.scroll");
        section_heading(context, "demo.primitives.heading.text", "Text");
        GUI::text(context, make_id(context, "demo.primitives.text.normal"),
            "Text is measured by its VG font arrangement callback.", fill_width_layout(28.0f));
        GUI::TextDesc centered;
        centered.horizontal_alignment = GUI::TextAlignment::center;
        centered.font_size = 22.0f;
        GUI::text(context, make_id(context, "demo.primitives.text.centered"),
            "Centered 22 pt text", fill_width_layout(42.0f), centered);

        section_heading(context, "demo.primitives.heading.image", "Image");
        GUICore::ElementHandle image_row = GUI::begin_h_layout(context,
            make_id(context, "demo.primitives.image.row"), "Image row", fill_width_layout(120.0f));
        GUICore::LayoutConfig image_layout = fixed_layout(104.0f, 104.0f);
        image_layout.margin = Float4U(4.0f);
        GUI::image(context, make_id(context, "demo.primitives.image"), state.image_texture, image_layout);
        GUICore::LayoutConfig description_layout = growing_layout();
        description_layout.height.kind = GUICore::SizeKind::percent;
        description_layout.height.value = 1.0f;
        GUICore::ElementHandle description = GUI::begin_v_layout(context,
            make_id(context, "demo.primitives.image.description"), "Image description", description_layout);
        GUI::text(context, make_id(context, "demo.primitives.image.title"), "RHI texture rendered by the Image widget",
            fill_width_layout(30.0f, 2.0f), heading_desc(17.0f));
        GUI::text(context, make_id(context, "demo.primitives.image.body"),
            "The sample uses a generated checker texture and the default linear sampler.", fill_width_layout(46.0f, 2.0f));
        GUICore::FlexLayoutDesc description_flex;
        description_flex.axis = GUICore::LayoutAxis::y;
        description_flex.main_alignment = GUICore::FlexAlignment::center;
        GUI::end_v_layout(context, description, description_flex);
        GUICore::FlexLayoutDesc image_row_flex;
        image_row_flex.axis = GUICore::LayoutAxis::x;
        image_row_flex.cross_alignment = GUICore::FlexAlignment::center;
        image_row_flex.main_axis_gap = 12.0f;
        GUI::end_h_layout(context, image_row, image_row_flex);

        section_heading(context, "demo.primitives.heading.progress", "Progress Bar");
        GUI::progress_bar(context, make_id(context, "demo.primitives.progress"), state.slider_value,
            fill_width_layout(28.0f));
        GUI::text(context, make_id(context, "demo.primitives.progress.note"),
            "The progress value is shared with the floating-point slider on the Input tab.", fill_width_layout(28.0f));
        GUI::end_scroll_view(context);
    }

    void build_buttons_page(GUICore::IContext* context, DemoState& state, FrameHandles& handles)
    {
        begin_page(context, "demo.buttons.scroll");
        section_heading(context, "demo.buttons.heading", "Button Containers");
        GUI::text_button(context, make_id(context, "demo.buttons.text"), "Text Button", fill_width_layout(38.0f));
        GUI::ButtonDesc disabled;
        disabled.enabled = false;
        GUI::text_button(context, make_id(context, "demo.buttons.disabled"), "Disabled Button",
            fill_width_layout(38.0f), disabled);

        handles.container_button = GUI::begin_button(context, make_id(context, "demo.buttons.container"),
            "Container Button", fill_width_layout(46.0f));
        GUI::text(context, make_id(context, "demo.buttons.container.prefix"), "[+] ");
        GUI::text(context, make_id(context, "demo.buttons.container.label"),
            "Button is a container and does not render its debug label");
        GUI::end_button(context);

        c8 status[96];
        snprintf(status, sizeof(status), "Container button clicks: %d", state.button_clicks);
        GUI::text(context, make_id(context, "demo.buttons.clicks"), status, fill_width_layout(28.0f));

        section_heading(context, "demo.buttons.group.heading", "Button Group");
        const c8* items[] = { "Build", "Run", "Profile", "Ship" };
        GUI::button_group(context, make_id(context, "demo.buttons.group"),
            Span<const c8*>(items, 4), &state.selected_group, fill_width_layout(38.0f));
        snprintf(status, sizeof(status), "Selected item: %s", items[clamp(state.selected_group, 0, 3)]);
        GUI::text(context, make_id(context, "demo.buttons.group.status"), status, fill_width_layout(28.0f));

        section_heading(context, "demo.buttons.group.multi.heading", "Multiple Selection Button Group");
        GUI::button_group(context, make_id(context, "demo.buttons.group.multi"),
            Span<const c8*>(items, 4), Span<bool>(state.selected_group_multi, 4), fill_width_layout(38.0f));

        section_heading(context, "demo.buttons.shape.heading", "Shape, Shape Button and Hit Box");
        GUICore::ElementHandle shape_row = GUI::begin_h_layout(context,
            make_id(context, "demo.buttons.shape.row"), "Shape controls", fill_width_layout(52.0f));
        GUI::shape(context, make_id(context, "demo.buttons.shape"), state.icon_shape, fixed_layout(42.0f, 42.0f));
        GUI::shape_button(context, make_id(context, "demo.buttons.shape_button"), "Shape Button",
            state.icon_shape, fixed_layout(54.0f, 42.0f));
        handles.hit_box = GUI::hit_box(context, make_id(context, "demo.buttons.hit_box"), fixed_layout(120.0f, 42.0f));
        GUI::text(context, make_id(context, "demo.buttons.hit_box.label"), "Invisible hit box", fixed_layout(150.0f, 42.0f));
        GUICore::FlexLayoutDesc shape_flex;
        shape_flex.main_axis_gap = 10.0f;
        shape_flex.cross_alignment = GUICore::FlexAlignment::center;
        GUI::end_h_layout(context, shape_row, shape_flex);

        section_heading(context, "demo.buttons.choice.heading", "Selection Controls");
        handles.selectable = GUI::selectable(context, make_id(context, "demo.buttons.selectable"),
            "Selectable item", state.selectable_selected, fill_width_layout(34.0f));
        GUI::checkbox(context, make_id(context, "demo.buttons.checkbox"), "Checkbox", &state.checkbox_value,
            fill_width_layout(34.0f));
        GUI::radio_button(context, make_id(context, "demo.buttons.radio.0"), "Radio A", &state.radio_value, 0,
            fill_width_layout(34.0f));
        GUI::radio_button(context, make_id(context, "demo.buttons.radio.1"), "Radio B", &state.radio_value, 1,
            fill_width_layout(34.0f));
        GUI::toggle_switch(context, make_id(context, "demo.buttons.switch"), "Toggle Switch", &state.switch_value,
            fill_width_layout(34.0f));

        section_heading(context, "demo.buttons.disclosure.heading", "Disclosure and Tree");
        if(GUI::collapsing_header(context, make_id(context, "demo.buttons.header"), "Collapsing Header",
            fill_width_layout(34.0f)))
        {
            GUI::text(context, make_id(context, "demo.buttons.header.content"),
                "Header content is conditionally submitted.", fill_width_layout(28.0f));
        }
        if(GUI::tree_node(context, make_id(context, "demo.buttons.tree.root"), "Root Node",
            GUI::TreeNodeFlag::none, 0, fill_width_layout(32.0f)))
        {
            GUI::tree_node(context, make_id(context, "demo.buttons.tree.branch"), "Branch Node",
                GUI::TreeNodeFlag::none, 1, fill_width_layout(32.0f));
            GUI::tree_node(context, make_id(context, "demo.buttons.tree.leaf"), "Leaf Node",
                GUI::TreeNodeFlag::leaf, 2, fill_width_layout(32.0f));
        }
        GUI::end_scroll_view(context);
    }

    void build_input_page(GUICore::IContext* context, DemoState& state)
    {
        begin_page(context, "demo.input.scroll");
        section_heading(context, "demo.input.heading.text", "Text Input");
        GUI::TextInputDesc input_desc;
        input_desc.placeholder = "Enter text";
        GUI::input_text(context, make_id(context, "demo.input.editable"), state.input_value,
            fill_width_layout(38.0f), input_desc);
        GUI::input_text(context, make_id(context, "demo.input.notes"), state.notes,
            fill_width_layout(38.0f), input_desc);
        GUI::TextInputDesc read_only;
        read_only.read_only = true;
        GUI::input_text(context, make_id(context, "demo.input.readonly"), state.read_only_value,
            fill_width_layout(38.0f), read_only);
        GUI::TextInputDesc disabled;
        disabled.enabled = false;
        GUI::input_text(context, make_id(context, "demo.input.disabled"), state.read_only_value,
            fill_width_layout(38.0f), disabled);

        section_heading(context, "demo.input.heading.slider", "Slider");
        GUI::slider_float(context, make_id(context, "demo.input.slider.float"), &state.slider_value,
            0.0f, 1.0f, fill_width_layout(30.0f));
        GUI::slider_int(context, make_id(context, "demo.input.slider.int"), &state.slider_integer,
            0, 100, fill_width_layout(30.0f));
        GUI::slider_float3(context, make_id(context, "demo.input.slider.float3"), state.slider_float3,
            0.0f, 1.0f, fill_width_layout(30.0f));
        GUI::slider_int3(context, make_id(context, "demo.input.slider.int3"), state.slider_int3,
            0, 100, fill_width_layout(30.0f));
        c8 values[128];
        snprintf(values, sizeof(values), "float = %.3f    integer = %d", state.slider_value, state.slider_integer);
        GUI::text(context, make_id(context, "demo.input.slider.values"), values, fill_width_layout(28.0f));

        section_heading(context, "demo.input.heading.drag", "Drag Editors");
        GUI::DragDesc float_drag;
        float_drag.speed = 0.05f;
        GUI::drag_float(context, make_id(context, "demo.input.drag.float"), &state.drag_float_value,
            -100.0f, 100.0f, fill_width_layout(34.0f), float_drag);
        GUI::DragDesc int_drag;
        int_drag.speed = 1.0f;
        GUI::drag_int(context, make_id(context, "demo.input.drag.int"), &state.drag_int_value,
            -100, 100, fill_width_layout(34.0f), int_drag);
        GUI::drag_float3(context, make_id(context, "demo.input.drag.float3"), state.drag_float3,
            -100.0f, 100.0f, fill_width_layout(34.0f), float_drag);
        GUI::drag_int3(context, make_id(context, "demo.input.drag.int3"), state.drag_int3,
            -100, 100, fill_width_layout(34.0f), int_drag);
        GUI::end_scroll_view(context);
    }

    void build_layouts_page(GUICore::IContext* context)
    {
        begin_page(context, "demo.layouts.scroll");
        section_heading(context, "demo.layouts.heading.flex", "HLayout and VLayout");
        GUICore::ElementHandle horizontal = GUI::begin_h_layout(context,
            make_id(context, "demo.layouts.horizontal"), "Horizontal flex", fill_width_layout(54.0f));
        GUI::text_button(context, make_id(context, "demo.layouts.horizontal.fixed"), "Fixed",
            fixed_layout(120.0f, 42.0f));
        GUICore::LayoutConfig growing = fixed_layout(120.0f, 42.0f);
        growing.flex_grow = 1.0f;
        GUI::text_button(context, make_id(context, "demo.layouts.horizontal.grow"), "flex_grow = 1", growing);
        GUI::text_button(context, make_id(context, "demo.layouts.horizontal.trailing"), "Fixed 140",
            fixed_layout(140.0f, 42.0f));
        GUICore::FlexLayoutDesc horizontal_desc;
        horizontal_desc.axis = GUICore::LayoutAxis::x;
        horizontal_desc.main_axis_gap = 8.0f;
        horizontal_desc.cross_alignment = GUICore::FlexAlignment::center;
        GUI::end_h_layout(context, horizontal, horizontal_desc);

        GUICore::ElementHandle vertical = GUI::begin_v_layout(context,
            make_id(context, "demo.layouts.vertical"), "Vertical flex", fill_width_layout(150.0f));
        GUI::text_button(context, make_id(context, "demo.layouts.vertical.a"), "Vertical A", fill_width_layout(34.0f, 0.0f));
        GUI::text_button(context, make_id(context, "demo.layouts.vertical.b"), "Vertical B", fill_width_layout(34.0f, 0.0f));
        GUI::text_button(context, make_id(context, "demo.layouts.vertical.c"), "Vertical C", fill_width_layout(34.0f, 0.0f));
        GUICore::FlexLayoutDesc vertical_desc;
        vertical_desc.axis = GUICore::LayoutAxis::y;
        vertical_desc.main_axis_gap = 6.0f;
        GUI::end_v_layout(context, vertical, vertical_desc);

        section_heading(context, "demo.layouts.heading.grid", "GridLayout");
        GUICore::LayoutConfig grid_layout = fill_width_layout(148.0f);
        GUICore::ElementHandle grid = GUI::begin_grid_layout(context,
            make_id(context, "demo.layouts.grid"), "Grid", grid_layout);
        GUI::id_t grid_id = make_id(context, "demo.layouts.grid.items");
        for(u64 i = 0; i < 8; ++i)
        {
            c8 label[32];
            snprintf(label, sizeof(label), "Cell %llu", (unsigned long long)(i + 1));
            GUI::text_button(context, make_child_id(grid_id, i), label);
        }
        GUICore::GridLayoutDesc grid_desc;
        grid_desc.mode = GUICore::GridLayoutMode::fixed_column_count;
        grid_desc.column_count = 4;
        grid_desc.cell_size.y = 58.0f;
        grid_desc.gap = Float2U(8.0f);
        GUI::end_grid_layout(context, grid, grid_desc);

        section_heading(context, "demo.layouts.heading.canvas", "CanvasLayout");
        GUICore::ElementHandle canvas = GUI::begin_canvas_layout(context,
            make_id(context, "demo.layouts.canvas"), "Canvas", fill_width_layout(220.0f));
        GUI::id_t top_left_id = make_id(context, "demo.layouts.canvas.top_left");
        GUI::id_t center_id = make_id(context, "demo.layouts.canvas.center");
        GUI::id_t bottom_right_id = make_id(context, "demo.layouts.canvas.bottom_right");
        GUI::text_button(context, top_left_id, "Top Left", fixed_layout(120.0f, 42.0f));
        GUI::text_button(context, center_id, "Centered", fixed_layout(140.0f, 48.0f));
        GUI::text_button(context, bottom_right_id, "Bottom Right", fixed_layout(140.0f, 42.0f));
        GUICore::CanvasLayoutItem items[3];
        items[0].element_id = top_left_id;
        items[0].offset = Float4U(12.0f, 12.0f, 0.0f, 0.0f);
        items[1].element_id = center_id;
        items[1].anchor_min = Float2U(0.5f, 0.5f);
        items[1].anchor_max = items[1].anchor_min;
        items[1].pivot = Float2U(0.5f, 0.5f);
        items[2].element_id = bottom_right_id;
        items[2].anchor_min = Float2U(1.0f, 1.0f);
        items[2].anchor_max = items[2].anchor_min;
        items[2].offset = Float4U(-12.0f, -12.0f, 0.0f, 0.0f);
        items[2].pivot = Float2U(1.0f, 1.0f);
        GUICore::CanvasLayoutDesc canvas_desc;
        canvas_desc.items = Span<const GUICore::CanvasLayoutItem>(items, 3);
        GUI::end_canvas_layout(context, canvas, canvas_desc);
        GUI::end_scroll_view(context);
    }

    void add_scroll_rows(GUICore::IContext* context, GUI::id_t base_id, const c8* prefix, u32 count)
    {
        for(u32 i = 0; i < count; ++i)
        {
            c8 label[96];
            snprintf(label, sizeof(label), "%s row %02u", prefix, i + 1);
            GUI::text_button(context, make_child_id(base_id, i), label, fill_width_layout(34.0f, 3.0f));
        }
    }

    void build_scroll_page(GUICore::IContext* context)
    {
        GUICore::ElementHandle row = GUI::begin_h_layout(context, make_id(context, "demo.scroll.columns"),
            "Scroll view columns", fill_layout());
        GUI::ScrollViewDesc dynamic;
        dynamic.horizontal = false;
        GUICore::LayoutConfig column_layout = growing_layout();
        column_layout.height.kind = GUICore::SizeKind::percent;
        column_layout.height.value = 1.0f;
        GUI::begin_scroll_view(context, make_id(context, "demo.scroll.dynamic"), "Dynamic overlay scroll view",
            column_layout, dynamic);
        section_heading(context, "demo.scroll.dynamic.heading", "Dynamic Overlay");
        add_scroll_rows(context, make_id(context, "demo.scroll.dynamic.rows"), "Overlay", 40);
        GUI::end_scroll_view(context);

        GUI::ScrollViewDesc persistent;
        persistent.scrollbar_mode = GUI::ScrollBarMode::always_visible;
        persistent.horizontal = false;
        GUI::begin_scroll_view(context, make_id(context, "demo.scroll.persistent"), "Persistent scroll view",
            column_layout, persistent);
        section_heading(context, "demo.scroll.persistent.heading", "Always Visible");
        add_scroll_rows(context, make_id(context, "demo.scroll.persistent.rows"), "Persistent", 40);
        GUI::end_scroll_view(context);
        GUICore::FlexLayoutDesc row_desc;
        row_desc.axis = GUICore::LayoutAxis::x;
        row_desc.main_axis_gap = 12.0f;
        row_desc.cross_alignment = GUICore::FlexAlignment::stretch;
        GUI::end_h_layout(context, row, row_desc);
    }

    void build_tables_page(GUICore::IContext* context, DemoState& state)
    {
        begin_page(context, "demo.tables.scroll");

        section_heading(context, "demo.tables.focus.heading", "Focus Scope");
        GUI::text(context, make_id(context, "demo.tables.focus.note"),
            "Click a button, then use Tab or directional navigation. Automatic navigation stays in its scope.",
            fill_width_layout(30.0f));
        GUICore::ElementHandle scopes = GUI::begin_h_layout(context,
            make_id(context, "demo.tables.focus.scopes"), "Focus scopes", fill_width_layout(142.0f));
        for(u64 scope_index = 0; scope_index < 2; ++scope_index)
        {
            GUI::id_t scope_id = make_child_id(make_id(context, "demo.tables.focus.scope"), scope_index);
            GUICore::LayoutConfig scope_layout = fixed_layout(240.0f, 134.0f);
            scope_layout.flex_grow = 1.0f;
            GUICore::ElementHandle scope = GUI::begin_focus_scope(context, scope_id,
                scope_index == 0 ? "Focus Scope A" : "Focus Scope B", scope_layout);
            for(u64 item_index = 0; item_index < 3; ++item_index)
            {
                c8 label[48];
                snprintf(label, sizeof(label), "Scope %c / Item %llu", (c8)('A' + scope_index),
                    (unsigned long long)(item_index + 1));
                GUI::text_button(context, make_child_id(scope_id, item_index), label,
                    fill_width_layout(34.0f, 2.0f));
            }
            GUI::end_focus_scope(context, scope);
        }
        GUICore::FlexLayoutDesc scopes_desc;
        scopes_desc.axis = GUICore::LayoutAxis::x;
        scopes_desc.main_axis_gap = 12.0f;
        scopes_desc.cross_alignment = GUICore::FlexAlignment::stretch;
        GUI::end_h_layout(context, scopes, scopes_desc);

        section_heading(context, "demo.tables.table.heading", "TableLayout");
        c8 table_status[160];
        snprintf(table_status, sizeof(table_status),
            "1000 fixed-height logical rows; previous frame submitted %u rows. Drag a column separator to resize.",
            state.table_submitted_rows);
        GUI::text(context, make_id(context, "demo.tables.table.status"), table_status,
            fill_width_layout(30.0f));

        GUI::TableDesc table_desc;
        table_desc.gap = Float2U(1.0f);
        table_desc.cell_padding = Float4U(8.0f, 4.0f, 8.0f, 4.0f);
        table_desc.fixed_row_height_mode = true;
        table_desc.fixed_row_height = 30.0f;
        table_desc.virtualize_fixed_rows = true;
        table_desc.resizable_columns = true;
        table_desc.resize_handle_width = 8.0f;
        GUICore::LayoutConfig table_layout;
        table_layout.width.kind = GUICore::SizeKind::percent;
        table_layout.width.value = 1.0f;
        table_layout.height.kind = GUICore::SizeKind::fit;
        table_layout.margin = Float4U(8.0f, 4.0f, 8.0f, 8.0f);
        GUI::id_t table_id = make_id(context, "demo.tables.table");
        GUICore::ElementHandle table = GUI::begin_table_layout(context, table_id,
            "Virtualized resizable table", table_layout, table_desc);

        GUICore::TableTrackDesc columns[4];
        const f32 column_widths[4] = { 100.0f, 560.0f, 180.0f, 160.0f };
        const f32 column_minimums[4] = { 64.0f, 180.0f, 100.0f, 100.0f };
        for(usize i = 0; i < 4; ++i)
        {
            columns[i].kind = GUICore::TableTrackSizeKind::pixels;
            columns[i].value = column_widths[i];
            columns[i].min = column_minimums[i];
        }
        GUI::set_table_columns(context, Span<const GUICore::TableTrackDesc>(columns, 4));

        GUI::id_t cell_scope = make_id(context, "demo.tables.table.cells");
        u32 submitted_rows = 0;
        for(u32 row = 0; row < 1000; ++row)
        {
            bool submit_row = GUI::begin_table_row(context);
            if(submit_row)
            {
                ++submitted_rows;
                c8 index[16];
                c8 name[80];
                c8 category[32];
                c8 value[32];
                snprintf(index, sizeof(index), "%04u", row);
                snprintf(name, sizeof(name), "Generated sample row %04u", row);
                snprintf(category, sizeof(category), "Group %u", row % 12);
                snprintf(value, sizeof(value), "%u.%02u", row * 3, row % 100);
                GUI::text(context, make_child_id(cell_scope, (u64)row * 4), index);
                GUI::text(context, make_child_id(cell_scope, (u64)row * 4 + 1), name);
                GUI::text(context, make_child_id(cell_scope, (u64)row * 4 + 2), category);
                GUI::text(context, make_child_id(cell_scope, (u64)row * 4 + 3), value);
            }
            GUI::end_table_row(context);
        }
        GUI::end_table_layout(context, table);
        state.table_submitted_rows = submitted_rows;

        GUI::end_scroll_view(context);
    }

    Float2U popup_position_below(const GUICore::InteractionState& interaction)
    {
        return Float2U(
            interaction.clicked_screen_position.x - interaction.clicked_element_position.x,
            interaction.clicked_screen_position.y - interaction.clicked_element_position.y +
                interaction.clicked_element_rect.height);
    }

    void build_overlay_page(GUICore::IContext* context, DemoState& state)
    {
        begin_page(context, "demo.overlay.scroll");

        section_heading(context, "demo.overlay.combo.heading", "Combo");
        const c8* combo_items[] = { "Alpha", "Beta", "Gamma", "Delta" };
        GUI::combo(context, make_id(context, "demo.overlay.combo"), "Example Combo", &state.combo_item,
            Span<const c8*>(combo_items, 4), fill_width_layout(36.0f));

        section_heading(context, "demo.overlay.tooltip.heading", "Tooltip");
        GUICore::ElementHandle tooltip_owner = GUI::text_button(context,
            make_id(context, "demo.overlay.tooltip.owner"), "Hover for a delayed tooltip",
            fill_width_layout(38.0f));
        GUI::TooltipDesc tooltip_desc;
        tooltip_desc.delay = 0.35f;
        GUI::set_item_tooltip(context, make_id(context, "demo.overlay.tooltip"), tooltip_owner,
            "Tooltip content lives in its own layer and does not block the owner.", tooltip_desc);

        section_heading(context, "demo.overlay.popup.heading", "Popup Stack");
        GUI::id_t popup_id = make_id(context, "demo.overlay.popup");
        GUICore::ElementHandle popup_button = GUI::text_button(context,
            make_id(context, "demo.overlay.popup.open"), "Open Popup", fill_width_layout(38.0f));
        if(GUI::is_item_clicked(context, popup_button))
        {
            state.popup_position = popup_position_below(context->get_interaction_state(popup_button.id));
            GUI::open_popup(context, popup_id);
        }
        GUI::PopupDesc popup_desc;
        popup_desc.position = state.popup_position;
        popup_desc.layout = fixed_layout(300.0f, 142.0f);
        GUICore::ElementHandle popup;
        if(GUI::begin_popup(context, popup_id, popup_desc, &popup))
        {
            GUI::text(context, make_child_id(popup_id, 0), "This popup closes on outside click or Escape.",
                fill_width_layout(28.0f, 2.0f));
            GUI::id_t child_popup_id = make_child_id(popup_id, 1);
            GUICore::ElementHandle child_button = GUI::text_button(context, make_child_id(popup_id, 2),
                "Open Child Popup", fill_width_layout(34.0f, 2.0f));
            if(GUI::is_item_clicked(context, child_button))
            {
                state.child_popup_position = popup_position_below(context->get_interaction_state(child_button.id));
                GUI::open_popup(context, child_popup_id);
            }
            GUI::PopupDesc child_desc;
            child_desc.position = state.child_popup_position;
            child_desc.layout = fixed_layout(230.0f, 72.0f);
            GUICore::ElementHandle child_popup;
            if(GUI::begin_popup(context, child_popup_id, child_desc, &child_popup))
            {
                GUI::text(context, make_child_id(child_popup_id, 0), "Nested popup layer",
                    fill_width_layout(30.0f, 2.0f));
                lupanic_if_failed(GUI::end_popup(context, child_popup, RectF(0.0f, 0.0f, 230.0f, 72.0f)));
            }
            lupanic_if_failed(GUI::end_popup(context, popup, RectF(0.0f, 0.0f, 300.0f, 142.0f)));
        }

        section_heading(context, "demo.overlay.menu.heading", "Menu Bar and Menu Items");
        GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context,
            make_id(context, "demo.overlay.menu_bar"), "Showcase Menu Bar", fill_width_layout(34.0f));
        if(GUI::begin_menu(context, make_id(context, "demo.overlay.menu.file"), "File"))
        {
            GUI::MenuItemDesc new_desc;
            new_desc.shortcut = "Ctrl+N";
            GUI::menu_item(context, make_id(context, "demo.overlay.menu.file.new"), "New", false, new_desc);
            GUI::menu_separator(context, make_id(context, "demo.overlay.menu.file.separator"));
            GUI::MenuItemDesc disabled_desc;
            disabled_desc.enabled = false;
            GUI::menu_item(context, make_id(context, "demo.overlay.menu.file.disabled"), "Unavailable", false,
                disabled_desc);
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 88.0f)));
        }
        if(GUI::begin_menu(context, make_id(context, "demo.overlay.menu.view"), "View"))
        {
            GUI::menu_item(context, make_id(context, "demo.overlay.menu.view.grid"), "Show Grid",
                &state.menu_grid);
            if(GUI::begin_menu(context, make_id(context, "demo.overlay.menu.view.theme"), "Theme"))
            {
                GUI::menu_item(context, make_id(context, "demo.overlay.menu.view.theme.dark"), "Dark");
                GUI::menu_item(context, make_id(context, "demo.overlay.menu.view.theme.light"), "Light");
                lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 70.0f)));
            }
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 76.0f)));
        }
        GUI::end_menu_bar(context, menu_bar);

        GUI::end_scroll_view(context);
    }

    GUICore::ElementHandle build_frame(GUICore::IContext* context, DemoState& state, FrameHandles& handles)
    {
        context->push_layer(1, Float2U(0.0f));
        GUICore::LayoutConfig root_layout = fill_layout();
        root_layout.padding = Float4U(12.0f);
        GUICore::ElementHandle root = GUI::begin_v_layout(context, make_id(context, "demo.root"),
            "GUI Showcase Root", root_layout);
        GUI::text(context, make_id(context, "demo.title"), "Luna Editor GUI Showcase",
            fill_width_layout(34.0f, 0.0f), heading_desc(22.0f));

        GUICore::LayoutConfig tabs_layout = growing_layout();
        tabs_layout.width.kind = GUICore::SizeKind::percent;
        tabs_layout.width.value = 1.0f;
        GUICore::ElementHandle tabs = GUI::begin_tab_bar(context, make_id(context, "demo.tabs"),
            &state.selected_tab, tabs_layout);
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.primitives"), "Primitives"))
        {
            build_primitives_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.buttons"), "Buttons"))
        {
            build_buttons_page(context, state, handles);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.input"), "Input"))
        {
            build_input_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.layouts"), "Layouts"))
        {
            build_layouts_page(context);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.scroll"), "Scroll Views"))
        {
            build_scroll_page(context);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.tables"), "Tables"))
        {
            build_tables_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, make_id(context, "demo.tab.overlay"), "Overlay"))
        {
            build_overlay_page(context, state);
            GUI::end_tab_item(context);
        }
        GUI::end_tab_bar(context);
        (void)tabs;

        GUICore::FlexLayoutDesc root_flex;
        root_flex.axis = GUICore::LayoutAxis::y;
        root_flex.main_axis_gap = 8.0f;
        root_flex.cross_alignment = GUICore::FlexAlignment::stretch;
        GUI::end_v_layout(context, root, root_flex);
        context->pop_layer();
        return root;
    }

    RV create_checker_texture(DemoApp& app)
    {
        lutry
        {
            constexpr u32 texture_size = 64;
            u32 pixels[texture_size * texture_size];
            for(u32 y = 0; y < texture_size; ++y)
            {
                for(u32 x = 0; x < texture_size; ++x)
                {
                    bool checker = ((x / 8) + (y / 8)) % 2 == 0;
                    u8 r = checker ? 48 : 18;
                    u8 g = checker ? 145 : 65;
                    u8 b = checker ? 232 : 132;
                    pixels[y * texture_size + x] = ((u32)255 << 24) | ((u32)b << 16) | ((u32)g << 8) | r;
                }
            }
            RHI::IDevice* device = RHI::get_main_device();
            luset(app.state.image_texture, device->new_texture(RHI::MemoryType::local,
                RHI::TextureDesc::tex2d(RHI::Format::rgba8_unorm,
                    RHI::TextureUsageFlag::copy_dest | RHI::TextureUsageFlag::read_texture,
                    texture_size, texture_size, 1, 1)));
            Ref<RHIUtility::IResourceWriteContext> writer = RHIUtility::new_resource_write_context(device);
            u32 row_pitch = 0;
            u32 slice_pitch = 0;
            lulet(mapped, writer->write_texture(app.state.image_texture, RHI::SubresourceIndex(0, 0),
                0, 0, 0, texture_size, texture_size, 1, row_pitch, slice_pitch));
            memcpy_bitmap(mapped, pixels, texture_size * sizeof(u32), texture_size,
                row_pitch, texture_size * sizeof(u32));
            luexp(writer->commit(app.cmdbuf, true));
        }
        lucatchret;
        return ok;
    }

    void create_test_shape(DemoApp& app)
    {
        app.state.icon_shape_buffer = VG::new_shape_buffer();
        Vector<f32>& points = app.state.icon_shape_buffer->get_shape_points(true);
        app.state.icon_shape.buffer = app.state.icon_shape_buffer;
        app.state.icon_shape.first_command = (u32)points.size();
        VG::ShapeBuilder::add_circle_filled(points, 12.0f, 12.0f, 9.0f);
        app.state.icon_shape.num_commands = (u32)points.size() - app.state.icon_shape.first_command;
        app.state.icon_shape.bounds = RectF(0.0f, 0.0f, 24.0f, 24.0f);
    }

    RV init_demo(DemoApp& app)
    {
        lutry
        {
            luexp(add_modules({
                module_window(),
                module_rhi(),
                module_rhi_utility(),
                module_font(),
                module_vg(),
                GUICore::module_gui_core(),
                GUI::module_gui(),
                GUIWindow::module_gui_window()
            }));
            luexp(init_modules());

            luset(app.window, Window::new_window("Luna Editor GUI Showcase"));
            RHI::IDevice* device = RHI::get_main_device();
            for(u32 i = 0; i < device->get_num_command_queues(); ++i)
            {
                if(device->get_command_queue_desc(i).type == RHI::CommandQueueType::graphics)
                {
                    app.queue = i;
                    break;
                }
            }
            lucheck_msg(app.queue != U32_MAX, "No graphics queue available.");
            UInt2U size = app.window->get_framebuffer_size();
            luset(app.swap_chain, device->new_swap_chain(app.queue, app.window,
                RHI::SwapChainDesc({ size.x, size.y, 2, RHI::Format::bgra8_unorm, true })));
            luset(app.cmdbuf, device->new_command_buffer(app.queue));
            app.draw_list = VG::new_shape_draw_list(device);
            app.renderer = VG::new_fill_shape_renderer();
            app.gui = GUICore::new_context();
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            GUI::register_style_schemas(app.gui);
            luexp(create_checker_texture(app));
            create_test_shape(app);
        }
        lucatchret;
        return ok;
    }

    RV render_demo(DemoApp& app, RHI::ITexture* back_buffer)
    {
        lutry
        {
            luexp(app.gui->compile_draw_commands(app.draw_list));
            luexp(app.draw_list->compile());
            Span<const VG::ShapeDrawCall> draw_calls = app.draw_list->get_draw_calls();
            if(!draw_calls.empty())
            {
                GUICore::FrameDesc frame = app.gui->get_frame_desc();
                Float4x4U transform = ProjectionMatrix::make_orthographic_off_center(
                    0.0f, max(frame.screen_size.x, 1.0f),
                    0.0f, max(frame.screen_size.y, 1.0f),
                    0.0f, 1.0f);
                luexp(app.renderer->begin(back_buffer));
                app.renderer->draw(app.draw_list->get_vertex_buffer(), app.draw_list->get_index_buffer(),
                    draw_calls, &transform);
                luexp(app.renderer->end());
                app.renderer->submit(app.cmdbuf);
            }
        }
        lucatchret;
        return ok;
    }

    RV run_demo(const DemoOptions& options)
    {
        lutry
        {
            DemoApp app;
            luexp(init_demo(app));
            app.state.selected_tab = clamp(options.selected_tab, 0, 6);
            GUIWindow::GUICoreWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            i32 frame_index = 0;
            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed()) break;
                if(app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }
                UInt2U framebuffer_size = app.window->get_framebuffer_size();
                if(framebuffer_size.x && framebuffer_size.y &&
                    (framebuffer_size.x != app.width || framebuffer_size.y != app.height))
                {
                    luexp(app.swap_chain->reset({ framebuffer_size.x, framebuffer_size.y, 2,
                        RHI::Format::unknown, true }));
                    app.width = framebuffer_size.x;
                    app.height = framebuffer_size.y;
                }

                UInt2U logical_size = app.window->get_size();
                GUICore::FrameDesc frame;
                frame.screen_size = Float2U((f32)logical_size.x, (f32)logical_size.y);
                frame.framebuffer_size = framebuffer_size;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);

                FrameHandles handles;
                GUICore::ElementHandle root = build_frame(app.gui, app.state, handles);
                luexp(GUI::layout_tree(app.gui, root,
                    RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                app.gui->route_input();
                GUI::ResolveResult resolved = GUI::resolve_interactions(app.gui);
                if(GUI::is_item_valid(app.gui, handles.container_button) &&
                    GUI::is_item_clicked(app.gui, handles.container_button))
                {
                    ++app.state.button_clicks;
                }
                if(GUI::is_item_valid(app.gui, handles.selectable) &&
                    GUI::is_item_clicked(app.gui, handles.selectable))
                {
                    app.state.selectable_selected = !app.state.selectable_selected;
                }
                if(resolved.relayout_requested)
                {
                    luexp(GUI::layout_tree(app.gui, root,
                        RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(app.gui->generate_draw_commands());

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                RHI::RenderPassDesc render_pass;
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer,
                    RHI::LoadOp::clear, RHI::StoreOp::store, Float4U(0.035f, 0.045f, 0.060f, 1.0f));
                app.cmdbuf->begin_render_pass(render_pass);
                app.cmdbuf->end_render_pass();
                luexp(render_demo(app, back_buffer));
                app.cmdbuf->resource_barrier({}, {
                    { back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                        RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none }
                });
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
                ++frame_index;
                if(options.max_frames >= 0 && frame_index >= options.max_frames) break;
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    DemoOptions options;
    for(int i = 1; i < argc; ++i)
    {
        i32 value = 0;
        if(sscanf(argv[i], "--tab=%d", &value) == 1) options.selected_tab = value;
        else if(sscanf(argv[i], "--frames=%d", &value) == 1) options.max_frames = value;
    }
    Luna::init();
    lupanic_if_failed(run_demo(options));
    Luna::close();
    return 0;
}
