/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Window.cpp
* @author JXMaster
* @date 2024/7/8
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Widgets.hpp"
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        inline struct nk_rect encode_rect(const RectF& r)
        {
            struct nk_rect rect;
            rect.x = r.offset_x;
            rect.y = r.offset_y;
            rect.w = r.width;
            rect.h = r.height;
            return rect;
        }
        LUNA_GUI_API bool begin(IContext* ctx, const c8* title, const RectF& bounding_rect, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_begin(&c->m_ctx, title, encode_rect(bounding_rect), (nk_flags)flags);
        }
        LUNA_GUI_API bool begin_titled(IContext* ctx, const c8* name, const c8* title, const RectF& bounding_rect, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            struct nk_rect rect;
            rect.x = bounding_rect.offset_x;
            rect.y = bounding_rect.offset_y;
            rect.w = bounding_rect.width;
            rect.h = bounding_rect.height;
            return nk_begin_titled(&c->m_ctx, name, title, encode_rect(bounding_rect), (nk_flags)flags);
        }
        LUNA_GUI_API void end(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_end(&c->m_ctx);
        }
        inline RectF decode_rect(const struct nk_rect& rect)
        {
            return RectF(rect.x, rect.y, rect.w, rect.h);
        }
        LUNA_GUI_API RectF get_current_window_bounds(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_window_get_bounds(&c->m_ctx));
        }
        inline Float2U decode_vec2(const struct nk_vec2& vec)
        {
            return Float2U(vec.x, vec.y);
        }
        LUNA_GUI_API Float2U get_current_window_position(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_window_get_position(&c->m_ctx));
        }
        LUNA_GUI_API Float2U get_current_window_size(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_window_get_size(&c->m_ctx));
        }
        LUNA_GUI_API f32 get_current_window_width(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_get_width(&c->m_ctx);
        }
        LUNA_GUI_API f32 get_current_window_height(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_get_height(&c->m_ctx);
        }
        LUNA_GUI_API RectF get_current_window_content_region(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_rect(nk_window_get_content_region(&c->m_ctx));
        }
        LUNA_GUI_API Float2U get_current_window_content_region_min(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_window_get_content_region_min(&c->m_ctx));
        }
        LUNA_GUI_API Float2U get_current_window_content_region_max(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_window_get_content_region_max(&c->m_ctx));
        }
        LUNA_GUI_API Float2U get_current_window_content_region_size(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return decode_vec2(nk_window_get_content_region_size(&c->m_ctx));
        }
        LUNA_GUI_API UInt2U get_current_window_scroll(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            u32 x, y;
            nk_window_get_scroll(&c->m_ctx, &x, &y);
            return UInt2U(x, y);
        }
        LUNA_GUI_API bool is_current_window_focused(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_has_focus(&c->m_ctx);
        }
        LUNA_GUI_API bool is_current_window_hovered(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_hovered(&c->m_ctx);
        }
        LUNA_GUI_API bool is_window_collapsed(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_collapsed(&c->m_ctx, window_name);
        }
        LUNA_GUI_API bool is_window_closed(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_closed(&c->m_ctx, window_name);
        }
        LUNA_GUI_API bool is_window_hidden(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_hidden(&c->m_ctx, window_name);
        }
        LUNA_GUI_API bool is_window_active(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_active(&c->m_ctx, window_name);
        }
        LUNA_GUI_API bool is_any_window_hovered(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_window_is_any_hovered(&c->m_ctx);
        }
        LUNA_GUI_API bool is_any_window_active(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_item_is_any_active(&c->m_ctx);
        }
        LUNA_GUI_API void set_window_bounds(IContext* ctx, const c8* window_name, const RectF& bounding_rect)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_set_bounds(&c->m_ctx, window_name, encode_rect(bounding_rect));
        }
        inline struct nk_vec2 encode_vec2(const Float2U& vec)
        {
            struct nk_vec2 r;
            r.x = vec.x;
            r.y = vec.y;
            return r;   
        }
        LUNA_GUI_API void set_window_position(IContext* ctx, const c8* window_name, const Float2U& pos)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_set_position(&c->m_ctx, window_name, encode_vec2(pos));
        }
        LUNA_GUI_API void set_window_size(IContext* ctx, const c8* window_name, const Float2U& size)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_set_size(&c->m_ctx, window_name, encode_vec2(size));
        }
        LUNA_GUI_API void set_window_focused(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_set_focus(&c->m_ctx, window_name);
        }
        LUNA_GUI_API void set_window_scroll(IContext* ctx, f32 scroll_x, f32 scroll_y)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_set_scroll(&c->m_ctx, scroll_x, scroll_y);
        }
        LUNA_GUI_API void close_window(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_close(&c->m_ctx, window_name);
        }
        LUNA_GUI_API void collapse_window(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_collapse(&c->m_ctx, window_name, NK_MINIMIZED);
        }
        LUNA_GUI_API void expand_window(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_collapse(&c->m_ctx, window_name, NK_MAXIMIZED);
        }
        LUNA_GUI_API void show_window(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_show(&c->m_ctx, window_name, NK_SHOWN);
        }
        LUNA_GUI_API void hide_window(IContext* ctx, const c8* window_name)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_window_show(&c->m_ctx, window_name, NK_HIDDEN);
        }
    }
}