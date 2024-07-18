/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.hpp
* @author JXMaster
* @date 2024/3/30
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include "Widget.hpp"
#include <Luna/HID/Mouse.hpp>
#include <Luna/HID/KeyCode.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct ContextIO
        {
            //! The width of the viewport.
            u32 width;
            //! The height of the viewport.
            u32 height;
            //! The mouse X position.
            i32 mouse_x;
            //! The mouse Y position.
            i32 mouse_y;
            //! The mouse button state.
            HID::MouseButton pressed_mouse_buttons;
            //! The keyboard key state.
            bool key_pressed[(u32)HID::KeyCode::count];
        };

        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset);

        enum class WidgetStateLifetime : u8
        {
            frame = 0, // until current frame end.
            next_frame = 1, // until next frame end.
            process = 2, // until module close.
            persistent = 3 // persistent (will be saved to file on module close).
        };

        struct IContext : virtual Interface
        {
            luiid("{8d1a5f1d-d7f2-46a5-82e7-2b382af47a9e}");
            
            //! Gets the IO state that will be parsed in the next @ref update call.
            virtual ContextIO& get_io() = 0;

            virtual void set_widget(IWidget* root_widget) = 0;

            //! Gets widget implicit state.
            virtual object_t get_widget_state(widget_id_t id) = 0;

            //! Sets widget implicit state.
            virtual void set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime = WidgetStateLifetime::next_frame) = 0;

            //! Updates the internal state (like input, animation, etc) of the context.
            virtual RV update() = 0;

            virtual VG::IFontAtlas* get_font_altas() = 0;

            //! Renders the context.
            virtual RV render(IDrawList* draw_list) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context();
    }
}