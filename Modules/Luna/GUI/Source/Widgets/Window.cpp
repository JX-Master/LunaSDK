/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2024/6/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Window.hpp"
#include "../../Context.hpp"
#include "../../Widgets.hpp"
#include "../../WidgetDraw.hpp"

namespace Luna
{
    namespace GUI
    {
        RV Window::update(IContext *ctx, const OffsetRectF& layout_rect)
        {
            Ref<WindowState> state = cast_object<WindowState>(ctx->get_widget_state(id));
            if (state)
            {
                if(pos_condition == Condition::always || pos_condition == Condition::overwrite)
                {
                    state->pos = pos;
                }
                if(size_condition == Condition::always || size_condition == Condition::overwrite)
                {
                    state->size = size;
                }
            }
            else
            {
                state = new_object<WindowState>();
                f32 parent_width = layout_rect.right - layout_rect.left;
                f32 parent_height = layout_rect.bottom - layout_rect.top;
                if(pos_condition == Condition::always || pos_condition == Condition::first_time)
                {
                    state->pos = pos;
                }
                else
                {
                    state->pos = Int2U((i32)(parent_width * 0.15f), (i32)(parent_height * 0.15f));
                }
                if(size_condition == Condition::always || size_condition == Condition::first_time)
                {
                    state->size = size;
                }
                else
                {
                    state->size = UInt2U((u32)(parent_width * 0.7f), (u32)(parent_height * 0.7f));
                }
            }
            ctx->set_widget_state(id, state, WidgetStateLifetime::next_frame);
            bounding_rect = OffsetRectF(
                layout_rect.left + state->pos.x,
                layout_rect.top + state->pos.y,
                layout_rect.left + state->pos.x + state->size.x,
                layout_rect.top + state->pos.y + state->size.y
            );
            f32 text_size = get_sattr(SATTR_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
            // Calculate child rect.
            OffsetRectF child_rect {
                bounding_rect.left + 2,
                bounding_rect.right - 2,
                bounding_rect.top + text_size + 4,
                bounding_rect.bottom - 2
            };
            child_rect.right = max(child_rect.right, child_rect.left);
            child_rect.bottom = max(child_rect.bottom, child_rect.top);
            lutry
            {
                for(auto& child : children)
                {
                    luexp(child->update(ctx, child_rect));
                }
            }
            lucatchret;
            return ok;
        }
        RV Window::draw(IContext* ctx, VG::IShapeDrawList* draw_list)
        {
            lutry
            {
                // draw window rect.
                Float4U background_color = get_vattr(VATTR_BACKGROUND_COLOR, true, Float4U(0.94f, 0.94f, 0.94f, 1.0f));
                auto& points = draw_list->get_shape_points();
                auto& io = ctx->get_io();
                if(background_color.w != 0)
                {
                    draw_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color);
                }
                // draw title.
                f32 text_size = get_sattr(SATTR_TEXT_SIZE, true, DEFAULT_TEXT_SIZE);
                Font::IFontFile* font = cast_object<Font::IFontFile>(get_oattr(OATTR_FONT, true, Font::get_default_font()));
                u32 font_index = get_sattr(SATTR_FONT_INDEX, true, 0);
                luexp(draw_text(ctx, draw_list, title.c_str(), title.size(), Float4U(0.1f, 0.1f, 0.1f, 1.0f), text_size, 
                    bounding_rect.left + 2, bounding_rect.top + 2, bounding_rect.right - 2, bounding_rect.top + text_size + 2, 
                    font, font_index));
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API void begin_window(IContext* ctx, const c8* title)
        {
            ctx->push_id(title, strlen(title));
            Ref<Window> widget = new_object<Window>();
            widget->title = title;
            ctx->add_widget(widget);
            ctx->push_widget(widget);
        }
        LUNA_GUI_API void end_window(IContext* ctx)
        {
            ctx->pop_id();
            ctx->pop_widget();
        }
        LUNA_GUI_API void set_window_flags(IContext* ctx, WindowFlag flags)
        {
            Window* widget = cast_object<Window>(ctx->get_current_widget());
            if(!widget) return;
            widget->flags = flags;
        }
        LUNA_GUI_API void set_window_pos(IContext* ctx, i32 x, i32 y, Condition condition)
        {
            Window* widget = cast_object<Window>(ctx->get_current_widget());
            if(!widget) return;
            widget->pos = Int2U{x, y};
            widget->pos_condition = condition;
        }
        LUNA_GUI_API void set_window_size(IContext* ctx, u32 width, u32 height, Condition condition)
        {
            Window* widget = cast_object<Window>(ctx->get_current_widget());
            if(!widget) return;
            widget->size = UInt2U{width, height};
            widget->size_condition = condition;
        }
        LUNA_GUI_API void set_window_title(IContext* ctx, const c8* title, usize title_len)
        {
            Window* widget = cast_object<Window>(ctx->get_current_widget());
            if(!widget) return;
            if(title_len == USIZE_MAX) title_len = strlen(title);
            widget->title = Name(title, title_len);
        }
    }
}