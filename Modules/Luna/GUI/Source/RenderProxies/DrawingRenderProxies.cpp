/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "DrawingRenderProxies.hpp"
#include "../Nodes/DrawingNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static void draw_default_image(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const ImageNode* typed = cast_node<ImageNode>(node);
            if(!typed) return;
            ctx.draw_rect(rect, clip_rect, Float4U(1.0f), 0.0f, typed->image, typed->flags);
        }

        static void draw_default_shape(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const ShapeNode* typed = cast_node<ShapeNode>(node);
            if(!typed || !typed->buffer || typed->num_commands == 0 || rect.width <= 0.0f || rect.height <= 0.0f ||
                typed->bounds.width <= 0.0f || typed->bounds.height <= 0.0f)
            {
                return;
            }
            RectF r(rect.offset_x, state.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
            RectF c(clip_rect.offset_x, state.surface_size.y - clip_rect.offset_y - clip_rect.height, clip_rect.width, clip_rect.height);
            IDrawList* draw_list = ctx.draw_list();
            DrawListState draw_state = draw_list->get_state();
            draw_state.shape_buffer = typed->buffer;
            draw_state.texture = typed->texture;
            draw_state.clip_rect = c;
            u32 pop_id = draw_list->push_state(&draw_state);
            Float2U shape_min(typed->bounds.offset_x, typed->bounds.offset_y + typed->bounds.height);
            Float2U shape_max(typed->bounds.offset_x + typed->bounds.width, typed->bounds.offset_y);
            draw_list->add_shape(typed->first_command, typed->num_commands,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                shape_min, shape_max,
                Float4U(1.0f),
                Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            draw_list->pop_state(pop_id);
        }

        static void draw_default_draw_rect(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const DrawRectNode* typed = cast_node<DrawRectNode>(node);
            if(!typed) return;
            ctx.draw_rect(rect, clip_rect, typed->color, typed->radius);
        }

        static void draw_default_draw_circle(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const DrawCircleNode* typed = cast_node<DrawCircleNode>(node);
            if(!typed) return;
            ctx.draw_circle(rect, clip_rect, typed->color);
        }

        static void draw_default_draw_line(NodeRenderContext& ctx, const Node& node, const RectF&, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const DrawLineNode* typed = cast_node<DrawLineNode>(node);
            if(!typed) return;
            ctx.draw_line(typed->begin, typed->end, clip_rect, typed->color, typed->width);
        }

        static void draw_default_draw_text(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const DrawTextNode* typed = cast_node<DrawTextNode>(node);
            if(!typed) return;
            ctx.draw_text(rect, clip_rect, node.text.c_str(), typed->font_size, typed->color,
                typed->horizontal_alignment, typed->vertical_alignment);
        }

        static void draw_default_draw_image(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const DrawImageNode* typed = cast_node<DrawImageNode>(node);
            if(!typed) return;
            ctx.draw_rect(rect, clip_rect, typed->color, 0.0f, typed->image, typed->flags);
        }

        RenderProxyDesc default_image_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_image;
            return desc;
        }

        RenderProxyDesc default_shape_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_shape;
            return desc;
        }

        RenderProxyDesc default_draw_rect_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_draw_rect;
            return desc;
        }

        RenderProxyDesc default_draw_circle_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_draw_circle;
            return desc;
        }

        RenderProxyDesc default_draw_line_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_draw_line;
            return desc;
        }

        RenderProxyDesc default_draw_text_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_draw_text;
            return desc;
        }

        RenderProxyDesc default_draw_image_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_draw_image;
            return desc;
        }
    }
}
