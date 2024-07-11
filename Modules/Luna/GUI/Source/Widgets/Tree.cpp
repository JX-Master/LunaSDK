/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Tree.cpp
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
        LUNA_GUI_API bool begin_section(IContext* ctx, const c8* title, bool* collapsed)
        {
            Context* c = (Context*)(ctx->get_object());
            enum nk_collapse_states state = collapsed ? (*collapsed ? NK_MINIMIZED : NK_MAXIMIZED) : NK_MAXIMIZED;
            bool r = nk_tree_state_push(&c->m_ctx, NK_TREE_NODE, title, &state);
            if (collapsed) *collapsed = state == NK_MINIMIZED;
            return r;
        }
        LUNA_GUI_API bool begin_image_section(IContext* ctx, RHI::ITexture* image, const OffsetRectU& image_rect_offset, const c8* title, bool* collapsed)
        {
            Context* c = (Context*)(ctx->get_object());
            struct nk_image img = encode_image(image, image_rect_offset);
            enum nk_collapse_states state = collapsed ? (*collapsed ? NK_MINIMIZED : NK_MAXIMIZED) : NK_MAXIMIZED;
            bool r = nk_tree_state_image_push(&c->m_ctx, NK_TREE_NODE, img, title, &state);
            if (collapsed) *collapsed = state == NK_MINIMIZED;
            return r;
        }
        LUNA_GUI_API void end_section(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_tree_state_pop(&c->m_ctx);
        }
        LUNA_GUI_API bool begin_treelist(IContext* ctx, const c8* title, bool* collapsed)
        {
            Context* c = (Context*)(ctx->get_object());
            enum nk_collapse_states state = collapsed ? (*collapsed ? NK_MINIMIZED : NK_MAXIMIZED) : NK_MAXIMIZED;
            bool r = nk_tree_state_push(&c->m_ctx, NK_TREE_TAB, title, &state);
            if (collapsed) *collapsed = state == NK_MINIMIZED;
            return r;
        }
        LUNA_GUI_API bool begin_image_treelist(IContext* ctx, RHI::ITexture* image, const OffsetRectU& image_rect_offset, const c8* title, bool* collapsed)
        {
            Context* c = (Context*)(ctx->get_object());
            struct nk_image img = encode_image(image, image_rect_offset);
            enum nk_collapse_states state = collapsed ? (*collapsed ? NK_MINIMIZED : NK_MAXIMIZED) : NK_MAXIMIZED;
            bool r = nk_tree_state_image_push(&c->m_ctx, NK_TREE_TAB, img, title, &state);
            if (collapsed) *collapsed = state == NK_MINIMIZED;
            return r;
        }
        LUNA_GUI_API void end_treelist(IContext* ctx)
        {
            Context* c = (Context*)(ctx->get_object());
            nk_tree_state_pop(&c->m_ctx);
        }
    }
}