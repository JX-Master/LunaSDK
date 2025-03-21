/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2025/3/20
*/
#include <Luna/Window/Application.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Window/AppMain.hpp>

namespace Luna
{
    void on_window_close(Window::IWindow* window)
    {
        window->close();
    }

    void on_window_key_pressed(Window::IWindow* window, HID::KeyCode key)
    {
        if(key == HID::KeyCode::spacebar)
        {
            if(window->is_full_screen())
            {
                lupanic_if_failed(window->set_display_settings(Window::WindowDisplaySettings::as_windowed()));
            }
            else
            {
                lupanic_if_failed(window->set_display_settings(Window::WindowDisplaySettings::as_full_screen()));
            }
        }
        else if(key == HID::KeyCode::r)
        {
            lupanic_if_failed(window->set_resizable(!window->is_resizable()));
        }
    }

    Ref<Window::IWindow> g_main_window;

    Window::AppResult app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppResult::failed;
        lutry
        {
            luexp(add_modules({module_window()}));
            Window::StartupParams params;
            Window::set_startup_params(params);
            luexp(init_modules());
            luset(g_main_window, Window::new_window("Window Test", Window::WindowDisplaySettings::as_windowed(), 
                Window::WindowCreationFlag::resizable));
            g_main_window->get_close_event().add_handler(on_window_close);
            g_main_window->get_key_down_event().add_handler(on_window_key_pressed);
        }
        lucatch
        {
            return Window::AppResult::failed;
        }
        return Window::AppResult::ok;
    }

    Window::AppResult app_update(opaque_t app_state)
    {
        if(g_main_window->is_closed()) return Window::AppResult::exiting;
        sleep(16);
        return Window::AppResult::ok;
    }

    void app_close(opaque_t app_state)
    {
        g_main_window.reset();
        Luna::close();
    }
}