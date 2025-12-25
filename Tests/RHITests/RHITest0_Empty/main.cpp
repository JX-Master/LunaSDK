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
#include <Luna/Window/Event.hpp>
#include <Luna/Window/AppMain.hpp>

using namespace Luna;

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    lutry
    {
        luexp(add_modules({module_rhi_test_bed()}));
        luexp(init_modules());
        luexp(RHITestBed::init());
        while(true)
        {
            Window::poll_events();
            auto window = RHITestBed::get_window();
            if(window->is_closed()) break;
            if(window->is_minimized())
            {
                sleep(100);
                continue;
            }
            luexp(RHITestBed::update());
        }
        RHITestBed::close();
    }
    lucatch
    {
        log_error("RHITest", "%s", explain(luerr));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}