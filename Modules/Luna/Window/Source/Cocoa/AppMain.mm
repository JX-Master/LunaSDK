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

#import <Cocoa/Cocoa.h>

@interface LunaAppDelegate : NSObject<NSApplicationDelegate>

@end

@implementation LunaAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    // Application initialization is done in main
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return NO; // Let the user handle termination
}

// - (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
// {
//     // Request termination, but let the update loop handle it
//     self.appStatus = Luna::Window::AppStatus::exiting;
//     return NSTerminateCancel; // We'll terminate manually
// }

@end

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API void cocoa_app_init()
        {
            @autoreleasepool
            {
                // Create application
                [NSApplication sharedApplication];
                
                // Create app delegate
                LunaAppDelegate* appDelegate = [[LunaAppDelegate alloc] init];
                [NSApp setDelegate:appDelegate];
                [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
                
                // Finish app launching
                [NSApp finishLaunching];
            }
        }
    }
}

