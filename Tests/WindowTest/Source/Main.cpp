/*!
* This file is a portion of LunaSDK.
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
#include <Luna/Window/Window.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Runtime/Log.hpp>

int luna_main(int argc, const char* argv[])
{
    using namespace Luna;
    if(!Luna::init()) return -1;
    lutry
    {
        luexp(add_modules({module_window()}));
        Window::StartupParams params;
        Window::set_startup_params(params);
        luexp(init_modules());

        Window::set_event_handler([](object_t event, void* userdata)
        {
            if(auto e = cast_object<Window::WindowKeyDownEvent>(event))
            {
                if(e->key == HID::KeyCode::r)
                {
                    auto style = e->window->get_style();
                    set_flags(style, Window::WindowStyleFlag::resizable, !test_flags(style, Window::WindowStyleFlag::resizable));
                    lupanic_if_failed(e->window->set_style(style));
                }
            }
        }, nullptr);

        lulet(window, Window::new_window("Window Test"));

        while(true)
        {
            Window::poll_events();
            if(window->is_closed()) break;
            sleep(16);
        }
    }
    lucatch
    {
        log_error("WindowTest", "%s", explain(luerr));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}