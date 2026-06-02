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

        String* InputTextNode::string_value() const
        {
            return value;
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

        NumericValueKind SliderFloatNode::numeric_value_kind() const
        {
            return NumericValueKind::f32;
        }

        NumericInteractionKind SliderFloatNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::slider;
        }

        bool SliderFloatNode::uses_context_render() const
        {
            return true;
        }

        f32* SliderFloatNode::f32_values() const
        {
            return binding.f32_value;
        }

        u8 SliderFloatNode::f32_values_count() const
        {
            return binding.value_count;
        }

        f32 SliderFloatNode::min_value() const
        {
            return binding.min_value;
        }

        f32 SliderFloatNode::max_value() const
        {
            return binding.max_value;
        }

        id_t SliderFloatNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart SliderFloatNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics SliderFloatNode::measure() const
        {
            return numeric_edit_metrics(*this);
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

        NumericValueKind SliderIntNode::numeric_value_kind() const
        {
            return NumericValueKind::i32;
        }

        NumericInteractionKind SliderIntNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::slider;
        }

        bool SliderIntNode::uses_context_render() const
        {
            return true;
        }

        i32* SliderIntNode::i32_values() const
        {
            return binding.i32_value;
        }

        u8 SliderIntNode::i32_values_count() const
        {
            return binding.value_count;
        }

        f32 SliderIntNode::min_value() const
        {
            return binding.min_value;
        }

        f32 SliderIntNode::max_value() const
        {
            return binding.max_value;
        }

        id_t SliderIntNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart SliderIntNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics SliderIntNode::measure() const
        {
            return numeric_edit_metrics(*this);
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

        NumericValueKind InputFloatNode::numeric_value_kind() const
        {
            return NumericValueKind::f32;
        }

        NumericInteractionKind InputFloatNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::input;
        }

        bool InputFloatNode::uses_context_render() const
        {
            return true;
        }

        f32* InputFloatNode::f32_values() const
        {
            return binding.f32_value;
        }

        u8 InputFloatNode::f32_values_count() const
        {
            return binding.value_count;
        }

        f32 InputFloatNode::min_value() const
        {
            return binding.min_value;
        }

        f32 InputFloatNode::max_value() const
        {
            return binding.max_value;
        }

        id_t InputFloatNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart InputFloatNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics InputFloatNode::measure() const
        {
            return numeric_edit_metrics(*this);
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

        NumericValueKind InputIntNode::numeric_value_kind() const
        {
            return NumericValueKind::i32;
        }

        NumericInteractionKind InputIntNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::input;
        }

        bool InputIntNode::uses_context_render() const
        {
            return true;
        }

        i32* InputIntNode::i32_values() const
        {
            return binding.i32_value;
        }

        u8 InputIntNode::i32_values_count() const
        {
            return binding.value_count;
        }

        f32 InputIntNode::min_value() const
        {
            return binding.min_value;
        }

        f32 InputIntNode::max_value() const
        {
            return binding.max_value;
        }

        id_t InputIntNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart InputIntNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics InputIntNode::measure() const
        {
            return numeric_edit_metrics(*this);
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

        NumericValueKind DragFloatNode::numeric_value_kind() const
        {
            return NumericValueKind::f32;
        }

        NumericInteractionKind DragFloatNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::drag;
        }

        bool DragFloatNode::uses_context_render() const
        {
            return true;
        }

        f32* DragFloatNode::f32_values() const
        {
            return binding.f32_value;
        }

        u8 DragFloatNode::f32_values_count() const
        {
            return binding.value_count;
        }

        bool DragFloatNode::uses_f32_color_components() const
        {
            return binding.f32_color;
        }

        f32 DragFloatNode::min_value() const
        {
            return binding.min_value;
        }

        f32 DragFloatNode::max_value() const
        {
            return binding.max_value;
        }

        f32 DragFloatNode::step_value() const
        {
            return binding.step_value;
        }

        NumericEditFlag DragFloatNode::numeric_edit_flags() const
        {
            return binding.flags;
        }

        id_t DragFloatNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart DragFloatNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics DragFloatNode::measure() const
        {
            return numeric_edit_metrics(*this);
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

        NumericValueKind DragIntNode::numeric_value_kind() const
        {
            return NumericValueKind::i32;
        }

        NumericInteractionKind DragIntNode::numeric_interaction_kind() const
        {
            return NumericInteractionKind::drag;
        }

        bool DragIntNode::uses_context_render() const
        {
            return true;
        }

        i32* DragIntNode::i32_values() const
        {
            return binding.i32_value;
        }

        u8 DragIntNode::i32_values_count() const
        {
            return binding.value_count;
        }

        f32 DragIntNode::min_value() const
        {
            return binding.min_value;
        }

        f32 DragIntNode::max_value() const
        {
            return binding.max_value;
        }

        f32 DragIntNode::step_value() const
        {
            return binding.step_value;
        }

        NumericEditFlag DragIntNode::numeric_edit_flags() const
        {
            return binding.flags;
        }

        id_t DragIntNode::color_owner() const
        {
            return binding.color_owner_id;
        }

        ColorChannelPart DragIntNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics DragIntNode::measure() const
        {
            return numeric_edit_metrics(*this);
        }

    }
}
