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

            virtual IWidget* get_widget() = 0;

            virtual void set_widget(IWidget* root_widget) = 0;

            //! Gets widget implicit state.
            virtual object_t get_widget_state(widget_id_t id) = 0;

            //! Sets widget implicit state.
            virtual void set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime = WidgetStateLifetime::next_frame) = 0;

            //! Pushes event to the event FIFO queue. The event will be processed in the next call to @ref update.
            virtual void push_event(object_t event) = 0;

            //! Captures event type, so events of that type are always sent to the specified widget.
            //! @param[in] widget The widget to capture the event.
            //! @param[in] event_type The event the widget wants to capture.
            //! @remark Every time one new update is triggered by @ref update, all previously set captures will be released before 
            //! @ref IWidget::begin_update is called. In order to retain the capture between updates, the widget should call
            //! this function in @ref IWidget::begin_update, so that the widget gains capture before event handing stage take place.
            //! 
            //! The capture does not need to be released explicitly, since they will be released before next @ref update call.
            virtual void capture_event(IWidget* widget, typeinfo_t event_type) = 0;

            //! Updates the internal state (like input, animation, etc) of the context.
            virtual RV update() = 0;

            virtual VG::IFontAtlas* get_font_altas() = 0;

            //! Renders the context.
            virtual RV render(IDrawList* draw_list) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context();
    }
}