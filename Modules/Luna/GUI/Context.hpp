/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.hpp
* @author JXMaster
* @date 2024/6/28
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/HID/Mouse.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct IContext : virtual Interface
        {
            luiid("cb9e827a-e632-4c5e-ab93-acade2f614a0");

            virtual UInt2U get_viewport_size() = 0;
            virtual void set_viewport_size(const UInt2U& size) = 0;
            virtual f32 get_font_size() = 0;
            virtual void set_font_size(f32 font_size) = 0;
            virtual VG::IFontAtlas* get_font_atlas() = 0;
            virtual void set_font_atlas(VG::IFontAtlas* font_atlas) = 0;

            //! Begins a new GUI frame.
            //! This will clear all recorded frame data and prepare the context for a new frame.
            virtual void begin_frame() = 0;

            virtual void begin_input() = 0;

            virtual void input_mouse_move(i32 x, i32 y) = 0;

            virtual void input_key(HID::KeyCode key, bool pressed) = 0;

            virtual void input_mouse_button(HID::MouseButton button, i32 x, i32 y, bool pressed) = 0;

            virtual void input_mouse_wheel(f32 scroll_x, f32 scroll_y) = 0;

            virtual void input_character(c32 ch) = 0;

            virtual void end_input() = 0;

            //! Renders the generated data.
            virtual void render(VG::IShapeDrawList* draw_list) = 0;
        };

        LUNA_GUI_API Ref<IContext> new_context(VG::IFontAtlas* font_atlas);
    }
}