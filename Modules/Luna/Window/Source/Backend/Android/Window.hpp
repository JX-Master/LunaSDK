/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2025/11/25
*/
#pragma once
#include "../../../Android/AndroidWindow.hpp"

#include <android/native_window.h>

namespace Luna
{
    namespace Window
    {
        struct AndroidWindow : IAndroidWindow
        {
            lustruct("Window::AndroidWindow", "41e2a511-bdf4-4003-8501-9aabeb566ab1");
            luiimpl();
            
            ANativeWindow* m_window = nullptr;
            bool m_text_input_active = false;
            bool m_closed = false;

            virtual bool is_closed() override
            {
                return m_closed;
            }
            virtual bool has_input_focus() override;
            virtual bool has_mouse_focus() override;
            virtual bool is_minimized() override;
            virtual Int2U get_position() override;
            virtual UInt2U get_size() override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
            virtual RV begin_text_input() override;
            virtual RV set_text_input_area(const RectI& input_rect, i32 cursor) override;
            virtual RV end_text_input() override;
            virtual bool is_text_input_active() override;
            virtual opaque_t get_native_window() override
            {
                return (opaque_t)m_window;
            }
        };
    }
}