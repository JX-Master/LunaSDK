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
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] value Null-terminated UTF-8 text copied into package frame storage.
        //! @param[in] layout Layout configuration for the text element.
        //! @param[in] desc Text appearance and alignment.
        //! @return Returns the created text element.
        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TextDesc& desc = TextDesc());

        //! Begins a button container. The label identifies the element and is not rendered automatically.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable button ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the button container.
        //! @param[in] desc Button interaction behavior.
        //! @return Returns the created button element. Call @ref end_button after submitting its children.
        LUNA_GUI_API GUICore::ElementHandle begin_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ButtonDesc& desc = ButtonDesc());
        //! Ends the current button container.
        //! @param[in] context The GUI Core context containing the open button scope.
        LUNA_GUI_API void end_button(GUICore::IContext* context);
        //! Adds a button that displays one text label.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable button ID.
        //! @param[in] value Null-terminated UTF-8 label.
        //! @param[in] layout Layout configuration for the button.
        //! @param[in] desc Button interaction behavior.
        //! @return Returns the created button element.
        LUNA_GUI_API GUICore::ElementHandle text_button(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ButtonDesc& desc = ButtonDesc());

        //! Adds one shape element.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] value VG shape range and optional texture mask.
        //! @param[in] layout Layout configuration for the shape element.
        //! @param[in] desc Shape appearance.
        //! @return Returns the created shape element.
        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, id_t id,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ShapeWidgetDesc& desc = ShapeWidgetDesc());
        //! Adds an invisible interactive element.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] layout Layout configuration that defines the hit region.
        //! @param[in] desc Hit-box interaction behavior.
        //! @return Returns the created hit-box element.
        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const HitBoxDesc& desc = HitBoxDesc());
        //! Adds a button containing one shape.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable button ID.
        //! @param[in] label Optional debug label. This value is not rendered automatically.
        //! @param[in] value VG shape range and optional texture mask.
        //! @param[in] layout Layout configuration for the button.
        //! @param[in] desc Button and shape appearance.
        //! @return Returns the created shape-button element.
        LUNA_GUI_API GUICore::ElementHandle shape_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ShapeButtonDesc& desc = ShapeButtonDesc());

        //! Adds a selectable text item.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in] selected Whether to use selected presentation for this frame.
        //! @param[in] layout Layout configuration for the item.
        //! @param[in] desc Choice interaction behavior.
        //! @return Returns the created selectable element. Query the returned handle for click state.
        LUNA_GUI_API GUICore::ElementHandle selectable(GUICore::IContext* context, id_t id, const c8* label,
            bool selected, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds a checkbox bound to a boolean value.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Boolean value toggled during @ref resolve_interactions.
        //! @param[in] layout Layout configuration for the checkbox.
        //! @param[in] desc Choice interaction behavior.
        //! @return Returns the created checkbox element.
        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds a radio button bound to an integer selection.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Selection value written during @ref resolve_interactions.
        //! @param[in] button_value Value assigned when the radio button is activated.
        //! @param[in] layout Layout configuration for the radio button.
        //! @param[in] desc Choice interaction behavior.
        //! @return Returns the created radio-button element.
        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, id_t id, const c8* label,
            i32* value, i32 button_value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());
        //! Adds an animated switch bound to a boolean value.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Boolean value toggled during @ref resolve_interactions.
        //! @param[in] layout Layout configuration for the switch.
        //! @param[in] desc Choice interaction behavior.
        //! @return Returns the created switch element.
        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ChoiceDesc& desc = ChoiceDesc());

        //! Adds a single-selection horizontal button group.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in] items Null-terminated UTF-8 item labels.
        //! @param[in,out] selected_index Selected item index updated during @ref resolve_interactions.
        //! @param[in] layout Layout configuration for the group.
        //! @param[in] desc Group interaction and sizing behavior.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, i32* selected_index, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ButtonGroupDesc& desc = ButtonGroupDesc());
        //! Adds a multiple-selection horizontal button group.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in] items Null-terminated UTF-8 item labels.
        //! @param[in,out] selected Boolean selections corresponding to @p items.
        //! @param[in] layout Layout configuration for the group.
        //! @param[in] desc Group interaction and sizing behavior.
        //! @return Returns the created group element.
        //! @remark @p selected must contain at least as many entries as @p items and remain valid through
        //! @ref resolve_interactions.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, Span<bool> selected, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ButtonGroupDesc& desc = ButtonGroupDesc());

        //! Adds one image element.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] texture Texture sampled by the image. The caller retains ownership.
        //! @param[in] layout Layout configuration for the image element.
        //! @param[in] desc Image coordinates, tint and sampling behavior.
        //! @return Returns the created image element.
        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ImageDesc& desc = ImageDesc());

        //! Adds one editable single-line text input.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable input ID.
        //! @param[in,out] value Text storage read during interaction resolution and delayed drawing.
        //! @param[in] layout Layout configuration for the input.
        //! @param[in] desc Editing behavior and placeholder text.
        //! @return Returns the created text-input element.
        //! @remark @p value must remain valid until @ref resolve_interactions and GUI Core draw-command generation
        //! have both completed for the current frame.
        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, id_t id, String& value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TextInputDesc& desc = TextInputDesc());

        //! Adds an RGB color editor bound to normalized floating-point channels.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Array of three normalized RGB channels.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            f32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to normalized floating-point channels.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Array of four normalized RGBA channels.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            f32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGB color editor bound to 8-bit channels.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Array of three RGB channels.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            u8* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to 8-bit channels.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Array of four RGBA channels.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            u8* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGB color editor bound to an `0xAABBGGRR` packed color.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Packed color storage.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        //! @remark The alpha byte is always written as 255.
        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, id_t id, const c8* label,
            u32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());
        //! Adds an RGBA color editor bound to an `0xAABBGGRR` packed color.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in] label Null-terminated UTF-8 label.
        //! @param[in,out] value Packed color storage.
        //! @param[in] layout Layout configuration for the preview control.
        //! @param[in] desc Color-editor interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, id_t id, const c8* label,
            u32* value, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ColorEditDesc& desc = ColorEditDesc());

        //! Adds one floating-point slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable slider ID.
        //! @param[in,out] value Scalar value clamped to the resolved range during submission and interaction.
        //! @param[in] minimum Minimum slider value.
        //! @param[in] maximum Maximum slider value. Reversed endpoints are accepted and swapped.
        //! @param[in] layout Layout configuration for the slider.
        //! @param[in] desc Slider interaction behavior.
        //! @return Returns the created slider element.
        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds one integer slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable slider ID.
        //! @param[in,out] value Scalar value clamped to the resolved range during submission and interaction.
        //! @param[in] minimum Minimum slider value.
        //! @param[in] maximum Maximum slider value. Reversed endpoints are accepted and swapped.
        //! @param[in] layout Layout configuration for the slider.
        //! @param[in] desc Slider interaction behavior.
        //! @return Returns the created slider element.
        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a two-component floating-point slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of two values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_float2(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a three-component floating-point slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of three values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_float3(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a four-component floating-point slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of four values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_float4(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a two-component integer slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of two values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_int2(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a three-component integer slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of three values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_int3(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds a four-component integer slider.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of four values.
        //! @param[in] minimum Minimum value for every component.
        //! @param[in] maximum Maximum value for every component.
        //! @param[in] layout Layout configuration for the slider group.
        //! @param[in] desc Slider interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle slider_int4(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());

        //! Adds a scalar floating-point drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in,out] value Scalar value updated during @ref resolve_interactions.
        //! @param[in] minimum Minimum value when the editor is bounded.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor.
        //! @param[in] desc Drag interaction behavior.
        //! @return Returns the created drag-editor element.
        LUNA_GUI_API GUICore::ElementHandle drag_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a two-component floating-point drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of two values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_float2(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a three-component floating-point drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of three values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_float3(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a four-component floating-point drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of four values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_float4(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a scalar integer drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable editor ID.
        //! @param[in,out] value Scalar value updated during @ref resolve_interactions.
        //! @param[in] minimum Minimum value when the editor is bounded.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor.
        //! @param[in] desc Drag interaction behavior.
        //! @return Returns the created drag-editor element.
        LUNA_GUI_API GUICore::ElementHandle drag_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a two-component integer drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of two values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_int2(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a three-component integer drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of three values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_int3(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());
        //! Adds a four-component integer drag editor.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable group ID.
        //! @param[in,out] value Array of four values.
        //! @param[in] minimum Minimum value for every bounded component.
        //! @param[in] maximum Maximum value. Values less than or equal to @p minimum disable clamping.
        //! @param[in] layout Layout configuration for the editor group.
        //! @param[in] desc Drag interaction behavior applied to every component.
        //! @return Returns the created group element.
        LUNA_GUI_API GUICore::ElementHandle drag_int4(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DragDesc& desc = DragDesc());

        //! Adds a collapsible section header and returns its current open state.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable disclosure ID used for persistent open state.
        //! @param[in] label Null-terminated UTF-8 header label.
        //! @param[in] layout Layout configuration for the header.
        //! @param[in] desc Disclosure interaction and initial-state behavior.
        //! @param[out] out_handle Optional destination for the created header element.
        //! @return Returns whether the section content should be submitted this frame.
        LUNA_GUI_API bool collapsing_header(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DisclosureDesc& desc = DisclosureDesc(), GUICore::ElementHandle* out_handle = nullptr);
        //! Adds a collapsible tree node and returns its current open state.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable node ID used for persistent open state.
        //! @param[in] label Null-terminated UTF-8 node label.
        //! @param[in] flags Tree-node presentation and disclosure behavior.
        //! @param[in] indent_depth Number of tree indentation levels applied before the node content.
        //! @param[in] layout Layout configuration for the node row.
        //! @param[in] desc Disclosure interaction and initial-state behavior.
        //! @param[out] out_handle Optional destination for the created node element.
        //! @return Returns whether child tree content should be submitted this frame.
        LUNA_GUI_API bool tree_node(GUICore::IContext* context, id_t id, const c8* label,
            TreeNodeFlag flags = TreeNodeFlag::none, u32 indent_depth = 0,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DisclosureDesc& desc = DisclosureDesc(), GUICore::ElementHandle* out_handle = nullptr);

        //! Adds one progress bar. Values are clamped to the 0-1 range.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable element ID.
        //! @param[in] fraction Completion fraction.
        //! @param[in] layout Layout configuration for the progress bar.
        //! @param[in] desc Overlay-text presentation.
        //! @return Returns the created progress-bar element.
        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, id_t id, f32 fraction,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ProgressBarDesc& desc = ProgressBarDesc());
    }
}
