/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainCocoa.inl
* @author JXMaster
* @date 2025/10/5
*/
#pragma once
#include "EventHandling.hpp"
#include <Luna/Window/AppMainCallbacks.hpp>
#include "AppMainCocoa.hpp"

// Main entry point for Cocoa applications
int main(int argc, char* argv[])
{
    Luna::Window::cocoa_app_init();
    Luna::opaque_t app_state = nullptr;
    Luna::Window::AppStatus status = Luna::app_init(&app_state, argc, argv);

    while (status == Luna::Window::AppStatus::running)
    {
        // Process all pending events
        Luna::Window::poll_cocoa_events();
                    
        // Update user app
        status = Luna::app_update(app_state);
    }
        
    // Cleanup
    Luna::app_close(app_state, status);
        
    // Return appropriate exit code
    return (status == Luna::Window::AppStatus::exiting) ? 0 : 1;
}

