/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "MenuRenderProxies.hpp"
#include "../Nodes/MenuNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static void draw_default_menu_separator(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            f32 y = rect.offset_y + rect.height * 0.5f;
            f32 padding = style_f32(ctx, node, Name("gui.menu_separator.padding"), 8.0f);
            Float4U color = style_f32x4(ctx, node, Name("gui.menu_separator.color"), Float4U(0.24f, 0.29f, 0.36f, 1.0f));
            ctx.draw_line(Float2U(rect.offset_x + padding, y), Float2U(rect.offset_x + max(rect.width - padding, padding), y),
                clip_rect, color, style_f32(ctx, node, Name("gui.menu_separator.width"), 1.0f));
        }

        static void draw_default_menu_item(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const MenuItemNode* menu_item = cast_node<MenuItemNode>(node);
            if(!menu_item) return;
            bool open = menu_item->popup_id && ctx.is_popup_open(menu_item->popup_id);
            Float4U text_color = menu_item->enabled ?
                style_f32x4(ctx, node, Name("gui.menu_item.text_color"), Float4U(1.0f)) :
                style_f32x4(ctx, node, Name("gui.menu_item.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
            if(open || state.hovered || state.active)
            {
                Float4U active_color = style_f32x4(ctx, node, Name("gui.menu_item.background_active"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));
                Float4U hovered_color = style_f32x4(ctx, node, Name("gui.menu_item.background_hovered"), Float4U(0.20f, 0.30f, 0.44f, 1.0f));
                f32 radius = style_f32(ctx, node, Name(menu_item->top_level_menu ? "gui.menu_item.top_level_radius" : "gui.menu_item.radius"),
                    menu_item->top_level_menu ? 4.0f : 3.0f);
                ctx.draw_rect(rect, clip_rect, state.active || open ? active_color : hovered_color, radius);
            }
            if(menu_item->checked() && !menu_item->top_level_menu)
            {
                f32 x0 = rect.offset_x + 8.0f;
                f32 y0 = rect.offset_y + rect.height * 0.56f;
                f32 x1 = rect.offset_x + 13.0f;
                f32 y1 = rect.offset_y + rect.height * 0.72f;
                f32 x2 = rect.offset_x + 22.0f;
                f32 y2 = rect.offset_y + rect.height * 0.32f;
                f32 width = style_f32(ctx, node, Name("gui.menu_item.check_width"), 2.0f);
                ctx.draw_line(Float2U(x0, y0), Float2U(x1, y1), clip_rect, text_color, width);
                ctx.draw_line(Float2U(x1, y1), Float2U(x2, y2), clip_rect, text_color, width);
            }
            f32 text_x = menu_item->top_level_menu ? 10.0f : 30.0f;
            f32 text_pad_right = menu_item->top_level_menu ? 20.0f : (menu_item->popup_id ? 50.0f : 74.0f);
            f32 font_size = style_f32(ctx, node, Name("gui.menu_item.font_size"), 15.0f);
            ctx.draw_text(RectF(rect.offset_x + text_x, rect.offset_y, max(rect.width - text_x - text_pad_right, 1.0f), rect.height),
                clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
            if(menu_item->popup_id && !menu_item->top_level_menu)
            {
                ctx.draw_text(RectF(rect.offset_x + max(rect.width - 22.0f, 0.0f), rect.offset_y, 18.0f, rect.height),
                    clip_rect, ">", font_size, text_color, TextAlignment::center);
            }
            else if(!menu_item->shortcut.empty() && !menu_item->top_level_menu)
            {
                ctx.draw_text(RectF(rect.offset_x + max(rect.width - 88.0f, 0.0f), rect.offset_y, 80.0f, rect.height),
                    clip_rect, menu_item->shortcut.c_str(), style_f32(ctx, node, Name("gui.menu_item.shortcut_font_size"), 14.0f),
                    Float4U(text_color.x, text_color.y, text_color.z, style_f32(ctx, node, Name("gui.menu_item.shortcut_alpha"), 0.72f)),
                    TextAlignment::end);
            }
        }

        RenderProxyDesc default_menu_separator_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_menu_separator;
            return desc;
        }

        RenderProxyDesc default_menu_item_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_menu_item;
            return desc;
        }
    }
}
