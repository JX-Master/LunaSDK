/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIWindowShared.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include <Luna/HID/Mouse.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/Window/Clipboard.hpp>
#include <Luna/Window/Window.hpp>

namespace Luna
{
    namespace GUIWindow
    {
        inline bool is_client_position_valid(Window::IWindow* window, const Float2U& position)
        {
            UInt2U size = window->get_size();
            return position.x >= 0.0f && position.y >= 0.0f &&
                position.x < (f32)size.x && position.y < (f32)size.y;
        }

        inline Float2U get_client_mouse_pos_unchecked(Window::IWindow* window)
        {
            Int2U screen_pos = HID::get_mouse_pos();
            Int2U client_pos = window->screen_to_client(screen_pos);
            return Float2U((f32)client_pos.x, (f32)client_pos.y);
        }

        inline bool get_client_mouse_pos(Window::IWindow* window, Float2U& position)
        {
            position = get_client_mouse_pos_unchecked(window);
            return is_client_position_valid(window, position);
        }

        inline RectI to_window_text_input_rect(const RectF& rect)
        {
            return RectI(
                (i32)floor(rect.offset_x),
                (i32)floor(rect.offset_y),
                max((i32)ceil(rect.width), 1),
                max((i32)ceil(rect.height), 1));
        }

        inline RV get_window_clipboard_text(String& out_text, void*)
        {
            return Window::get_clipboard_text(out_text);
        }

        inline RV set_window_clipboard_text(const c8* text, usize size, void*)
        {
            return Window::set_clipboard_text(text, size);
        }
    }
}
