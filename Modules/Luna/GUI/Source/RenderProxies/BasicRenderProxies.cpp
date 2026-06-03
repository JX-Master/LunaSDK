/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "BasicRenderProxies.hpp"
#include "../Nodes/BasicNodes.hpp"
#include "../GUI.hpp"
#include "../../State.hpp"

namespace Luna
{
    namespace GUI
    {
        static void draw_default_button(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            Float4U default_color = style_f32x4(ctx, node, Name("gui.button.background"), Float4U(0.18f, 0.28f, 0.45f, 1.0f));
            Float4U hovered_color = style_f32x4(ctx, node, Name("gui.button.background_hovered"), Float4U(0.26f, 0.43f, 0.72f, 1.0f));
            Float4U active_color = style_f32x4(ctx, node, Name("gui.button.background_active"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));
            Float4U target_color = state.active ? active_color : (state.hovered ? hovered_color : default_color);
            Float4U color = target_color;
            ButtonAnimationState* animation_state = ctx.get_widget_state<ButtonAnimationState>(node.id);
            if(animation_state && animation_state->initialized)
            {
                f32 blend = clamp(state.delta_time * style_f32(ctx, node, Name("gui.button.animation_speed"), 14.0f), 0.0f, 1.0f);
                color = smooth_color(animation_state->color, target_color, blend);
            }
            Ref<ButtonAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonAnimationState>(node.id);
            next_animation_state->color = color;
            next_animation_state->initialized = true;
            f32 radius = style_f32(ctx, node, Name("gui.button.radius"), 5.0f);
            f32 font_size = style_f32(ctx, node, Name("gui.button.font_size"), 16.0f);
            Float4U text_color = style_f32x4(ctx, node, Name("gui.button.text_color"), Float4U(1.0f));
            ctx.draw_rect(rect, clip_rect, color, radius);
            ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height),
                clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::center);
        }

        static void draw_default_text(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const TextNode* text_node = cast_node<TextNode>(node);
            f32 default_font_size = text_node ? text_node->font_size : 16.0f;
            Float4U default_color = text_node ? text_node->color : Float4U(1.0f);
            f32 font_size = style_f32(ctx, node, Name("gui.text.font_size"), default_font_size);
            Float4U color = style_f32x4(ctx, node, Name("gui.text.color"), default_color);
            ctx.draw_text(rect, clip_rect, node.text.c_str(), font_size, color, TextAlignment::begin);
        }

        RenderProxyDesc default_button_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_button;
            return desc;
        }

        RenderProxyDesc default_text_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_text;
            return desc;
        }
    }
}
