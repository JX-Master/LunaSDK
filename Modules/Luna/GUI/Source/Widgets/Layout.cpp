/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.cpp
* @author JXMaster
* @date 2024/7/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "WidgetCommon.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void set_min_row_height(IContext* ctx, f32 height)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_set_min_row_height(&c->m_ctx, height);
        }
        LUNA_GUI_API void reset_min_row_height(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_reset_min_row_height(&c->m_ctx);
        }
        LUNA_GUI_API RectF get_next_row_bounds(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_layout_widget_bounds(&c->m_ctx));
        }
        LUNA_GUI_API void layout_row_dynamic(IContext* ctx, f32 height, u32 cols)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_dynamic(&c->m_ctx, height, cols);
        }
        LUNA_GUI_API void layout_row_static(IContext* ctx, f32 height, u32 item_width, u32 cols)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_static(&c->m_ctx, height, item_width, cols);
        }
        LUNA_GUI_API void layout_row_dynamic(IContext* ctx, f32 height, Span<const f32> col_width_ratios)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row(&c->m_ctx, NK_DYNAMIC, height, (int)col_width_ratios.size(), col_width_ratios.data());
        }
        LUNA_GUI_API void layout_row_static(IContext* ctx, f32 height, Span<const f32> col_widths)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row(&c->m_ctx, NK_STATIC, height, (int)col_widths.size(), col_widths.data());
        }
        LUNA_GUI_API void layout_row_begin_dynamic(IContext* ctx, f32 height, u32 cols)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_begin(&c->m_ctx, NK_DYNAMIC, height, cols);
        }
        LUNA_GUI_API void layout_row_begin_static(IContext* ctx, f32 height, u32 cols)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_begin(&c->m_ctx, NK_STATIC, height, cols);
        }
        LUNA_GUI_API void layout_row_push(IContext* ctx, f32 width_or_width_ratio)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_push(&c->m_ctx, width_or_width_ratio);
        }
        LUNA_GUI_API void layout_row_end(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_end(&c->m_ctx);
        }
        LUNA_GUI_API void layout_row_template_begin(IContext* ctx, f32 height)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_template_begin(&c->m_ctx, height);
        }
        LUNA_GUI_API void layout_row_template_push_dynamic(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_template_push_dynamic(&c->m_ctx);
        }
        LUNA_GUI_API void layout_row_template_push_variable(IContext* ctx, f32 min_width)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_template_push_variable(&c->m_ctx, min_width);
        }
        LUNA_GUI_API void layout_row_template_push_static(IContext* ctx, f32 width)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_template_push_static(&c->m_ctx, width);
        }
        LUNA_GUI_API void layout_row_template_end(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_row_template_end(&c->m_ctx);
        }
        LUNA_GUI_API void layout_space_begin_dynamic(IContext* ctx, f32 height, f32 num_widgets)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_space_begin(&c->m_ctx, NK_DYNAMIC, height, num_widgets);
        }
        LUNA_GUI_API void layout_space_begin_static(IContext* ctx, f32 height, f32 num_widgets)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_space_begin(&c->m_ctx, NK_STATIC, height, num_widgets);
        }
        LUNA_GUI_API void layout_space_push(IContext* ctx, const RectF& bounding_rect)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_space_push(&c->m_ctx, encode_rect(bounding_rect));
        }
        LUNA_GUI_API void layout_space_end(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_layout_space_end(&c->m_ctx);
        }
        LUNA_GUI_API RectF get_layout_space_bounds(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_layout_space_bounds(&c->m_ctx));
        }
        LUNA_GUI_API Float2U layout_space_local_to_screen(IContext* ctx, const Float2& pos)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_layout_space_to_screen(&c->m_ctx, encode_vec2(pos)));
        }
        LUNA_GUI_API Float2U layout_space_screen_to_local(IContext* ctx, const Float2& pos)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_layout_space_to_local(&c->m_ctx, encode_vec2(pos)));
        }
        LUNA_GUI_API RectF layout_space_local_to_screen_rect(IContext* ctx, const RectF& rect)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_layout_space_rect_to_screen(&c->m_ctx, encode_rect(rect)));
        }
        LUNA_GUI_API RectF layout_space_screen_to_local_rect(IContext* ctx, const RectF& rect)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_layout_space_rect_to_local(&c->m_ctx, encode_rect(rect)));
        }
        LUNA_GUI_API void spacer(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_spacer(&c->m_ctx);
        }
    }
}