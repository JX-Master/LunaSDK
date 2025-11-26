/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2025/11/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include "../../Window.hpp"
#include "../../../Android/AppMainAndroid.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include "../../Event.hpp"

extern "C"
{
#include "../../../Android/native_app_glue/android_native_app_glue.h"
#include <android/native_activity.h>
#include <android/configuration.h>
}

namespace Luna
{
    namespace Window
    {
        static android_app* g_android_app = nullptr;
        Ref<AndroidWindow> g_window;

        RV platform_init()
        {
            lutry
            {
                register_struct_type<AndroidWindow>({});
                impl_interface_for_type<AndroidWindow, IAndroidWindow, IWindow>();
            }
            lucatchret
            return ok;
        }

        void platform_close()
        {
            g_window.reset();
        }

        LUNA_WINDOW_API IWindow* get_system_window()
        {
            return g_window.get();
        }

        void handle_cmd(android_app *pApp, int32_t cmd) 
        {
            switch (cmd) 
            {
                case APP_CMD_INIT_WINDOW:
                {
                    g_window = new_object<AndroidWindow>();
                    g_window->m_window = pApp->window;
                }
                    break;
                case APP_CMD_TERM_WINDOW:
                {
                    auto e = new_object<WindowClosedEvent>();
                    e->window = g_window;
                    dispatch_event_to_handler(e.object());
                    g_window->m_window = nullptr;
                    g_window.reset();
                }
                    break;
                case APP_CMD_WINDOW_RESIZED:
                {
                    auto e = new_object<WindowResizeEvent>();
                    e->window = g_window;
                    e->width = ANativeWindow_getWidth(g_window->m_window);
                    e->height = ANativeWindow_getHeight(g_window->m_window);
                    dispatch_event_to_handler(e.object());
                }
                    break;

                case APP_CMD_GAINED_FOCUS:
                {
                    auto e = new_object<WindowInputFocusEvent>();
                    e->window = g_window;
                    dispatch_event_to_handler(e.object());
                }
                    break;

                case APP_CMD_LOST_FOCUS:
                {
                    auto e = new_object<WindowLoseInputFocusEvent>();
                    e->window = g_window;
                    dispatch_event_to_handler(e.object());
                }
                    break;
                case APP_CMD_LOW_MEMORY:
                {
                    auto e = new_object<ApplicationDidReceiveMemoryWarningEvent>();
                    dispatch_event_to_handler(e.object());
                }
                    break;
                case APP_CMD_START:
                {
                    // Do nothing.
                }
                    break;
                case APP_CMD_RESUME:
                {
                    auto e = new_object<ApplicationDidEnterForegroundEvent>();
                    dispatch_event_to_handler(e.object());
                }
                    break;

                case APP_CMD_PAUSE:
                {
                    auto e = new_object<ApplicationDidEnterBackgroundEvent>();
                    dispatch_event_to_handler(e.object());
                }
                    break;

                case APP_CMD_STOP:
                {
                    // Do nothing.
                }
                    break;
                case APP_CMD_DESTROY:
                {
                    auto e = new_object<ApplicationWillTerminateEvent>();
                    dispatch_event_to_handler(e.object());
                }
                    break;
                default:
                    break;
            }
        }
        
        LUNA_WINDOW_API void set_android_app(android_app *pApp)
        {
            g_android_app = pApp;
            g_android_app->onAppCmd = handle_cmd;
        }
        
        bool AndroidWindow::has_input_focus()
        {
            lutsassert_main_thread();
            return !is_closed();
        }

        bool AndroidWindow::has_mouse_focus()
        {
            lutsassert_main_thread();
            return !is_closed();
        }

        bool AndroidWindow::is_minimized()
        {
            lutsassert_main_thread();
            // Android native window is always fullscreen; treat as not minimized.
            return false;
        }

        Int2U AndroidWindow::get_position()
        {
            lutsassert_main_thread();
            // Fullscreen window at (0, 0).
            return Int2U(0, 0);
        }

        UInt2U AndroidWindow::get_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            return UInt2U((u32)ANativeWindow_getWidth(m_window),
                (u32)ANativeWindow_getHeight(m_window));
        }

        UInt2U AndroidWindow::get_framebuffer_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            // On Android, framebuffer matches ANativeWindow buffer size.
            return UInt2U((u32)ANativeWindow_getWidth(m_window),
                (u32)ANativeWindow_getHeight(m_window));
        }

        f32 AndroidWindow::get_dpi_scale_factor()
        {
            lutsassert_main_thread();
            // Use configuration density (160 is baseline mdpi, i.e., scale 1.0).
            if (g_android_app && g_android_app->config)
            {
                int32_t density = AConfiguration_getDensity(g_android_app->config);
                // Handle special values.
                if (density > 0 &&
                    density != ACONFIGURATION_DENSITY_DEFAULT &&
                    density != ACONFIGURATION_DENSITY_NONE &&
                    density != ACONFIGURATION_DENSITY_ANY)
                {
                    return (f32)density / 160.0f;
                }
            }
            return 1.0f;
        }

        Int2U AndroidWindow::screen_to_client(const Int2U& point)
        {
            lutsassert_main_thread();
            // Fullscreen window with origin at (0, 0).
            return point;
        }

        Int2U AndroidWindow::client_to_screen(const Int2U& point)
        {
            lutsassert_main_thread();
            // Fullscreen window with origin at (0, 0).
            return point;
        }

        RV AndroidWindow::begin_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = true;
            if (g_android_app && g_android_app->activity)
            {
                ANativeActivity_showSoftInput(g_android_app->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
            }
            return ok;
        }

        RV AndroidWindow::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            lutsassert_main_thread();
            // No-op on Android for now.
            (void)input_rect;
            (void)cursor;
            return ok;
        }

        RV AndroidWindow::end_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = false;
            if (g_android_app && g_android_app->activity)
            {
                ANativeActivity_hideSoftInput(g_android_app->activity, ANATIVEACTIVITY_HIDE_SOFT_INPUT_NOT_ALWAYS);
            }
            return ok;
        }

        bool AndroidWindow::is_text_input_active()
        {
            return m_text_input_active;
        }
    }
}