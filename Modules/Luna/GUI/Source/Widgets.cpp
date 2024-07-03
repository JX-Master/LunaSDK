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
#include "../Widgets.hpp"
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API bool begin(IContext* ctx, const c8* title, const RectF& bounding_rect, WindowFlag flags)
        {
            Context* c = (Context*)(ctx->get_object());
            struct nk_rect rect;
            rect.x = bounding_rect.offset_x;
            rect.y = bounding_rect.offset_y;
            rect.w = bounding_rect.width;
            rect.h = bounding_rect.height;
            return nk_begin(&c->m_ctx, title, rect, (nk_flags)flags);
        }
        LUNA_GUI_API void end(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_end(&c->m_ctx);
        }
        inline nk_flags encode_text_alignment(TextAlignment alignment)
        {
            switch(alignment)
            {
                case TextAlignment::top_left: return NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_LEFT;
                case TextAlignment::top_centered: return NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_CENTERED;
                case TextAlignment::top_right: return NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_RIGHT;
                case TextAlignment::middle_left: return NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_LEFT;
                case TextAlignment::middle_centered: return NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED;
                case TextAlignment::middle_right: return NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_RIGHT;
                case TextAlignment::bottom_left: return NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_LEFT;
                case TextAlignment::bottom_centered: return NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_CENTERED;
                case TextAlignment::bottom_right: return NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_RIGHT;
            }
            lupanic();
            return 0;
        }
        LUNA_GUI_API void text(IContext* ctx, const c8* text, u32 size, TextAlignment alignment)
        {
            Context* c = (Context*)(ctx->get_object());
            if(size == U32_MAX)
            {
                size = strlen(text);
            }
            nk_text(&c->m_ctx, text, size, encode_text_alignment(alignment));
        }
    }
}