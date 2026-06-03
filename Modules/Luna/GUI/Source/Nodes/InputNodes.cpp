/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "InputNodes.hpp"
#include "../RenderProxies/InputRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        InputTextNode::InputTextNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_input_text_render_proxy();
        }

        Guid InputTextNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputTextNode::clone() const
        {
            return new_object<InputTextNode>(*this);
        }

        LayoutMetrics InputTextNode::measure() const
        {
            return fixed_height_metrics(80.0f, 240.0f, 30.0f);
        }

        SliderFloatNode::SliderFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid SliderFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> SliderFloatNode::clone() const
        {
            return new_object<SliderFloatNode>(*this);
        }

        LayoutMetrics SliderFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        SliderIntNode::SliderIntNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid SliderIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> SliderIntNode::clone() const
        {
            return new_object<SliderIntNode>(*this);
        }

        LayoutMetrics SliderIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        InputFloatNode::InputFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid InputFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputFloatNode::clone() const
        {
            return new_object<InputFloatNode>(*this);
        }

        LayoutMetrics InputFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        InputIntNode::InputIntNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid InputIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputIntNode::clone() const
        {
            return new_object<InputIntNode>(*this);
        }

        LayoutMetrics InputIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        DragFloatNode::DragFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid DragFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DragFloatNode::clone() const
        {
            return new_object<DragFloatNode>(*this);
        }

        LayoutMetrics DragFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        DragIntNode::DragIntNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_numeric_render_proxy();
        }

        Guid DragIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DragIntNode::clone() const
        {
            return new_object<DragIntNode>(*this);
        }

        LayoutMetrics DragIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

    }
}
