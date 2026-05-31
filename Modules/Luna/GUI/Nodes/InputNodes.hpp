/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ComboNode : Node
        {
            lustruct("GUI::ComboNode", "{702842A3-F200-4103-8EE4-9B23FB918E8E}");

            i32* current_item = nullptr;
            Vector<String> combo_items;

            ComboNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_combo() const override;
            virtual i32* combo_current_item() const override;
            virtual usize combo_item_count() const override;
            virtual const c8* combo_item_text(usize index) const override;

            static f32 label_width(const Node& node, const RectF& rect);

            static RectF value_rect(const Node& node, const RectF& rect);

            static f32 item_height();

            static RectF dropdown_rect(const Node& node, const RectF& rect, const Float2U& surface_size);

            static i32 dropdown_item_at(const Node& node, const RectF& dropdown, const Float2U& pos);

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct InputTextNode : Node
        {
            lustruct("GUI::InputTextNode", "{14C55BCE-735A-4F3C-8B96-AE6743C5797B}");

            String* value = nullptr;

            InputTextNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_input_text() const override;
            virtual bool uses_context_render() const override;
            virtual String* string_value() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct SliderFloatNode : Node
        {
            lustruct("GUI::SliderFloatNode", "{1832CB45-7F7E-483D-B665-25940619DF56}");

            NumericBinding binding;

            SliderFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_float_numeric() const override;
            virtual bool is_slider_numeric() const override;
            virtual bool is_numeric_pointer_edit() const override;
            virtual bool uses_context_render() const override;
            virtual f32* f32_values() const override;
            virtual u8 f32_values_count() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct SliderIntNode : Node
        {
            lustruct("GUI::SliderIntNode", "{7BBCB122-B2A5-4C13-850D-590D21610C93}");

            NumericBinding binding;

            SliderIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_int_numeric() const override;
            virtual bool is_slider_numeric() const override;
            virtual bool is_numeric_pointer_edit() const override;
            virtual bool uses_context_render() const override;
            virtual i32* i32_values() const override;
            virtual u8 i32_values_count() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct InputFloatNode : Node
        {
            lustruct("GUI::InputFloatNode", "{BEE71E3F-890A-452F-9EB6-16FD9D605B29}");

            NumericBinding binding;

            InputFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_float_numeric() const override;
            virtual bool is_numeric_input() const override;
            virtual bool uses_context_render() const override;
            virtual f32* f32_values() const override;
            virtual u8 f32_values_count() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct InputIntNode : Node
        {
            lustruct("GUI::InputIntNode", "{0B504420-16B0-437A-9454-8D340C60275C}");

            NumericBinding binding;

            InputIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_int_numeric() const override;
            virtual bool is_numeric_input() const override;
            virtual bool uses_context_render() const override;
            virtual i32* i32_values() const override;
            virtual u8 i32_values_count() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct DragFloatNode : Node
        {
            lustruct("GUI::DragFloatNode", "{1528F7FB-101A-4673-AFDA-24C1D011FA41}");

            NumericBinding binding;

            DragFloatNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_float_numeric() const override;
            virtual bool is_drag_numeric() const override;
            virtual bool is_numeric_input() const override;
            virtual bool is_numeric_pointer_edit() const override;
            virtual bool uses_context_render() const override;
            virtual f32* f32_values() const override;
            virtual u8 f32_values_count() const override;
            virtual bool uses_f32_color_components() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual f32 step_value() const override;
            virtual NumericEditFlag numeric_edit_flags() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct DragIntNode : Node
        {
            lustruct("GUI::DragIntNode", "{68CE96E0-8C8B-4DF5-A1AD-7DD6DC5E18DE}");

            NumericBinding binding;

            DragIntNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_int_numeric() const override;
            virtual bool is_drag_numeric() const override;
            virtual bool is_numeric_input() const override;
            virtual bool is_numeric_pointer_edit() const override;
            virtual bool uses_context_render() const override;
            virtual i32* i32_values() const override;
            virtual u8 i32_values_count() const override;
            virtual f32 min_value() const override;
            virtual f32 max_value() const override;
            virtual f32 step_value() const override;
            virtual NumericEditFlag numeric_edit_flags() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
    }
}
