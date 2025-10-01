/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>

#include <Luna/Window/AppMain.hpp>

namespace Luna
{
    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppStatus::failing;
        lutry
        {
            luexp(add_modules({module_rhi_test_bed()}));
            luexp(init_modules());
            luexp(RHITestBed::init());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    Window::AppStatus app_update(opaque_t app_state)
    {
        auto window = RHITestBed::get_window();
        if(window->is_closed()) return Window::AppStatus::exiting;
        if(window->is_minimized())
        {
            sleep(100);
            return Window::AppStatus::running;
        }
        lutry
        {
            luexp(RHITestBed::update());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    void app_close(opaque_t app_state, Window::AppStatus status)
    {
        RHITestBed::close();
        Luna::close();
    }
}