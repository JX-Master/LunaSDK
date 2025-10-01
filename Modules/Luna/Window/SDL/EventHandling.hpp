/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventHandling.hpp
* @author JXMaster
* @date 2025/3/25
*/
#pragma once
#include <SDL3/SDL.h>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API void handle_sdl_event(SDL_Event& event);
    }
}