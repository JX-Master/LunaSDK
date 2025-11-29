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
#include "Window.h"
#include "../../Window.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "../../../Event.hpp"
#include "EventDispatching.h"
#import <objc/runtime.h>

inline void dispatch_event_to_handler(Luna::object_t event)
{
    void(*event_handler)(Luna::object_t event, void* userdata);
    void* event_handler_userdata;
    Luna::Window::get_event_handler(&event_handler, &event_handler_userdata);
    if(event_handler)
    {
        event_handler(event, event_handler_userdata);
    }
}

// Forward declaration for the delegate
@interface LunaWindowDelegate : NSResponder<NSWindowDelegate>
@property (nonatomic, assign) Luna::Window::Window* lunaWindow;
@end

@implementation LunaTextInputView

- (void)setInputRect:(Luna::RectI)inputRect
{
    _inputRect = inputRect;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self)
    {
        _markedText = [[NSMutableAttributedString alloc] init];
        _selectedRange = NSMakeRange(NSNotFound, 0);
        _markedRange = NSMakeRange(NSNotFound, 0);
        _pendingKey = -1;
        _pendingKeyCode = Luna::HID::KeyCode::unknown;
    }
    return self;
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    // Read IME text. 
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        NSString* characters = [string isKindOfClass:[NSAttributedString class]] ? 
            [(NSAttributedString*)string string] : (NSString*)string;

        if ([self hasMarkedText]) 
        {
            [self unmarkText];
        }
        
        const char* utf8 = [characters UTF8String];
        if (utf8)
        {
            auto e = Luna::new_object<Luna::Window::WindowInputTextEvent>();
            e->window = self.lunaWindow;
            e->text = utf8;
            dispatch_event_to_handler(e.object());
        }
    }
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    // IME inputing.
    if ([string isKindOfClass:[NSAttributedString class]])
    {
        _markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    }
    else
    {
        _markedText = [[NSMutableAttributedString alloc] initWithString:string];
    }

    if([_markedText length] == 0)
    {
        [self unmarkText];
        return;
    }
    
    _markedRange = NSMakeRange(0, [_markedText length]);
    _selectedRange = selectedRange;

    [self clearPendingKey];
}

- (void)unmarkText
{
    _markedText = nil;
    _markedRange = NSMakeRange(NSNotFound, 0);
    [self clearPendingKey];
}

- (NSRange)selectedRange
{
    return _selectedRange;
}

- (NSRange)markedRange
{
    return _markedRange;
}

- (BOOL)hasMarkedText
{
    return _markedText != nil;
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    if (actualRange)
        *actualRange = range;
    
    if (range.location < [_markedText length])
    {
        return [_markedText attributedSubstringFromRange:range];
    }
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText
{
    return @[];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    Luna::Window::Window* window = self.lunaWindow;
    NSWindow* nswindow = (NSWindow*)window->m_window;
    NSRect contentRect = [nswindow contentRectForFrameRect:[nswindow frame]];
    float windowHeight = contentRect.size.height;
    NSRect rect = NSMakeRect(_inputRect.offset_x, windowHeight - _inputRect.offset_y - _inputRect.height,
                             _inputRect.width, _inputRect.height);

    if (actualRange) {
        *actualRange = range;
    }
    
    rect = [nswindow convertRectToScreen:rect];

    return rect;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (void)doCommandBySelector:(SEL)selector
{
    // Handle special keys (like enter, delete, etc.).
    // Can handle non-text input keys here.
}

- (void)setPendingKey:(int)key keyCode:(Luna::HID::KeyCode)keyCode
{
    _pendingKey = key;
    _pendingKeyCode = keyCode;
}

- (void)processPendingKeyEvent
{
    if(_pendingKey < 0)
    {
        return;
    }
    auto e = Luna::new_object<Luna::Window::WindowKeyDownEvent>();
    e->window = self.lunaWindow;
    e->key = _pendingKeyCode;
    dispatch_event_to_handler(e.object());
    [self clearPendingKey];
}

- (void)clearPendingKey
{
    _pendingKey = -1;
}

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
            m_destructing = true;
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
                    [window close];
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
                    post_custom_event_to_queue(this, APP_DEFINED_EVENT_SHOW, 0, 0);
                }
                else
                {
                    [window orderOut:nil];
                    post_custom_event_to_queue(this, APP_DEFINED_EVENT_HIDE, 0, 0);
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

        RV Window::begin_text_input()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                NSView* parentView;
                NSWindow* window = (NSWindow*)m_window;

                parentView = [window contentView];
                LunaTextInputView* input_view = nil;

                if(!m_input_view)
                {
                    input_view = [[LunaTextInputView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 0.0, 0.0)];
                    input_view.lunaWindow = this;
                    m_input_view = input_view;
                }
                input_view = (LunaTextInputView*)m_input_view;
                
                if(![[input_view superview] isEqual:parentView])
                {
                    [parentView addSubview:input_view];
                    [window makeFirstResponder:input_view];
                }
            }
            m_text_input_active = true;
            return ok;
        }

        RV Window::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                if(m_input_view)
                {
                    LunaTextInputView* input_view = (LunaTextInputView*)m_input_view;
                    [input_view setInputRect: input_rect];
                }
            }
            return ok;
        }

        RV Window::end_text_input()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                if(m_input_view)
                {
                    LunaTextInputView* input_view = (LunaTextInputView*)m_input_view;
                    [input_view removeFromSuperview];
                }
            }
            m_text_input_active = false;
            return ok;
        }

        bool Window::is_text_input_active()
        {
            return m_text_input_active;
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

        StartupParams g_startup_params;

        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
		{
			g_startup_params = params;
		}
        LUNA_WINDOW_API const c8* get_app_name()
        {
            return g_startup_params.name;
        }
        LUNA_RUNTIME_API Version get_app_version()
        {
            return g_startup_params.version;
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
                [window setNextResponder: delegate];
                lunaWindow->m_delegate = delegate;
                
                // Register drag and drop
                [window registerForDraggedTypes:@[NSPasteboardTypeFileURL]];

                objc_setAssociatedObject(window, (const void*)WINDOW_POINTER_KEY, 
                    [NSValue valueWithPointer:lunaWindow.get()], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                
                // Show window if not hidden
                if (!test_flags(flags, WindowCreationFlag::hidden))
                {
                    [window makeKeyAndOrderFront:nil];
                    post_custom_event_to_queue(lunaWindow.get(), APP_DEFINED_EVENT_SHOW, 0, 0);
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
        auto e = Luna::new_object<Luna::Window::WindowRequestCloseEvent>();
        e->window = self.lunaWindow;
        e->do_close = true;
        dispatch_event_to_handler(e.object());
        return e->do_close ? YES : NO;
    }
    return YES;
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        auto e = Luna::new_object<Luna::Window::WindowInputFocusEvent>();
        e->window = self.lunaWindow;
        dispatch_event_to_handler(e.object());
    }
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        auto e = Luna::new_object<Luna::Window::WindowLoseInputFocusEvent>();
        e->window = self.lunaWindow;
        dispatch_event_to_handler(e.object());
    }
}

- (void)windowDidResize:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        Luna::UInt2U size = self.lunaWindow->get_size();
        {
            auto e = Luna::new_object<Luna::Window::WindowResizeEvent>();
            e->window = self.lunaWindow;
            e->width = size.x;
            e->height = size.y;
            dispatch_event_to_handler(e.object());
        }
        
        Luna::UInt2U fbSize = self.lunaWindow->get_framebuffer_size();
        {
            auto e = Luna::new_object<Luna::Window::WindowFramebufferResizeEvent>();
            e->window = self.lunaWindow;
            e->width = fbSize.x;
            e->height = fbSize.y;
            dispatch_event_to_handler(e.object());
        }
    }
}

- (void)windowDidMove:(NSNotification*)notification
{
    if (self.lunaWindow && !self.lunaWindow->is_closed())
    {
        Luna::Int2U pos = self.lunaWindow->get_position();
        auto e = Luna::new_object<Luna::Window::WindowMoveEvent>();
        e->window = self.lunaWindow;
        e->x = pos.x;
        e->y = pos.y;
        dispatch_event_to_handler(e.object());
    }
}

- (void)windowWillClose:(NSNotification *)notification 
{
    if (self.lunaWindow)
    {
        objc_setAssociatedObject(self.lunaWindow->m_window, (const void*)Luna::Window::WINDOW_POINTER_KEY, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        
        if(!self.lunaWindow->m_destructing)
        {
            // Trigger closed event
            auto e = Luna::new_object<Luna::Window::WindowClosedEvent>();
            e->window = self.lunaWindow;
            dispatch_event_to_handler(e.object());
        }
        
        self.lunaWindow->m_input_view = nil;
        self.lunaWindow->m_window = nil;
        self.lunaWindow = nil;
    }
}

- (void)keyDown:(NSEvent *)event
{
    // Prevent the system beep.
}

- (void)keyUp:(NSEvent *)event
{
    // Prevent the system beep.
}

@end
