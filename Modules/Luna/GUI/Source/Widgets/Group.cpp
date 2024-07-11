/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Group.cpp
* @author JXMaster
* @date 2024/7/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "WidgetCommon.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API bool begin_group(IContext* ctx, const c8* title, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_group_begin(&c->m_ctx, title, (nk_flags)flags);
        }
        LUNA_GUI_API bool begin_group_titled(IContext* ctx, const c8* name, const c8* title, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_group_begin_titled(&c->m_ctx, name, title, (nk_flags)flags);
        }
        LUNA_GUI_API void end_group(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_group_end(&c->m_ctx);
        }
        LUNA_GUI_API bool begin_scrolled_group(IContext* ctx, u32* x_offset, u32* y_offset, const c8* title, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_group_scrolled_offset_begin(&c->m_ctx, x_offset, y_offset, title, (nk_flags)flags);
        }
        LUNA_GUI_API void end_scrolled_group(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_group_scrolled_end(&c->m_ctx);
        }
        LUNA_GUI_API UInt2U get_group_scroll(IContext* ctx, const c8* name)
        {
            Context* c = (Context*)(ctx->get_object());
            u32 x_offset, y_offset;
            nk_group_get_scroll(&c->m_ctx, name, &x_offset, &y_offset);
            return UInt2U(x_offset, y_offset);
        }
        LUNA_GUI_API void set_group_scroll(IContext* ctx, const c8* name, f32 scroll_x, f32 scroll_y)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_group_set_scroll(&c->m_ctx, name, scroll_x, scroll_y);
        }
    }
}