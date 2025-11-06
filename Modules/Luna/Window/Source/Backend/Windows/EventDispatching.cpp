/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.cpp
* @author JXMaster
* @date 2025/10/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <windowsx.h>
#include <shellapi.h>
#include <Luna/Runtime/Unicode.hpp>
#include "../../../Event.hpp"
#include "../../Event.hpp"
#include <Luna/Runtime/TSAssert.hpp>

#pragma comment(lib, "Shell32.lib")

namespace Luna
{
    namespace Window
    {
        static HID::KeyCode translate_virtual_key(int vk)
        {
            if (vk >= 0x30 && vk <= 0x39)
            {
                return (HID::KeyCode)((u16)HID::KeyCode::num0 + vk - 0x30);
            }
            if (vk >= 0x41 && vk <= 0x5A)
            {
                return (HID::KeyCode)((u16)HID::KeyCode::a + vk - 0x41);
            }
            if (vk >= VK_F1 && vk <= VK_F12)
            {
                return (HID::KeyCode)((u16)HID::KeyCode::f1 + vk - VK_F1);
            }
            if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
            {
                return (HID::KeyCode)((u16)HID::KeyCode::numpad0 + vk - VK_NUMPAD0);
            }
            switch (vk)
            {
            case VK_ESCAPE:        return HID::KeyCode::esc;
            case VK_OEM_3:        return HID::KeyCode::grave;
            case VK_OEM_PLUS:    return HID::KeyCode::equal;
            case VK_OEM_MINUS:    return HID::KeyCode::minus;
            case VK_BACK:        return HID::KeyCode::backspace;
            case VK_TAB:        return HID::KeyCode::tab;
            case VK_CAPITAL:    return HID::KeyCode::caps_lock;
            case VK_RETURN:        return HID::KeyCode::enter;
            case VK_LCONTROL:    return HID::KeyCode::l_ctrl;
            case VK_RCONTROL:    return HID::KeyCode::r_ctrl;
            case VK_CONTROL:    return HID::KeyCode::ctrl;
            case VK_LSHIFT:        return HID::KeyCode::l_shift;
            case VK_RSHIFT:        return HID::KeyCode::r_shift;
            case VK_SHIFT:        return HID::KeyCode::shift;
            case VK_LMENU:        return HID::KeyCode::l_menu;
            case VK_RMENU:        return HID::KeyCode::r_menu;
            case VK_MENU:        return HID::KeyCode::menu;
            case VK_LWIN:        return HID::KeyCode::l_system;
            case VK_RWIN:        return HID::KeyCode::r_system;
            case VK_APPS:        return HID::KeyCode::apps;
            case VK_SPACE:        return HID::KeyCode::spacebar;
            case VK_OEM_4:        return HID::KeyCode::l_branket;
            case VK_OEM_6:        return HID::KeyCode::r_branket;
            case VK_OEM_5:        return HID::KeyCode::backslash;
            case VK_OEM_1:        return HID::KeyCode::semicolon;
            case VK_OEM_7:        return HID::KeyCode::quote;
            case VK_OEM_COMMA:    return HID::KeyCode::comma;
            case VK_OEM_PERIOD:    return HID::KeyCode::period;
            case VK_OEM_2:        return HID::KeyCode::slash;
            case VK_SNAPSHOT:    return HID::KeyCode::print_screen;
            case VK_SCROLL:        return HID::KeyCode::scroll_lock;
            case VK_PAUSE:        return HID::KeyCode::pause;
            case VK_INSERT:        return HID::KeyCode::insert;
            case VK_HOME:        return HID::KeyCode::home;
            case VK_PRIOR:        return HID::KeyCode::page_up;
            case VK_NEXT:        return HID::KeyCode::page_down;
            case VK_DELETE:        return HID::KeyCode::del;
            case VK_END:        return HID::KeyCode::end;
            case VK_LEFT:        return HID::KeyCode::left;
            case VK_UP:            return HID::KeyCode::up;
            case VK_RIGHT:        return HID::KeyCode::right;
            case VK_DOWN:        return HID::KeyCode::down;
            case VK_NUMLOCK:    return HID::KeyCode::num_lock;
            case VK_DECIMAL:    return HID::KeyCode::numpad_decimal;
            case VK_ADD:        return HID::KeyCode::numpad_add;
            case VK_SUBTRACT:    return HID::KeyCode::numpad_subtract;
            case VK_MULTIPLY:    return HID::KeyCode::numpad_multiply;
            case VK_DIVIDE:        return HID::KeyCode::numpad_divide;
            //case VK_CLEAR:        return HID::KeyCode::clear;
            default: lupanic(); return HID::KeyCode::unknown;
            }
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            lutsassert_main_thread();
            MSG msg;
            if (wait_events)
            {
                // Wait for at least one event.
                if (GetMessageW(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }
            // Poll all remaining events.
            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }
}

LRESULT CALLBACK luna_window_win_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    using namespace Luna;
    using namespace Luna::Window;
    Luna::Window::Window* pw = (Luna::Window::Window*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    if (!pw)
    {
        // If the window object is not ready, we simply do not handle all messages.
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    switch (msg)
    {
    case WM_CLOSE:
        {
            auto event = new_object<WindowRequestCloseEvent>();
            event->window = pw;
            event->do_close = true;
            dispatch_event_to_handler(event.object());
            if(event->do_close)
            {
                return DefWindowProcW(hWnd, msg, wParam, lParam);
            }
        }
        return 0;
    case WM_DESTROY:
        {
            if(!pw->m_destructing)
            {
                auto event = new_object<WindowClosedEvent>();
                event->window = pw;
                dispatch_event_to_handler(event.object());
            }
        }
        if(pw->m_hwnd)
        {
            pw->m_hwnd = nullptr;
        }
        return 0;
    case WM_SETFOCUS:
        {
            auto event = new_object<WindowInputFocusEvent>();
            event->window = pw;
            dispatch_event_to_handler(event.object());
        }
        return 0;
    case WM_KILLFOCUS:
        {
            auto event = new_object<WindowLoseInputFocusEvent>();
            event->window = pw;
            dispatch_event_to_handler(event.object());
        }
        return 0;
    case WM_SHOWWINDOW:
        if (wParam == TRUE)
        {
            auto event = new_object<WindowShowEvent>();
            event->window = pw;
            dispatch_event_to_handler(event.object());
        }
        else
        {
            auto event = new_object<WindowHideEvent>();
            event->window = pw;
            dispatch_event_to_handler(event.object());
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    case WM_SIZE:
    {
        u32 width = LOWORD(lParam);
        u32 height = HIWORD(lParam);
        {
            auto event = new_object<WindowResizeEvent>();
            event->window = pw;
            event->width = width;
            event->height = height;
            dispatch_event_to_handler(event.object());
        }
        {
            auto event = new_object<WindowFramebufferResizeEvent>();
            event->window = pw;
            event->width = width;
            event->height = height;
            dispatch_event_to_handler(event.object());
        }
        return 0;
    }
    case WM_MOVE:
    {
        i32 x = (int)(short)LOWORD(lParam);
        i32 y = (int)(short)HIWORD(lParam);
        auto event = new_object<WindowMoveEvent>();
        event->window = pw;
        event->x = x;
        event->y = y;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    // case WM_ENTERSIZEMOVE:
    //     //pw->m_events.begin_resize_move(pw);
    //     return 0;
    // case WM_EXITSIZEMOVE:
    //     //pw->m_events.end_resize_move(pw);
    //     return 0;
    case WM_DPICHANGED:
    {
        auto event = new_object<WindowDPIScaleChangedEvent>();
        event->window = pw;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_KEYDOWN:
    {
        int key = (int)wParam;
        if (key == VK_PROCESSKEY)
        {
            return 0;
        }
        auto event = new_object<WindowKeyDownEvent>();
        event->window = pw;
        event->key = translate_virtual_key(key);
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_KEYUP:
    {
        int key = (int)wParam;
        if (key == VK_PROCESSKEY)
        {
            return 0;
        }
        auto event = new_object<WindowKeyUpEvent>();
        event->window = pw;
        event->key = translate_virtual_key(key);
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_CHAR:
    {
        if(pw->m_text_input_active)
        {
            auto character = (c32)wParam;
            auto event = new_object<WindowInputTextEvent>();
            event->window = pw;
            c8 buf[6];
            usize size = utf8_encode_char(buf, character);
            event->text.append(buf, size);
            dispatch_event_to_handler(event.object());
        }
        return 0;
    }
    case WM_UNICHAR:
    {
        if (wParam == UNICODE_NOCHAR)
        {
            return TRUE; // We support WM_UNICHAR
        }
        if (pw->m_text_input_active)
        {
            c32 character = (c32)wParam;
            auto event = new_object<WindowInputTextEvent>();
            event->window = pw;
            c8 buf[6];
            usize size = utf8_encode_char(buf, character);
            event->text.append(buf, size);
            dispatch_event_to_handler(event.object());
            return FALSE;
        }
        return FALSE;
    }
    case WM_MOUSEHOVER:
    {
        auto event = new_object<WindowMouseEnterEvent>();
        event->window = pw;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_MOUSELEAVE:
    {
        auto event = new_object<WindowMouseLeaveEvent>();
        event->window = pw;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        i32 x = GET_X_LPARAM(lParam);
        i32 y = GET_Y_LPARAM(lParam);
        auto event = new_object<WindowMouseMoveEvent>();
        event->window = pw;
        event->x = x;
        event->y = y;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
        HID::MouseButton btn;
        if (msg == WM_LBUTTONDOWN) btn = HID::MouseButton::left;
        else if (msg == WM_RBUTTONDOWN) btn = HID::MouseButton::right;
        else btn = HID::MouseButton::middle;
        auto event = new_object<WindowMouseDownEvent>();
        event->window = pw;
        event->button = btn;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_XBUTTONDOWN:
    {
        short x = HIWORD(wParam);
        auto button = x & XBUTTON1 ? HID::MouseButton::function1 : HID::MouseButton::function2;
        auto event = new_object<WindowMouseDownEvent>();
        event->window = pw;
        event->button = button;
        dispatch_event_to_handler(event.object());
        return TRUE;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        HID::MouseButton button;
        if (msg == WM_LBUTTONUP) button = HID::MouseButton::left;
        else if (msg == WM_RBUTTONUP) button = HID::MouseButton::right;
        else button = HID::MouseButton::middle;
        auto event = new_object<WindowMouseUpEvent>();
        event->window = pw;
        event->button = button;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_XBUTTONUP:
    {
        short x = HIWORD(wParam);
        auto button = x & XBUTTON1 ? HID::MouseButton::function1 : HID::MouseButton::function2;
        auto event = new_object<WindowMouseUpEvent>();
        event->window = pw;
        event->button = button;
        dispatch_event_to_handler(event.object());
        return TRUE;
    }
    /*case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    {
        short modifiers = LOWORD(wParam);
        WindowEventMouseButton e;
        e.modifier_flags = set_modifier_flags(modifiers);
        if (msg == WM_LBUTTONDBLCLK) e.button = HID::MouseButton::left;
        else if (msg == WM_RBUTTONDBLCLK) e.button = HID::MouseButton::right;
        else e.button = HID::MouseButton::middle;
        pw->broadcast_event(WindowEventType::mouse_dbclk, &e, sizeof(e));
        return 0;
    }*/
    case WM_MOUSEWHEEL:
    {
        short modifiers = LOWORD(wParam);
        auto x_wheel_delta = 0.0f;
        auto y_wheel_delta = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        //e.modifier_flags = set_modifier_flags(modifiers);
        auto event = new_object<WindowScrollEvent>();
        event->window = pw;
        event->scroll_x = x_wheel_delta;
        event->scroll_y = y_wheel_delta;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_MOUSEHWHEEL:
    {
        short modifiers = LOWORD(wParam);
        auto x_wheel_delta = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        auto y_wheel_delta = 0.0f;
        //e.modifier_flags = set_modifier_flags(modifiers);
        auto event = new_object<WindowScrollEvent>();
        event->window = pw;
        event->scroll_x = x_wheel_delta;
        event->scroll_y = y_wheel_delta;
        dispatch_event_to_handler(event.object());
        return 0;
    }
    case WM_DROPFILES:
    {
        HDROP hdrop = (HDROP)wParam;
        UINT file_count = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);

        StackAllocator alloc;
        Array<String> files(file_count);
        
        for (UINT i = 0; i < file_count; ++i)
        {
            UINT path_len = DragQueryFileW(hdrop, i, NULL, 0);
            WCHAR* buf = (WCHAR*)alloc.allocate(sizeof(WCHAR) * (path_len + 1));
            DragQueryFileW(hdrop, i, buf, path_len + 1);
            usize utf8_len = utf16_to_utf8_len((const c16*)buf);
            c8* buf_2 = (c8*)alloc.allocate(sizeof(c8) * (utf8_len + 1));
            utf16_to_utf8(buf_2, utf8_len + 1, (const c16*)buf);
            files[i] = buf_2;
        }

        POINT pt = {0, 0};
        DragQueryPoint(hdrop, &pt);
        auto event = new_object<WindowDropFilesEvent>();
        event->window = pw;
        event->files = move(files);
        event->x = pt.x;
        event->y = pt.y;
        dispatch_event_to_handler(event.object());

        DragFinish(hdrop);
        return 0;
    }
    default: break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}