/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/ColorNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        ColorPickerNode::ColorPickerNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid ColorPickerNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ColorPickerNode::clone() const
        {
            return new_object<ColorPickerNode>(*this);
        }

        bool ColorPickerNode::uses_context_render() const
        {
            return true;
        }

        LayoutMetrics ColorPickerNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(360.0f, 240.0f);
            metrics.preferred_size = Float2U(520.0f, 300.0f);
            metrics.max_size = Float2U(F32_MAX, 300.0f);
            return metrics;
        }
    }
}
