/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2025/10/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include "../../Window.hpp"
#include <shellapi.h>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Imm32.lib")

namespace Luna
{
    namespace Window
    {
        constexpr wchar_t* WIN32_CLASS_NAME = L"LunaWindow";

        RV platform_init()
        {
            register_boxed_type<Window>();
            impl_interface_for_type<Window, IWin32Window, IWindow>();
            lutry
            {
                if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                {
                    luthrow(set_error(BasicError::bad_platform_call(), "SetProcessDpiAwarenessContext failed"));
                }
                if (g_startup_params.hInstance == NULL)
                {
                    g_startup_params.hInstance = GetModuleHandleW(NULL);
                }
                WNDCLASSEXW wcex;
                wcex.cbSize = sizeof(WNDCLASSEXW);
                wcex.style = CS_HREDRAW | CS_VREDRAW;// | CS_DBLCLKS;
                wcex.lpfnWndProc = luna_window_win_proc;
                wcex.cbClsExtra = 0;
                wcex.cbWndExtra = sizeof(void*);
                wcex.hInstance = g_startup_params.hInstance;
                wcex.hIcon = g_startup_params.hIcon;
                wcex.hIconSm = g_startup_params.hIconSm;
                wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW); //We need to set the mouse cursor manually, so this must be NULL.
                wcex.hbrBackground = CreateSolidBrush(RGB(30, 30, 30)); //(HBRUSH)(COLOR_WINDOW + 1);
                wcex.lpszMenuName = NULL;
                wcex.lpszClassName = WIN32_CLASS_NAME;
                if (!RegisterClassExW(&wcex))
                {
                    luthrow(set_error(BasicError::bad_platform_call(), "RegisterClassExW failed."));
                }
            }
            lucatchret;
            return ok;
        }
        void platform_close()
        {
            UnregisterClassW(WIN32_CLASS_NAME, g_startup_params.hInstance);
        }
        StartupParams g_startup_params;
        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
		{
			g_startup_params = params;
		}
        LUNA_WINDOW_API const c8* get_app_name()
        {
            return g_startup_params.name;
        }
        LUNA_RUNTIME_API Version get_app_version()
        {
            return g_startup_params.version;
        }
        void Window::close()
        {
            lutsassert_main_thread();
            if (m_hwnd)
            {
                DestroyWindow(m_hwnd);
                luassert(m_hwnd == nullptr); // Set to null when handling WM_DESTROY.
            }
        }
        bool Window::is_closed()
        {
            return m_hwnd == nullptr;
        }
        bool Window::has_input_focus()
        {
            lutsassert_main_thread();
            return m_hwnd && GetFocus() == m_hwnd;
        }
        bool Window::has_mouse_focus()
        {
            lutsassert_main_thread();
            return m_hwnd && GetCapture() == m_hwnd;
        }
        RV Window::set_foreground()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            SetForegroundWindow(m_hwnd);
            return ok;
        }
        bool Window::is_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            return IsIconic(m_hwnd) != 0;
        }
        bool Window::is_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            return IsZoomed(m_hwnd) != 0;
        }
        RV Window::set_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            ShowWindow(m_hwnd, SW_MINIMIZE);
            return ok;
        }
        RV Window::set_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            ShowWindow(m_hwnd, SW_MAXIMIZE);
            return ok;
        }
        RV Window::set_restored()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            ShowWindow(m_hwnd, SW_RESTORE);
            return ok;
        }
        bool Window::is_hovered()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            POINT pt;
            if (!GetCursorPos(&pt)) return false;
            return WindowFromPoint(pt) == m_hwnd;
        }
        bool Window::is_visible()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            return IsWindowVisible(m_hwnd) != 0;
        }
        RV Window::set_visible(bool visible)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
            return ok;
        }
        WindowStyleFlag Window::get_style()
        {
            lutsassert_main_thread();
            return m_style;
        }
        static LONG encode_style(WindowStyleFlag flags)
        {
            LONG wstyle = 0;
            if(test_flags(flags, WindowStyleFlag::borderless))
            {
                wstyle = WS_POPUP;
            }
            else
            {
                wstyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
                if(test_flags(flags, WindowStyleFlag::resizable))
                {
                    wstyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
                }
            }
            return wstyle;
        }
        RV Window::set_style(WindowStyleFlag style)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            m_style = style;
            LONG wstyle = GetWindowLongW(m_hwnd, GWL_STYLE);
            // Reset all window style bits.
            wstyle &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX);
            wstyle |= encode_style(m_style);
            SetWindowLongW(m_hwnd, GWL_STYLE, wstyle);
            if(!SetWindowPos(m_hwnd, NULL, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
            {
                DWORD err = GetLastError();
                return set_error(BasicError::bad_platform_call(), "SetWindowPos failed. Error code: %u", err);
            }
            return ok;
        }
        Int2U Window::get_position()
        {
            lutsassert_main_thread();
            if (is_closed()) return Int2U(0, 0);
            RECT rect;
            GetWindowRect(m_hwnd, &rect);
            return Int2U(rect.left, rect.top);
        }
        static RectI client_rect_to_window_rect(Window* window, const RectI& client_rect)
        {
            RECT rect = { client_rect.offset_x, client_rect.offset_y, client_rect.offset_x + client_rect.width, client_rect.offset_y + client_rect.height };
            DWORD style = GetWindowLongW(window->m_hwnd, GWL_STYLE);
            DWORD ex_style = GetWindowLongW(window->m_hwnd, GWL_EXSTYLE);
            AdjustWindowRectEx(&rect, style, FALSE, ex_style);
            return RectI(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
        }
        RV Window::set_position(i32 x, i32 y)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            
            RectI rect(x, y, 100, 100);
            rect = client_rect_to_window_rect(this, rect);
            SetWindowPos(m_hwnd, NULL, 
                rect.offset_x, rect.offset_y, 0, 0, 
                SWP_NOSIZE | SWP_NOZORDER);
            return ok;
        }
        UInt2U Window::get_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            return UInt2U((u32)(rect.right - rect.left), (u32)(rect.bottom - rect.top));
        }
        RV Window::set_size(u32 width, u32 height)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            
            RectI rect(0, 0, width, height);
            rect = client_rect_to_window_rect(this, rect);
            SetWindowPos(m_hwnd, NULL, 0, 0, 
                rect.width, rect.height, 
                SWP_NOMOVE | SWP_NOZORDER);
            return ok;
        }
        UInt2U Window::get_framebuffer_size()
        {
            // On Windows, framebuffer size equals client size
            return get_size();
        }
        f32 Window::get_dpi_scale_factor()
        {
            lutsassert_main_thread();
            if (is_closed()) return 1.0f;
            UINT dpi = GetDpiForWindow(m_hwnd);
            return (f32)dpi / (f32)USER_DEFAULT_SCREEN_DPI;
        }
        RV Window::set_title(const c8* title)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            
            usize title_len = utf8_to_utf16_len(title);
            StackAllocator alloc;
            c16* buf = (c16*)alloc.allocate(sizeof(c16) * (title_len + 1));
            utf8_to_utf16(buf, title_len + 1, title);
            
            if (!SetWindowTextW(m_hwnd, (LPCWSTR)buf))
            {
                return set_error(BasicError::bad_platform_call(), "SetWindowTextW failed");
            }
            return ok;
        }
        Int2U Window::screen_to_client(const Int2U& point)
        {
            lutsassert_main_thread();
            POINT pt = { point.x, point.y };
            ScreenToClient(m_hwnd, &pt);
            return Int2U(pt.x, pt.y);
        }
        Int2U Window::client_to_screen(const Int2U& point)
        {
            lutsassert_main_thread();
            POINT pt = { point.x, point.y };
            ClientToScreen(m_hwnd, &pt);
            return Int2U(pt.x, pt.y);
        }
        RV Window::begin_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = true;
            return ok;
        }
        RV Window::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            
            // Get IME context
            HIMC himc = ImmGetContext(m_hwnd);
            if (himc)
            {
                // Set composition window position
                COMPOSITIONFORM cf;
                cf.dwStyle = CFS_POINT;
                cf.ptCurrentPos.x = input_rect.offset_x + cursor;
                cf.ptCurrentPos.y = input_rect.offset_y;
                ImmSetCompositionWindow(himc, &cf);
                
                // Set candidate window position
                CANDIDATEFORM cdf;
                cdf.dwStyle = CFS_CANDIDATEPOS;
                cdf.ptCurrentPos.x = input_rect.offset_x + cursor;
                cdf.ptCurrentPos.y = input_rect.offset_y + input_rect.height;
                ImmSetCandidateWindow(himc, &cdf);
                
                ImmReleaseContext(m_hwnd, himc);
            }
            
            return ok;
        }
        RV Window::end_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = false;
            return ok;
        }
        bool Window::is_text_input_active()
        {
            return m_text_input_active;
        }
        HWND Window::get_hwnd()
        {
            lutsassert_main_thread();
            return m_hwnd;
        }
        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
            i32 x,
            i32 y,
            u32 width,
            u32 height,
            WindowStyleFlag style_flags,
            WindowCreationFlag creation_flags)
        {
            Ref<IWindow> ret;
            lutry
            {
                DWORD style = encode_style(style_flags);
                if (x == DEFAULT_POS || y == DEFAULT_POS || width == 0 || height == 0)
                {
                    lulet(screen_rect, get_display_working_area(get_primary_display()));
                    if(width == 0)
                    {
                        width = (u32)screen_rect.width * 7 / 10;
                    }
                    if(height == 0)
                    {
                        height = (u32)screen_rect.height * 7 / 10;
                    }
                    if(x == DEFAULT_POS)
                    {
                        x = screen_rect.offset_x + (screen_rect.width - width) / 2;
                    }
                    if(y == DEFAULT_POS)
                    {
                        y = screen_rect.offset_y + (screen_rect.height - height) / 2;
                    }
                }
                {
                    // Add paddings to the client rect.
                    RECT rect;
                    rect.left = x;
                    rect.top = y;
                    rect.right = width + x;
                    rect.bottom = height + y;
                    AdjustWindowRectEx(&rect, style, FALSE, WS_EX_APPWINDOW);
                    x = rect.left;
                    y = rect.top;
                    width = rect.right - rect.left;
                    height = rect.bottom - rect.top;
                }
                usize title_sz = utf8_to_utf16_len(title);
                StackAllocator alloc;
                wchar_t* window_namew = (wchar_t*)alloc.allocate(sizeof(wchar_t) * (title_sz + 1));
                utf8_to_utf16((c16*)window_namew, title_sz + 1, title);
                HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW, WIN32_CLASS_NAME, window_namew, style, x, y, width, height,
                    nullptr, nullptr, g_startup_params.hInstance, nullptr);
                if (!hwnd)
                {
                    DWORD err = GetLastError();
                    return set_error(BasicError::bad_platform_call(), "CreateWindowExW failed. Error code: %u", err);
                }
                Ref<Window> window = new_object<Window>();
                window->m_hwnd = hwnd;
                window->m_style = style_flags;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window.object());
                // Enable drag and drop.
                DragAcceptFiles(hwnd, TRUE);
                // Enable mouse enter/leave messages.
                {
                    TRACKMOUSEEVENT track_mouse;
                    track_mouse.cbSize = sizeof(TRACKMOUSEEVENT);
                    track_mouse.dwFlags = TME_HOVER | TME_LEAVE;
                    track_mouse.hwndTrack = hwnd;
                    track_mouse.dwHoverTime = HOVER_DEFAULT;
                    if(!TrackMouseEvent(&track_mouse))
                    {
                        DWORD err = GetLastError();
                        return set_error(BasicError::bad_platform_call(), "TrackMouseEvent failed. Error code: %u", err);
                    }
                }
                // Show window if not hidden
                if (!test_flags(creation_flags, WindowCreationFlag::hidden))
                {
                    ShowWindow(hwnd, SW_SHOW);
                    UpdateWindow(hwnd);
                }
                ret = window;
            }
            lucatchret;
            return ret;
        }
    }
}