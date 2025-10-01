/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.hpp
* @author JXMaster
* @date 2024/6/16
*/
#include "../../Display.hpp"
#include <Luna/Runtime/Result.hpp>
#include <SDL3/SDL.h>

namespace Luna
{
    namespace Window
    {
        struct Display
        {
            Name m_name;
            SDL_DisplayID m_id;
            bool m_disconnected = false;
        };

        RV display_init();
        void display_close();

        RV refresh_display_list();
        display_t get_display_from_display_id(SDL_DisplayID id);
    }
}