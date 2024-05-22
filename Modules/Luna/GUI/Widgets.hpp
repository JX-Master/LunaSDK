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
#include "WidgetList.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void begin_resizable_window(IWidgetList* list);

        LUNA_GUI_API void begin_rectangle(IWidgetList* list);

        LUNA_GUI_API void end(IWidgetList* list);

        inline constexpr u32 VATTR_ANTHOR = strhash32("anthor");

        LUNA_GUI_API void set_anthor(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom);

        inline constexpr u32 VATTR_OFFSET = strhash32("offset");

        LUNA_GUI_API void set_offset(IWidgetList* list, f32 left, f32 top, f32 right, f32 bottom);

        //! Adds one text widget.
        //! @param[in] text The text to draw.
        LUNA_GUI_API void text(IWidgetList* list, const Name& text);

        //! Sets widget scalar attribute value.
        LUNA_GUI_API void set_sattr(IWidgetList* list, u32 kind, f32 value);

        //! Sets widget vector attribute value.
        LUNA_GUI_API void set_vattr(IWidgetList* list, u32 kind, const Float4U& value);

        //! Sets widget text attribute value.
        LUNA_GUI_API void set_tattr(IWidgetList* list, u32 kind, const Name& value);

        inline constexpr u32 VATTR_BACKGROUND_COLOR = strhash32("background_color");

        inline constexpr u32 SATTR_TEXT_SIZE = strhash32("text_size");

        inline constexpr u32 VATTR_TEXT_COLOR = strhash32("text_color");

        inline constexpr u32 TATTR_TEXT = strhash32("text");
    }
}