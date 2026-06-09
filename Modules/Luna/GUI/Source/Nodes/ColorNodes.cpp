/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "ColorNodes.hpp"
#include "../RenderProxies/ColorRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        ColorPickerNode::ColorPickerNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_color_picker_render_proxy();
        }

        Guid ColorPickerNode::type_guid() const
        {
            return Meta::StructMetaData<ColorPickerNode>::__guid;
        }

        Ref<Node> ColorPickerNode::clone() const
        {
            return new_object<ColorPickerNode>(*this);
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
