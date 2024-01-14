/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WinMain.cpp
* @author JXMaster
* @date 2022/3/15
*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Window/Window.hpp>

namespace Luna
{
    void multi_window_test_run();
}

void run(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR    lpCmdLine,
    int nCmdShow)
{
    using namespace Luna;
    lupanic_if_failed(add_modules({module_window()}));
    init_modules();
    Luna::multi_window_test_run();
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    Luna::init();
    run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    Luna::close();
    return 0;
}