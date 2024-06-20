/*!
* This file is a portion of Luna SDK.
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
            err = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface);
            if (err)
            {
                return encode_vk_result(err).errcode();
            }
#else
            return BasicError::not_supported();
#endif
            return surface;
        }
    }
}