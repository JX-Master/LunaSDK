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

// Defines the real main function here.

int main(int argc, char* argv[])
{
    return luna_main(argc, argv);
}