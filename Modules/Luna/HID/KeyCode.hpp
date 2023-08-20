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
		//! A platform-independent key code mapping for standard QWERTY keyboard.
		enum class KeyCode : u16
		{
			unknown = 0x00,

			esc,
			f1,
			f2,
			f3,
			f4,
			f5,
			f6,
			f7,
			f8,
			f9,
			f10,
			f11,
			f12,
			grave,		// ~` key on US standard keyboard
			num0,
			num1,
			num2,
			num3,
			num4,
			num5,
			num6,
			num7,
			num8,
			num9,
			equal,		// =+ key
			minus,		// -_ key
			backspace,
			a,
			b,
			c,
			d,
			e,
			f,
			g,
			h,
			i,
			j,
			k,
			l,
			m,
			n,
			o,
			p,
			q,
			r,
			s,
			t,
			u,
			v,
			w,
			x,
			y,
			z,
			tab,
			caps_lock,
			enter,
			ctrl,
			l_ctrl,
			r_ctrl,
			shift,
			l_shift,
			r_shift,
			menu,
			l_menu,	// Left alt key on Windows, left option key on Mac.
			r_menu,	// Right alt key on Windows, right option key on Mac.
			l_system, // Left Windows key on Windows, Left command key on Mac.
			r_system, // Right Windows key on Windows, Right command key on Mac.
			apps,	  // Applications key on Windows.
			spacebar,
			l_branket,	// [{ key on US standard keyboard
			r_branket,	// ]} key on US standard keyboard
			backslash,	// \| key on US standard keyboard
			semicolon,	// ;: key on US standard keyboard
			quote,		// '" key on US standard keyboard
			comma,		// ,< key
			period,		// .> key
			slash,		// /? key on US standard keyboard
			
			print_screen,
			scroll_lock,
			pause,

			insert,
			home,
			page_up,
			page_down,
			del,
			end,

			left,
			up,
			right,
			down,
			
			num_lock, // clear on Mac.
			numpad0,
			numpad1,
			numpad2,
			numpad3,
			numpad4,
			numpad5,
			numpad6,
			numpad7,
			numpad8,
			numpad9,
			numpad_decimal, // . on numpad.
			numpad_add,     // + on numpad.
			numpad_subtract,// - on numpad.
			numpad_multiply,// * on numpad.
			numpad_divide,  // / on numpad.
            numpad_equal,   // = on numpad (Mac only)
            numpad_enter,   // enter on numpad (Mac only, Windows will always return normal "enter").
		};

		enum class MouseButton : u8
		{
			none = 0x00,
			left = 0x01,
			right = 0x02,
			middle = 0x04,
			function1 = 0x08,
			function2 = 0x10,
		};
	}
}
