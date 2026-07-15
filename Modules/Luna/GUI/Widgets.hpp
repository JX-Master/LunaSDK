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

        //! Adds a single-selection horizontal button group.
        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, i32* selected_index, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ButtonGroupDesc& desc = ButtonGroupDesc());

        //! Adds one image element.
        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const ImageDesc& desc = ImageDesc());

        //! Adds one editable single-line text input.
        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, id_t id, String& value,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TextInputDesc& desc = TextInputDesc());

        //! Adds one floating-point slider.
        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());
        //! Adds one integer slider.
        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const SliderDesc& desc = SliderDesc());

        //! Adds one progress bar. Values are clamped to the 0-1 range.
        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, id_t id, f32 fraction,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ProgressBarDesc& desc = ProgressBarDesc());
    }
}
