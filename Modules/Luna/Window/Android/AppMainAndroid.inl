/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainAndroid.inl
* @author JXMaster
* @date 2025/11/25
*/
#pragma once
#include "../AppMainHeader.hpp"
#include "AppMainAndroid.hpp"

extern "C"
{

#include "native_app_glue/android_native_app_glue.c"

void android_main(struct android_app*pApp)
{
    Luna::Window::set_android_app(pApp);
    Luna::Window::wait_until_native_window_ready(); // We need NativeWindow to be ready before initializing Vulkan (RHI module).
    luna_main(0, nullptr);
}

}