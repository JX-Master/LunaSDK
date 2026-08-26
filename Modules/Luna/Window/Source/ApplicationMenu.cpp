/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ApplicationMenu.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../ApplicationMenu.hpp"

#ifndef LUNA_PLATFORM_MACOS

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API bool supports_application_menu()
        {
            return false;
        }

        LUNA_WINDOW_API RV set_application_menu(const ApplicationMenuDesc&)
        {
            return E_NOT_SUPPORTED;
        }

        LUNA_WINDOW_API RV reset_application_menu()
        {
            return E_NOT_SUPPORTED;
        }

        LUNA_WINDOW_API RV set_application_menu_item_state(application_menu_item_id_t,
            const ApplicationMenuItemState&)
        {
            return E_NOT_SUPPORTED;
        }

        LUNA_WINDOW_API RV set_application_menu_item_title(application_menu_item_id_t, const c8*)
        {
            return E_NOT_SUPPORTED;
        }
    }
}

#endif
