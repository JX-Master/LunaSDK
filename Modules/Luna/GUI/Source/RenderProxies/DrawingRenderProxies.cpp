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
