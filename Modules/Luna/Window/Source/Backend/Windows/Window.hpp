/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2025/10/3
*/
#pragma once
#include "../../../Windows/Win32Window.hpp"

namespace Luna
{
    namespace Window
    {
        struct Window : IWin32Window
        {
            lustruct("Window::Window", "{541DB2B8-3EB7-465B-BCCA-522AFFC157CA}");
            luiimpl();

            HWND m_hwnd;
            WindowStyleFlag m_style;
            bool m_text_input_active = false;
            bool m_destructing = false; // If `true`, close is called inside of ~Window, and we should not dispatch close message.
            Window() :
                m_hwnd(nullptr) {}
            ~Window()
            {
                m_destructing = true;
                close();
            }
            virtual void close() override;
            virtual bool is_closed() override;
            virtual bool has_input_focus() override;
            virtual bool has_mouse_focus() override;
            virtual RV set_foreground() override;
            virtual bool is_minimized() override;
            virtual bool is_maximized() override;
            virtual RV set_minimized() override;
            virtual RV set_maximized() override;
            virtual RV set_restored() override;
            virtual bool is_hovered() override;
            virtual bool is_visible() override;
            virtual RV set_visible(bool visible) override;
            virtual WindowStyleFlag get_style() override;
            virtual RV set_style(WindowStyleFlag style) override;
            virtual Int2U get_position() override;
            virtual RV set_position(i32 x, i32 y) override;
            virtual UInt2U get_size() override;
            virtual RV set_size(u32 width, u32 height) override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual RV set_title(const c8* title) override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
            virtual RV begin_text_input() override;
            virtual RV set_text_input_area(const RectI& input_rect, i32 cursor) override;
            virtual RV end_text_input() override;
            virtual bool is_text_input_active() override;
            virtual HWND get_hwnd() override;
        };
    }
}

LRESULT CALLBACK luna_window_win_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);