/*!
* This file is a portion of LunaSDK.
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

#include "../AppMainHeader.hpp"
#include <shellapi.h>
#include <Luna/Runtime/Unicode.hpp>
#include "../Window.hpp"

#ifdef LUNA_ENABLE_API_VALIDATION
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#endif

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "User32.lib")

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
    int r = luna_main(argc, (const char**)argv);
    HeapFree(GetProcessHeap(), 0, argv);
    return r;
}