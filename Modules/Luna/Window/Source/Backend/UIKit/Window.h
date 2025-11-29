/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.h
* @author JXMaster
* @date 2025/11/21
*/
#pragma once

#import <UIKit/UIKit.h>
#include "../../../UIKit/UIKitWindow.hpp"

namespace Luna
{
    namespace Window
    {
        struct UIKitWindow;
    }
}

@interface LunaWindowView : UIView 

@property(nonatomic) Luna::Window::UIKitWindow* lunaWindow;

- (instancetype) initWithFrame:(CGRect)frame;
- (void)updateDrawableSize;
+ (Class) layerClass;

@end

@interface LunaWindowViewController : UIViewController <UITextFieldDelegate>

@property(nonatomic) Luna::Window::UIKitWindow* lunaWindow;

- (instancetype)initWithLunaWindow:(Luna::Window::UIKitWindow *)window;

// Interaction with system UI.
// - (NSUInteger)supportedInterfaceOrientations;
// - (BOOL)prefersStatusBarHidden;
// - (BOOL)prefersHomeIndicatorAutoHidden;
// - (UIRectEdge)preferredScreenEdgesDeferringSystemGestures;

@property(nonatomic, assign) int homeIndicatorHidden;
// End.

// Keyboard support
- (void)initKeyboard;
- (void)closeKeyboard;
- (void)startTextInput;
- (void)stopTextInput;

- (void)keyboardWillShow:(NSNotification *)notification;
- (void)keyboardWillHide:(NSNotification *)notification;

- (void)updateKeyboard;

@property(nonatomic, assign, getter=isTextFieldFocused) BOOL textFieldFocused;
@property(nonatomic, assign) Luna::RectI textInputRect;
@property(nonatomic, assign) int keyboardHeight;
// End.

@end

namespace Luna
{
    namespace Window
    {
        struct UIKitWindow : IUIKitWindow
        {
            lustruct("Window::UIKitWindow", "28cb0e59-1ef2-4c47-bb37-4fc5a827640f");
            luiimpl();

            UIWindow* m_window = nil;
            LunaWindowViewController* m_view_controller = nil;
            LunaWindowView* m_view = nil;
            bool m_minimized = false;
            bool m_text_input_active = false;

            RV init(UIWindowScene* scene);

            virtual bool is_closed() override
            {
                return m_window == nil;
            }
            virtual bool has_input_focus() override
            {
                return !m_minimized;
            }
            virtual bool has_mouse_focus() override
            {
                return !m_minimized;
            }
            virtual bool is_minimized() override
            {
                return m_minimized;
            }
            virtual Int2U get_position() override;
            virtual UInt2U get_size() override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
            virtual RV begin_text_input() override;
            virtual RV set_text_input_area(const RectI& input_rect, i32 cursor) override;
            virtual RV end_text_input() override;
            virtual bool is_text_input_active() override;
            virtual id get_uiwindow() override
            {
                return m_window;
            }
        };

        extern Ref<UIKitWindow> g_window;
    }
}