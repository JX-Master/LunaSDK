/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2025/10/5
*/
#pragma once
#include "../../Window.hpp"
#include "../../../Cocoa/CocoaWindow.hpp"
#include <objc/objc.h>

namespace Luna
{
    namespace Window
    {
        struct Window : public ICocoaWindow
        {
            lustruct("Window::Window", "093df112-37e0-40de-b0db-90931cb106f7");
            luiimpl();

            id m_window; // NSWindow*
            id m_delegate; // NSWindowDelegate*
            WindowEvents m_events;
            
            // Used to cache files specified by drag and drop
            Vector<String> m_drop_files;
            f32 m_drop_x;
            f32 m_drop_y;
            
            bool m_text_input_active;

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
            virtual WindowEvents& get_events() override
            {
                return m_events;
            }
            virtual id get_nswindow() override;
            
            virtual RV start_text_input() override;
            virtual RV set_text_input_area(const RectI& input_rect, i32 cursor) override;
            virtual RV stop_text_input() override;

            Window();
            ~Window();
        };

        // Associated object key for storing Window* pointer in NSWindow
        constexpr usize WINDOW_POINTER_KEY = strhash<usize>("LunaWindowPointer");
    }
}

