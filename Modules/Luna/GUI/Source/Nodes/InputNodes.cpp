/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/InputNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        InputTextNode::InputTextNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid InputTextNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputTextNode::clone() const
        {
            return new_object<InputTextNode>(*this);
        }

        bool InputTextNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics InputTextNode::measure() const
        {
            return fixed_height_metrics(80.0f, 240.0f, 30.0f);
        }

        SliderFloatNode::SliderFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid SliderFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> SliderFloatNode::clone() const
        {
            return new_object<SliderFloatNode>(*this);
        }

        bool SliderFloatNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics SliderFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        SliderIntNode::SliderIntNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid SliderIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> SliderIntNode::clone() const
        {
            return new_object<SliderIntNode>(*this);
        }

        bool SliderIntNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics SliderIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        InputFloatNode::InputFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid InputFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputFloatNode::clone() const
        {
            return new_object<InputFloatNode>(*this);
        }

        bool InputFloatNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics InputFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        InputIntNode::InputIntNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid InputIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> InputIntNode::clone() const
        {
            return new_object<InputIntNode>(*this);
        }

        bool InputIntNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics InputIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        DragFloatNode::DragFloatNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid DragFloatNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DragFloatNode::clone() const
        {
            return new_object<DragFloatNode>(*this);
        }

        bool DragFloatNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics DragFloatNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

        DragIntNode::DragIntNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid DragIntNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DragIntNode::clone() const
        {
            return new_object<DragIntNode>(*this);
        }

        bool DragIntNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics DragIntNode::measure() const
        {
            return numeric_edit_metrics(*this, binding);
        }

    }
}
