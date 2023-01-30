/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Test.cpp
* @author JXMaster
* @date 2022/4/12
*/
#include <Runtime/PlatformDefines.hpp>
#ifdef LUNA_PLATFORM_LINUX

#include <Runtime/Module.hpp>
#include <Window/Window.hpp>
#include <Runtime/Runtime.hpp>

namespace Luna
{
void multi_window_test_run();
}

void run()
{
    using namespace Luna;
    using namespace Luna::Windowing;
    
    StartupParams params;
    params.main_window_title = "Window Test";
    params.main_window_flags = WindowCreationFlag::default_size |
        WindowCreationFlag::position_center |
        WindowCreationFlag::resizable |
        WindowCreationFlag::minimizable |
        WindowCreationFlag::maximizable;
    set_startup_params(params);
    luassert_always(succeeded(init_modules()));
    multi_window_test_run();
}

int main(int argc, char** argv)
{
    Luna::init();
    run();
    Luna::close();
    return 0;
}

#endif