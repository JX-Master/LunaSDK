/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SurfaceBind.hpp
* @author JXMaster
* @date 2024/6/20
*/
#pragma once
#include "Common.hpp"
#include <Luna/Window/Window.hpp>

namespace Luna
{
    namespace RHI
    {
        R<VkSurfaceKHR> new_surface_from_window(VkInstance instance, Window::IWindow* window);
    }
}