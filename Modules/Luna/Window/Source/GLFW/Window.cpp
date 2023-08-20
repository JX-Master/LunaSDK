/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2022/4/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>

#ifdef LUNA_WINDOW_GLFW
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <Luna/Runtime/Thread.hpp>
#include "Monitor.hpp"
#include "../../Application.hpp"
#include "../Window.hpp"
namespace Luna
{
    namespace Window
    {
        void Window::close()
        {
            GLFWwindow* ptr = (GLFWwindow*)atom_exchange_pointer((void**)&(m_window), nullptr);
            if (ptr)
            {
                m_events.reset();
                glfwDestroyWindow(ptr);
            }
        }
        Int2U Window::get_position()
        {
            int x, y;
            glfwGetWindowPos(m_window, &x, &y);
            if (!glfw_succeeded()) return Int2U(0, 0);
            return Int2U(x, y);
        }
        RV Window::set_position(i32 x, i32 y)
        {
            glfwSetWindowPos(m_window, x, y);
            return check_glfw_error();
        }
        UInt2U Window::get_size()
        {
            int width, height;
            glfwGetWindowSize(m_window, &width, &height);
            if (!glfw_succeeded()) return UInt2U(0, 0);
            return UInt2U((u32)width, (u32)height);
        }
        RV Window::set_size(u32 width, u32 height)
        {
            glfwSetWindowSize(m_window, width, height);
            return check_glfw_error();
        }
        UInt2U Window::get_framebuffer_size()
        {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);
            if (!glfw_succeeded()) return UInt2U(0, 0);
            return UInt2U((u32)width, (u32)height);
        }
        f32 Window::get_dpi_scale_factor()
        {
            f32 ret;
            glfwGetWindowContentScale(m_window, &ret, nullptr);
            if (!glfw_succeeded()) return 1.0f;
            return ret;
        }
        RV Window::set_display_settings(const WindowDisplaySettings& display_settings)
        {
            if (display_settings.full_screen)
            {
                GLFWmonitor* monitor = (GLFWmonitor*)display_settings.monitor;
                if (!monitor)
                {
                    monitor = glfwGetPrimaryMonitor();
                }
                auto video_mode = glfwGetVideoMode(monitor);
                u32 width = display_settings.width ? display_settings.width : (u32)video_mode->width;
                u32 height = display_settings.height ? display_settings.height : (u32)video_mode->height;
                u32 refresh_rate = display_settings.refresh_rate ? display_settings.refresh_rate : GLFW_DONT_CARE;
                if (glfwGetWindowMonitor(m_window) == NULL)
                {
                    int x, y;
                    glfwGetWindowSize(m_window, &x, &y);
                    m_windowed_width = (u32)x;
                    m_windowed_height = (u32)y;
                    glfwGetWindowPos(m_window, &x, &y);
                    m_windowed_pos_x = (i32)x;
                    m_windowed_pos_y = (i32)y;
                }
                glfwSetWindowMonitor(m_window, monitor, 0, 0, (int)width, (int)height, (int)refresh_rate);
            }
            else
            {
                u32 width, height;
                i32 pos_x, pos_y;
                if (glfwGetWindowMonitor(m_window))
                {
                    width = display_settings.width ? display_settings.width : m_windowed_width;
                    height = display_settings.height ? display_settings.height : m_windowed_height;
                    pos_x = display_settings.x == DEFAULT_POS ? m_windowed_pos_x : display_settings.x;
                    pos_y = display_settings.y == DEFAULT_POS ? m_windowed_pos_y : display_settings.y;
                }
                else
                {
                    int x, y;
                    glfwGetWindowSize(m_window, &x, &y);
                    width = display_settings.width ? display_settings.width : (u32)x;
                    height = display_settings.height ? display_settings.height : (u32)y;
                    glfwGetWindowPos(m_window, &x, &y);
                    pos_x = display_settings.x == DEFAULT_POS ? (i32)x : display_settings.x;
                    pos_y = display_settings.y == DEFAULT_POS ? (i32)y : display_settings.y;
                }
                glfwSetWindowMonitor(m_window, NULL, (int)pos_x, (int)pos_y, (int)width, (int)height, GLFW_DONT_CARE);
            }
            return check_glfw_error();
        }
        Int2U Window::screen_to_client(const Int2U& point)
        {
            auto pos = get_position();
            return Int2U(point.x - pos.x, point.y - pos.y);
        }
        Int2U Window::client_to_screen(const Int2U& point)
        {
            auto pos = get_position();
            return Int2U(point.x + pos.x, point.y + pos.y);
        }
        RV platform_init()
        {
            register_boxed_type<Window>();
#if defined(LUNA_PLATFORM_WINDOWS)
            impl_interface_for_type<Window, IGLFWWindow, IWin32Window, IWindow>();
#elif defined(LUNA_PLATFORM_MACOS)
            impl_interface_for_type<Window, IGLFWWindow, ICocoaWindow, IWindow>();
#else
            impl_interface_for_type<Window, IGLFWWindow, IWindow>();
#endif
            
            if(!glfwInit())
            {
                return BasicError::error_object();
            }
            monitor_init();
            return ok;
        }

        void platform_close()
        {
            monitor_close();
            glfwTerminate();
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            if (wait_events)
            {
                glfwWaitEvents();
            }
            else
            {
                glfwPollEvents();
            }   
        }

        inline ModifierKeyFlag glfw_translate_mods(int mods)
        {
            ModifierKeyFlag flags = ModifierKeyFlag::none;
            if (mods & GLFW_MOD_SHIFT) set_flags(flags, ModifierKeyFlag::shift);
            if (mods & GLFW_MOD_CONTROL) set_flags(flags, ModifierKeyFlag::ctrl);
            if (mods & GLFW_MOD_ALT) set_flags(flags, ModifierKeyFlag::menu);
            if (mods & GLFW_MOD_SUPER) set_flags(flags, ModifierKeyFlag::system);
            return flags;
        }

        inline HID::KeyCode glfw_translate_key(int key)
        {
            if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) return (HID::KeyCode)((int)HID::KeyCode::num0 + key - GLFW_KEY_0);
            if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return (HID::KeyCode)((int)HID::KeyCode::a + key - GLFW_KEY_A);
            if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12) return (HID::KeyCode)((int)HID::KeyCode::f1 + key - GLFW_KEY_F1);
            if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) return (HID::KeyCode)((int)HID::KeyCode::numpad0 + key - GLFW_KEY_KP_0);
            switch (key)
            {
            case GLFW_KEY_SPACE: return HID::KeyCode::spacebar;
            case GLFW_KEY_APOSTROPHE: return HID::KeyCode::quote;
            case GLFW_KEY_COMMA: return HID::KeyCode::comma;
            case GLFW_KEY_MINUS: return HID::KeyCode::minus;
            case GLFW_KEY_PERIOD: return HID::KeyCode::period;
            case GLFW_KEY_SLASH: return HID::KeyCode::slash;
            case GLFW_KEY_SEMICOLON: return HID::KeyCode::semicolon;
            case GLFW_KEY_EQUAL: return HID::KeyCode::equal;
            case GLFW_KEY_LEFT_BRACKET: return HID::KeyCode::l_branket;
            case GLFW_KEY_BACKSLASH: return HID::KeyCode::backslash;
            case GLFW_KEY_RIGHT_BRACKET: return HID::KeyCode::r_branket;
            case GLFW_KEY_GRAVE_ACCENT: return HID::KeyCode::grave;
            case GLFW_KEY_ESCAPE: return HID::KeyCode::esc;
            case GLFW_KEY_ENTER: return HID::KeyCode::enter;
            case GLFW_KEY_TAB: return HID::KeyCode::tab;
            case GLFW_KEY_BACKSPACE: return HID::KeyCode::backspace;
            case GLFW_KEY_INSERT: return HID::KeyCode::insert;
            case GLFW_KEY_DELETE: return HID::KeyCode::del;
            case GLFW_KEY_RIGHT: return HID::KeyCode::right;
            case GLFW_KEY_LEFT: return HID::KeyCode::left;
            case GLFW_KEY_DOWN: return HID::KeyCode::down;
            case GLFW_KEY_UP: return HID::KeyCode::up;
            case GLFW_KEY_PAGE_UP: return HID::KeyCode::page_up;
            case GLFW_KEY_PAGE_DOWN: return HID::KeyCode::page_down;
            case GLFW_KEY_HOME: return HID::KeyCode::home;
            case GLFW_KEY_END: return HID::KeyCode::end;
            case GLFW_KEY_CAPS_LOCK: return HID::KeyCode::caps_lock;
            case GLFW_KEY_SCROLL_LOCK: return HID::KeyCode::scroll_lock;
            case GLFW_KEY_NUM_LOCK: return HID::KeyCode::num_lock;
            case GLFW_KEY_PRINT_SCREEN: return HID::KeyCode::print_screen;
            case GLFW_KEY_PAUSE: return HID::KeyCode::pause;
            case GLFW_KEY_KP_DECIMAL: return HID::KeyCode::numpad_decimal;
            case GLFW_KEY_KP_DIVIDE: return HID::KeyCode::numpad_divide;
            case GLFW_KEY_KP_MULTIPLY: return HID::KeyCode::numpad_multiply;
            case GLFW_KEY_KP_SUBTRACT: return HID::KeyCode::numpad_subtract;
            case GLFW_KEY_KP_ADD: return HID::KeyCode::numpad_add;
            case GLFW_KEY_KP_ENTER: return HID::KeyCode::enter;
            case GLFW_KEY_KP_EQUAL: return HID::KeyCode::equal;
            case GLFW_KEY_LEFT_SHIFT: return HID::KeyCode::shift;
            case GLFW_KEY_LEFT_CONTROL: return HID::KeyCode::ctrl;
            case GLFW_KEY_LEFT_ALT: return HID::KeyCode::menu;
            case GLFW_KEY_LEFT_SUPER: return HID::KeyCode::l_system;
            case GLFW_KEY_RIGHT_SHIFT: return HID::KeyCode::shift;
            case GLFW_KEY_RIGHT_CONTROL: return HID::KeyCode::ctrl;
            case GLFW_KEY_RIGHT_ALT: return HID::KeyCode::menu;
            case GLFW_KEY_RIGHT_SUPER: return HID::KeyCode::r_system;
            case GLFW_KEY_MENU: return HID::KeyCode::apps;
            }
            return HID::KeyCode::unknown;
        }

        // Event handling.
        static void glfw_on_close(GLFWwindow* window)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            pw->m_events.close(static_cast<IWindow*>(pw));
            // Always set to false, since we will handle the close message and destroy the window manually.
            glfwSetWindowShouldClose(window, GLFW_FALSE);
        }
        static void glfw_on_resize(GLFWwindow* window, int width, int height)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            pw->m_events.resize(static_cast<IWindow*>(pw), (u32)width, (u32)height);
        }
        static void glfw_on_framebuffer_resize(GLFWwindow* window, int width, int height)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            pw->m_events.framebuffer_resize(static_cast<IWindow*>(pw), (u32)width, (u32)height);
        }
        static void glfw_on_move(GLFWwindow* window, int xpos, int ypos)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            pw->m_events.move(static_cast<IWindow*>(pw), xpos, ypos);
        }
        static void glfw_on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            auto hid_key = glfw_translate_key(key);
            if (hid_key == HID::KeyCode::unknown) return;
            if (action == GLFW_PRESS) pw->m_events.key_down(static_cast<IWindow*>(pw), hid_key);
            else if (action == GLFW_RELEASE) pw->m_events.key_up(static_cast<IWindow*>(pw), hid_key);
        }
        static void glfw_on_char(GLFWwindow* window, unsigned int codepoint)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            c32 character = codepoint;
            pw->m_events.input_character(static_cast<IWindow*>(pw), character);
        }
        static void glfw_on_mouse_move(GLFWwindow* window, double xpos, double ypos)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            i32 x = (i32)xpos;
            i32 y = (i32)ypos;
            pw->m_events.mouse_move(static_cast<IWindow*>(pw), x, y);
        }
        static void glfw_on_mouse_button(GLFWwindow* window, int button, int action, int mods)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            HID::MouseButton btn;
            if (button == GLFW_MOUSE_BUTTON_LEFT) btn = HID::MouseButton::left;
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) btn = HID::MouseButton::right;
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE) btn = HID::MouseButton::middle;
            else if (button == GLFW_MOUSE_BUTTON_4) btn = HID::MouseButton::function1;
            else if (button == GLFW_MOUSE_BUTTON_5) btn = HID::MouseButton::function2;
            else return;
            auto modifier_flags = glfw_translate_mods(mods);
            if (action == GLFW_PRESS) pw->m_events.mouse_down(static_cast<IWindow*>(pw), modifier_flags, btn);
            else if (action == GLFW_RELEASE) pw->m_events.mouse_up(static_cast<IWindow*>(pw), modifier_flags, btn);
        }
        static void glfw_on_mouse_wheel(GLFWwindow* window, double xoffset, double yoffset)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            f32 x_wheel_delta = (f32)xoffset;
            f32 y_wheel_delta = (f32)yoffset;
            pw->m_events.mouse_wheel(static_cast<IWindow*>(pw), x_wheel_delta, y_wheel_delta);
        }
        static void glfw_on_drop_file(GLFWwindow* window, int count, const char** paths)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            i32 mouse_position_x = (i32)x;
            i32 mouse_position_y = (i32)y;
            pw->m_events.drop_file(static_cast<IWindow*>(pw), mouse_position_x, mouse_position_y, Span<const c8*>(paths, count));
        }
        static void glfw_on_dpi_change(GLFWwindow* window, float xscale, float yscale)
        {
            Window* pw = (Window*)glfwGetWindowUserPointer(window);
            f32 scale = xscale;
            pw->m_events.dpi_changed(static_cast<IWindow*>(pw), scale);
        }
        static void register_event_callbacks(GLFWwindow* window)
        {
            glfwSetWindowCloseCallback(window, glfw_on_close);
            glfwSetWindowSizeCallback(window, glfw_on_resize);
            glfwSetFramebufferSizeCallback(window, glfw_on_framebuffer_resize);
            glfwSetWindowPosCallback(window, glfw_on_move);
            glfwSetKeyCallback(window, glfw_on_key);
            glfwSetCharCallback(window, glfw_on_char);
            glfwSetCursorPosCallback(window, glfw_on_mouse_move);
            glfwSetMouseButtonCallback(window, glfw_on_mouse_button);
            glfwSetScrollCallback(window, glfw_on_mouse_wheel);
            glfwSetDropCallback(window, glfw_on_drop_file);
            glfwSetWindowContentScaleCallback(window, glfw_on_dpi_change);
        }

        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, const WindowDisplaySettings& display_settings, WindowCreationFlag flags)
        {
            lucheck_msg(get_current_thread() == get_main_thread(), "RHI::new_window must only be called from the main thread.");
            Ref<Window> window = new_object<Window>();
            // We don't want any render context here, since we will create the context later in `Gfx`.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
            // Test flags.
            glfwWindowHint(GLFW_DECORATED, test_flags(flags, WindowCreationFlag::borderless) ? GLFW_FALSE : GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, test_flags(flags, WindowCreationFlag::resizable) ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_VISIBLE, test_flags(flags, WindowCreationFlag::hidden) ? GLFW_FALSE : GLFW_TRUE);
            // Read monitor info.
            GLFWmonitor* monitor = NULL;
            if (display_settings.full_screen)
            {
                monitor = (GLFWmonitor*)display_settings.monitor;
            }
            if (!monitor) monitor = glfwGetPrimaryMonitor();
            auto mode = glfwGetVideoMode(monitor);
            if (display_settings.full_screen)
            {
                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            }
            else
            {
                glfwWindowHint(GLFW_RED_BITS, GLFW_DONT_CARE);
                glfwWindowHint(GLFW_GREEN_BITS, GLFW_DONT_CARE);
                glfwWindowHint(GLFW_BLUE_BITS, GLFW_DONT_CARE);
            }
            // Calculate window size.
            int screen_x;
            int screen_y;
            int screen_w;
            int screen_h;
            glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &screen_x, &screen_y, &screen_w, &screen_h);
            if (display_settings.full_screen)
            {
                window->m_windowed_width = screen_w * 3 / 4;
                window->m_windowed_height = screen_h * 3 / 4;
                window->m_windowed_pos_x = screen_x + (screen_w - window->m_windowed_width) / 2;
                window->m_windowed_pos_y = screen_y + (screen_h - window->m_windowed_height) / 2;
            }
            else
            {
                window->m_windowed_width = display_settings.width ? display_settings.width : screen_w * 3 / 4;
                window->m_windowed_height = display_settings.height ? display_settings.height : screen_h * 3 / 4;
                window->m_windowed_pos_x = display_settings.x != DEFAULT_POS ? display_settings.x : screen_x + (screen_w - window->m_windowed_width) / 2;
                window->m_windowed_pos_y = display_settings.y != DEFAULT_POS ? display_settings.y : screen_y + (screen_h - window->m_windowed_height) / 2;
            }
            // Create normal window first.
            window->m_window = glfwCreateWindow(window->m_windowed_width, window->m_windowed_height, title, NULL, NULL);
            if (!window->m_window)
            {
                return encode_glfw_error();
            }
            // Set to full screen or position the window.
            if (display_settings.full_screen)
            {   
                int width = display_settings.width ? (int)display_settings.width : mode->width;
                int height = display_settings.height ? (int)display_settings.height : mode->height;
                glfwSetWindowMonitor(window->m_window, monitor, 0, 0, width, height, display_settings.refresh_rate ? display_settings.refresh_rate : mode->refreshRate);
            }
            else
            {
                glfwSetWindowPos(window->m_window, window->m_windowed_pos_x, window->m_windowed_pos_y);
            }
            ErrCode err = encode_glfw_error();
            if (err.code) return err;
            // Set userdata.
            glfwSetWindowUserPointer(window->m_window, window.object());
            // Set callbacks.
            register_event_callbacks(window->m_window);
            return Ref<IWindow>(window);
        }

        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
        {
            g_startup_params = params;
        }
    }
}
#endif
