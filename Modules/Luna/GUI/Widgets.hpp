/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! Adds one text element.
        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TextDesc& desc = TextDesc());

        //! Begins a button container. The label identifies the element and is not rendered automatically.
        LUNA_GUI_API GUICore::ElementHandle begin_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ButtonDesc& desc = ButtonDesc());
        //! Ends the current button container.
        LUNA_GUI_API void end_button(GUICore::IContext* context);
        //! Adds a button that displays one text label.
        LUNA_GUI_API GUICore::ElementHandle text_button(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ButtonDesc& desc = ButtonDesc());

        //! Adds one shape element.
        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, id_t id,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ShapeWidgetDesc& desc = ShapeWidgetDesc());
        //! Adds an invisible interactive element.
        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const HitBoxDesc& desc = HitBoxDesc());
        //! Adds a button containing one shape.
        LUNA_GUI_API GUICore::ElementHandle shape_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ShapeButtonDesc& desc = ShapeButtonDesc());

        //! Adds a selectable text item.
        LUNA_GUI_API GUICore::ElementHandle selectable(GUICore::IContext* context, id_t id, const c8* label,
            bool selected, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds a checkbox bound to a boolean value.
        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds a radio button bound to an integer selection.
        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, id_t id, const c8* label,
            i32* value, i32 button_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds an animated switch bound to a boolean value.
        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());

        //! Adds a single-selection horizontal button group.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, i32* selected_index, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ButtonGroupDesc& desc = ButtonGroupDesc());
        //! Adds a multiple-selection horizontal button group.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, Span<bool> selected, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ButtonGroupDesc& desc = ButtonGroupDesc());

        //! Adds one image element.
        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ImageDesc& desc = ImageDesc());

        //! Adds one editable single-line text input.
        //! @param[in,out] value Text storage read during interaction resolution and delayed drawing.
        //! @remark @p value must remain valid until @ref resolve_interactions and GUI Core draw-command generation
        //! have both completed for the current frame.
        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, id_t id, String& value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TextInputDesc& desc = TextInputDesc());

        //! Adds an RGB color editor bound to normalized floating-point channels.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            f32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to normalized floating-point channels.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            f32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGB color editor bound to 8-bit channels.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            u8* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to 8-bit channels.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            u8* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGB color editor bound to an `0xAABBGGRR` packed color.
        //! @remark The alpha byte is always written as 255.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            u32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to an `0xAABBGGRR` packed color.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            u32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());

        //! Adds one floating-point slider.
        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds one integer slider.
        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a two-component floating-point slider.
        LUNA_GUI_API GUICore::ElementHandle slider_float2(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a three-component floating-point slider.
        LUNA_GUI_API GUICore::ElementHandle slider_float3(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a four-component floating-point slider.
        LUNA_GUI_API GUICore::ElementHandle slider_float4(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a two-component integer slider.
        LUNA_GUI_API GUICore::ElementHandle slider_int2(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a three-component integer slider.
        LUNA_GUI_API GUICore::ElementHandle slider_int3(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a four-component integer slider.
        LUNA_GUI_API GUICore::ElementHandle slider_int4(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());

        //! Adds a scalar floating-point drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a two-component floating-point drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_float2(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a three-component floating-point drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_float3(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a four-component floating-point drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_float4(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a scalar integer drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a two-component integer drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_int2(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a three-component integer drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_int3(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a four-component integer drag editor.
        LUNA_GUI_API GUICore::ElementHandle drag_int4(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());

        //! Adds a collapsible section header and returns its current open state.
        LUNA_GUI_API bool collapsing_header(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DisclosureDesc& desc = DisclosureDesc(), GUICore::ElementHandle* out_handle = nullptr);
        //! Adds a collapsible tree node and returns its current open state.
        LUNA_GUI_API bool tree_node(GUICore::IContext* context, id_t id, const c8* label,
            TreeNodeFlag flags = TreeNodeFlag::none, u32 indent_depth = 0,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DisclosureDesc& desc = DisclosureDesc(), GUICore::ElementHandle* out_handle = nullptr);

        //! Adds one progress bar. Values are clamped to the 0-1 range.
        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, id_t id, f32 fraction,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ProgressBarDesc& desc = ProgressBarDesc());
    }
}
