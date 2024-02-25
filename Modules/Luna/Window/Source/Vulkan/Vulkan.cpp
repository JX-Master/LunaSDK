/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Vulkan.cpp
* @author JXMaster
* @date 2024/2/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../Vulkan/Vulkan.hpp"
#include <GLFW/glfw3.h>
#include <Luna/Window/GLFW/GLFWWindow.hpp>
namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API R<VkSurfaceKHR> new_vulkan_surface_from_window(VkInstance instance, IWindow* window)
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            lutry
            {
#ifdef LUNA_WINDOW_GLFW
                if(glfwVulkanSupported() == GLFW_FALSE)
                {
                    return set_error(BasicError::not_supported(), "Vulkan is not supported on this platform.");
                }
                Window::IGLFWWindow* glfw_window = query_interface<Window::IGLFWWindow>(window->get_object());
                if (!glfw_window)
                {
                    return BasicError::not_supported();
                }
                GLFWwindow* glfw_handle = glfw_window->get_glfw_window_handle();
                VkResult r = glfwCreateWindowSurface(instance, glfw_handle, nullptr, &surface);
                if(r != VK_SUCCESS)
                {
                    if(r == VK_ERROR_INITIALIZATION_FAILED) return BasicError::not_supported();
                    else if(r == VK_ERROR_EXTENSION_NOT_PRESENT) return BasicError::not_supported();
                    else return BasicError::bad_platform_call();
                }
#else
                return BasicError::not_supported();
#endif
            }
            lucatchret;
            return surface;
        }

        LUNA_WINDOW_API Span<const c8*> get_required_vulkan_instance_extensions()
        {
#ifdef LUNA_WINDOW_GLFW
            u32 count = 0;
            const char** extensions = glfwGetRequiredInstanceExtensions(&count);
            return {extensions, count};
#else
            return {nullptr, 0};
#endif
        }
    }
}