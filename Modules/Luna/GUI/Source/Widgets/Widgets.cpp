/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.cpp
* @author JXMaster
* @date 2024/6/28
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "WidgetCommon.hpp"

namespace Luna
{
    namespace GUI
    {   
        LUNA_GUI_API bool button_text(IContext* ctx, const c8* text, u32 size)
        {
            Context* c = (Context*)(ctx->get_object());
            if(size == U32_MAX)
            {
                size = strlen(text);
            }
            return nk_button_text(&c->m_ctx, text, size);
        }
        LUNA_GUI_API bool button_label(IContext* ctx, const c8* text)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_button_label(&c->m_ctx, text);
        }
        LUNA_GUI_API bool button_color(IContext* ctx, const u32 color_rgba)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_button_color(&c->m_ctx, encode_color_from_rgba8(color_rgba));
        }
        LUNA_GUI_API bool button_symbol(IContext* ctx, SymbolType symbol)
        {
            Context* c = (Context*)(ctx->get_object());
            return nk_button_symbol(&c->m_ctx, (enum nk_symbol_type)symbol);
        }
    }
}