/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "DrawingNodes.hpp"
#include "../RenderProxies/DrawingRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        Guid HitBoxNode::type_guid() const
        {
            return Meta::StructMetaData<HitBoxNode>::__guid;
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

        ImageNode::ImageNode()
        {
            render_proxy = default_image_render_proxy();
        }

        Guid ImageNode::type_guid() const
        {
            return Meta::StructMetaData<ImageNode>::__guid;
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

        DrawRectNode::DrawRectNode()
        {
            render_proxy = default_draw_rect_render_proxy();
        }

        Guid DrawRectNode::type_guid() const
        {
            return Meta::StructMetaData<DrawRectNode>::__guid;
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

        DrawCircleNode::DrawCircleNode()
        {
            render_proxy = default_draw_circle_render_proxy();
        }

        Guid DrawCircleNode::type_guid() const
        {
            return Meta::StructMetaData<DrawCircleNode>::__guid;
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

        DrawLineNode::DrawLineNode()
        {
            render_proxy = default_draw_line_render_proxy();
        }

        Guid DrawLineNode::type_guid() const
        {
            return Meta::StructMetaData<DrawLineNode>::__guid;
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

        DrawTextNode::DrawTextNode()
        {
            render_proxy = default_draw_text_render_proxy();
        }

        Guid DrawTextNode::type_guid() const
        {
            return Meta::StructMetaData<DrawTextNode>::__guid;
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

        DrawImageNode::DrawImageNode()
        {
            render_proxy = default_draw_image_render_proxy();
        }

        Guid DrawImageNode::type_guid() const
        {
            return Meta::StructMetaData<DrawImageNode>::__guid;
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

    }
}
