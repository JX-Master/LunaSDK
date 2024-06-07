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

            //! Begins a new frame recording.
            virtual void begin_frame() = 0;

            //! Ends one frame.
            virtual void end_frame() = 0;

            // Widget ID APIs.

            virtual void push_id(const Name& name_id) = 0;
            virtual void push_id(const c8* str_id, usize str_len) = 0;
            virtual void push_id(const void* ptr_id) = 0;
            virtual void push_id(i32 int_id) = 0;
            virtual void pop_id() = 0;

            //! Generates widget ID based on the current widget ID stack.
            virtual widget_id_t get_id() = 0;
            virtual widget_id_t get_id(const Name& name_id) = 0;
            virtual widget_id_t get_id(const c8* str_id, usize str_len) = 0;
            virtual widget_id_t get_id(const void* ptr_id) = 0;
            virtual widget_id_t get_id(i32 int_id) = 0;

            virtual Widget* get_current_widget() = 0;

            virtual void add_widget(Widget* widget) = 0;

            //! Pushes one widget to the widget stack so that new widgets will be created as child widgets of this widget.
            virtual void push_widget(Widget* widget) = 0;

            //! Pops one widget from the widget stack.
            virtual void pop_widget() = 0;

            //! Gets widget implicit state.
            virtual object_t get_widget_state(widget_id_t id) = 0;

            //! Sets widget implicit state.
            virtual void set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime = WidgetStateLifetime::next_frame) = 0;

            //! Updates the internal state (like input, animation, etc) of the context.
            virtual RV update() = 0;

            virtual VG::IFontAtlas* get_font_altas() = 0;

            //! Renders the context.
            virtual RV render(VG::IShapeDrawList* draw_list) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context();
    }
}