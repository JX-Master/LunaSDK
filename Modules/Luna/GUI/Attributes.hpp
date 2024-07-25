/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Attributes.hpp
* @author JXMaster
* @date 2024/6/5
*/
#pragma once

#include <Luna/Runtime/Hash.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        enum class Condition : u8
        {
            //! The attribute is set always.
            always = 0,
            //! The attribute is set if such attribute is not set before.
            first_time = 1,
            //! The attribute is set if such attribute is set before.
            overwrite = 2,
            //! This attribute is never set. This can be used to clear one attribute set by former calls.
            never = 3,
        };

        inline constexpr u32 VATTR_ANTHOR = strhash32("anthor");

        inline constexpr u32 VATTR_OFFSET = strhash32("offset");

        inline constexpr u32 VATTR_BACKGROUND_COLOR = strhash32("background_color");

        inline constexpr u32 VATTR_BORDER_COLOR = strhash32("border_color");

        inline constexpr u32 VATTR_BUTTON_BACKGROUND_COLOR = strhash32("button_background_color");

        inline constexpr u32 VATTR_BUTTON_BORDER_COLOR = strhash32("button_border_color");

        inline constexpr u32 VATTR_BUTTON_HOVERED_BACKGROUND_COLOR = strhash32("button_hovered_background_color");

        inline constexpr u32 VATTR_BUTTON_HOVERED_BORDER_COLOR = strhash32("button_hovered_border_color");

        inline constexpr u32 VATTR_BUTTON_PRESSED_BACKGROUND_COLOR = strhash32("button_pressed_background_color");

        inline constexpr u32 VATTR_BUTTON_PRESSED_BORDER_COLOR = strhash32("button_pressed_border_color");

        inline constexpr u32 SATTR_BUTTON_BORDER_WIDTH = strhash32("button_border_width");

        inline constexpr u32 SATTR_BUTTON_ROUNDED_CORNER_RADIUS = strhash32("button_rounded_corner_radius");

        inline constexpr u32 SATTR_TEXT_SIZE = strhash32("text_size");

        inline constexpr f32 DEFAULT_TEXT_SIZE = 18;

        inline constexpr u32 VATTR_TEXT_COLOR = strhash32("text_color");

        inline constexpr u32 OATTR_FONT = strhash32("font");

        inline constexpr u32 SATTR_FONT_INDEX = strhash32("font_index");

        inline constexpr u32 SATTR_ROUNDED_CORNER_RADIUS = strhash32("rounded_corner_radius");

        inline constexpr u32 SATTR_REQUIRED_SIZE_X = strhash32("required_size_x");

        inline constexpr u32 SATTR_REQUIRED_SIZE_Y = strhash32("required_size_y");

        inline constexpr u32 SATTR_PREFERRED_SIZE_X = strhash32("preferred_size_x");

        inline constexpr u32 SATTR_PREFERRED_SIZE_Y = strhash32("preferred_size_y");

        inline constexpr u32 SATTR_FILLING_SIZE_X = strhash32("filling_size_x");

        inline constexpr u32 SATTR_FILLING_SIZE_Y = strhash32("filling_size_y");
    }
}