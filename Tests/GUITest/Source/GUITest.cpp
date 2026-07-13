/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include "../../Shared/InteractiveGUIDemo.hpp"
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Runtime/MemoryUtils.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/Window/AppMain.hpp>
#include <cstdio>
#include <cstring>

using namespace Luna;

namespace
{
    struct EditorDemoState
    {
        bool checkbox = true;
        bool toggle = false;
        bool radio_a = true;
        bool radio_b = false;
        bool multi[4] = { true, false, true, false };
        i32 radio_value = 1;
        i32 button_group_value = 0;
        i32 combo_value = 0;
        String input = "Editable text";
        String readonly_input = "Readonly text";
        f32 slider_float = 0.45f;
        f32 slider_float2[2] = { 0.25f, 0.75f };
        f32 slider_float3[3] = { 0.2f, 0.5f, 0.8f };
        f32 slider_float4[4] = { 0.15f, 0.35f, 0.65f, 0.9f };
        i32 slider_int = 42;
        i32 slider_int2[2] = { 16, 84 };
        i32 slider_int3[3] = { 8, 32, 64 };
        i32 slider_int4[4] = { 12, 36, 68, 92 };
        f32 drag_float = 12.5f;
        f32 drag_float2[2] = { -5.0f, 5.0f };
        f32 drag_float3[3] = { -1.0f, 0.0f, 1.0f };
        f32 drag_float4[4] = { 0.0f, 1.0f, 2.0f, 3.0f };
        i32 drag_int = 16;
        i32 drag_int2[2] = { -16, 16 };
        i32 drag_int3[3] = { -8, 0, 8 };
        i32 drag_int4[4] = { -4, -2, 2, 4 };
        f32 color3[3] = { 0.20f, 0.55f, 0.90f };
        f32 color4[4] = { 0.84f, 0.32f, 0.18f, 0.80f };
        u8 color3_u8[3] = { 48, 190, 126 };
        u8 color4_u8[4] = { 210, 80, 172, 200 };
        u32 color3_rgba8 = 0xCC6633FFu;
        u32 color4_rgba8 = 0xFFC020CCu;
        Ref<VG::IShapeBuffer> shape_buffer;
        GUICore::ShapeDesc icon_shape;
        Ref<RHI::ITexture> image_texture;
        bool tab_a_open = true;
        bool tab_b_open = true;
        bool tab_c_open = true;
        bool menu_show_grid = true;
        bool menu_snap = false;
        bool popup_toggle = false;
        bool dock_left_open = true;
        bool dock_right_open = true;
        bool dock_bottom_open = true;
        bool dock_layout_initialized = false;
        bool show_debug_panel = false;
    };

    GUICore::LayoutConfig row_layout()
    {
        return Test::fill_width_layout(32.0f);
    }

    GUICore::LayoutConfig tall_row_layout()
    {
        return Test::fill_width_layout(42.0f);
    }

    void text_line(GUICore::IContext* context, GUICore::id_t id, const c8* text)
    {
        GUI::text(context, id, text, row_layout());
    }

    void ensure_shapes(EditorDemoState& state)
    {
        if(state.shape_buffer)
        {
            return;
        }
        state.shape_buffer = VG::new_shape_buffer();
        Vector<f32>& points = state.shape_buffer->get_shape_points(true);
        state.icon_shape.buffer = state.shape_buffer;
        state.icon_shape.first_command = (u32)points.size();
        VG::ShapeBuilder::move_to(points, 4.0f, 14.0f);
        VG::ShapeBuilder::line_to(points, 10.0f, 8.0f);
        VG::ShapeBuilder::line_to(points, 14.0f, 12.0f);
        VG::ShapeBuilder::line_to(points, 20.0f, 6.0f);
        VG::ShapeBuilder::line_to(points, 20.0f, 12.0f);
        VG::ShapeBuilder::line_to(points, 14.0f, 18.0f);
        VG::ShapeBuilder::line_to(points, 10.0f, 14.0f);
        VG::ShapeBuilder::line_to(points, 6.0f, 18.0f);
        state.icon_shape.num_commands = (u32)points.size() - state.icon_shape.first_command;
        state.icon_shape.bounds = RectF(0.0f, 0.0f, 24.0f, 24.0f);
    }

    RV init_demo_resources(Test::InteractiveGUIDemoApp& app, void* userdata)
    {
        lutry
        {
            EditorDemoState& state = *(EditorDemoState*)userdata;
            constexpr u32 TEX_SIZE = 64;
            u32 pixels[TEX_SIZE * TEX_SIZE];
            for(u32 y = 0; y < TEX_SIZE; ++y)
            {
                for(u32 x = 0; x < TEX_SIZE; ++x)
                {
                    bool checker = ((x / 8) + (y / 8)) % 2 == 0;
                    u8 r = checker ? 48 : 18;
                    u8 g = checker ? 145 : 65;
                    u8 b = checker ? 232 : 132;
                    pixels[y * TEX_SIZE + x] = ((u32)255 << 24) | ((u32)b << 16) | ((u32)g << 8) | r;
                }
            }
            auto device = RHI::get_main_device();
            luset(state.image_texture, device->new_texture(RHI::MemoryType::local,
                RHI::TextureDesc::tex2d(RHI::Format::rgba8_unorm,
                    RHI::TextureUsageFlag::copy_dest | RHI::TextureUsageFlag::read_texture, TEX_SIZE, TEX_SIZE, 1, 1)));
            auto writer = RHIUtility::new_resource_write_context(device);
            u32 row_pitch = 0;
            u32 slice_pitch = 0;
            lulet(mapped, writer->write_texture(state.image_texture, RHI::SubresourceIndex(0, 0),
                0, 0, 0, TEX_SIZE, TEX_SIZE, 1, row_pitch, slice_pitch));
            memcpy_bitmap(mapped, pixels, TEX_SIZE * sizeof(u32), TEX_SIZE, row_pitch, TEX_SIZE * sizeof(u32));
            luexp(writer->commit(app.cmdbuf, true));
        }
        lucatchret;
        return ok;
    }

    void labeled_text(GUICore::IContext* context, GUICore::id_t id, const c8* label, const c8* value)
    {
        Test::label_value(context, id, label, value);
    }

    void end_page(GUICore::IContext* context, const GUICore::ElementHandle& body, const GUICore::ElementHandle& scroll)
    {
        GUICore::FlexLayoutDesc desc;
        desc.axis = GUICore::LayoutAxis::y;
        desc.main_axis_gap = 8.0f;
        lupanic_if_failed(GUI::end_v_layout(context, body, desc));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));
    }

    void build_buttons_page(GUICore::IContext* context, EditorDemoState& state)
    {
        ensure_shapes(state);
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.buttons.scroll"), "Buttons page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.buttons.body"), "Buttons body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.buttons.title"), "Buttons, containers and progress");

        GUI::text_button(context, Test::demo_id("gui.buttons.text"), "Text Button", row_layout());
        GUI::text_button(context, Test::demo_id("gui.buttons.disabled"), "Disabled Text Button", row_layout(), false);
        GUICore::ElementHandle shape_row = GUI::begin_h_layout(context, Test::demo_id("gui.buttons.shape.row"), "Shape row", Test::fill_width_layout(42.0f));
        GUI::shape(context, Test::demo_id("gui.buttons.shape"), state.icon_shape, Test::fixed_layout(40.0f, 36.0f));
        GUI::shape_button(context, Test::demo_id("gui.buttons.shape.button"), "Shape Button", state.icon_shape, Test::fixed_layout(80.0f, 36.0f));
        GUI::shape_button(context, Test::demo_id("gui.buttons.shape.button.disabled"), "Disabled Shape Button",
            state.icon_shape, Test::fixed_layout(80.0f, 36.0f), 6.0f, false);
        GUICore::FlexLayoutDesc shape_row_desc;
        shape_row_desc.axis = GUICore::LayoutAxis::x;
        shape_row_desc.main_axis_gap = 8.0f;
        lupanic_if_failed(GUI::end_h_layout(context, shape_row, shape_row_desc));
        GUICore::ElementHandle button = GUI::begin_button(context, Test::demo_id("gui.buttons.container"), "Container Button", tall_row_layout());
        GUI::text(context, Test::demo_id("gui.buttons.container.text"), "Button container with custom child text", Test::fill_layout());
        GUI::end_button(context);

        f32 progress = state.slider_float;
        char overlay[64];
        snprintf(overlay, sizeof(overlay), "%.0f%%", progress * 100.0f);
        GUI::progress_bar(context, Test::demo_id("gui.buttons.progress"), progress, overlay, row_layout());
        GUI::slider_float_with_input(context, Test::demo_id("gui.buttons.progress.slider"), "Progress value",
            &state.slider_float, 0.0f, 1.0f, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());

        GUICore::InteractionState interaction = context->get_interaction_state(button.id);
        char text[160];
        snprintf(text, sizeof(text), "hovered=%s active=%s clicked=%s",
            interaction.hovered ? "yes" : "no", interaction.active ? "yes" : "no", interaction.clicked ? "yes" : "no");
        labeled_text(context, Test::demo_id("gui.buttons.state"), "Container state", text);

        RectF button_rect = GUI::get_item_rect(context, button);
        RectF button_clip = GUI::get_item_clip_rect(context, button);
        snprintf(text, sizeof(text), "valid=%s clicked=%s right=%s double=%s hover=%s active=%s focused=%s",
            GUI::is_item_valid(context, button) ? "yes" : "no",
            GUI::is_item_clicked(context, button) ? "yes" : "no",
            GUI::is_item_right_clicked(context, button) ? "yes" : "no",
            GUI::is_item_double_clicked(context, button) ? "yes" : "no",
            GUI::is_item_hovered(context, button) ? "yes" : "no",
            GUI::is_item_active(context, button) ? "yes" : "no",
            GUI::is_item_focused(context, button) ? "yes" : "no");
        labeled_text(context, Test::demo_id("gui.buttons.query.state"), "Item queries", text);
        snprintf(text, sizeof(text), "rect=(%.1f %.1f %.1f %.1f) clip=(%.1f %.1f %.1f %.1f)",
            button_rect.offset_x, button_rect.offset_y, button_rect.width, button_rect.height,
            button_clip.offset_x, button_clip.offset_y, button_clip.width, button_clip.height);
        labeled_text(context, Test::demo_id("gui.buttons.query.rect"), "Last layout", text);

        GUICore::ElementHandle hit = GUI::hit_box(context, Test::demo_id("gui.buttons.hitbox"), Test::fill_width_layout(42.0f));
        GUICore::DrawCommand hit_bg;
        hit_bg.type = GUICore::DrawCommandType::rounded_rect;
        hit_bg.rect_reference = GUICore::DrawCommandRectReference::element;
        hit_bg.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        hit_bg.color = GUI::is_item_hovered(context, hit) ? Float4U(0.16f, 0.34f, 0.48f, 1.0f) : Float4U(0.08f, 0.12f, 0.16f, 1.0f);
        hit_bg.radius = 5.0f;
        context->draw_for_element(hit, hit_bg);
        GUICore::DrawCommand hit_text;
        hit_text.type = GUICore::DrawCommandType::text;
        hit_text.rect_reference = GUICore::DrawCommandRectReference::element;
        hit_text.rect = RectF(12.0f, 0.0f, -24.0f, 0.0f);
        hit_text.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        hit_text.color = Float4U(0.90f, 0.93f, 0.96f, 1.0f);
        hit_text.font_size = 15.0f;
        hit_text.text = "Invisible hit_box with custom element-relative drawing";
        context->draw_for_element(hit, hit_text);
        snprintf(text, sizeof(text), "hovered=%s clicked=%s focused=%s",
            GUI::is_item_hovered(context, hit) ? "yes" : "no",
            GUI::is_item_clicked(context, hit) ? "yes" : "no",
            GUI::is_item_focused(context, hit) ? "yes" : "no");
        labeled_text(context, Test::demo_id("gui.buttons.hitbox.state"), "HitBox state", text);
        end_page(context, body, scroll);
    }

    void build_selection_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.select.scroll"), "Selection page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.select.body"), "Selection body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.select.title"), "Selection widgets");
        GUI::checkbox(context, Test::demo_id("gui.select.checkbox"), "Checkbox", &state.checkbox, row_layout());
        GUI::checkbox(context, Test::demo_id("gui.select.checkbox.disabled"), "Disabled Checkbox", true, row_layout(), false);
        GUI::toggle_switch(context, Test::demo_id("gui.select.switch"), "Switch", &state.toggle, row_layout());
        GUI::toggle_switch(context, Test::demo_id("gui.select.switch.disabled"), "Disabled Switch", true, row_layout(), false);
        GUI::radio_button(context, Test::demo_id("gui.select.radio.a"), "Radio bool A", &state.radio_a, row_layout());
        GUI::radio_button(context, Test::demo_id("gui.select.radio.b"), "Radio bool B", &state.radio_b, row_layout());
        GUI::radio_button(context, Test::demo_id("gui.select.radio.disabled"), "Disabled Radio", true, row_layout(), false);
        GUI::radio_button(context, Test::demo_id("gui.select.radio.0"), "Radio integer 0", &state.radio_value, 0, row_layout());
        GUI::radio_button(context, Test::demo_id("gui.select.radio.1"), "Radio integer 1", &state.radio_value, 1, row_layout());
        GUI::selectable(context, Test::demo_id("gui.select.selectable"), "Selectable row", state.checkbox, row_layout());
        GUI::selectable(context, Test::demo_id("gui.select.selectable.disabled"), "Disabled selectable", true, row_layout(), false);
        const c8* group_items[] = { "Title", "Settings", "Advanced", "Output" };
        GUI::button_group(context, Test::demo_id("gui.select.group.single"), &state.button_group_value,
            Span<const c8*>(group_items, 4), row_layout());
        GUI::button_group(context, Test::demo_id("gui.select.group.multi"), Span<bool>(state.multi, 4),
            Span<const c8*>(group_items, 4), row_layout());
        if(GUI::collapsing_header(context, Test::demo_id("gui.select.header"), "Collapsing Header", true, row_layout()))
        {
            text_line(context, Test::demo_id("gui.select.header.text"), "Header content is built only when open.");
        }
        if(GUI::tree_node(context, Test::demo_id("gui.select.tree"), "Tree Node", GUI::TreeNodeFlag::default_open, 0, row_layout()))
        {
            GUI::tree_node(context, Test::demo_id("gui.select.tree.leaf.a"), "Leaf A", GUI::TreeNodeFlag::leaf, 1, row_layout());
            GUI::tree_node(context, Test::demo_id("gui.select.tree.leaf.b"), "Leaf B", GUI::TreeNodeFlag::leaf | GUI::TreeNodeFlag::selected, 1, row_layout());
        }
        end_page(context, body, scroll);
    }

    void build_input_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.input.scroll"), "Input page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.input.body"), "Input body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.input.title"), "Text, numeric sliders and drags");
        GUI::input_text(context, Test::demo_id("gui.input.text"), state.input, row_layout());
        GUI::input_text(context, Test::demo_id("gui.input.readonly"), state.readonly_input, row_layout(), true, true);
        GUI::slider_float(context, Test::demo_id("gui.input.slider.float"), &state.slider_float, 0.0f, 1.0f, row_layout());
        GUI::slider_float2(context, Test::demo_id("gui.input.slider.float2"), state.slider_float2, 0.0f, 1.0f, row_layout());
        GUI::slider_float3(context, Test::demo_id("gui.input.slider.float3"), state.slider_float3, 0.0f, 1.0f, row_layout());
        GUI::slider_float4(context, Test::demo_id("gui.input.slider.float4"), state.slider_float4, 0.0f, 1.0f, row_layout());
        GUI::slider_int(context, Test::demo_id("gui.input.slider.int"), &state.slider_int, 0, 100, row_layout());
        GUI::slider_int2(context, Test::demo_id("gui.input.slider.int2"), state.slider_int2, 0, 100, row_layout());
        GUI::slider_int3(context, Test::demo_id("gui.input.slider.int3"), state.slider_int3, 0, 100, row_layout());
        GUI::slider_int4(context, Test::demo_id("gui.input.slider.int4"), state.slider_int4, 0, 100, row_layout());
        GUI::slider_float_with_input(context, Test::demo_id("gui.input.slider.float.with.input"), "SliderFloatWithInput",
            &state.slider_float, 0.0f, 1.0f, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_float2_with_input(context, Test::demo_id("gui.input.slider.float2.with.input"), "SliderFloat2WithInput",
            state.slider_float2, 0.0f, 1.0f, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_float3_with_input(context, Test::demo_id("gui.input.slider.float3.with.input"), "SliderFloat3WithInput",
            state.slider_float3, 0.0f, 1.0f, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_float4_with_input(context, Test::demo_id("gui.input.slider.float4.with.input"), "SliderFloat4WithInput",
            state.slider_float4, 0.0f, 1.0f, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_int_with_input(context, Test::demo_id("gui.input.slider.int.with.input"), "SliderIntWithInput",
            &state.slider_int, 0, 100, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_int2_with_input(context, Test::demo_id("gui.input.slider.int2.with.input"), "SliderInt2WithInput",
            state.slider_int2, 0, 100, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_int3_with_input(context, Test::demo_id("gui.input.slider.int3.with.input"), "SliderInt3WithInput",
            state.slider_int3, 0, 100, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::slider_int4_with_input(context, Test::demo_id("gui.input.slider.int4.with.input"), "SliderInt4WithInput",
            state.slider_int4, 0, 100, RectF(0.0f, 0.0f, 420.0f, 30.0f), row_layout());
        GUI::drag_float(context, Test::demo_id("gui.input.drag.float"), &state.drag_float, 0.1f, -100.0f, 100.0f, row_layout());
        GUI::drag_float2(context, Test::demo_id("gui.input.drag.float2"), state.drag_float2, 0.05f, -10.0f, 10.0f, row_layout());
        GUI::drag_float3(context, Test::demo_id("gui.input.drag.float3"), state.drag_float3, 0.05f, -10.0f, 10.0f, row_layout());
        GUI::drag_float4(context, Test::demo_id("gui.input.drag.float4"), state.drag_float4, 0.05f, -10.0f, 10.0f, row_layout());
        GUI::drag_int(context, Test::demo_id("gui.input.drag.int"), &state.drag_int, 1.0f, -100, 100, row_layout());
        GUI::drag_int2(context, Test::demo_id("gui.input.drag.int2"), state.drag_int2, 1.0f, -100, 100, row_layout());
        GUI::drag_int3(context, Test::demo_id("gui.input.drag.int3"), state.drag_int3, 1.0f, -100, 100, row_layout());
        GUI::drag_int4(context, Test::demo_id("gui.input.drag.int4"), state.drag_int4, 1.0f, -100, 100, row_layout());
        end_page(context, body, scroll);
    }

    void build_image_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.image.scroll"), "Image page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.image.body"), "Image body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.image.title"), "Image widget and absolute draw_image command");
        if(state.image_texture)
        {
            GUI::image(context, Test::demo_id("gui.image.widget"), state.image_texture, Test::fixed_layout(160.0f, 160.0f), GUI::ImageFlag::nearest);
            GUICore::ElementHandle canvas = GUI::begin_canvas_layout(context, Test::demo_id("gui.image.canvas"), "Image canvas", Test::fill_width_layout(220.0f));
            GUI::draw_rect(context, Test::demo_id("gui.image.canvas.bg"), RectF(0.0f, 0.0f, 360.0f, 200.0f),
                Float4U(0.08f, 0.10f, 0.13f, 1.0f), 6.0f);
            GUI::draw_image(context, Test::demo_id("gui.image.draw.normal"), state.image_texture,
                RectF(24.0f, 24.0f, 96.0f, 96.0f), Float4U(1.0f), GUI::ImageFlag::nearest);
            GUI::draw_image(context, Test::demo_id("gui.image.draw.flip"), state.image_texture,
                RectF(150.0f, 24.0f, 96.0f, 96.0f), Float4U(1.0f), GUI::ImageFlag::nearest | GUI::ImageFlag::flip_y);
            GUI::draw_text(context, Test::demo_id("gui.image.draw.label"), RectF(24.0f, 140.0f, 300.0f, 32.0f),
                "left: normal, right: flip_y", Float4U(0.88f, 0.91f, 0.95f, 1.0f), 16.0f);
            GUICore::CanvasLayoutDesc canvas_desc;
            lupanic_if_failed(GUI::end_canvas_layout(context, canvas, canvas_desc));
        }
        else
        {
            text_line(context, Test::demo_id("gui.image.missing"), "Texture initialization failed.");
        }
        end_page(context, body, scroll);
    }

    void build_color_combo_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.color.scroll"), "Color page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.color.body"), "Color body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.color.title"), "Combo and color edit views");
        const c8* items[] = { "Alpha", "Beta", "Gamma", "Delta" };
        GUI::combo(context, Test::demo_id("gui.color.combo"), "Combo dropdown", &state.combo_value, Span<const c8*>(items, 4), row_layout());
        GUI::color_edit3(context, Test::demo_id("gui.color.f32.3"), "ColorEdit3 f32", state.color3, row_layout());
        GUI::color_edit4(context, Test::demo_id("gui.color.f32.4"), "ColorEdit4 f32", state.color4, row_layout());
        GUI::color_edit3(context, Test::demo_id("gui.color.u8.3"), "ColorEdit3 u8", state.color3_u8, row_layout());
        GUI::color_edit4(context, Test::demo_id("gui.color.u8.4"), "ColorEdit4 u8", state.color4_u8, row_layout());
        GUI::color_edit3(context, Test::demo_id("gui.color.rgba8.3"), "ColorEdit3 RGBA8", &state.color3_rgba8, row_layout());
        GUI::color_edit4(context, Test::demo_id("gui.color.rgba8.4"), "ColorEdit4 RGBA8", &state.color4_rgba8, row_layout());
        end_page(context, body, scroll);
    }

    void build_layout_page(GUICore::IContext* context)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.layout.scroll"), "Layout page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.layout.body"), "Layout body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.layout.title"), "Editor layout helpers");
        GUICore::ElementHandle h = GUI::begin_h_layout(context, Test::demo_id("gui.layout.h"), "HLayout", row_layout());
        GUI::text_button(context, Test::demo_id("gui.layout.h.0"), "Fixed", Test::fixed_layout(120.0f, 28.0f));
        GUI::text_button(context, Test::demo_id("gui.layout.h.1"), "Fill", Test::fill_width_layout(28.0f));
        GUI::text_button(context, Test::demo_id("gui.layout.h.2"), "Fixed", Test::fixed_layout(120.0f, 28.0f));
        GUICore::FlexLayoutDesc h_desc;
        h_desc.axis = GUICore::LayoutAxis::x;
        h_desc.main_axis_gap = 8.0f;
        lupanic_if_failed(GUI::end_h_layout(context, h, h_desc));

        GUICore::ElementHandle grid = GUI::begin_grid_layout(context, Test::demo_id("gui.layout.grid"), "GridLayout", Test::fill_width_layout(180.0f));
        for(u32 i = 0; i < 10; ++i)
        {
            char label[24];
            snprintf(label, sizeof(label), "Tile %u", i);
            GUI::text_button(context, Test::demo_id("gui.layout.grid.tile", i), label, Test::fill_layout());
        }
        GUICore::GridLayoutDesc grid_desc;
        grid_desc.mode = GUICore::GridLayoutMode::fixed_column_count;
        grid_desc.column_count = 5;
        grid_desc.cell_size.y = 42.0f;
        grid_desc.gap = Float2U(8.0f, 8.0f);
        lupanic_if_failed(GUI::end_grid_layout(context, grid, grid_desc));

        GUICore::ElementHandle table = GUI::begin_table_layout(context, Test::demo_id("gui.layout.table"), "TableLayout", Test::fill_width_layout(180.0f));
        GUICore::TableTrackDesc cols[4];
        cols[0].kind = GUICore::TableTrackSizeKind::pixels;
        cols[0].value = 90.0f;
        cols[1].kind = GUICore::TableTrackSizeKind::ratio;
        cols[1].value = 1.0f;
        cols[2].kind = GUICore::TableTrackSizeKind::pixels;
        cols[2].value = 120.0f;
        cols[3].kind = GUICore::TableTrackSizeKind::pixels;
        cols[3].value = 90.0f;
        GUI::set_table_columns(context, Span<const GUICore::TableTrackDesc>(cols, 4));
        GUI::set_table_gap(context, Float2U(3.0f, 3.0f));
        GUI::set_table_cell_padding(context, Float4U(4.0f, 2.0f, 4.0f, 2.0f));
        for(u32 row = 0; row < 5; ++row)
        {
            GUI::begin_table_row(context, GUICore::TableTrackDesc { GUICore::TableTrackSizeKind::pixels, 30.0f, 0.0f, -1.0f });
            for(u32 col = 0; col < 4; ++col)
            {
                char label[24];
                snprintf(label, sizeof(label), "%u:%u", row, col);
                GUI::text(context, Test::demo_id("gui.layout.table.cell", row * 8 + col), label, Test::fill_layout());
            }
            GUI::end_table_row(context);
        }
        lupanic_if_failed(GUI::end_table_layout(context, table));
        end_page(context, body, scroll);
    }

    void build_overlay_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.overlay.scroll"), "Overlay page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.overlay.body"), "Overlay body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.overlay.title"), "Menus, popups, tooltips and nested tabs");
        GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, Test::demo_id("gui.overlay.menu.bar"), "Menu Bar", row_layout());
        if(GUI::begin_menu(context, Test::demo_id("gui.overlay.menu.file"), "File"))
        {
            GUI::menu_item(context, Test::demo_id("gui.overlay.menu.file.new"), "New", "Ctrl+N");
            GUI::menu_item(context, Test::demo_id("gui.overlay.menu.file.open"), "Open", "Ctrl+O");
            GUI::menu_separator(context, Test::demo_id("gui.overlay.menu.file.sep"));
            GUI::menu_item(context, Test::demo_id("gui.overlay.menu.file.enabled"), "Enabled Item");
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 160.0f)));
        }
        if(GUI::begin_menu(context, Test::demo_id("gui.overlay.menu.view"), "View"))
        {
            GUI::menu_item(context, Test::demo_id("gui.overlay.menu.view.grid"), "Show Grid", nullptr, &state.menu_show_grid);
            GUI::menu_item(context, Test::demo_id("gui.overlay.menu.view.snap"), "Snap To Grid", nullptr, &state.menu_snap);
            if(GUI::begin_menu(context, Test::demo_id("gui.overlay.menu.view.theme"), "Theme"))
            {
                GUI::menu_item(context, Test::demo_id("gui.overlay.menu.view.theme.dark"), "Dark");
                GUI::menu_item(context, Test::demo_id("gui.overlay.menu.view.theme.light"), "Light", nullptr, false, false);
                lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 180.0f, 100.0f)));
            }
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 240.0f, 130.0f)));
        }
        lupanic_if_failed(GUI::end_menu_bar(context, menu_bar));

        GUICore::ElementHandle popup_button = GUI::text_button(context, Test::demo_id("gui.overlay.popup.button"), "Open Popup", row_layout());
        if(GUI::is_item_clicked(context, popup_button))
        {
            GUI::open_popup(context, Test::demo_id("gui.overlay.popup"));
        }
        labeled_text(context, Test::demo_id("gui.overlay.popup.state"), "Popup open",
            GUI::is_popup_open(context, Test::demo_id("gui.overlay.popup")) ? "yes" : "no");
        GUI::PopupDesc popup_desc;
        popup_desc.position = context->get_pointer_position();
        popup_desc.layout = Test::fixed_layout(260.0f, 140.0f);
        GUICore::ElementHandle popup;
        if(GUI::begin_popup(context, Test::demo_id("gui.overlay.popup"), popup_desc, &popup))
        {
            GUI::text(context, Test::demo_id("gui.overlay.popup.text"), "Popup content", row_layout());
            GUI::checkbox(context, Test::demo_id("gui.overlay.popup.check"), "Popup checkbox", &state.popup_toggle, row_layout());
            GUICore::ElementHandle close_button = GUI::text_button(context, Test::demo_id("gui.overlay.popup.close"), "Close Popup", row_layout());
            if(GUI::is_item_clicked(context, close_button))
            {
                GUI::close_popup(context, Test::demo_id("gui.overlay.popup"));
            }
            lupanic_if_failed(GUI::end_popup(context, popup, RectF(0.0f, 0.0f, 260.0f, 140.0f)));
        }

        GUICore::ElementHandle tooltip_owner = GUI::text_button(context, Test::demo_id("gui.overlay.tooltip.owner"), "Hover for tooltip", row_layout());
        GUI::set_item_tooltip(context, Test::demo_id("gui.overlay.tooltip"), tooltip_owner,
            "Tooltip content is rendered in its own layer.");
        GUICore::ElementHandle custom_tooltip_owner = GUI::text_button(context, Test::demo_id("gui.overlay.tooltip.custom.owner"),
            "Hover for custom tooltip", row_layout());
        GUI::TooltipDesc custom_tooltip_desc;
        custom_tooltip_desc.delay = 0.0f;
        custom_tooltip_desc.layout = Test::fixed_layout(310.0f, 86.0f);
        GUICore::ElementHandle custom_tooltip;
        if(GUI::begin_tooltip(context, Test::demo_id("gui.overlay.tooltip.custom"), custom_tooltip_owner,
            custom_tooltip_desc, &custom_tooltip))
        {
            GUI::text(context, Test::demo_id("gui.overlay.tooltip.custom.title"), "Custom tooltip layer", row_layout());
            GUI::checkbox(context, Test::demo_id("gui.overlay.tooltip.custom.check"), "Interactive content can be built here",
                &state.popup_toggle, row_layout());
            lupanic_if_failed(GUI::end_tooltip(context, custom_tooltip, RectF(0.0f, 0.0f, 310.0f, 86.0f)));
        }

        GUICore::ElementHandle nested_tabs = GUI::begin_tab_bar(context, Test::demo_id("gui.overlay.tabs"), "Nested Tabs",
            GUI::TabBarFlag::fitting_shrink | GUI::TabBarFlag::reorderable | GUI::TabBarFlag::auto_select_new_tabs,
            Test::fill_width_layout(170.0f));
        if(GUI::begin_tab_item(context, Test::demo_id("gui.overlay.tab.a"), "Tab A", &state.tab_a_open,
            GUI::TabItemFlag::selected))
        {
            text_line(context, Test::demo_id("gui.overlay.tab.a.text"), "Tab A content");
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.overlay.tab.b"), "Tab B", &state.tab_b_open,
            GUI::TabItemFlag::unsaved_document))
        {
            text_line(context, Test::demo_id("gui.overlay.tab.b.text"), "Tab B content, marked as unsaved");
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.overlay.tab.c"), "Pinned", &state.tab_c_open,
            GUI::TabItemFlag::no_close_button | GUI::TabItemFlag::no_reorder))
        {
            text_line(context, Test::demo_id("gui.overlay.tab.c.text"), "Pinned tab has no close button and cannot reorder.");
            GUI::end_tab_item(context);
        }
        GUI::begin_tab_item(context, Test::demo_id("gui.overlay.tab.button"), "+", nullptr, GUI::TabItemFlag::button);
        lupanic_if_failed(GUI::end_tab_bar(context, nested_tabs));

        end_page(context, body, scroll);
    }

    void init_dock_layout(GUICore::IContext* context, EditorDemoState& state)
    {
        if(state.dock_layout_initialized)
        {
            return;
        }
        GUI::DockSpaceLayoutDesc layout;
        layout.nodes.resize(3);
        layout.root_node = 0;
        layout.nodes[0].split = true;
        layout.nodes[0].split_axis = GUI::DockSplitAxis::x;
        layout.nodes[0].split_ratio = 0.62f;
        layout.nodes[0].child0 = 1;
        layout.nodes[0].child1 = 2;
        layout.nodes[1].tabs.push_back(Test::demo_id("gui.dock.left"));
        layout.nodes[1].tabs.push_back(Test::demo_id("gui.dock.bottom"));
        layout.nodes[1].selected_tab = Test::demo_id("gui.dock.left");
        layout.nodes[2].tabs.push_back(Test::demo_id("gui.dock.right"));
        layout.nodes[2].selected_tab = Test::demo_id("gui.dock.right");
        GUI::set_dockspace_layout(context, Test::demo_id("gui.dock.space"), layout);
        state.dock_layout_initialized = true;
    }

    void build_dock_page(GUICore::IContext* context, EditorDemoState& state, const Float2U& surface_size)
    {
        init_dock_layout(context, state);
        GUI::draw_text(context, Test::demo_id("gui.dock.title"), RectF(24.0f, 92.0f, 680.0f, 26.0f),
            "DockSpace and DockPanel: drag title bars, resize floating panels and splitters.",
            Float4U(0.90f, 0.92f, 0.95f, 1.0f), 16.0f);
        RectF dock_rect(24.0f, 126.0f, max(surface_size.x - 48.0f, 320.0f), max(surface_size.y - 150.0f, 260.0f));
        GUICore::ElementHandle dock = GUI::begin_dock_space(context, Test::demo_id("gui.dock.space"), "Dock demo", Test::fixed_layout(dock_rect.width, dock_rect.height));
        if(GUI::begin_dock_panel(context, Test::demo_id("gui.dock.left"), "Hierarchy", &state.dock_left_open))
        {
            GUI::text(context, Test::demo_id("gui.dock.left.text"), "Docked panel content", Test::fill_width_layout(28.0f));
            GUI::text_button(context, Test::demo_id("gui.dock.left.button"), "Panel Button", Test::fill_width_layout(30.0f));
            GUI::end_dock_panel(context);
        }
        if(GUI::begin_dock_panel(context, Test::demo_id("gui.dock.bottom"), "Inspector", &state.dock_bottom_open))
        {
            GUI::checkbox(context, Test::demo_id("gui.dock.bottom.check"), "Panel checkbox", &state.checkbox, Test::fill_width_layout(30.0f));
            GUI::end_dock_panel(context);
        }
        if(GUI::begin_dock_panel(context, Test::demo_id("gui.dock.right"), "Properties", &state.dock_right_open))
        {
            GUI::input_text(context, Test::demo_id("gui.dock.right.input"), state.input, Test::fill_width_layout(32.0f));
            GUI::slider_float(context, Test::demo_id("gui.dock.right.slider"), &state.slider_float, 0.0f, 1.0f, Test::fill_width_layout(30.0f));
            GUI::end_dock_panel(context);
        }
        lupanic_if_failed(GUI::end_dock_space(context, dock, dock_rect));
    }

    void build_debug_page(GUICore::IContext* context, EditorDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("gui.debug.scroll"), "Debug page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("gui.debug.body"), "Debug body", Test::fill_layout());
        text_line(context, Test::demo_id("gui.debug.title"), "Debug panel toggle");
        GUI::checkbox(context, Test::demo_id("gui.debug.toggle"), "Show Debug Panel", &state.show_debug_panel, row_layout());
        GUICore::PerformanceCounters counters = context->get_performance_counters();
        char text[160];
        snprintf(text, sizeof(text), "elements=%u draw_commands=%u route=%.3f ms render=%.3f ms",
            counters.element_count, counters.draw_command_count, counters.input_route_ms, counters.draw_compile_ms);
        labeled_text(context, Test::demo_id("gui.debug.counters"), "Previous frame", text);
        end_page(context, body, scroll);

        if(state.show_debug_panel)
        {
            GUI::show_debug_panel(context, Test::demo_id("gui.debug.panel"), Test::fixed_layout(720.0f, 480.0f));
        }
    }

    void build_demo(GUICore::IContext* context, const GUICore::ElementHandle& root, const Float2U& surface_size, void* userdata)
    {
        EditorDemoState& state = *(EditorDemoState*)userdata;
        (void)root;
        GUI::draw_rect(context, Test::demo_id("gui.background"), RectF(0.0f, 0.0f, surface_size.x, surface_size.y),
            Float4U(0.045f, 0.055f, 0.070f, 1.0f));
        GUI::draw_text(context, Test::demo_id("gui.title"), RectF(14.0f, 8.0f, surface_size.x - 28.0f, 28.0f),
            "Luna Editor GUI Interactive Test", Float4U(0.92f, 0.94f, 0.96f, 1.0f), 18.0f);

        GUICore::ElementHandle tabs = GUI::begin_tab_bar(context, Test::demo_id("gui.tabs"), "Widget Tabs");
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.buttons"), "Buttons"))
        {
            build_buttons_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.selection"), "Selection"))
        {
            build_selection_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.input"), "Input/Numeric"))
        {
            build_input_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.color"), "Combo/Color"))
        {
            build_color_combo_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.image"), "Image"))
        {
            build_image_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.layout"), "Layout/Table"))
        {
            build_layout_page(context);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.overlay"), "Overlay"))
        {
            build_overlay_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.dock"), "Dock"))
        {
            build_dock_page(context, state, surface_size);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("gui.tab.debug"), "Debug"))
        {
            build_debug_page(context, state);
            GUI::end_tab_item(context);
        }
        lupanic_if_failed(GUI::end_tab_bar(context, tabs, RectF(12.0f, 44.0f,
            max(surface_size.x - 24.0f, 1.0f), max(surface_size.y - 56.0f, 1.0f))));
    }
}

int luna_main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;
    if(!Luna::init())
    {
        return -1;
    }
    i32 exit_code = 0;
    {
        EditorDemoState state;
        Test::InteractiveGUIDemoDesc desc;
        desc.title = "Luna Editor GUI Interactive Test";
        desc.init = init_demo_resources;
        desc.build = build_demo;
        desc.userdata = &state;
        RV r = Test::run_interactive_gui_demo(desc);
        if(failed(r))
        {
            log_error("GUITest", "%s", explain(r.errcode()));
            exit_code = -1;
        }
    }
    Luna::close();
    return exit_code;
}
