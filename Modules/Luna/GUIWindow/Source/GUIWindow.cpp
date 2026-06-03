/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIWindow.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_WINDOW_API LUNA_EXPORT
#include "../GUIWindow.hpp"
#include <Luna/HID/HID.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/HID/Mouse.hpp>
#include <Luna/Window/Clipboard.hpp>
#include <Luna/Window/Event.hpp>

namespace Luna
{
    namespace GUIWindow
    {
        static GUI::PointerButton to_gui_button(HID::MouseButton button)
        {
            switch(button)
            {
            case HID::MouseButton::left: return GUI::PointerButton::left;
            case HID::MouseButton::right: return GUI::PointerButton::right;
            case HID::MouseButton::middle: return GUI::PointerButton::middle;
            case HID::MouseButton::function1: return GUI::PointerButton::extra1;
            case HID::MouseButton::function2: return GUI::PointerButton::extra2;
            default: return GUI::PointerButton::left;
            }
        }

        static GUI::Key to_gui_key(HID::KeyCode key)
        {
            using namespace HID;
            switch(key)
            {
            case KeyCode::tab: return GUI::Key::tab;
            case KeyCode::left: return GUI::Key::left;
            case KeyCode::right: return GUI::Key::right;
            case KeyCode::up: return GUI::Key::up;
            case KeyCode::down: return GUI::Key::down;
            case KeyCode::enter: return GUI::Key::enter;
            case KeyCode::esc: return GUI::Key::esc;
            case KeyCode::backspace: return GUI::Key::backspace;
            case KeyCode::del: return GUI::Key::del;
            case KeyCode::spacebar: return GUI::Key::space;
            case KeyCode::a: return GUI::Key::a;
            case KeyCode::b: return GUI::Key::b;
            case KeyCode::c: return GUI::Key::c;
            case KeyCode::d: return GUI::Key::d;
            case KeyCode::e: return GUI::Key::e;
            case KeyCode::f: return GUI::Key::f;
            case KeyCode::g: return GUI::Key::g;
            case KeyCode::h: return GUI::Key::h;
            case KeyCode::i: return GUI::Key::i;
            case KeyCode::j: return GUI::Key::j;
            case KeyCode::k: return GUI::Key::k;
            case KeyCode::l: return GUI::Key::l;
            case KeyCode::m: return GUI::Key::m;
            case KeyCode::n: return GUI::Key::n;
            case KeyCode::o: return GUI::Key::o;
            case KeyCode::p: return GUI::Key::p;
            case KeyCode::q: return GUI::Key::q;
            case KeyCode::r: return GUI::Key::r;
            case KeyCode::s: return GUI::Key::s;
            case KeyCode::t: return GUI::Key::t;
            case KeyCode::u: return GUI::Key::u;
            case KeyCode::v: return GUI::Key::v;
            case KeyCode::w: return GUI::Key::w;
            case KeyCode::x: return GUI::Key::x;
            case KeyCode::y: return GUI::Key::y;
            case KeyCode::z: return GUI::Key::z;
            default: return GUI::Key::unknown;
            }
        }

        static GUI::KeyModifierFlag get_gui_modifiers()
        {
            u8 flags = 0;
            if(HID::get_key_state(HID::KeyCode::ctrl)) flags |= (u8)GUI::KeyModifierFlag::ctrl;
            if(HID::get_key_state(HID::KeyCode::shift)) flags |= (u8)GUI::KeyModifierFlag::shift;
            if(HID::get_key_state(HID::KeyCode::menu)) flags |= (u8)GUI::KeyModifierFlag::alt;
            if(HID::get_key_state(HID::KeyCode::system)) flags |= (u8)GUI::KeyModifierFlag::system;
            return (GUI::KeyModifierFlag)flags;
        }

        static bool is_client_position_valid(Window::IWindow* window, const Float2U& position)
        {
            UInt2U size = window->get_size();
            return position.x >= 0.0f && position.y >= 0.0f &&
                position.x < (f32)size.x && position.y < (f32)size.y;
        }

        static Float2U get_client_mouse_pos_unchecked(Window::IWindow* window)
        {
            Int2U screen_pos = HID::get_mouse_pos();
            Int2U client_pos = window->screen_to_client(screen_pos);
            return Float2U((f32)client_pos.x, (f32)client_pos.y);
        }

        static bool get_client_mouse_pos(Window::IWindow* window, Float2U& position)
        {
            position = get_client_mouse_pos_unchecked(window);
            return is_client_position_valid(window, position);
        }

        LUNA_GUI_WINDOW_API bool handle_window_event(object_t event, Window::IWindow* window, GUI::IContext* gui)
        {
            if(!window || !gui) return false;
            auto window_event = cast_object<Window::WindowEvent>(event);
            if(!window_event || window_event->window != window) return false;

            GUI::InputEvent ge;
            if(cast_object<Window::WindowMouseEnterEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUI::InputEventType::pointer_enter;
            }
            else if(cast_object<Window::WindowMouseLeaveEvent>(event))
            {
                ge.type = GUI::InputEventType::pointer_leave;
                ge.position = get_client_mouse_pos_unchecked(window);
            }
            else if(auto e = cast_object<Window::WindowMouseMoveEvent>(event))
            {
                ge.position = Float2U((f32)e->x, (f32)e->y);
                if(is_client_position_valid(window, ge.position) || get_client_mouse_pos(window, ge.position))
                {
                    ge.type = GUI::InputEventType::pointer_move;
                }
                else
                {
                    ge.type = GUI::InputEventType::pointer_leave;
                    ge.position = get_client_mouse_pos_unchecked(window);
                }
            }
            else if(auto e = cast_object<Window::WindowMouseDownEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUI::InputEventType::pointer_down;
                ge.button = to_gui_button(e->button);
            }
            else if(auto e = cast_object<Window::WindowMouseUpEvent>(event))
            {
                ge.type = GUI::InputEventType::pointer_up;
                ge.position = get_client_mouse_pos_unchecked(window);
                ge.button = to_gui_button(e->button);
            }
            else if(auto e = cast_object<Window::WindowScrollEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUI::InputEventType::pointer_wheel;
                ge.wheel_delta = Float2U(e->scroll_x, e->scroll_y);
            }
            else if(auto e = cast_object<Window::WindowTouchDownEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                if(!is_client_position_valid(window, ge.position)) return false;
                ge.type = GUI::InputEventType::pointer_down;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                ge.button = GUI::PointerButton::left;
            }
            else if(auto e = cast_object<Window::WindowTouchMoveEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                ge.type = GUI::InputEventType::pointer_move;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                if(!is_client_position_valid(window, ge.position))
                {
                    ge.type = GUI::InputEventType::pointer_leave;
                }
            }
            else if(auto e = cast_object<Window::WindowTouchUpEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                ge.type = GUI::InputEventType::pointer_up;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                ge.button = GUI::PointerButton::left;
            }
            else if(auto e = cast_object<Window::WindowKeyDownEvent>(event))
            {
                ge.type = GUI::InputEventType::key_down;
                ge.key = to_gui_key(e->key);
                ge.modifiers = get_gui_modifiers();
            }
            else if(auto e = cast_object<Window::WindowKeyUpEvent>(event))
            {
                ge.type = GUI::InputEventType::key_up;
                ge.key = to_gui_key(e->key);
                ge.modifiers = get_gui_modifiers();
            }
            else if(auto e = cast_object<Window::WindowInputTextEvent>(event))
            {
                ge.type = GUI::InputEventType::text_utf8;
                ge.text = e->text;
            }
            else if(cast_object<Window::WindowInputFocusEvent>(event))
            {
                ge.type = GUI::InputEventType::focus;
            }
            else if(cast_object<Window::WindowLoseInputFocusEvent>(event))
            {
                ge.type = GUI::InputEventType::blur;
            }
            else
            {
                return false;
            }
            gui->add_input_event(ge);
            return true;
        }

        static void gui_window_event_handler(object_t event, void* userdata)
        {
            auto adapter = (GUIWindowInputAdapter*)userdata;
            if(!adapter) return;
            handle_window_event(event, adapter->window, adapter->gui);
            if(adapter->forward_events && adapter->next_event_handler)
            {
                adapter->next_event_handler(event, adapter->next_event_userdata);
            }
        }

        LUNA_GUI_WINDOW_API void install_window_event_handler(GUIWindowInputAdapter* adapter)
        {
            if(!adapter) return;
            void(*previous_handler)(object_t event, void* userdata) = nullptr;
            void* previous_userdata = nullptr;
            Window::get_event_handler(&previous_handler, &previous_userdata);
            if(previous_handler == gui_window_event_handler && previous_userdata == adapter)
            {
                return;
            }
            adapter->next_event_handler = previous_handler;
            adapter->next_event_userdata = previous_userdata;
            Window::set_event_handler(gui_window_event_handler, adapter);
        }

        LUNA_GUI_WINDOW_API void uninstall_window_event_handler(GUIWindowInputAdapter* adapter)
        {
            if(!adapter) return;
            void(*current_handler)(object_t event, void* userdata) = nullptr;
            void* current_userdata = nullptr;
            Window::get_event_handler(&current_handler, &current_userdata);
            if(current_handler == gui_window_event_handler && current_userdata == adapter)
            {
                Window::set_event_handler(adapter->next_event_handler, adapter->next_event_userdata);
            }
            adapter->next_event_handler = nullptr;
            adapter->next_event_userdata = nullptr;
        }

        static RectI to_window_text_input_rect(const GUI::TextInputState& state)
        {
            return RectI(
                (i32)floor(state.rect.offset_x),
                (i32)floor(state.rect.offset_y),
                max((i32)ceil(state.rect.width), 1),
                max((i32)ceil(state.rect.height), 1));
        }

        static RV get_window_clipboard_text(String& out_text, void*)
        {
            return Window::get_clipboard_text(out_text);
        }

        static RV set_window_clipboard_text(const c8* text, usize size, void*)
        {
            return Window::set_clipboard_text(text, size);
        }

        static void update_pointer_state(Window::IWindow* window, GUI::IContext* gui)
        {
            GUI::InputEvent event;
            if(get_client_mouse_pos(window, event.position))
            {
                event.type = GUI::InputEventType::pointer_move;
            }
            else
            {
                event.type = GUI::InputEventType::pointer_leave;
                event.position = get_client_mouse_pos_unchecked(window);
            }
            gui->add_input_event(event);
        }

        LUNA_GUI_WINDOW_API RV update_text_input(Window::IWindow* window, GUI::IContext* gui)
        {
            if(!window || !gui) return ok;
            lutry
            {
                update_pointer_state(window, gui);

                GUI::ClipboardIO clipboard_io;
                clipboard_io.get_text = get_window_clipboard_text;
                clipboard_io.set_text = set_window_clipboard_text;
                gui->set_clipboard_io(clipboard_io);

                GUI::TextInputState state = gui->get_text_input_state();
                if(state.active)
                {
                    if(!window->is_text_input_active())
                    {
                        luexp(window->begin_text_input());
                    }
                    luexp(window->set_text_input_area(to_window_text_input_rect(state), state.cursor));
                }
                else if(window->is_text_input_active())
                {
                    luexp(window->end_text_input());
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_WINDOW_API RV update_text_input(GUIWindowInputAdapter* adapter)
        {
            return adapter ? update_text_input(adapter->window, adapter->gui) : ok;
        }

        struct GUIWindowModule : public Module
        {
            virtual const c8* get_name() override { return "GUIWindow"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {GUI::module_gui(), module_window(), module_hid()});
            }
            virtual RV on_init() override { return ok; }
            virtual void on_close() override {}
        };

        LUNA_GUI_WINDOW_API Module* module_gui_window()
        {
            static GUIWindowModule m;
            return &m;
        }
    }
}
