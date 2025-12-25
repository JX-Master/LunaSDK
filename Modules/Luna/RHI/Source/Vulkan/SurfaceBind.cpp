/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SurfaceBind.cpp
* @author JXMaster
* @date 2024/6/20
*/
#include "SurfaceBind.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include <Luna/Window/Windows/Win32Window.hpp>
#include <vulkan/vulkan_win32.h>
#endif

#if defined(LUNA_PLATFORM_ANDROID)
#include <Luna/Window/Android/AndroidWindow.hpp>
#include <vulkan/vulkan_android.h>
#endif

namespace Luna
{
    namespace RHI
    {
        R<VkSurfaceKHR> new_surface_from_window(VkInstance instance, Window::IWindow* window)
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(LUNA_PLATFORM_WINDOWS)
            VkResult err;
            VkWin32SurfaceCreateInfoKHR info;
            memset(&info, 0, sizeof(info));
            info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            info.hinstance = GetModuleHandle(NULL);
            Window::IWin32Window* win32_window = query_interface<Window::IWin32Window>(window->get_object());
            if (!win32_window)
            {
                return BasicError::not_supported();
            }
            info.hwnd = win32_window->get_hwnd();
            auto func = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
            if (!func)
            {
                return BasicError::not_supported();
            }
            err = func(instance, &info, nullptr, &surface);
            if (err)
            {
                return encode_vk_result(err).errcode();
            }
#elif defined(LUNA_PLATFORM_ANDROID)
            Window::IAndroidWindow* android_window = query_interface<Window::IAndroidWindow>(window->get_object());
            if (!android_window)
            {
                return BasicError::not_supported();
            }
            ANativeWindow* native_window = (ANativeWindow*)android_window->get_native_window();
            if (!native_window)
            {
                return BasicError::bad_calling_time();
            }
            VkResult err;
            VkAndroidSurfaceCreateInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            info.pNext = nullptr;
            info.flags = 0;
            info.window = native_window;
            auto func = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
            if (!func)
            {
                return BasicError::not_supported();
            }
            err = func(instance, &info, nullptr, &surface);
            if (err)
            {
                return encode_vk_result(err).errcode();
            }
#else
#error "Unsupported platform"
#endif
            return surface;
        }
    }
}