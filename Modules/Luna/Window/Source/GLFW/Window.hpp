/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2022/4/10
*/
#pragma once
#include "../../Window.hpp"

#include <GLFW/glfw3.h>
#include "../WindowEvents.hpp"

#ifdef LUNA_PLATFORM_WINDOWS
#include "../../Windows/Win32Window.hpp"
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#ifdef LUNA_PLATFORM_MACOS
#include "../../Cocoa/CocoaWindow.hpp"
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

namespace Luna
{
    namespace Window
    {
        inline ErrCode encode_glfw_error()
        {
            const char* description;
            int code = glfwGetError(&description);
            if (code != GLFW_NO_ERROR)
            {
                ErrCode err_code;
                switch (code)
                {
                case GLFW_NOT_INITIALIZED: err_code = BasicError::bad_calling_time(); break;
                case GLFW_NO_CURRENT_CONTEXT: err_code = BasicError::bad_platform_call(); break;
                case GLFW_INVALID_ENUM: err_code = BasicError::bad_arguments(); break;
                case GLFW_INVALID_VALUE: err_code = BasicError::bad_arguments(); break;
                case GLFW_OUT_OF_MEMORY: err_code = BasicError::out_of_memory(); break;
                case GLFW_API_UNAVAILABLE: err_code = BasicError::not_supported(); break;
                case GLFW_VERSION_UNAVAILABLE: err_code = BasicError::not_supported(); break;
                case GLFW_PLATFORM_ERROR: err_code = BasicError::bad_platform_call(); break;
                case GLFW_FORMAT_UNAVAILABLE: err_code = BasicError::not_supported(); break;
                default: err_code = BasicError::bad_platform_call(); break;
                }
                // Handle error.
                if (description)
                {
                    set_error(err_code, description);
                    return BasicError::error_object();
                }
                return err_code;
            }
            return ErrCode(0);
        }

        inline RV check_glfw_error()
        {
            ErrCode code = encode_glfw_error();
            if (code.code != 0)
            {
                return code;
            }
            return ok;
        }

        inline bool glfw_succeeded()
        {
            return glfwGetError(NULL) == GLFW_NO_ERROR;
        }

#if defined(LUNA_PLATFORM_WINDOWS)
        struct Window : public IWin32Window
#elif defined(LUNA_PLATFORM_MACOS)
        struct Window : public ICocoaWindow
#else
        struct Window : public IWindow
#endif
        {
            lustruct("RHI::Window", "{2b85ad9e-f949-448a-8d4a-98c4ed39d537}");
            luiimpl();

            GLFWwindow* m_window;
            WindowCallbacks m_callbacks;

            // Used to restore window size and pos when exiting from full screen mode.
            u32 m_windowed_width;
            u32 m_windowed_height;
            i32 m_windowed_pos_x;
            i32 m_windowed_pos_y;

            virtual void close() override;
            virtual bool is_closed() override { return m_window == nullptr; }
            virtual bool is_focused() override { return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0; }
            virtual RV set_focus() override{ glfwFocusWindow(m_window); return check_glfw_error(); }
            virtual bool is_minimized() override { return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0; }
            virtual bool is_maximized() override { return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != 0; }
            virtual RV set_minimized() override { glfwIconifyWindow(m_window); return check_glfw_error(); }
            virtual RV set_maximized() override { glfwMaximizeWindow(m_window); return check_glfw_error(); }
            virtual RV set_restored() override { glfwRestoreWindow(m_window); return check_glfw_error(); }
            virtual bool is_hovered() override{ return glfwGetWindowAttrib(m_window, GLFW_HOVERED) != 0; }
            virtual bool is_visible() override { return glfwGetWindowAttrib(m_window, GLFW_VISIBLE) != 0; }
            virtual RV set_visible(bool visible) override { if (visible) glfwShowWindow(m_window); else glfwHideWindow(m_window); return check_glfw_error(); }
            virtual bool is_resizable() override { return glfwGetWindowAttrib(m_window, GLFW_RESIZABLE) != 0; }
            virtual RV set_resizable(bool resizable) override { glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE); return check_glfw_error(); }
            virtual bool is_borderless() override { return glfwGetWindowAttrib(m_window, GLFW_DECORATED) == 0; }
            virtual RV set_borderless(bool borderless) override { glfwSetWindowAttrib(m_window, GLFW_DECORATED, borderless ? GLFW_FALSE : GLFW_TRUE); return check_glfw_error(); }
            virtual Int2U get_position() override;
            virtual RV set_position(i32 x, i32 y) override;
            virtual UInt2U get_size() override;
            virtual RV set_size(u32 width, u32 height) override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual bool is_full_screen() override { return get_monitor() != nullptr; }
            virtual display_t get_monitor() override {return glfwGetWindowMonitor(m_window); }
            virtual RV set_title(const c8* title) override { glfwSetWindowTitle(m_window, title); return check_glfw_error(); }
            virtual RV set_display_settings(const WindowDisplaySettings& display_settings) override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
#ifdef LUNA_PLATFORM_WINDOWS
            virtual HWND get_hwnd() override
            {
                return glfwGetWin32Window(m_window);
            }
#endif
#ifdef LUNA_PLATFORM_MACOS
            virtual id get_nswindow() override
            {
                return glfwGetCocoaWindow(m_window);
            }
#endif
            virtual WindowCallbacks& get_callbacks() override
            {
                return m_callbacks;
            }
            Window() :
                m_window(nullptr) {}
            ~Window()
            {
                close();
            }
        };
    }
}