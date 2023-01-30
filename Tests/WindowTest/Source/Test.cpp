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

	void multi_window_test_run()
	{
        using namespace Window;
        auto main_window = new_window("Window Test", 0, 0, 0, 0, nullptr, WindowCreationFlag::default_size |
            WindowCreationFlag::position_center |
            WindowCreationFlag::resizable |
            WindowCreationFlag::minimizable |
            WindowCreationFlag::maximizable).get();
        main_window->get_close_event() += on_window_close;

        auto normal_window = new_window("Normal Window", 100, 100, 640, 480, nullptr, WindowCreationFlag::resizable |
            WindowCreationFlag::minimizable | WindowCreationFlag::maximizable).get();

        normal_window->get_close_event() += on_window_close;

        auto borderless_window = new_window("Borderless Window", 100, 100, 640, 480, nullptr, WindowCreationFlag::borderless |
            WindowCreationFlag::resizable | WindowCreationFlag::minimizable | WindowCreationFlag::maximizable).get();

        borderless_window->get_close_event() += on_window_close;

        luassert_always(normal_window->get_size() == UInt2U(640, 480));
        luassert_always(borderless_window->get_size() == UInt2U(640, 480));

        while (!main_window->is_closed())
        {
            // Update window system.
            Window::poll_events();
            sleep(16);
        }
	}
}
