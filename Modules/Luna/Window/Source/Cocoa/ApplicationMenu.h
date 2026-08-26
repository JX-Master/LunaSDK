/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ApplicationMenu.h
* @author JXMaster
* @date 2026/8/26
*/
#pragma once
#include <Luna/Runtime/Result.hpp>

@class NSEvent;

namespace Luna
{
    namespace Window
    {
        // Prepares the Cocoa application delegate, action target and default application menu.
        // This entry point is safe to call before Luna Runtime is initialized.
        void cocoa_application_menu_bootstrap();

        // Enables application event dispatch and refreshes the default menu with the configured name.
        RV cocoa_application_menu_platform_init(const c8* app_name);

        // Disconnects Luna-owned application menu objects and disables application event dispatch.
        void cocoa_application_menu_platform_close();

        // Gives the application main menu priority over a Luna key-down event.
        bool cocoa_application_menu_handle_key_down(NSEvent* event);

        // Checks and clears suppression for the key-up matching a handled menu shortcut.
        bool cocoa_application_menu_handle_key_up(NSEvent* event);
    }
}
