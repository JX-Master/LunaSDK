/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"
#include "InputNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ColorPickerNode : Node
        {
            lustruct("GUI::ColorPickerNode", "{BE28EC1C-7058-43E0-A970-FCFB1B2629FC}");

            ColorBinding binding;

            ColorPickerNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_color_picker() const override;
            virtual bool uses_context_render() const override;
            virtual f32* f32_values() const override;
            virtual u8* u8_values() const override;
            virtual u32* u32_value() const override;
            virtual u8 f32_values_count() const override;
            virtual ColorValueType color_type() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual LayoutMetrics measure() const override;
        };
        struct ColorEditNode : Node
        {
            lustruct("GUI::ColorEditNode", "{3009B8C8-A0C0-4A6A-B565-CB55217D554E}");

            ColorBinding binding;
            id_t picker_popup_id = 0;

            ColorEditNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool is_color_edit() const override;
            virtual bool uses_context_render() const override;
            virtual f32* f32_values() const override;
            virtual u8* u8_values() const override;
            virtual u32* u32_value() const override;
            virtual u8 f32_values_count() const override;
            virtual ColorValueType color_type() const override;
            virtual id_t color_owner() const override;
            virtual ColorEditPart color_part() const override;
            virtual id_t menu_popup() const override;
            virtual void set_menu_popup(id_t value) override;
            virtual LayoutMetrics measure() const override;
        };
    }
}
