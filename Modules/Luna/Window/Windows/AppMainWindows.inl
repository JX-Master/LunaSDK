/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainWindows.inl
* @author JXMaster
* @date 2025/3/19
*/
#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../AppMain.hpp"
#include <shellapi.h>
#include <Luna/Runtime/Unicode.hpp>
#include "../Window.hpp"

#ifdef LUNA_ENABLE_API_VALIDATION
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#endif

#pragma comment(lib, "shell32.lib")

namespace Luna
{
    namespace Window
    {
        char** win32_get_argv(int* argc)
        {
            LPWSTR command_line = GetCommandLineW();
            int num_args;
            LPWSTR* wargv = CommandLineToArgvW(command_line, &num_args);
            if(argc)
            {
                *argc = num_args;
            }
            usize allocate_size = sizeof(char*) * num_args;
            for(usize i = 0; i < num_args; ++i)
            {
                usize len = utf16_to_utf8_len((c16*)(wargv[i]));
                allocate_size += (len + 1) * sizeof(c8);
            }
            void* mem = HeapAlloc(GetProcessHeap(), 0, allocate_size);
            char** arr = (char**)mem;
            char* str = (char*)((usize)mem + sizeof(char*) * num_args);
            for(usize i = 0; i < num_args; ++i)
            {
                arr[i] = str;
                usize len = utf16_to_utf8(str, allocate_size - ((usize)str - (usize)mem), (c16*)wargv[i]);
                str += len + 1;
            }
            LocalFree(wargv);
            return arr;
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    int argc;
    char** argv = Luna::Window::win32_get_argv(&argc);
    using namespace Luna;
    opaque_t app_state = nullptr;
    Window::AppResult result = app_init(&app_state, argc, argv);
#ifdef LUNA_ENABLE_API_VALIDATION
    if(result == Window::AppResult::ok)
    {
        lucheck_msg(Luna::is_initialized(), "Luna::init must be called in app_init.");
        lucheck_msg(Luna::is_module_initialized(module_window()), "Window module must be initialized in app_init.");
    }
#endif
    while(result == Window::AppResult::ok)
    {
        Window::poll_events();
        result = app_update(app_state);
    }
    app_close(app_state);
    HeapFree(GetProcessHeap(), 0, argv);
    return result == Window::AppResult::failed ? -1 : 0;
}