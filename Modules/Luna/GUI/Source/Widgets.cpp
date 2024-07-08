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
        inline struct nk_color encode_color_from_rgba8(u32 c)
        {
            struct nk_color r;
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
            r.r = (c & 0x000000ff);
            r.g = (c & 0x0000ff00) >> 8;
            r.b = (c & 0x00ff0000) >> 16;
            r.a = (c & 0xff000000) >> 24;
#else
            r.r = (c & 0xff000000) >> 24;
            r.g = (c & 0x00ff0000) >> 16;
            r.b = (c & 0x0000ff00) >> 8;
            r.a = (c & 0x000000ff);
#endif
            return r;
        }
        LUNA_GUI_API void horizontal_rule(IContext* ctx, u32 color_rgba, bool rounding)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_rule_horizontal(&c->m_ctx, encode_color_from_rgba8(color_rgba), rounding);
        }
        LUNA_GUI_API void text_colored(IContext* ctx, const u32 color_rgba, const c8* text, u32 size, TextAlignment alignment)
        {
            Context* c = (Context*)(ctx->get_object());
            if(size == U32_MAX)
            {
                size = strlen(text);
            }
            nk_text_colored(&c->m_ctx, text, size, encode_text_alignment(alignment), encode_color_from_rgba8(color_rgba));
        }
        LUNA_GUI_API void text_wrap(IContext* ctx, const c8* text, u32 size)
        {
            Context* c = (Context*)(ctx->get_object());
            if(size == U32_MAX)
            {
                size = strlen(text);
            }
            nk_text_wrap(&c->m_ctx, text, size);
        }
        LUNA_GUI_API void text_wrap_colored(IContext* ctx, const u32 color_rgba, const c8* text, u32 size)
        {
            Context* c = (Context*)(ctx->get_object());
            if(size == U32_MAX)
            {
                size = strlen(text);
            }
            nk_text_wrap_colored(&c->m_ctx, text, size, encode_color_from_rgba8(color_rgba));
        }
        LUNA_GUI_API void label(IContext* ctx, const c8* text, TextAlignment alignment)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_label(&c->m_ctx, text, encode_text_alignment(alignment));
        }
        LUNA_GUI_API void label_colored(IContext* ctx, const c8* text, const u32 color_rgba, TextAlignment alignment)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_label_colored(&c->m_ctx, text, encode_text_alignment(alignment), encode_color_from_rgba8(color_rgba));
        }
        LUNA_GUI_API void label_wrap(IContext* ctx, const c8* text)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_label_wrap(&c->m_ctx, text);
        }
        LUNA_GUI_API void label_colored_wrap(IContext* ctx, const c8* text, const u32 color_rgba)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_label_colored_wrap(&c->m_ctx, text, encode_color_from_rgba8(color_rgba));
        }
        LUNA_GUI_API void image(IContext* ctx, RHI::ITexture* image, const OffsetRectU offsets)
        {
            Context* c = (Context*)(ctx->get_object());
            struct nk_image img;
            img.handle.ptr = image;
            auto desc = image->get_desc();
            img.w = desc.width;
            img.h = desc.height;
            img.region[0] = offsets.left;
            img.region[1] = offsets.top;
            img.region[2] = offsets.right;
            img.region[3] = offsets.bottom;
            nk_image(&c->m_ctx, img);
        }
        LUNA_GUI_API void labelf(IContext* ctx, TextAlignment alignment, const c8* fmt, ...)
        {
            VarList args;
            va_start(args, fmt);
            labelfv(ctx, alignment, fmt, args);
            va_end(args);
        }
        LUNA_GUI_API void labelf_colored(IContext* ctx, TextAlignment alignment, const u32 color_rgba, const c8* fmt, ...)
        {
            VarList args;
            va_start(args, fmt);
            labelfv_colored(ctx, alignment, color_rgba, fmt, args);
            va_end(args);
        }
        LUNA_GUI_API void labelf_wrap(IContext* ctx, const c8* fmt, ...)
        {
            VarList args;
            va_start(args, fmt);
            labelfv_wrap(ctx, fmt, args);
            va_end(args);
        }
        LUNA_GUI_API void labelf_colored_wrap(IContext* ctx, const u32 color_rgba, const c8* fmt, ...)
        {
            VarList args;
            va_start(args, fmt);
            labelfv_colored_wrap(ctx, color_rgba, fmt, args);
            va_end(args);
        }
        LUNA_GUI_API void labelfv(IContext* ctx, TextAlignment alignment, const c8* fmt, VarList args)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_labelfv(&c->m_ctx, encode_text_alignment(alignment), fmt, args);
        }
        LUNA_GUI_API void labelfv_colored(IContext* ctx, TextAlignment alignment, const u32 color_rgba, const c8* fmt, VarList args)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_labelfv_colored(&c->m_ctx, encode_text_alignment(alignment), encode_color_from_rgba8(color_rgba), fmt, args);
        }
        LUNA_GUI_API void labelfv_wrap(IContext* ctx, const c8* fmt, VarList args)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_labelfv_wrap(&c->m_ctx, fmt, args);
        }
        LUNA_GUI_API void labelfv_colored_wrap(IContext* ctx, const u32 color_rgba, const c8* fmt, VarList args)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_labelfv_colored_wrap(&c->m_ctx, encode_color_from_rgba8(color_rgba), fmt, args);
        }
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
        
    }
}