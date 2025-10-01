/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChainBind.mm
* @author JXMaster
* @date 2023/8/3
*/
#include "SwapChain.hpp"
#include <Luna/Window/Cocoa/CocoaWindow.hpp>
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

namespace Luna
{
    namespace RHI
    {
        RV bind_layer_to_window(Window::IWindow* window, CA::MetalLayer* layer, ColorSpace color_space, u32 buffer_count)
        {
            @autoreleasepool
            {
                Window::ICocoaWindow* cocoa_window = query_interface<Window::ICocoaWindow>(window->get_object());
                NSWindow* ns_window = cocoa_window->get_nswindow();
                CAMetalLayer* native_layer = (__bridge CAMetalLayer*)layer;
                if(color_space != ColorSpace::unspecified)
                {
                    CGColorSpaceRef cs;
                    switch(color_space)
                    {
                        case ColorSpace::srgb:
                        cs = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                        break;
                        case ColorSpace::scrgb_linear:
                        cs = CGColorSpaceCreateWithName(kCGColorSpaceExtendedLinearSRGB);
                        break;
                        case ColorSpace::bt2020:
                        cs = CGColorSpaceCreateWithName(kCGColorSpaceITUR_2020);
                        break;
                        case ColorSpace::display_p3:
                        cs = CGColorSpaceCreateWithName(kCGColorSpaceDisplayP3);
                        break;
                        case ColorSpace::acescg_linear:
                        cs = CGColorSpaceCreateWithName(kCGColorSpaceACESCGLinear);
                        break;
                        default: 
                        return RHIError::color_space_not_supported();
                    }
                    [native_layer setColorspace:cs];
                }
                [[ns_window contentView] setLayer:native_layer];
                [native_layer setMaximumDrawableCount:buffer_count];
                [[ns_window contentView] setWantsLayer:YES];
                [[ns_window contentView] setNeedsLayout:YES];
                return ok;
            }
        }
    }
}