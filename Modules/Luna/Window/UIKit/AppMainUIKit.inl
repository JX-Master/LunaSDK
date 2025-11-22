/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainUIKit.inl
* @author JXMaster
* @date 2025/11/19
*/
#pragma once
#include "../AppMainHeader.hpp"
#include "AppMainUIKit.hpp"

// Main entry point for Cocoa applications
int main(int argc, char * argv[])
{
    return Luna::Window::uikit_app_run(argc, argv, luna_main);
}