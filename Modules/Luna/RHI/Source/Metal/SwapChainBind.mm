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

#if defined(LUNA_PLATFORM_MACOS)
#include <Luna/Window/Cocoa/CocoaWindow.hpp>
#include <Cocoa/Cocoa.h>
#elif defined(LUNA_PLATFORM_IOS)
#include <Luna/Window/UIKit/UIKitWindow.hpp>
#include <UIKit/UIKit.h>
#endif
#import <QuartzCore/CAMetalLayer.h>

#import <QuartzCore/CAMetalLayer.h>

namespace Luna
{
    namespace RHI
    {
        RV bind_layer_to_window(Window::IWindow* window, CAMetalLayer* layer, ColorSpace color_space, u32 buffer_count);

        RV set_metal_layer(CAMetalLayer* layer, const SwapChainDesc& desc)
        {
            layer.framebufferOnly = YES;
            layer.pixelFormat = (MTLPixelFormat)encode_pixel_format(desc.format);
            CGSize size;
            size.width = desc.width;
            size.height = desc.height;
            layer.drawableSize = size;
            layer.maximumDrawableCount = desc.buffer_count;
            if(desc.color_space != ColorSpace::unspecified)
            {
                CGColorSpaceRef cs;
                switch(desc.color_space)
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
                layer.colorspace = cs;
            }
            return ok;
        }

        RV SwapChain::init_metal_layer(const SwapChainDesc& desc)
        {
            @autoreleasepool
            {
#if defined(LUNA_PLATFORM_MACOS)
                CAMetalLayer* layer = [CAMetalLayer layer];
                m_metal_layer = retain((__bridge CA::MetalLayer*)layer);
                m_metal_layer->setDevice(m_device->m_device.get());
                auto r = set_metal_layer(layer, desc);
                if(failed(r)) return r;
                Window::ICocoaWindow* cocoa_window = query_interface<Window::ICocoaWindow>(m_window->get_object());
                NSWindow* ns_window = cocoa_window->get_nswindow();
                [[ns_window contentView] setLayer:layer];
                [[ns_window contentView] setWantsLayer:YES];
                [[ns_window contentView] setNeedsLayout:YES];
#elif defined(LUNA_PLATFORM_IOS)
                Ref<Window::IUIKitWindow> uikitwindow = m_window;
                luassert(uikitwindow);
                UIWindow* uiwindow = uikitwindow->get_uiwindow();
                if(![uiwindow.layer isKindOfClass:[CAMetalLayer class]])
                {
                    return BasicError::not_supported();
                }
                CAMetalLayer* layer = (CAMetalLayer*)uiwindow.layer;
                m_metal_layer = retain((__bridge CA::MetalLayer*)layer);
                m_metal_layer->setDevice(m_device->m_device.get());
                auto r = set_metal_layer(layer, desc);
                if(failed(r)) return r;
#endif
            }
            return ok;
        }
    }
}