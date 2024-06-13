/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2024/6/13
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Window/Window.hpp>

namespace Luna
{
    void multi_window_test_run();
}

void run()
{
    using namespace Luna;
    lupanic_if_failed(add_modules({module_window()}));
    lupanic_if_failed(init_modules());
    Luna::multi_window_test_run();
}

#define LUNA_WINDOWS_WMAIN
#include <Luna/Window/Main.hpp>

int luna_main(int argc, char* argv[])
{
    Luna::init();
    run();
    Luna::close();
    return 0;
}