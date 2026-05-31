/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/DrawingNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        Guid HitBoxNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> HitBoxNode::clone() const
        {
            return new_object<HitBoxNode>(*this);
        }

        LayoutMetrics HitBoxNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(1.0f, 1.0f);
            metrics.preferred_size = Float2U(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            metrics.max_size = Float2U(F32_MAX, F32_MAX);
            return metrics;
        }

        Guid ImageNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ImageNode::clone() const
        {
            return new_object<ImageNode>(*this);
        }

        LayoutMetrics ImageNode::measure() const
        {
            Float2U image_size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = image_size;
            metrics.preferred_size = image_size;
            metrics.max_size = image_size;
            return metrics;
        }

        void ImageNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_rect(rect, clip_rect, Float4U(1.0f), 0.0f, image, flags);
        }

        Guid DrawRectNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DrawRectNode::clone() const
        {
            return new_object<DrawRectNode>(*this);
        }

        LayoutMetrics DrawRectNode::measure() const
        {
            Float2U size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = size;
            metrics.preferred_size = size;
            metrics.max_size = size;
            return metrics;
        }

        void DrawRectNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_rect(rect, clip_rect, color, radius);
        }

        Guid DrawCircleNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DrawCircleNode::clone() const
        {
            return new_object<DrawCircleNode>(*this);
        }

        LayoutMetrics DrawCircleNode::measure() const
        {
            Float2U size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = size;
            metrics.preferred_size = size;
            metrics.max_size = size;
            return metrics;
        }

        void DrawCircleNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_circle(rect, clip_rect, color);
        }

        Guid DrawLineNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DrawLineNode::clone() const
        {
            return new_object<DrawLineNode>(*this);
        }

        LayoutMetrics DrawLineNode::measure() const
        {
            Float2U size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = size;
            metrics.preferred_size = size;
            metrics.max_size = size;
            return metrics;
        }

        void DrawLineNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_line(begin, end, clip_rect, color, width);
        }

        Guid DrawTextNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DrawTextNode::clone() const
        {
            return new_object<DrawTextNode>(*this);
        }

        LayoutMetrics DrawTextNode::measure() const
        {
            Float2U size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = size;
            metrics.preferred_size = size;
            metrics.max_size = size;
            return metrics;
        }

        void DrawTextNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_text(rect, clip_rect, text.c_str(), font_size, color, horizontal_alignment, vertical_alignment);
        }

        Guid DrawImageNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DrawImageNode::clone() const
        {
            return new_object<DrawImageNode>(*this);
        }

        LayoutMetrics DrawImageNode::measure() const
        {
            Float2U size(max(requested_size.width, 1.0f), max(requested_size.height, 1.0f));
            LayoutMetrics metrics;
            metrics.min_size = size;
            metrics.preferred_size = size;
            metrics.max_size = size;
            return metrics;
        }

        void DrawImageNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.render_rect(rect, clip_rect, color, 0.0f, image, flags);
        }

    }
}
