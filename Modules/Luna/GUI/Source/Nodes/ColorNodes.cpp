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

        ColorWidgetKind ColorPickerNode::color_widget_kind() const
        {
            return ColorWidgetKind::picker;
        }

        bool ColorPickerNode::uses_context_render() const
        {
            return true;
        }

        f32* ColorPickerNode::f32_values() const
        {
            return binding.f32_value;
        }

        u8* ColorPickerNode::u8_values() const
        {
            return binding.u8_value;
        }

        u32* ColorPickerNode::u32_value() const
        {
            return binding.u32_value;
        }

        u8 ColorPickerNode::f32_values_count() const
        {
            return binding.value_count;
        }

        ColorValueType ColorPickerNode::color_type() const
        {
            return binding.type;
        }

        id_t ColorPickerNode::color_owner() const
        {
            return binding.owner_id;
        }

        ColorChannelPart ColorPickerNode::color_part() const
        {
            return binding.part;
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
