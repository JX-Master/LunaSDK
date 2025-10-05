/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.mm
* @author JXMaster
* @date 2025/10/5
*/
#include <CoreGraphics/CGGeometry.h>
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../../Display.hpp"
#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <Luna/Runtime/StackAllocator.hpp>

namespace Luna
{
    namespace Window
    {
        static_assert(sizeof(display_t) >= sizeof(CGDirectDisplayID), "Wrong display_t size");

        LUNA_WINDOW_API display_t get_primary_display()
        {
            CGDirectDisplayID main_display = CGMainDisplayID();
            return (display_t)(usize)main_display;
        }
        LUNA_WINDOW_API void get_displays(Vector<display_t>& out_displays)
        {
            uint32_t num_displays = 0;
            CGGetOnlineDisplayList(0, nullptr, &num_displays);
            StackAllocator alloc;
            CGDirectDisplayID* display_ids = (CGDirectDisplayID*)alloc.allocate(sizeof(CGDirectDisplayID) * num_displays);
            CGGetOnlineDisplayList(num_displays, display_ids, &num_displays);
            out_displays.reserve(out_displays.size() + num_displays);
            for(u32 i = 0; i < num_displays; ++i)
            {
                out_displays.push_back((display_t)(usize)display_ids[i]);
            }
        }
        static VideoMode encode_video_mode(CGDisplayModeRef mode)
        {
            VideoMode dst_mode;
            dst_mode.width = (u32)CGDisplayModeGetWidth(mode);
            dst_mode.height = (u32)CGDisplayModeGetHeight(mode);
            dst_mode.refresh_rate = (u32)CGDisplayModeGetRefreshRate(mode);

            // CGDisplayModeCopyPixelEncoding is deprecated and currently there is no way to get bpp on macOS, we just 
            // use 32 bits for now.
            dst_mode.bits_per_pixel = 32;
            
            return dst_mode;
        }
        LUNA_WINDOW_API RV get_display_supported_video_modes(display_t display, Vector<VideoMode>& out_video_modes)
        {
            CGDirectDisplayID id = (CGDirectDisplayID)(usize)display;
            CFArrayRef modes = CGDisplayCopyAllDisplayModes(id, nullptr);
            if(!modes)
            {
                return set_error(BasicError::bad_platform_call(), "CGDisplayCopyAllDisplayModes failed");
            }
            CFIndex count = CFArrayGetCount(modes);
            for(CFIndex i = 0; i < count; ++i)
            {
                CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
                VideoMode vm = encode_video_mode(mode);
                out_video_modes.push_back(vm);
            }
            CFRelease(modes);
            return ok;
        }
        LUNA_WINDOW_API R<VideoMode> get_display_video_mode(display_t display)
        {
            CGDirectDisplayID id = (CGDirectDisplayID)(usize)display;
            CGDisplayModeRef mode = CGDisplayCopyDisplayMode(id);
            if (!mode)
            {
                return set_error(BasicError::bad_platform_call(), "CGDisplayCopyDisplayMode failed");
            }
            VideoMode vm = encode_video_mode(mode);
            CGDisplayModeRelease(mode);
            return vm;
        }
        LUNA_WINDOW_API R<Int2U> get_display_position(display_t display)
        {
            CGDirectDisplayID id = (CGDirectDisplayID)(usize)display;
            
            CGRect bounds = CGDisplayBounds(id);
            if(bounds.origin.x == CGRectZero.origin.x && bounds.origin.y == CGRectZero.origin.y &&
                bounds.size.width == CGRectZero.size.width && bounds.size.height == CGRectZero.size.height)
            {
                return set_error(BasicError::bad_platform_call(), "CGDisplayBounds failed");
            }
            return Int2U((i32)bounds.origin.x, (i32)bounds.origin.y);
        }
        LUNA_WINDOW_API R<RectI> get_display_working_area(display_t display)
        {
            CGDirectDisplayID id = (CGDirectDisplayID)(usize)display;
            RectI ret;
            @autoreleasepool
            {
                NSScreen* screen = nil;
                for (NSScreen* s in [NSScreen screens])
                {
                    NSDictionary* desc = [s deviceDescription];
                    NSNumber* screenID = [desc objectForKey:@"NSScreenNumber"];
                    if ([screenID unsignedIntValue] == id)
                    {
                        screen = s;
                        break;
                    }
                }
                
                if (!screen)
                {
                    return set_error(BasicError::bad_platform_call(), "Failed to find NSScreen for display");
                }
                
                NSRect visible = [screen visibleFrame];
                NSRect full = [screen frame];
                
                // Convert from Cocoa coordinates (origin at bottom-left) to screen coordinates (origin at top-left)
                CGFloat screenHeight = full.size.height;
                i32 x = (i32)visible.origin.x;
                i32 y = (i32)(screenHeight - visible.origin.y - visible.size.height);
                
                ret = RectI(x, y, (i32)visible.size.width, (i32)visible.size.height);
            }
            return ret;
        }
        LUNA_WINDOW_API R<Name> get_display_name(display_t display)
        {
            CGDirectDisplayID id = (CGDirectDisplayID)(usize)display;
            Name display_name;
            @autoreleasepool
            {
                NSScreen* screen = nil;
                for (NSScreen* s in [NSScreen screens])
                {
                    NSDictionary* desc = [s deviceDescription];
                    NSNumber* screenID = [desc objectForKey:@"NSScreenNumber"];
                    if ([screenID unsignedIntValue] == id)
                    {
                        screen = s;
                        break;
                    }
                }
                if (screen)
                {
                    display_name = [[screen localizedName] UTF8String];
                }
                else
                {
                    return set_error(BasicError::bad_platform_call(), "Failed to find NSScreen for display");
                }
            }
            return display_name;
        }
    }
}
