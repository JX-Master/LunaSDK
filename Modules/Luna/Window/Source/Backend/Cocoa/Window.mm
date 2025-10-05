/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.mm
* @author JXMaster
* @date 2025/10/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include "../../Window.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Mutex.hpp>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

// Forward declaration for the delegate
@interface LunaWindowDelegate : NSObject<NSWindowDelegate>
@property (nonatomic, assign) Luna::Window::Window* lunaWindow;
@end

namespace Luna
{
    namespace Window
    {
        Window::Window() :
            m_window(nil),
            m_delegate(nil),
            m_text_input_active(false),
            m_drop_x(0),
            m_drop_y(0)
        {
        }

        Window::~Window()
        {
            close();
        }

        void Window::close()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                if(m_window)
                {
                    NSWindow* window = (NSWindow*)m_window;
                    
                    objc_setAssociatedObject(window, (const void*)WINDOW_POINTER_KEY, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                    
                    // Trigger destroy event
                    m_events.destroy(this);
                    
                    [window setDelegate: nil];
                    [window close];
                    //[window performSelector:@selector(close) withObject:nil afterDelay:0];
                    
                    if (m_delegate)
                    {
                        LunaWindowDelegate* delegate = (LunaWindowDelegate*)m_delegate;
                        delegate.lunaWindow = nullptr;
                        m_delegate = nil;
                    }
                    
                    m_window = nil;
                }
            }
        }

        bool Window::is_closed()
        {
            lutsassert_main_thread();
            return m_window == nil;
        }

        bool Window::has_input_focus()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return [window isKeyWindow];
            }
        }

        bool Window::has_mouse_focus()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return [window isKeyWindow];
            }
        }

        RV Window::set_foreground()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                [window makeKeyAndOrderFront:nil];
                [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
            }
            return ok;
        }

        bool Window::is_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return [window isMiniaturized];
            }
        }

        bool Window::is_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return [window isZoomed];
            }
        }

        RV Window::set_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                [window miniaturize:nil];
            }
            return ok;
        }

        RV Window::set_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                if (![window isZoomed])
                {
                    [window zoom:nil];
                }
            }
            return ok;
        }

        RV Window::set_restored()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                if ([window isMiniaturized])
                {
                    [window deminiaturize:nil];
                }
                else if ([window isZoomed])
                {
                    [window zoom:nil];
                }
            }
            return ok;
        }

        bool Window::is_hovered()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSPoint mouseLocation = [NSEvent mouseLocation];
                NSRect frame = [window frame];
                return NSPointInRect(mouseLocation, frame);
            }
        }

        bool Window::is_visible()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return [window isVisible];
            }
        }

        RV Window::set_visible(bool visible)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                if (visible)
                {
                    [window orderFront:nil];
                }
                else
                {
                    [window orderOut:nil];
                }
            }
            return ok;
        }

        WindowStyleFlag Window::get_style()
        {
            lutsassert_main_thread();
            if (is_closed()) return WindowStyleFlag::none;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSWindowStyleMask styleMask = [window styleMask];
                
                WindowStyleFlag style = WindowStyleFlag::none;
                if (styleMask & NSWindowStyleMaskResizable)
                {
                    set_flags(style, WindowStyleFlag::resizable);
                }
                if (styleMask == NSWindowStyleMaskBorderless)
                {
                    set_flags(style, WindowStyleFlag::borderless);
                }
                return style;
            }
        }

        RV Window::set_style(WindowStyleFlag style)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSWindowStyleMask styleMask;
                
                if (test_flags(style, WindowStyleFlag::borderless))
                {
                    styleMask = NSWindowStyleMaskBorderless;
                }
                else
                {
                    styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
                    if (test_flags(style, WindowStyleFlag::resizable))
                    {
                        styleMask |= NSWindowStyleMaskResizable;
                    }
                }
                
                [window setStyleMask:styleMask];
            }
            return ok;
        }

        Int2U Window::get_position()
        {
            lutsassert_main_thread();
            if (is_closed()) return Int2U(0, 0);
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSRect frame = [window frame];
                NSRect screenFrame = [[window screen] frame];
                
                // Convert from Cocoa coordinates (bottom-left origin) to screen coordinates (top-left origin)
                i32 x = (i32)frame.origin.x;
                i32 y = (i32)(screenFrame.size.height - frame.origin.y - frame.size.height);
                
                return Int2U(x, y);
            }
        }

        RV Window::set_position(i32 x, i32 y)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSRect frame = [window frame];
                NSRect screenFrame = [[window screen] frame];
                
                // Convert from screen coordinates (top-left origin) to Cocoa coordinates (bottom-left origin)
                CGFloat cocoaY = screenFrame.size.height - y - frame.size.height;
                
                NSPoint origin = NSMakePoint((CGFloat)x, cocoaY);
                [window setFrameOrigin:origin];
            }
            return ok;
        }

        UInt2U Window::get_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSRect contentRect = [[window contentView] frame];
                return UInt2U((u32)contentRect.size.width, (u32)contentRect.size.height);
            }
        }

        RV Window::set_size(u32 width, u32 height)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSRect frame = [window frame];
                NSRect contentRect = [window contentRectForFrameRect:frame];
                contentRect.size = NSMakeSize((CGFloat)width, (CGFloat)height);
                NSRect newFrame = [window frameRectForContentRect:contentRect];
                [window setFrame:newFrame display:YES];
            }
            return ok;
        }

        UInt2U Window::get_framebuffer_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSView* contentView = [window contentView];
                NSRect backingRect = [contentView convertRectToBacking:[contentView bounds]];
                return UInt2U((u32)backingRect.size.width, (u32)backingRect.size.height);
            }
        }

        f32 Window::get_dpi_scale_factor()
        {
            lutsassert_main_thread();
            if (is_closed()) return 1.0f;
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                return (f32)[window backingScaleFactor];
            }
        }

        RV Window::set_title(const c8* title)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            @autoreleasepool
            {
                NSWindow* window = (NSWindow*)m_window;
                NSString* titleStr = [NSString stringWithUTF8String:title];
                [window setTitle:titleStr];
            }
            return ok;
        }

        Int2U Window::screen_to_client(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x - pos.x, point.y - pos.y);
        }

        Int2U Window::client_to_screen(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x + pos.x, point.y + pos.y);
        }

        id Window::get_nswindow()
        {
            lutsassert_main_thread();
            return m_window;
        }

        RV Window::start_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = true;
            return ok;
        }

        RV Window::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            lutsassert_main_thread();
            // macOS handles IME positioning automatically
            return ok;
        }

        RV Window::stop_text_input()
        {
            lutsassert_main_thread();
            m_text_input_active = false;
            return ok;
        }

        RV platform_init()
        {
            @autoreleasepool
            {
                // Initialize application if not already done
                [NSApplication sharedApplication];
                [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
                
                register_boxed_type<Window>();
                impl_interface_for_type<Window, ICocoaWindow, IWindow>();
                
                return ok;
            }
        }

        void platform_close()
        {   
            
        }

        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
		{
			g_startup_params = params;
		}

        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
            i32 x,
            i32 y,
            u32 width,
            u32 height,
            WindowStyleFlag style_flags,
            WindowCreationFlag flags)
        {
            lutsassert_main_thread();
            
            @autoreleasepool
            {
                // Determine window style
                NSWindowStyleMask styleMask;
                if (test_flags(style_flags, WindowStyleFlag::borderless))
                {
                    styleMask = NSWindowStyleMaskBorderless;
                }
                else
                {
                    styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
                    if (test_flags(style_flags, WindowStyleFlag::resizable))
                    {
                        styleMask |= NSWindowStyleMaskResizable;
                    }
                }

                NSRect screen_rect = [[NSScreen mainScreen] frame];
                
                // Get default size from primary display if needed
                if (width == 0 || height == 0 || x == DEFAULT_POS || y == DEFAULT_POS)
                {
                    lutry
                    {
                        lulet(mode, get_display_video_mode(get_primary_display()));
                        if(width == 0)
                        {
                            width = (u32)screen_rect.size.width * 7 / 10;
                        }
                        if(height == 0)
                        {
                            height = (u32)screen_rect.size.height * 7 / 10;
                        }
                        if(x == DEFAULT_POS)
                        {
                            x = screen_rect.origin.x + (screen_rect.size.width - width) / 2;
                        }
                        if(y == DEFAULT_POS)
                        {
                            y = screen_rect.origin.y + (screen_rect.size.height - height) / 2;
                        }
                    }
                    lucatchret;
                }

                // Convert to Cocoa coordinates.
                y = screen_rect.size.height - y - height;
                
                // Create content rect
                NSRect contentRect = NSMakeRect(x, y, (CGFloat)width, (CGFloat)height);
                
                // Create window
                NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect
                                                               styleMask:styleMask
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO];

                [window setReleasedWhenClosed: FALSE];
                
                if (!window)
                {
                    return set_error(BasicError::bad_platform_call(), "Failed to create NSWindow");
                }
                
                // Set title
                NSString* titleStr = [NSString stringWithUTF8String:title];
                [window setTitle:titleStr];
                
                // Can enter full screen mode.
                if(test_flags(style_flags, WindowStyleFlag::resizable))
                {
                    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
                }
                
                // Create window object
                Ref<Window> lunaWindow = new_object<Window>();
                lunaWindow->m_window = window;
                
                // Create and set delegate
                LunaWindowDelegate* delegate = [[LunaWindowDelegate alloc] init];
                delegate.lunaWindow = lunaWindow.get();
                [window setDelegate:delegate];
                lunaWindow->m_delegate = delegate;
                
                // Register drag and drop
                [window registerForDraggedTypes:@[NSPasteboardTypeFileURL]];

                objc_setAssociatedObject(window, (const void*)WINDOW_POINTER_KEY, 
                    [NSValue valueWithPointer:lunaWindow.get()], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                
                // Show window if not hidden
                if (!test_flags(flags, WindowCreationFlag::hidden))
                {
                    [window makeKeyAndOrderFront:nil];
                }
                
                return Ref<IWindow>(lunaWindow);
            }
        }
    }
}

// Implement the delegate
@implementation LunaWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)sender
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().close(self.lunaWindow);
        return NO; // Let the user close the window by calling close() in the event handler
    }
    return YES;
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().input_focus(self.lunaWindow);
    }
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().lose_input_focus(self.lunaWindow);
    }
}

- (void)windowDidResize:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        Luna::UInt2U size = self.lunaWindow->get_size();
        self.lunaWindow->get_events().resize(self.lunaWindow, size.x, size.y);
        
        Luna::UInt2U fbSize = self.lunaWindow->get_framebuffer_size();
        self.lunaWindow->get_events().framebuffer_resize(self.lunaWindow, fbSize.x, fbSize.y);
    }
}

- (void)windowDidMove:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        Luna::Int2U pos = self.lunaWindow->get_position();
        self.lunaWindow->get_events().move(self.lunaWindow, pos.x, pos.y);
    }
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().hide(self.lunaWindow);
    }
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().show(self.lunaWindow);
    }
}

- (void)windowDidChangeBackingProperties:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        self.lunaWindow->get_events().dpi_scale_changed(self.lunaWindow);
    }
}

@end
