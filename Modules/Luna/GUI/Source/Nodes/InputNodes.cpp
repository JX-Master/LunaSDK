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
        ComboNode::ComboNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid ComboNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ComboNode::clone() const
        {
            return new_object<ComboNode>(*this);
        }

        i32* ComboNode::combo_current_item() const
        {
            return current_item;
        }

        usize ComboNode::combo_item_count() const
        {
            return combo_items.size();
        }

        const c8* ComboNode::combo_item_text(usize index) const
        {
            return index < combo_items.size() ? combo_items[index].c_str() : "";
        }

        f32 ComboNode::label_width(const Node& node, const RectF& rect)
        {
            if(node.text.empty()) return 0.0f;
            return min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
        }

        RectF ComboNode::value_rect(const Node& node, const RectF& rect)
        {
            f32 label_w = label_width(node, rect);
            return RectF(rect.offset_x + label_w, rect.offset_y, max(rect.width - label_w, 1.0f), rect.height);
        }

        f32 ComboNode::item_height()
        {
            return 26.0f;
        }

        RectF ComboNode::dropdown_rect(const Node& node, const RectF& rect, const Float2U& surface_size)
        {
            RectF value = value_rect(node, rect);
            f32 dropdown_width = max(value.width, 120.0f);
            f32 dropdown_height = max((f32)node.combo_item_count() * item_height(), item_height());
            dropdown_width = min(dropdown_width, max(surface_size.x, 1.0f));
            dropdown_height = min(dropdown_height, max(surface_size.y, item_height()));
            f32 x = min(value.offset_x, max(surface_size.x - dropdown_width, 0.0f));
            f32 y = value.offset_y + value.height + 2.0f;
            if(y + dropdown_height > surface_size.y && value.offset_y - dropdown_height - 2.0f >= 0.0f)
            {
                y = value.offset_y - dropdown_height - 2.0f;
            }
            y = min(y, max(surface_size.y - dropdown_height, 0.0f));
            return RectF(x, y, dropdown_width, dropdown_height);
        }

        i32 ComboNode::dropdown_item_at(const Node& node, const RectF& dropdown, const Float2U& pos)
        {
            if(pos.x < dropdown.offset_x || pos.x >= dropdown.offset_x + dropdown.width ||
                pos.y < dropdown.offset_y || pos.y >= dropdown.offset_y + dropdown.height ||
                !node.combo_item_count())
            {
                return -1;
            }
            i32 index = (i32)((pos.y - dropdown.offset_y) / item_height());
            return index >= 0 && (usize)index < node.combo_item_count() ? index : -1;
        }

        LayoutMetrics ComboNode::measure() const
        {
            f32 text_width = (f32)text.size() * 16.0f * 0.52f;
            return fixed_height_metrics(140.0f, max(text_width + 160.0f, 220.0f), 30.0f);
        }

        void ComboNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            f32 label_w = label_width(*this, rect);
            if(label_w > 0.0f)
            {
                ctx.draw_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip_rect,
                    text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::begin);
            }
            RectF value = value_rect(*this, rect);
            bool open = ctx.is_combo_open(id);
            Float4U value_color = open ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                (state.hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f));
            ctx.draw_rect(value, clip_rect, value_color, 4.0f);

            const c8* item_name = "";
            if(current_item && *current_item >= 0 && (usize)*current_item < combo_items.size())
            {
                item_name = combo_items[*current_item].c_str();
            }
            ctx.draw_text(RectF(value.offset_x + 8.0f, value.offset_y, max(value.width - 34.0f, 1.0f), value.height),
                clip_rect, item_name, 16.0f, Float4U(1.0f), TextAlignment::begin);

            f32 arrow_x = value.offset_x + value.width - 18.0f;
            f32 arrow_y = value.offset_y + value.height * 0.5f;
            if(open)
            {
                ctx.draw_line(Float2U(arrow_x - 5.0f, arrow_y + 3.0f), Float2U(arrow_x, arrow_y - 3.0f), clip_rect, Float4U(1.0f), 1.8f);
                ctx.draw_line(Float2U(arrow_x, arrow_y - 3.0f), Float2U(arrow_x + 5.0f, arrow_y + 3.0f), clip_rect, Float4U(1.0f), 1.8f);
            }
            else
            {
                ctx.draw_line(Float2U(arrow_x - 5.0f, arrow_y - 3.0f), Float2U(arrow_x, arrow_y + 3.0f), clip_rect, Float4U(1.0f), 1.8f);
                ctx.draw_line(Float2U(arrow_x, arrow_y + 3.0f), Float2U(arrow_x + 5.0f, arrow_y - 3.0f), clip_rect, Float4U(1.0f), 1.8f);
            }

            if(open)
            {
                RectF surface_clip(0.0f, 0.0f, state.surface_size.x, state.surface_size.y);
                RectF dropdown = dropdown_rect(*this, rect, state.surface_size);
                ctx.draw_rect(dropdown, surface_clip, Float4U(0.07f, 0.09f, 0.12f, 0.98f), 5.0f);
                i32 hovered_item = dropdown_item_at(*this, dropdown, state.pointer_position);
                i32 selected_item = current_item ? *current_item : -1;
                for(usize item_index = 0; item_index < combo_items.size(); ++item_index)
                {
                    RectF item_rect(dropdown.offset_x, dropdown.offset_y + item_height() * (f32)item_index,
                        dropdown.width, item_height());
                    if(item_rect.offset_y >= dropdown.offset_y + dropdown.height) break;
                    bool selected = selected_item == (i32)item_index;
                    bool item_hovered = hovered_item == (i32)item_index;
                    if(selected || item_hovered)
                    {
                        ctx.draw_rect(item_rect, dropdown,
                            selected ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : Float4U(0.17f, 0.23f, 0.32f, 1.0f),
                            0.0f);
                    }
                    ctx.draw_text(RectF(item_rect.offset_x + 8.0f, item_rect.offset_y, max(item_rect.width - 34.0f, 1.0f), item_rect.height),
                        dropdown, combo_items[item_index].c_str(), 15.0f, Float4U(1.0f), TextAlignment::begin);
                    if(selected)
                    {
                        f32 x = item_rect.offset_x + item_rect.width - 20.0f;
                        f32 y = item_rect.offset_y + item_rect.height * 0.5f;
                        ctx.draw_line(Float2U(x, y), Float2U(x + 4.0f, y + 4.0f), dropdown, Float4U(1.0f), 2.0f);
                        ctx.draw_line(Float2U(x + 4.0f, y + 4.0f), Float2U(x + 12.0f, y - 5.0f), dropdown, Float4U(1.0f), 2.0f);
                    }
                }
            }
        }

        void ComboNode::update_state(NodeInputContext& ctx) const
        {
            ctx.set_state(Name("gui.open"), Any(ctx.is_combo_open(id)));
        }

        void ComboNode::on_click(NodeInputContext& ctx)
        {
            if(!current_item || combo_items.empty()) return;
            bool open = ctx.is_combo_open(id);
            if(open)
            {
                ctx.close_combo_dropdown(id);
            }
            else
            {
                ctx.open_combo_dropdown(id);
            }
            ctx.set_state(Name("gui.open"), Any(!open));
        }

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

        ColorEditPart SliderFloatNode::color_part() const
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

        ColorEditPart SliderIntNode::color_part() const
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

        ColorEditPart InputFloatNode::color_part() const
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

        ColorEditPart InputIntNode::color_part() const
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

        ColorEditPart DragFloatNode::color_part() const
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

        ColorEditPart DragIntNode::color_part() const
        {
            return binding.color_part;
        }

        LayoutMetrics DragIntNode::measure() const
        {
            return numeric_edit_metrics(*this);
        }

    }
}
