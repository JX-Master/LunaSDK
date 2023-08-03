/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChainBind.mm
* @author JXMaster
* @date 2023/8/3
*/
#include "SwapChain.hpp"

namespace Luna
{
    namespace RHI
    {
        void bind_layer_to_window(Window::IWindow* window, CA::MetalLayer* layer, u32 buffer_count)
        {
            ICocoaWindow* cocoa_window = query_interface<ICocoaWindow>(window->get_object());
            NSWindow* ns_window = cocoa_window->get_nswindow();
            CAMetalLayer* native_layer = (__bridge CAMetalLayer*)layer;
            [[ns_window contentView] setLayer:native_layer];
            [native_layer setMaximumDrawableCount:buffer_count];
            [[ns_window contentView] setWantsLayer:YES];
            [[ns_window contentView] setNeedsLayout:YES];
        }
    }
}