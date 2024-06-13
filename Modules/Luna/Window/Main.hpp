/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.hpp
* @author JXMaster
* @date 2024/6/13
* @brief Include this file once in your project main file to expand the platform-specific main entry point definition.
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>

#ifdef LUNA_PLATFORM_WINDOWS

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

inline char** allocate_argvw_windows(int argc, LPWSTR* argvw)
{
    char** argv = nullptr;
    if(argc && argvw)
    {
        argv = (char **)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (argc + 1) * sizeof(*argv));
        int i;
        for (i = 0; i < argc; ++i)
        {
            const int utf8_size = WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, NULL, 0, NULL, NULL);
            argv[i] = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf8_size);
            WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, argv[i], utf8_size, NULL, NULL);
        }
        argv[i] = nullptr;
    }
    return argv;
}
inline void free_argvw_windows(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i) 
    {
        HeapFree(GetProcessHeap(), 0, argv[i]);
    }
    HeapFree(GetProcessHeap(), 0, argv);
}

#endif

//! The entry function of the user program.
//! @details Your program must define this function as the entry point of the application. 
//! This function will be called by Luna SDK from the real main function implemented by the SDK.
//! @param[in] argc The number of program arguments passed from the system.
//! @param[in] argv An array of program arguments passed from the system.
//! @return The program exit code returned to the system.
int luna_main(int argc, char* argv[]);

// On Windows, the main function can be main, wmain (for command-line applications) or WinMain (for GUI applications). The user can select the main function used here.

// #define LUNA_WINDOWS_WINMAIN
// #define LUNA_WINDOWS_WMAIN

#if defined(LUNA_PLATFORM_WINDOWS)

#if defined(LUNA_WINDOWS_WINMAIN)
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR    lpCmdLine,
    int       nCmdShow)
{
    // Translate command line arguments.
    int argc;
    LPWSTR *argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
    char** argv = allocate_argvw_windows(argc, argvw);
    LocalFree(argvw);
    int ret = luna_main(argc, argv);
    free_argvw_windows(argc, argv);
    return ret;
}
#elif defined(LUNA_WINDOWS_WMAIN)
int wmain( int argc, wchar_t *argvw[] )
{
    char** argv = allocate_argvw_windows(argc, argvw);
    int ret = luna_main(argc, argv);
    free_argvw_windows(argc, argv);
    return ret;
}
#else
int main(int argc, char* argv[])
{
    return luna_main(argc, argv);
}
#endif

#else
int main(int argc, char* argv[])
{
    return luna_main(argc, argv);
}
#endif