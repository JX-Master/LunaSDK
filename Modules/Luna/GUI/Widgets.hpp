/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2024/6/28
*/
#pragma once
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class WindowFlag : u16
        {
            none = 0,
            border = 0x01,
            movable = 0x02,
            resizable = 0x04,
            closable = 0x08,
            minimizable = 0x10,
            no_scroll_bar = 0x20,
            title = 0x40,
            scroll_auto_hide = 0x80,
            background = 0x100,
            scale_left = 0x200,
            no_input = 0x400
        };

        enum class TextAlignment : u8
        {
            top_left,
            top_centered,
            top_right,
            middle_left,
            middle_centered,
            middle_right,
            bottom_left,
            bottom_centered,
            bottom_right
        };

        LUNA_GUI_API bool begin(IContext* ctx, const c8* title, const RectF& bounding_rect, WindowFlag flags = WindowFlag::none);
        LUNA_GUI_API void end(IContext* ctx);

        LUNA_GUI_API void text(IContext* ctx, const c8* text, u32 size = U32_MAX, TextAlignment alignment = TextAlignment::middle_left);
    }

}