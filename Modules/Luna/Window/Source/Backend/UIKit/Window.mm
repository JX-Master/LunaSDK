/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.mm
* @author JXMaster
* @date 2025/11/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.h"
#include "../../Window.hpp"
#include "../../Event.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/Unicode.hpp>

#import <UIKit/UIKit.h>
#import "UIKitAppDelegate.h"


namespace Luna
{
    namespace Window
    {
        Ref<UIKitWindow> g_window;
        RV platform_init()
        {
            lutry
            {
                // Create main window object.
                @autoreleasepool
                {
                    if([[UIApplication sharedApplication].delegate isKindOfClass:[LunaUIKitDelegate class]])
                    {
                        register_struct_type<UIKitWindow>({});
                        impl_interface_for_type<UIKitWindow, IUIKitWindow, IWindow>();
                        LunaUIKitDelegate* delegate = (LunaUIKitDelegate*)[UIApplication sharedApplication].delegate;
                        UIWindowScene* scene = delegate.primary_scene;
                        Ref<UIKitWindow> window = new_object<UIKitWindow>();
                        luexp(window->init(scene));
                        g_window = window;
                    }
                    else
                    {
                        lupanic();
                    }
                }
            }
            lucatchret;
            return ok;
        }

        void platform_close()
        {
            g_window.reset();
        }

        RV UIKitWindow::init(UIWindowScene* scene)
        {
            @autoreleasepool
            {
                m_window = [[UIWindow alloc] initWithWindowScene:scene];
                m_view = [[LunaWindowView alloc] initWithFrame:m_window.bounds];
                m_view.lunaWindow = this;
                m_view.layer.contentsScale = m_window.screen.nativeScale;
                [m_view updateDrawableSize];
                m_view_controller = [[LunaWindowViewController alloc] initWithLunaWindow:this];
                m_view_controller.view = m_view;
                m_window.rootViewController = m_view_controller;
                [m_window makeKeyAndVisible];
            }
            return ok;
        }

        Int2U UIKitWindow::get_position()
        {
            lutsassert_main_thread();
            if (is_closed()) return Int2U(0, 0);
            @autoreleasepool
            {
                CGRect frame = m_window.frame;
                return Int2U((i32)frame.origin.x, (i32)frame.origin.y);
            }
        }

        UInt2U UIKitWindow::get_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            @autoreleasepool
            {
                CGRect bounds = m_window.bounds;
                return UInt2U((u32)bounds.size.width, (u32)bounds.size.height);
            }
        }

        UInt2U UIKitWindow::get_framebuffer_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            @autoreleasepool
            {
                CGRect bounds = m_window.bounds;
                CGFloat scale = m_window.layer.contentsScale;
                return UInt2U((u32)(bounds.size.width * scale),
                              (u32)(bounds.size.height * scale));
            }
        }

        f32 UIKitWindow::get_dpi_scale_factor()
        {
            lutsassert_main_thread();
            if (is_closed()) return 1.0f;
            @autoreleasepool
            {
                return (f32)m_window.screen.scale;
            }
        }

        Int2U UIKitWindow::screen_to_client(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x - pos.x, point.y - pos.y);
        }

        Int2U UIKitWindow::client_to_screen(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x + pos.x, point.y + pos.y);
        }

        RV UIKitWindow::begin_text_input()
        {
            @autoreleasepool
            {
                [m_view_controller startTextInput];
            }
            return ok;
        }

        RV UIKitWindow::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            @autoreleasepool
            {
                m_view_controller.textInputRect = input_rect;
                if (m_view_controller.textFieldFocused) 
                {
                    [m_view_controller updateKeyboard];
                }
            }
            return ok;
        }

        RV UIKitWindow::end_text_input()
        {
            @autoreleasepool
            {
                [m_view_controller stopTextInput];
            }
            return ok;
        }

        bool UIKitWindow::is_text_input_active()
        {
            return m_text_input_active;
        }

        LUNA_WINDOW_API IWindow* get_system_window()
        {
            return g_window.get();
        }
    }
}

using namespace Luna;

@implementation LunaWindowView

- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) 
    {
        self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        self.autoresizesSubviews = YES;
    }
    return self;
}

+ (Class) layerClass
{
    return [CAMetalLayer class];
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (void)dispatchTouches:(NSSet<UITouch*>*)touches
               withType:(UIEventType)type
                  event:(UIEvent*)event
{
    using namespace Luna::Window;
    if (!self.lunaWindow || self.lunaWindow->is_closed())
    {
        return;
    }
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:self];
        u64 tid = (u64)(__bridge void*)touch;
        if (type == UIEventTypeTouches)
        {
            // Map phase to our touch events.
            switch (touch.phase)
            {
            case UITouchPhaseBegan:
            {
                auto e = Luna::new_object<WindowTouchDownEvent>();
                e->window = self.lunaWindow;
                e->id = tid;
                e->x = (f32)pt.x;
                e->y = (f32)pt.y;
                dispatch_event_to_handler(e.object());
                break;
            }
            case UITouchPhaseMoved:
            case UITouchPhaseStationary:
            {
                auto e = Luna::new_object<WindowTouchMoveEvent>();
                e->window = self.lunaWindow;
                e->id = tid;
                e->x = (f32)pt.x;
                e->y = (f32)pt.y;
                dispatch_event_to_handler(e.object());
                break;
            }
            case UITouchPhaseEnded:
            case UITouchPhaseCancelled:
            {
                auto e = Luna::new_object<WindowTouchUpEvent>();
                e->window = self.lunaWindow;
                e->id = tid;
                e->x = (f32)pt.x;
                e->y = (f32)pt.y;
                dispatch_event_to_handler(e.object());
                break;
            }
            default:
                break;
            }
        }
    }
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self dispatchTouches:touches withType:event.type event:event];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self dispatchTouches:touches withType:event.type event:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self dispatchTouches:touches withType:event.type event:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self dispatchTouches:touches withType:event.type event:event];
}

- (void)updateDrawableSize
{
    CGSize size = self.bounds.size;
    size.width *= self.layer.contentsScale;
    size.height *= self.layer.contentsScale;

    CAMetalLayer *metallayer = ((CAMetalLayer *)self.layer);
    if (metallayer.drawableSize.width != size.width ||
        metallayer.drawableSize.height != size.height) 
    {
        metallayer.drawableSize = size;
        auto e = new_object<Window::WindowFramebufferResizeEvent>();
        e->window = self.lunaWindow;
        e->width = size.width;
        e->height = size.height;
        Window::dispatch_event_to_handler(e.object());
    }
}
@end

@implementation LunaWindowViewController
{
    UITextField* textField;
    BOOL hidingKeyboard;
    BOOL rotatingOrientation;
    NSString* committedText;
    NSString* defaultText;
}

@synthesize lunaWindow;

- (instancetype)initWithLunaWindow:(Luna::Window::UIKitWindow *)window
{
    if (self = [super initWithNibName:nil bundle:nil]) 
    {
        self.lunaWindow = window;
        [self initKeyboard];
        hidingKeyboard = NO;
        rotatingOrientation = NO;
    }
    return self;
}

- (void)dealloc 
{
    [self closeKeyboard];
}

- (void)loadView
{
    // Do nothing.
}

- (void)viewDidLayoutSubviews
{
    const CGSize size = self.view.bounds.size;
    int w = (int)size.width;
    int h = (int)size.height;

    auto e = new_object<Window::WindowResizeEvent>();
    e->window = lunaWindow;
    e->width = w;
    e->height = h;
    Window::dispatch_event_to_handler(e.object());
}

- (void)setView:(UIView *)view
{
    [super setView:view];

    // TODO: enable game controller here.

    [view addSubview:textField];

    if (textFieldFocused) {
        [self startTextInput];
    }
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    rotatingOrientation = YES;
    [coordinator
        animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        }
        completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
          self->rotatingOrientation = NO;
        }];
}

@synthesize textInputRect;
@synthesize keyboardHeight;
@synthesize textFieldFocused;

- (void)initKeyboard 
{
    textField = [[UITextField alloc] initWithFrame:CGRectZero];
    textField.delegate = self;
    textField.hidden = YES;
    textFieldFocused = NO;
    defaultText = @"                                                                ";
    committedText = textField.text;
    textField.text = committedText;

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
            selector:@selector(keyboardWillShow:)
                name:UIKeyboardWillShowNotification
                object:nil];
    [center addObserver:self
            selector:@selector(keyboardWillHide:)
                name:UIKeyboardWillHideNotification
                object:nil];
    [center addObserver:self
            selector:@selector(keyboardDidHide:)
                name:UIKeyboardDidHideNotification
                object:nil];
    [center addObserver:self
            selector:@selector(textFieldTextDidChange:)
                name:UITextFieldTextDidChangeNotification
                object:nil];
}

- (void)closeKeyboard
{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self
                      name:UIKeyboardWillShowNotification
                    object:nil];
    [center removeObserver:self
                      name:UIKeyboardWillHideNotification
                    object:nil];
    [center removeObserver:self
                      name:UIKeyboardDidHideNotification
                    object:nil];
    [center removeObserver:self
                      name:UITextFieldTextDidChangeNotification
                    object:nil];
}

- (void)startTextInput
{
    if(lunaWindow->m_text_input_active) return;
    if (!textFieldFocused) 
    {
        textFieldFocused = YES;
        auto e = new_object<Window::ScreenKeyboardShownEvent>();
        Window::dispatch_event_to_handler(e.object());
    }

    if (!textField.window) 
    {
        return;
    }

    [textField becomeFirstResponder];
    lunaWindow->m_text_input_active = true;
}

- (void)stopTextInput
{
    if(!lunaWindow->m_text_input_active) return;
    if (textFieldFocused) 
    {
        textFieldFocused = NO;
        auto e = new_object<Window::ScreenKeyboardHiddenEvent>();
        Window::dispatch_event_to_handler(e.object());
    }

    if (!textField.window) 
    {
        return;
    }

    [self resetTextState];
    [textField resignFirstResponder];
    lunaWindow->m_text_input_active = false;
}

// UIKeyboardWillShowNotification
- (void)keyboardWillShow:(NSNotification *)notification
{
    CGRect kbrect = [[notification userInfo][UIKeyboardFrameEndUserInfoKey] CGRectValue];

    kbrect = [self.view convertRect:kbrect fromView:nil];

    [self setKeyboardHeight:(int)kbrect.size.height];

    if (hidingKeyboard) 
    {
        auto _ = lunaWindow->begin_text_input();
        hidingKeyboard = NO;
    }
}

// UIKeyboardWillHideNotification
- (void)keyboardWillHide:(NSNotification *)notification
{
    hidingKeyboard = YES;
    [self setKeyboardHeight:0];

    if (lunaWindow->is_text_input_active() && !rotatingOrientation) 
    {
        auto _ = lunaWindow->end_text_input();
    }
}

// UIKeyboardDidHideNotification
- (void)keyboardDidHide:(NSNotification *)notification
{
    hidingKeyboard = NO;
}

// UITextFieldTextDidChangeNotification
- (void)textFieldTextDidChange:(NSNotification *)notification
{
    if (textField.markedTextRange == nil) 
    {
        NSUInteger compareLength = min(textField.text.length, committedText.length);
        NSUInteger matchLength;

        for (matchLength = 0; matchLength < compareLength; ++matchLength) {
            if ([committedText characterAtIndex:matchLength] != [textField.text characterAtIndex:matchLength]) {
                break;
            }
        }
        if (matchLength < committedText.length) {
            size_t deleteLength = utf8_strlen([[committedText substringFromIndex:matchLength] UTF8String]);
            while (deleteLength > 0) 
            {
                // Send distinct down and up events for each backspace action
                auto e1 = new_object<Window::WindowKeyDownEvent>();
                auto e2 = new_object<Window::WindowKeyUpEvent>();
                e1->window = lunaWindow;
                e1->key = HID::KeyCode::backspace;
                e2->window = lunaWindow;
                e2->key = HID::KeyCode::backspace;
                Window::dispatch_event_to_handler(e1.object());
                Window::dispatch_event_to_handler(e2.object());
                --deleteLength;
            }
        }

        if (matchLength < textField.text.length) {
            NSString *pendingText = [textField.text substringFromIndex:matchLength];
            auto e = new_object<Window::WindowInputTextEvent>();
            e->window = lunaWindow;
            e->text = [pendingText UTF8String];
            Window::dispatch_event_to_handler(e.object());
        }
        committedText = textField.text;
    }
}

- (void)updateKeyboard
{
    CGAffineTransform t = self.view.transform;
    CGPoint offset = CGPointMake(0.0, 0.0);
    CGRect frame = lunaWindow->m_window.bounds;

    if (self.keyboardHeight && self.textInputRect.height) 
    {
        int rectbottom = (int)(self.textInputRect.offset_y + self.textInputRect.height);
        int keybottom = (int)(self.view.bounds.size.height - self.keyboardHeight);
        if (keybottom < rectbottom) 
        {
            offset.y = keybottom - rectbottom;
        }
    }

    t.tx = 0.0;
    t.ty = 0.0;
    offset = CGPointApplyAffineTransform(offset, t);
    
    frame.origin.x += offset.x;
    frame.origin.y += offset.y;

    self.view.frame = frame;
}

- (void)setKeyboardHeight:(int)height
{
    keyboardHeight = height;
    [self updateKeyboard];
}

// UITextFieldDelegate method.  Invoked when user types something.
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    if (textField.markedTextRange == nil && textField.text.length < 16) 
    {
        [self resetTextState];
    }
    return YES;
}

// Terminates the editing session
- (BOOL)textFieldShouldReturn:(UITextField *)_textField
{
    auto e1 = new_object<Window::WindowKeyDownEvent>();
    auto e2 = new_object<Window::WindowKeyUpEvent>();
    e1->window = lunaWindow;
    e1->key = HID::KeyCode::enter;
    e2->window = lunaWindow;
    e2->key = HID::KeyCode::enter;
    Window::dispatch_event_to_handler(e1.object());
    Window::dispatch_event_to_handler(e2.object());
    return YES;
}

- (void)resetTextState
{
    textField.text = defaultText;
    committedText = textField.text;
}

@end