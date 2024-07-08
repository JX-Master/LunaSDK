/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2024/6/28
*/
#pragma once
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class WindowFlag : u16
        {
            none = 0,
            border = 0x01,
            movable = 0x02,
            resizable = 0x04,
            closable = 0x08,
            minimizable = 0x10,
            no_scroll_bar = 0x20,
            title = 0x40,
            scroll_auto_hide = 0x80,
            background = 0x100,
            scale_left = 0x200,
            no_input = 0x400
        };

        enum class TextAlignment : u8
        {
            top_left,
            top_centered,
            top_right,
            middle_left,
            middle_centered,
            middle_right,
            bottom_left,
            bottom_centered,
            bottom_right
        };

        enum class SymbolType : u8
        {
            none = 0,
            x,
            underscore,
            circle_solid,
            circle_outline,
            rect_solid,
            rect_outline,
            triangle_up,
            triangle_down,
            triangle_left,
            triangle_right,
            plus,
            minus
        };

        // Windows.

        LUNA_GUI_API bool begin(IContext* ctx, const c8* title, const RectF& bounding_rect, WindowFlag flags = WindowFlag::none);
        LUNA_GUI_API bool begin_titled(IContext* ctx, const c8* name, const c8* title, const RectF& bounding_rect, WindowFlag flags = WindowFlag::none);
        LUNA_GUI_API void end(IContext* ctx);
        LUNA_GUI_API RectF get_current_window_bounds(IContext* ctx);
        LUNA_GUI_API Float2U get_current_window_position(IContext* ctx);
        LUNA_GUI_API Float2U get_current_window_size(IContext* ctx);
        LUNA_GUI_API f32 get_current_window_width(IContext* ctx);
        LUNA_GUI_API f32 get_current_window_height(IContext* ctx);
        LUNA_GUI_API RectF get_current_window_content_region(IContext* ctx);
        LUNA_GUI_API Float2U get_current_window_content_region_min(IContext* ctx);
        LUNA_GUI_API Float2U get_current_window_content_region_max(IContext* ctx);
        LUNA_GUI_API Float2U get_current_window_content_region_size(IContext* ctx);
        LUNA_GUI_API UInt2U get_current_window_scroll(IContext* ctx);
        LUNA_GUI_API bool is_current_window_focused(IContext* ctx);
        LUNA_GUI_API bool is_current_window_hovered(IContext* ctx);
        LUNA_GUI_API bool is_window_collapsed(IContext* ctx, const c8* window_name);
        LUNA_GUI_API bool is_window_closed(IContext* ctx, const c8* window_name);
        LUNA_GUI_API bool is_window_hidden(IContext* ctx, const c8* window_name);
        LUNA_GUI_API bool is_window_active(IContext* ctx, const c8* window_name);
        LUNA_GUI_API bool is_any_window_hovered(IContext* ctx);
        LUNA_GUI_API bool is_any_window_active(IContext* ctx);
        LUNA_GUI_API void set_window_bounds(IContext* ctx, const c8* window_name, const RectF& bounding_rect);
        LUNA_GUI_API void set_window_position(IContext* ctx, const c8* window_name, const Float2U& pos);
        LUNA_GUI_API void set_window_size(IContext* ctx, const c8* window_name, const Float2U& size);
        LUNA_GUI_API void set_window_focused(IContext* ctx, const c8* window_name);
        LUNA_GUI_API void set_window_scroll(IContext* ctx, f32 scroll_x, f32 scroll_y);
        LUNA_GUI_API void close_window(IContext* ctx, const c8* window_name);
        LUNA_GUI_API void collapse_window(IContext* ctx, const c8* window_name);
        LUNA_GUI_API void expand_window(IContext* ctx, const c8* window_name);
        LUNA_GUI_API void show_window(IContext* ctx, const c8* window_name);
        LUNA_GUI_API void hide_window(IContext* ctx, const c8* window_name);

        LUNA_GUI_API void horizontal_rule(IContext* ctx, u32 color_rgba, bool rounding);
        LUNA_GUI_API void text(IContext* ctx, const c8* text, u32 size = U32_MAX, TextAlignment alignment = TextAlignment::middle_left);
        LUNA_GUI_API void text_colored(IContext* ctx, const u32 color_rgba, const c8* text, u32 size = U32_MAX, TextAlignment alignment = TextAlignment::middle_left);
        LUNA_GUI_API void text_wrap(IContext* ctx, const c8* text, u32 size = U32_MAX);
        LUNA_GUI_API void text_wrap_colored(IContext* ctx, const u32 color_rgba, const c8* text, u32 size = U32_MAX);
        LUNA_GUI_API void label(IContext* ctx, const c8* text, TextAlignment alignment = TextAlignment::middle_left);
        LUNA_GUI_API void label_colored(IContext* ctx, const c8* text, const u32 color_rgba, TextAlignment alignment = TextAlignment::middle_left);
        LUNA_GUI_API void label_wrap(IContext* ctx, const c8* text);
        LUNA_GUI_API void label_colored_wrap(IContext* ctx, const c8* text, const u32 color_rgba);
        LUNA_GUI_API void image(IContext* ctx, RHI::ITexture* image, const OffsetRectU offsets = OffsetRectU(0, 0, 0, 0));
        LUNA_GUI_API void labelf(IContext* ctx, TextAlignment alignment, const c8* fmt, ...);
        LUNA_GUI_API void labelf_colored(IContext* ctx, TextAlignment alignment, const u32 color_rgba, const c8* fmt, ...);
        LUNA_GUI_API void labelf_wrap(IContext* ctx, const c8* fmt, ...);
        LUNA_GUI_API void labelf_colored_wrap(IContext* ctx, const u32 color_rgba, const c8* fmt, ...);
        LUNA_GUI_API void labelfv(IContext* ctx, TextAlignment alignment, const c8* fmt, VarList args);
        LUNA_GUI_API void labelfv_colored(IContext* ctx, TextAlignment alignment, const u32 color_rgba, const c8* fmt, VarList args);
        LUNA_GUI_API void labelfv_wrap(IContext* ctx, const c8* fmt, VarList args);
        LUNA_GUI_API void labelfv_colored_wrap(IContext* ctx, const u32 color_rgba, const c8* fmt, VarList args);
        
        LUNA_GUI_API bool button_text(IContext* ctx, const c8* text, u32 size = U32_MAX);
        LUNA_GUI_API bool button_label(IContext* ctx, const c8* text);
        LUNA_GUI_API bool button_color(IContext* ctx, const u32 color_rgba);
        LUNA_GUI_API bool button_symbol(IContext* ctx, SymbolType symbol);

        LUNA_GUI_API void layout_row_dynamic(IContext* ctx, f32 height, u32 cols);
        LUNA_GUI_API void layout_row_static(IContext* ctx, f32 height, u32 item_width, u32 cols);

    }
}