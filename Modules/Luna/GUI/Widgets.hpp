/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Context.hpp"
#include "State.hpp"
#include <Luna/Runtime/Math/Transform.hpp>

namespace Luna
{
    namespace GUI
    {
        enum class GizmoOperation : u32
        {
            translate = 0,
            rotate = 1,
            scale = 2,
            bounds = 3
        };

        enum class GizmoMode : u32
        {
            local = 0,
            world = 1
        };

        LUNA_GUI_API void push_id(u64 id);
        LUNA_GUI_API void push_id(const void* ptr);
        LUNA_GUI_API void push_id(const c8* str);
        LUNA_GUI_API void pop_id();
        LUNA_GUI_API void push_clip_rect(const RectF& rect);
        LUNA_GUI_API void pop_clip_rect();
        LUNA_GUI_API void tree_push();
        LUNA_GUI_API void tree_push(ItemHandle node);
        LUNA_GUI_API void tree_pop();

        LUNA_GUI_API void set_next_item_layout(const LayoutStyle& style);
        LUNA_GUI_API void set_next_canvas_item_layout(const CanvasItemLayout& layout);
        LUNA_GUI_API void set_next_dock_panel_style(const DockPanelStyle& style, bool* open = nullptr);

        LUNA_GUI_API ItemHandle begin_h_layout(const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_h_layout(const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_h_layout();
        LUNA_GUI_API ItemHandle begin_v_layout(const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_v_layout(const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_v_layout();
        LUNA_GUI_API ItemHandle begin_table_layout(const c8* label, const TableDesc& desc);
        LUNA_GUI_API void end_table_layout();
        LUNA_GUI_API void set_next_table_cell_color(const Float4U& color);
        LUNA_GUI_API ItemHandle begin_grid_layout(const c8* label, const GridLayoutDesc& desc);
        LUNA_GUI_API void end_grid_layout();
        LUNA_GUI_API ItemHandle begin_canvas_layout(const c8* label = nullptr, const Size& size = Size(), const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        LUNA_GUI_API ItemHandle begin_canvas_layout(const c8* label, const RectF& rect, const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        LUNA_GUI_API void end_canvas_layout();
        LUNA_GUI_API ItemHandle begin_dock_space(const c8* label, const Size& size = Size());
        LUNA_GUI_API void end_dock_space();
        LUNA_GUI_API ItemHandle begin_dock_panel(const c8* label, bool* open = nullptr, const DockPanelStyle& style = DockPanelStyle(), const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_dock_panel();
        LUNA_GUI_API ItemHandle begin_scroll_view(const c8* label, const Size& size);
        LUNA_GUI_API void end_scroll_view();
        LUNA_GUI_API ItemHandle begin_window(const c8* label, const Size& size = Size());
        LUNA_GUI_API ItemHandle begin_window(const c8* label, bool* open, const Size& size = Size());
        LUNA_GUI_API void end_window();
        LUNA_GUI_API ItemHandle begin_popup(const c8* label, const Float2U& position, const Size& size = Size());
        LUNA_GUI_API ItemHandle begin_popup(const c8* label, const PopupDesc& desc);
        LUNA_GUI_API void end_popup();
        LUNA_GUI_API void open_popup(ItemHandle popup);
        LUNA_GUI_API void close_popup(ItemHandle popup);
        LUNA_GUI_API void close_current_popup();
        LUNA_GUI_API void close_all_popups();
        LUNA_GUI_API bool is_popup_open(ItemHandle popup);
        LUNA_GUI_API ItemHandle begin_tooltip(ItemHandle owner, const c8* label = nullptr, const TooltipDesc& desc = TooltipDesc());
        LUNA_GUI_API void end_tooltip();
        LUNA_GUI_API ItemHandle set_item_tooltip(ItemHandle owner, const c8* text, const TooltipDesc& desc = TooltipDesc());
        LUNA_GUI_API ItemHandle begin_menu_bar(const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_menu_bar(const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_menu_bar();
        LUNA_GUI_API ItemHandle begin_menu(const c8* label, bool enabled = true);
        LUNA_GUI_API void end_menu();
        LUNA_GUI_API ItemHandle menu_item(const c8* label, const c8* shortcut = nullptr, bool selected = false, bool enabled = true);
        LUNA_GUI_API ItemHandle menu_item(const c8* label, const c8* shortcut, bool* selected, bool enabled = true);
        LUNA_GUI_API ItemHandle menu_separator();
        LUNA_GUI_API ItemHandle begin_tab_bar(const c8* label, TabBarFlag flags = TabBarFlag::fitting_shrink);
        LUNA_GUI_API void end_tab_bar();
        LUNA_GUI_API bool begin_tab_item(const c8* label, bool* open = nullptr, TabItemFlag flags = TabItemFlag::none);
        LUNA_GUI_API void end_tab_item();
        LUNA_GUI_API ItemHandle tab_item_button(const c8* label, TabItemFlag flags = TabItemFlag::none);
        LUNA_GUI_API void set_tab_item_closed(const c8* label);

        LUNA_GUI_API ItemHandle button(const c8* label);
        LUNA_GUI_API ItemHandle button(const c8* label, const RectF& rect);
        LUNA_GUI_API ItemHandle selectable(const c8* label, bool selected = false);
        LUNA_GUI_API ItemHandle text(const c8* text);
        LUNA_GUI_API ItemHandle checkbox(const c8* label, bool* value);
        LUNA_GUI_API ItemHandle radio_button(const c8* label, bool selected);
        LUNA_GUI_API ItemHandle radio_button(const c8* label, bool* value);
        LUNA_GUI_API ItemHandle radio_button(const c8* label, i32* value, i32 button_value);
        LUNA_GUI_API ItemHandle toggle_switch(const c8* label, bool* value);
        LUNA_GUI_API ItemHandle input_text(const c8* label, String& value);
        LUNA_GUI_API ItemHandle image(RHI::ITexture* texture, const Size& size, ImageFlag flags = ImageFlag::none);
        LUNA_GUI_API ItemHandle collapsing_header(const c8* label);
        LUNA_GUI_API ItemHandle tree_node(const c8* label, TreeNodeFlag flags = TreeNodeFlag::none);
        LUNA_GUI_API ItemHandle combo(const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API ItemHandle button_group(const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API ItemHandle button_group(const c8* label, Span<bool> selected, Span<const c8*> items);
        LUNA_GUI_API ItemHandle slider_float(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float2(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float3(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float4(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float_with_input(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float2_with_input(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float3_with_input(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float4_with_input(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_int(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int2(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int3(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int4(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int_with_input(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int2_with_input(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int3_with_input(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int4_with_input(const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle drag_float(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float2(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float3(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float4(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int2(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int3(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int4(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle color_edit3(const c8* label, f32* value);
        LUNA_GUI_API ItemHandle color_edit4(const c8* label, f32* value);
        LUNA_GUI_API ItemHandle color_edit3(const c8* label, u8* value);
        LUNA_GUI_API ItemHandle color_edit4(const c8* label, u8* value);
        LUNA_GUI_API ItemHandle color_edit3(const c8* label, u32* value);
        LUNA_GUI_API ItemHandle color_edit4(const c8* label, u32* value);
        LUNA_GUI_API ItemHandle gizmo(const c8* label, Float4x4& world_matrix, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect,
            GizmoOperation operation, GizmoMode mode, f32 snap = 0.0f, bool enabled = true, bool orthographic = false,
            Float4x4* delta_matrix = nullptr, bool* is_mouse_hover = nullptr, bool* is_mouse_moving = nullptr, bool* edited = nullptr);
        LUNA_GUI_API ItemHandle hit_box(const c8* label, const RectF& rect);
        LUNA_GUI_API Float2U get_pointer_position();
        LUNA_GUI_API bool is_pointer_button_down(PointerButton button);
        LUNA_GUI_API bool is_key_down(Key key);
        LUNA_GUI_API KeyModifierFlag get_key_modifiers();
        LUNA_GUI_API FrameDesc get_frame_desc();
        LUNA_GUI_API Float2U get_pointer_delta();
        LUNA_GUI_API ItemHandle draw_rect(const RectF& rect, const Float4U& color, f32 radius = 0.0f);
        LUNA_GUI_API ItemHandle draw_circle(const Float2U& center, f32 radius, const Float4U& color);
        LUNA_GUI_API ItemHandle draw_line(const Float2U& begin, const Float2U& end, const Float4U& color, f32 width = 1.0f);
        LUNA_GUI_API ItemHandle draw_text(const RectF& rect, const c8* text, const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            TextAlignment horizontal_alignment = TextAlignment::begin,
            TextAlignment vertical_alignment = TextAlignment::center);
        LUNA_GUI_API ItemHandle draw_image(RHI::ITexture* texture, const RectF& rect, const Float4U& color = Float4U(1.0f), ImageFlag flags = ImageFlag::none);
    }
}
