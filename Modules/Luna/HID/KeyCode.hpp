/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MathCore.hpp
* @author JXMaster
* @date 2019/1/5
 */
#pragma once
#include <Luna/Runtime/Base.hpp>

namespace Luna
{
    namespace HID
    {
        //! @addtogroup HID
        //! @{
        
        //! A platform-independent key code mapping for standard QWERTY keyboard.
        enum class KeyCode : u16
        {
            unknown = 0x00,

            //! ESC key.
            esc,
            //! F1 key.
            f1,
            //! F2 key.
            f2,
            //! F3 key.
            f3,
            //! F4 key.
            f4,
            //! F5 key.
            f5,
            //! F6 key.
            f6,
            //! F7 key.
            f7,
            //! F8 key.
            f8,
            //! F9 key.
            f9,
            //! F10 key.
            f10,
            //! F11 key.
            f11,
            //! F12 key.
            f12,
            //! ~ key.
            grave,
            //! 0 key.
            num0,
            //! 1 key.
            num1,
            //! 2 key.
            num2,
            //! 3 key.
            num3,
            //! 4 key.
            num4,
            //! 5 key.
            num5,
            //! 6 key.
            num6,
            //! 7 key.
            num7,
            //! 8 key.
            num8,
            //! 9 key.
            num9,
            //! =+ key.
            equal,
            //! -_ key.
            minus,
            //! Backspace key.
            backspace,
            //! A key.
            a,
            //! B key.
            b,
            //! C key.
            c,
            //! D key.
            d,
            //! E key.
            e,
            //! F key.
            f,
            //! G key.
            g,
            //! H key.
            h,
            //! I key.
            i,
            //! J key.
            j,
            //! K key.
            k,
            //! L key.
            l,
            //! M key.
            m,
            //! N key.
            n,
            //! O key.
            o,
            //! P key.
            p,
            //! Q key.
            q,
            //! R key.
            r,
            //! S key.
            s,
            //! T key.
            t,
            //! U key.
            u,
            //! V key.
            v,
            //! W key.
            w,
            //! X key.
            x,
            //! Y key.
            y,
            //! Z key.
            z,
            //! Tab key.
            tab,
            //! CapsLock key.
            caps_lock,
            //! Enter key.
            enter,
            //! Control key on any side.
            ctrl,
            //! Left control key.
            l_ctrl,
            //! Right control key.
            r_ctrl,
            //! Shift key on any side.
            shift,
            //! Left shift key.
            l_shift,
            //! Right shift key.
            r_shift,
            //! Alt key on Windows, option key on Mac, on any side.
            menu,
            //! Left alt key on Windows, left option key on Mac.
            l_menu,
            //! Right alt key on Windows, right option key on Mac.
            r_menu,
            //! Left Windows key on Windows, left command key on Mac.
            l_system,
            //! Right Windows key on Windows, right command key on Mac.
            r_system,
            //! Applications key on Windows, usually the at the right of the
            //! right Windows key. Not all keyboards have this key.
            apps,
            //! Spacebar key.
            spacebar,
            //! [{ key on US standard keyboard
            l_branket,
            //! ]} key on US standard keyboard
            r_branket,
            //! \| key on US standard keyboard
            backslash,
            //! ;: key on US standard keyboard
            semicolon,
            //! '" key on US standard keyboard
            quote,
            //! ,< key
            comma,
            //! .> key
            period,
            //! /? key on US standard keyboard
            slash,
            //! Print screen key.
            print_screen,
            //! Scroll lock key.
            scroll_lock,
            //! Pause key.
            pause,
            //! Insert key.
            insert,
            //! Home key.
            home,
            //! Page up key.
            page_up,
            //! Page down key.
            page_down,
            //! Delete key.
            del,
            //! End key.
            end,
            //! Left arrow key.
            left,
            //! Up arrow key.
            up,
            //! Right arrow key.
            right,
            //! Down arrow key.
            down,
            //! Number lock key on Windows, clear key on Mac.
            num_lock,
            //! 0 key on the number pad.
            numpad0,
            //! 1 key on the number pad.
            numpad1,
            //! 2 key on the number pad.
            numpad2,
            //! 3 key on the number pad.
            numpad3,
            //! 4 key on the number pad.
            numpad4,
            //! 5 key on the number pad.
            numpad5,
            //! 6 key on the number pad.
            numpad6,
            //! 7 key on the number pad.
            numpad7,
            //! 8 key on the number pad.
            numpad8,
            //! 9 key on the number pad.
            numpad9,
            // . key on the number pad.
            numpad_decimal,
            // + key on the number pad.
            numpad_add,
            // - key on the number pad.
            numpad_subtract,
            // * on the number pad.
            numpad_multiply,
            // / on the number pad.
            numpad_divide,
            // = on the number pad (Mac only)
            numpad_equal,
            // enter on the number pad (Mac only, Windows will always return normal "enter").
            numpad_enter,
        };

        //! Specifies mouse button. These values can be bitwise-OR combined to select multiple mouse buttons if needed.
        enum class MouseButton : u8
        {
            none = 0x00,
            //! The left mouse button.
            left = 0x01,
            //! The right mouse button.
            right = 0x02,
            //! The middle mouse button.
            middle = 0x04,
            //! The function 1 mouse button.
            function1 = 0x08,
            //! The function 2 mouse button.
            function2 = 0x10,
        };

        //! @}
    }
}
