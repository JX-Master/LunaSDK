/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Common.hpp
* @author JXMaster
* @date 2025/3/21
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include <SDL3/SDL.h>

namespace Luna
{
    namespace Window
    {
        inline RV encode_sdl_result(bool r)
        {
            return r ? ok : set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
        }
    }
}
