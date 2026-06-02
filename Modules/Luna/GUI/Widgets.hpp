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

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void push_id(IContext* context, u64 id);
        LUNA_GUI_API void push_id(IContext* context, const void* ptr);
        LUNA_GUI_API void push_id(IContext* context, const c8* str);
        LUNA_GUI_API void pop_id(IContext* context);
        LUNA_GUI_API void push_layer(IContext* context, id_t id, const Float2U& screen_position = Float2U(0.0f));
        LUNA_GUI_API void pop_layer(IContext* context);
        LUNA_GUI_API void push_clip_rect(IContext* context, const RectF& rect);
        LUNA_GUI_API void pop_clip_rect(IContext* context);
        LUNA_GUI_API void tree_push(IContext* context);
        LUNA_GUI_API void tree_push(IContext* context, ItemHandle node);
        LUNA_GUI_API void tree_pop(IContext* context);
        LUNA_GUI_API ItemHandle custom_node(IContext* context, Ref<Node> node, const c8* label = nullptr, bool interactive = false);

        LUNA_GUI_API void set_next_item_layout(IContext* context, const LayoutStyle& style);
        LUNA_GUI_API void set_next_canvas_item_layout(IContext* context, const CanvasItemLayout& layout);
        LUNA_GUI_API void set_next_dock_panel_style(IContext* context, const DockPanelStyle& style, bool* open = nullptr);

        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_h_layout(IContext* context);
        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_v_layout(IContext* context);
        LUNA_GUI_API ItemHandle begin_table_layout(IContext* context, const c8* label, const TableDesc& desc);
        LUNA_GUI_API void end_table_layout(IContext* context);
        LUNA_GUI_API void set_next_table_cell_color(IContext* context, const Float4U& color);
        LUNA_GUI_API ItemHandle begin_grid_layout(IContext* context, const c8* label, const GridLayoutDesc& desc);
        LUNA_GUI_API void end_grid_layout(IContext* context);
        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label = nullptr, const Size& size = Size(), const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label, const RectF& rect, const CanvasLayoutDesc& desc = CanvasLayoutDesc());
        LUNA_GUI_API void end_canvas_layout(IContext* context);
        LUNA_GUI_API ItemHandle begin_dock_space(IContext* context, const c8* label, const Size& size = Size());
        LUNA_GUI_API void end_dock_space(IContext* context);
        LUNA_GUI_API ItemHandle begin_dock_panel(IContext* context, const c8* label, bool* open = nullptr, const DockPanelStyle& style = DockPanelStyle(), const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_dock_panel(IContext* context);
        LUNA_GUI_API ItemHandle begin_scroll_view(IContext* context, const c8* label, const Size& size);
        LUNA_GUI_API void end_scroll_view(IContext* context);
        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, const Size& size = Size());
        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, bool* open, const Size& size = Size());
        LUNA_GUI_API void end_window(IContext* context);
        LUNA_GUI_API ItemHandle begin_popup(IContext* context, const c8* label, const Float2U& position, const Size& size = Size());
        LUNA_GUI_API ItemHandle begin_popup(IContext* context, const c8* label, const PopupDesc& desc);
        LUNA_GUI_API void end_popup(IContext* context);
        LUNA_GUI_API void open_popup(IContext* context, ItemHandle popup);
        LUNA_GUI_API void close_popup(IContext* context, ItemHandle popup);
        LUNA_GUI_API void close_current_popup(IContext* context);
        LUNA_GUI_API void close_all_popups(IContext* context);
        LUNA_GUI_API bool is_popup_open(IContext* context, ItemHandle popup);
        LUNA_GUI_API ItemHandle begin_tooltip(IContext* context, ItemHandle owner, const c8* label = nullptr, const TooltipDesc& desc = TooltipDesc());
        LUNA_GUI_API void end_tooltip(IContext* context);
        LUNA_GUI_API ItemHandle set_item_tooltip(IContext* context, ItemHandle owner, const c8* text, const TooltipDesc& desc = TooltipDesc());
        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label = nullptr, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc = LayoutDesc());
        LUNA_GUI_API void end_menu_bar(IContext* context);
        LUNA_GUI_API ItemHandle begin_menu(IContext* context, const c8* label, bool enabled = true);
        LUNA_GUI_API void end_menu(IContext* context);
        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut = nullptr, bool selected = false, bool enabled = true);
        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool* selected, bool enabled = true);
        LUNA_GUI_API ItemHandle menu_separator(IContext* context);
        LUNA_GUI_API ItemHandle begin_tab_bar(IContext* context, const c8* label, TabBarFlag flags = TabBarFlag::fitting_shrink);
        LUNA_GUI_API void end_tab_bar(IContext* context);
        LUNA_GUI_API bool begin_tab_item(IContext* context, const c8* label, bool* open = nullptr, TabItemFlag flags = TabItemFlag::none);
        LUNA_GUI_API void end_tab_item(IContext* context);
        LUNA_GUI_API ItemHandle tab_item_button(IContext* context, const c8* label, TabItemFlag flags = TabItemFlag::none);
        LUNA_GUI_API void set_tab_item_closed(IContext* context, const c8* label);

        LUNA_GUI_API ItemHandle button(IContext* context, const c8* label);
        LUNA_GUI_API ItemHandle button(IContext* context, const c8* label, const RectF& rect);
        LUNA_GUI_API ItemHandle selectable(IContext* context, const c8* label, bool selected = false);
        LUNA_GUI_API ItemHandle text(IContext* context, const c8* text);
        LUNA_GUI_API ItemHandle checkbox(IContext* context, const c8* label, bool* value);
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool selected);
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool* value);
        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, i32* value, i32 button_value);
        LUNA_GUI_API ItemHandle toggle_switch(IContext* context, const c8* label, bool* value);
        LUNA_GUI_API ItemHandle input_text(IContext* context, const c8* label, String& value);
        LUNA_GUI_API ItemHandle image(IContext* context, RHI::ITexture* texture, const Size& size, ImageFlag flags = ImageFlag::none);
        LUNA_GUI_API ItemHandle collapsing_header(IContext* context, const c8* label);
        LUNA_GUI_API ItemHandle tree_node(IContext* context, const c8* label, TreeNodeFlag flags = TreeNodeFlag::none);
        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, Span<bool> selected, Span<const c8*> items);
        LUNA_GUI_API ItemHandle slider_float(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float2(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float3(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_float4(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API ItemHandle slider_int(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int2(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int3(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle slider_int4(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value);
        LUNA_GUI_API ItemHandle drag_float(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float2(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float3(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_float4(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int2(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int3(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle drag_int4(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags = NumericEditFlag::none);
        LUNA_GUI_API ItemHandle hit_box(IContext* context, const c8* label, const RectF& rect);
        LUNA_GUI_API Float2U get_pointer_position(IContext* context);
        LUNA_GUI_API bool is_pointer_button_down(IContext* context, PointerButton button);
        LUNA_GUI_API bool is_key_down(IContext* context, Key key);
        LUNA_GUI_API KeyModifierFlag get_key_modifiers(IContext* context);
        LUNA_GUI_API FrameDesc get_frame_desc(IContext* context);
        LUNA_GUI_API Float2U get_pointer_delta(IContext* context);
        LUNA_GUI_API ItemHandle draw_rect(IContext* context, const RectF& rect, const Float4U& color, f32 radius = 0.0f);
        LUNA_GUI_API ItemHandle draw_circle(IContext* context, const Float2U& center, f32 radius, const Float4U& color);
        LUNA_GUI_API ItemHandle draw_line(IContext* context, const Float2U& begin, const Float2U& end, const Float4U& color, f32 width = 1.0f);
        LUNA_GUI_API ItemHandle draw_text(IContext* context, const RectF& rect, const c8* text, const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            TextAlignment horizontal_alignment = TextAlignment::begin,
            TextAlignment vertical_alignment = TextAlignment::center);
        LUNA_GUI_API ItemHandle draw_image(IContext* context, RHI::ITexture* texture, const RectF& rect, const Float4U& color = Float4U(1.0f), ImageFlag flags = ImageFlag::none);
    }
}

#include "Views.hpp"
