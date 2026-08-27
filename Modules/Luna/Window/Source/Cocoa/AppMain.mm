/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMain.mm
* @author JXMaster
* @date 2025/10/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../Cocoa/AppMainCocoa.hpp"
#include "ApplicationMenu.h"

#import <Cocoa/Cocoa.h>

namespace Luna
{
    namespace Window
    {
        static bool g_cocoa_app_initialized = false;

        LUNA_WINDOW_API void cocoa_app_init()
        {
            @autoreleasepool
            {
                if(g_cocoa_app_initialized) return;
                g_cocoa_app_initialized = true;

                // Create application
                [NSApplication sharedApplication];
                [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

                // Install the default application menu and the SDK delegate before AppKit finishes launching.
                // The bootstrap preserves a delegate or menu supplied by a custom native entry point.
                cocoa_application_menu_bootstrap();

                // Finish app launching
                [NSApp finishLaunching];
            }
        }
    }
}
