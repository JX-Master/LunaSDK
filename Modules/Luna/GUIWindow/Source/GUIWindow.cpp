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
#include "GUIWindowShared.hpp"
#include <Luna/HID/HID.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/Window/Event.hpp>

namespace Luna
{
    namespace GUIWindow
    {
        static GUICore::PointerButton to_core_button(HID::MouseButton button)
        {
            switch(button)
            {
            case HID::MouseButton::left: return GUICore::PointerButton::left;
            case HID::MouseButton::right: return GUICore::PointerButton::right;
            case HID::MouseButton::middle: return GUICore::PointerButton::middle;
            case HID::MouseButton::function1: return GUICore::PointerButton::extra1;
            case HID::MouseButton::function2: return GUICore::PointerButton::extra2;
            default: return GUICore::PointerButton::left;
            }
        }

        static GUICore::KeyModifierFlag get_core_modifiers()
        {
            u8 flags = 0;
            if(HID::get_key_state(KeyCode::ctrl)) flags |= (u8)GUICore::KeyModifierFlag::ctrl;
            if(HID::get_key_state(KeyCode::shift)) flags |= (u8)GUICore::KeyModifierFlag::shift;
            if(HID::get_key_state(KeyCode::menu)) flags |= (u8)GUICore::KeyModifierFlag::alt;
            if(HID::get_key_state(KeyCode::system)) flags |= (u8)GUICore::KeyModifierFlag::system;
            return (GUICore::KeyModifierFlag)flags;
        }

        static bool translate_core_window_event(object_t event, Window::IWindow* window, GUICore::InputEvent& ge)
        {
            if(!window) return false;
            auto window_event = cast_object<Window::WindowEvent>(event);
            if(!window_event || window_event->window != window) return false;

            ge = GUICore::InputEvent();
            if(cast_object<Window::WindowMouseEnterEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUICore::InputEventType::pointer_enter;
            }
            else if(cast_object<Window::WindowMouseLeaveEvent>(event))
            {
                ge.type = GUICore::InputEventType::pointer_leave;
                ge.position = get_client_mouse_pos_unchecked(window);
            }
            else if(auto e = cast_object<Window::WindowMouseMoveEvent>(event))
            {
                ge.position = Float2U((f32)e->x, (f32)e->y);
                if(is_client_position_valid(window, ge.position) || get_client_mouse_pos(window, ge.position))
                {
                    ge.type = GUICore::InputEventType::pointer_move;
                }
                else
                {
                    ge.type = GUICore::InputEventType::pointer_leave;
                    ge.position = get_client_mouse_pos_unchecked(window);
                }
            }
            else if(auto e = cast_object<Window::WindowMouseDownEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUICore::InputEventType::pointer_down;
                ge.button = to_core_button(e->button);
            }
            else if(auto e = cast_object<Window::WindowMouseUpEvent>(event))
            {
                ge.type = GUICore::InputEventType::pointer_up;
                ge.position = get_client_mouse_pos_unchecked(window);
                ge.button = to_core_button(e->button);
            }
            else if(auto e = cast_object<Window::WindowScrollEvent>(event))
            {
                if(!get_client_mouse_pos(window, ge.position)) return false;
                ge.type = GUICore::InputEventType::pointer_wheel;
                ge.wheel_delta = Float2U(e->scroll_x, e->scroll_y);
            }
            else if(auto e = cast_object<Window::WindowTouchDownEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                if(!is_client_position_valid(window, ge.position)) return false;
                ge.type = GUICore::InputEventType::pointer_down;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                ge.button = GUICore::PointerButton::left;
            }
            else if(auto e = cast_object<Window::WindowTouchMoveEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                ge.type = GUICore::InputEventType::pointer_move;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                if(!is_client_position_valid(window, ge.position))
                {
                    ge.type = GUICore::InputEventType::pointer_leave;
                }
            }
            else if(auto e = cast_object<Window::WindowTouchUpEvent>(event))
            {
                ge.position = Float2U(e->x, e->y);
                ge.type = GUICore::InputEventType::pointer_up;
                ge.device_id = 1;
                ge.pointer_id = e->id;
                ge.button = GUICore::PointerButton::left;
            }
            else if(auto e = cast_object<Window::WindowKeyDownEvent>(event))
            {
                ge.type = GUICore::InputEventType::key_down;
                ge.key = e->key;
                ge.modifiers = get_core_modifiers();
            }
            else if(auto e = cast_object<Window::WindowKeyUpEvent>(event))
            {
                ge.type = GUICore::InputEventType::key_up;
                ge.key = e->key;
                ge.modifiers = get_core_modifiers();
            }
            else if(auto e = cast_object<Window::WindowInputTextEvent>(event))
            {
                ge.type = GUICore::InputEventType::text_utf8;
                ge.text = e->text;
            }
            else if(cast_object<Window::WindowInputFocusEvent>(event))
            {
                ge.type = GUICore::InputEventType::focus;
            }
            else if(cast_object<Window::WindowLoseInputFocusEvent>(event))
            {
                ge.type = GUICore::InputEventType::blur;
            }
            else
            {
                return false;
            }
            return true;
        }

        LUNA_GUI_WINDOW_API bool handle_window_event(object_t event, Window::IWindow* window, GUICore::IContext* gui)
        {
            if(!window || !gui) return false;
            GUICore::InputEvent ge;
            if(!translate_core_window_event(event, window, ge)) return false;
            gui->add_input_event(ge);
            return true;
        }

        static void gui_core_window_event_handler(object_t event, void* userdata)
        {
            auto adapter = (GUICoreWindowInputAdapter*)userdata;
            if(!adapter) return;
            GUICore::InputEvent ge;
            if(translate_core_window_event(event, adapter->window, ge))
            {
                adapter->pending_events.push_back(move(ge));
            }
            if(adapter->forward_events && adapter->next_event_handler)
            {
                adapter->next_event_handler(event, adapter->next_event_userdata);
            }
        }

        LUNA_GUI_WINDOW_API void install_window_event_handler(GUICoreWindowInputAdapter* adapter)
        {
            if(!adapter) return;
            void(*previous_handler)(object_t event, void* userdata) = nullptr;
            void* previous_userdata = nullptr;
            Window::get_event_handler(&previous_handler, &previous_userdata);
            if(previous_handler == gui_core_window_event_handler && previous_userdata == adapter)
            {
                return;
            }
            adapter->next_event_handler = previous_handler;
            adapter->next_event_userdata = previous_userdata;
            Window::set_event_handler(gui_core_window_event_handler, adapter);
        }

        LUNA_GUI_WINDOW_API void uninstall_window_event_handler(GUICoreWindowInputAdapter* adapter)
        {
            if(!adapter) return;
            void(*current_handler)(object_t event, void* userdata) = nullptr;
            void* current_userdata = nullptr;
            Window::get_event_handler(&current_handler, &current_userdata);
            if(current_handler == gui_core_window_event_handler && current_userdata == adapter)
            {
                Window::set_event_handler(adapter->next_event_handler, adapter->next_event_userdata);
            }
            adapter->next_event_handler = nullptr;
            adapter->next_event_userdata = nullptr;
        }

        static void update_pointer_state(Window::IWindow* window, GUICore::IContext* gui)
        {
            GUICore::InputEvent event;
            if(get_client_mouse_pos(window, event.position))
            {
                event.type = GUICore::InputEventType::pointer_move;
            }
            else
            {
                event.type = GUICore::InputEventType::pointer_leave;
                event.position = get_client_mouse_pos_unchecked(window);
            }
            gui->add_input_event(event);
        }

        LUNA_GUI_WINDOW_API void update_input(Window::IWindow* window, GUICore::IContext* gui)
        {
            if(!window || !gui) return;
            GUICore::ClipboardIO clipboard_io;
            clipboard_io.get_text = get_window_clipboard_text;
            clipboard_io.set_text = set_window_clipboard_text;
            gui->set_clipboard_io(clipboard_io);
            update_pointer_state(window, gui);
        }

        LUNA_GUI_WINDOW_API RV update_text_input(Window::IWindow* window, GUICore::IContext* gui)
        {
            if(!window || !gui) return ok;
            lutry
            {
                GUICore::ClipboardIO clipboard_io;
                clipboard_io.get_text = get_window_clipboard_text;
                clipboard_io.set_text = set_window_clipboard_text;
                gui->set_clipboard_io(clipboard_io);

                GUICore::TextInputState state = gui->get_text_input_state();
                if(state.active)
                {
                    if(!window->is_text_input_active())
                    {
                        luexp(window->begin_text_input());
                    }
                    luexp(window->set_text_input_area(to_window_text_input_rect(state.rect), state.cursor));
                }
                else if(window->is_text_input_active())
                {
                    luexp(window->end_text_input());
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_GUI_WINDOW_API RV update_text_input(GUICoreWindowInputAdapter* adapter)
        {
            return adapter ? update_text_input(adapter->window, adapter->gui) : ok;
        }

        LUNA_GUI_WINDOW_API void update_input(GUICoreWindowInputAdapter* adapter)
        {
            if(adapter && adapter->gui)
            {
                if(!adapter->pending_events.empty())
                {
                    adapter->gui->add_input_events(Span<const GUICore::InputEvent>(
                        adapter->pending_events.data(), adapter->pending_events.size()));
                    adapter->pending_events.clear();
                }
                update_input(adapter->window, adapter->gui);
            }
        }
    }
}
