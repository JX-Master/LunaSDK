/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Test.cpp
* @author JXMaster
* @date 2022/4/12
*/
#include <Runtime/PlatformDefines.hpp>
#ifdef LUNA_PLATFORM_MACOS
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
