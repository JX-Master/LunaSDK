/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.hpp
* @author JXMaster
* @date 2024/3/29
*/
#pragma once
#include "WidgetBuilder.hpp"
#include "Attributes.hpp"
#include "Widgets/Canvas.hpp"
#include "Widgets/Rectangle.hpp"
#include "Widgets/Text.hpp"
#include "Widgets/Button.hpp"
#include "Widgets/Slider.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        //! Adds one text widget.
        //! @param[in] text The text to draw.
        LUNA_GUI_API Text* text(IWidgetBuilder* builder, const Name& text);

        //! Adds one rectangle.
        LUNA_GUI_API Rectangle* rectangle(IWidgetBuilder* builder);

        //! Sets widget scalar attribute value.
        LUNA_GUI_API void set_sattr(IWidgetBuilder* builder, u32 kind, f32 value);

        //! Sets widget vector attribute value.
        LUNA_GUI_API void set_vattr(IWidgetBuilder* builder, u32 kind, const Float4U& value);

        //! Sets widget text attribute value.
        LUNA_GUI_API void set_tattr(IWidgetBuilder* builder, u32 kind, const Name& value);

        LUNA_GUI_API Button* begin_button(IWidgetBuilder* builder, const Name& id, const Function<RV(void)>& on_click);

        LUNA_GUI_API void end_button(IWidgetBuilder* builder);

        LUNA_GUI_API Button* button(IWidgetBuilder* builder, const Name& text, const Function<RV(void)>& on_click, const Name& id = Name());

        inline RV default_slider_on_value_changed_handler(f32) { return ok; }

        LUNA_GUI_API Slider* slider(IWidgetBuilder* builder, widget_hash_t hash, f32* value, f32 min_value, f32 max_value, const Function<RV(f32)>& on_value_changed = default_slider_on_value_changed_handler);
    }
}