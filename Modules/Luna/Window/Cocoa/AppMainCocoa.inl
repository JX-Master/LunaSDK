/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainCocoa.inl
* @author JXMaster
* @date 2025/10/5
*/
#pragma once
#include "../AppMainHeader.hpp"
#include "AppMainCocoa.hpp"

// Main entry point for Cocoa applications
int main(int argc, const char* argv[])
{
    Luna::Window::cocoa_app_init();

    return luna_main(argc, argv);
}

