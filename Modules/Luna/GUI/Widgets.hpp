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
#include "Context.hpp"
#include "Attributes.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        // Layout widgets.

        // enum class VerticalAlignment : u8
        // {
        //     top,
        //     center,
        //     bottom
        // };

        // LUNA_GUI_API void begin_vertical_layout(IContext* ctx);

        // LUNA_GUI_API void set_element_vertical_alignment(IContext* ctx, VerticalAlignment alignment);

        // LUNA_GUI_API void set_element_horizontal_alignment(IContext* ctx, HorizontalAlignment alignment);

        // LUNA_GUI_API void end_vertical_layout(IContext* ctx);

        LUNA_GUI_API void begin_rectangle(IContext* ctx);

        LUNA_GUI_API void end_rectangle(IContext* ctx);

        LUNA_GUI_API void set_anthor(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom);

        LUNA_GUI_API void set_offset(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom);

        //! Adds one text widget.
        //! @param[in] text The text to draw.
        LUNA_GUI_API void text(IContext* ctx, const Name& text);

        //! Sets widget scalar attribute value.
        LUNA_GUI_API void set_sattr(IContext* ctx, u32 kind, f32 value);

        //! Sets widget vector attribute value.
        LUNA_GUI_API void set_vattr(IContext* ctx, u32 kind, const Float4U& value);

        //! Sets widget text attribute value.
        LUNA_GUI_API void set_tattr(IContext* ctx, u32 kind, const Name& value);

        LUNA_GUI_API void begin_window(IContext* ctx, const c8* title);

        enum class WindowFlag : u32
        {
            none = 0,
            movable = 1, // The window can be moved by dragging the title bar.
            resizable = 2, // The window can be resized by dragging the window border.
            borderless = 3, // The window does not have title bar and border (thus cannot be moved or resized even `movable | resizable` is set).
        };

        LUNA_GUI_API void end_window(IContext* ctx);

        LUNA_GUI_API void set_window_flags(IContext* ctx, WindowFlag flags);

        LUNA_GUI_API void set_window_pos(IContext* ctx, i32 x, i32 y, Condition condition = Condition::first_time);

        LUNA_GUI_API void set_window_size(IContext* ctx, u32 width, u32 height, Condition condition = Condition::first_time);

        LUNA_GUI_API void set_window_title(IContext* ctx, const c8* title, usize title_len = USIZE_MAX);
    }
}