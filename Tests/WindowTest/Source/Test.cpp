/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Test.cpp
* @author JXMaster
* @date 2022/4/12
*/
#include <Window/Window.hpp>
#include <Runtime/Thread.hpp>
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
                window->set_display_settings(Window::WindowDisplaySettings::as_windowed());
            }
            else
            {
                window->set_display_settings(Window::WindowDisplaySettings::as_full_screen());
            }
        }
        else if(key == HID::KeyCode::r)
        {
            window->set_resizable(!window->is_resizable());
        }
    }

	void multi_window_test_run()
	{
        using namespace Window;
        auto main_window = new_window("Window Test", WindowDisplaySettings::as_windowed(), 
            WindowCreationFlag::resizable).get();
        main_window->get_close_event() += on_window_close;
        main_window->get_key_down_event() += on_window_key_pressed;

        while (!main_window->is_closed())
        {
            // Update window system.
            Window::poll_events();
            sleep(16);
        }
	}
}
