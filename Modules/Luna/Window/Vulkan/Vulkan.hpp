/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Vulkan.hpp
* @author JXMaster
* @date 2024/2/25
*/
#pragma once
#include "../Window.hpp"
#include <volk.h>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{
        
        //! Creates a vulkan surface form window.
        //! @param[in] instance The Vulkan instance.
        //! @param[in] window The window to create surface for.
        //! @return Returns the created vulkan surface.
        LUNA_WINDOW_API R<VkSurfaceKHR> new_vulkan_surface_from_window(VkInstance instance, IWindow* window);

        //! @}
    }
}