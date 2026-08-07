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

        static GUI::KeyModifierFlag get_gui_modifiers()
        {
            u8 flags = 0;
            if(HID::get_key_state(KeyCode::ctrl)) flags |= (u8)GUI::KeyModifierFlag::ctrl;
            if(HID::get_key_state(KeyCode::shift)) flags |= (u8)GUI::KeyModifierFlag::shift;
            if(HID::get_key_state(KeyCode::menu)) flags |= (u8)GUI::KeyModifierFlag::alt;
            if(HID::get_key_state(KeyCode::system)) flags |= (u8)GUI::KeyModifierFlag::system;
            return (GUI::KeyModifierFlag)flags;
        }

        static bool translate_gui_window_event(object_t event, Window::IWindow* window, GUI::InputEvent& ge)
        {
            if(!window) return false;
            auto window_event = cast_object<Window::WindowEvent>(event);
            if(!window_event || window_event->window != window) return false;

            ge = GUI::InputEvent();
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
                ge.key = e->key;
                ge.modifiers = get_gui_modifiers();
            }
            else if(auto e = cast_object<Window::WindowKeyUpEvent>(event))
            {
                ge.type = GUI::InputEventType::key_up;
                ge.key = e->key;
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
            return true;
        }

        static bool make_navigation_event(const GUI::InputEvent& key_event, GUI::InputEvent& nav_event)
        {
            if(key_event.type != GUI::InputEventType::key_down)
            {
                return false;
            }
            nav_event = GUI::InputEvent();
            nav_event.device_id = key_event.device_id;
            nav_event.modifiers = key_event.modifiers;
            switch(key_event.key)
            {
            case KeyCode::left:
                nav_event.type = GUI::InputEventType::navigation_dpad;
                nav_event.navigation_direction = GUI::NavigationDirection::left;
                return true;
            case KeyCode::right:
                nav_event.type = GUI::InputEventType::navigation_dpad;
                nav_event.navigation_direction = GUI::NavigationDirection::right;
                return true;
            case KeyCode::up:
                nav_event.type = GUI::InputEventType::navigation_dpad;
                nav_event.navigation_direction = GUI::NavigationDirection::up;
                return true;
            case KeyCode::down:
                nav_event.type = GUI::InputEventType::navigation_dpad;
                nav_event.navigation_direction = GUI::NavigationDirection::down;
                return true;
            case KeyCode::tab:
                nav_event.type = GUI::InputEventType::navigation_move;
                nav_event.navigation_move = ((u8)key_event.modifiers & (u8)GUI::KeyModifierFlag::shift) ?
                    GUI::NavigationMove::backward : GUI::NavigationMove::forward;
                return true;
            case KeyCode::enter:
                nav_event.type = GUI::InputEventType::navigation_confirm;
                return true;
            case KeyCode::esc:
                nav_event.type = GUI::InputEventType::navigation_back;
                return true;
            default:
                return false;
            }
        }

        static void append_translated_input_event(Vector<GUI::InputEvent>& events, GUI::InputEvent&& event)
        {
            GUI::InputEvent nav_event;
            bool has_nav_event = make_navigation_event(event, nav_event);
            events.push_back(move(event));
            if(has_nav_event)
            {
                events.push_back(move(nav_event));
            }
        }

        LUNA_GUI_WINDOW_API bool handle_window_event(object_t event, Window::IWindow* window, GUI::IContext* gui)
        {
            if(!window || !gui) return false;
            GUI::InputEvent ge;
            if(!translate_gui_window_event(event, window, ge)) return false;
            gui->add_input_event(ge);
            GUI::InputEvent nav_event;
            if(make_navigation_event(ge, nav_event))
            {
                gui->add_input_event(nav_event);
            }
            return true;
        }

        static void gui_window_event_handler(object_t event, void* userdata)
        {
            auto adapter = (GUIWindowInputAdapter*)userdata;
            if(!adapter) return;
            GUI::InputEvent ge;
            if(translate_gui_window_event(event, adapter->window, ge))
            {
                append_translated_input_event(adapter->pending_events, move(ge));
            }
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

        LUNA_GUI_WINDOW_API void update_input(Window::IWindow* window, GUI::IContext* gui)
        {
            if(!window || !gui) return;
            GUI::ClipboardIO clipboard_io;
            clipboard_io.get_text = get_window_clipboard_text;
            clipboard_io.set_text = set_window_clipboard_text;
            gui->set_clipboard_io(clipboard_io);
            update_pointer_state(window, gui);
        }

        LUNA_GUI_WINDOW_API RV update_text_input(Window::IWindow* window, GUI::IContext* gui)
        {
            if(!window || !gui) return ok;
            lutry
            {
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

        LUNA_GUI_WINDOW_API RV update_text_input(GUIWindowInputAdapter* adapter)
        {
            return adapter ? update_text_input(adapter->window, adapter->gui) : ok;
        }

        LUNA_GUI_WINDOW_API void update_input(GUIWindowInputAdapter* adapter)
        {
            if(adapter && adapter->gui)
            {
                if(!adapter->pending_events.empty())
                {
                    adapter->gui->add_input_events(Span<const GUI::InputEvent>(
                        adapter->pending_events.data(), adapter->pending_events.size()));
                    adapter->pending_events.clear();
                }
                update_input(adapter->window, adapter->gui);
            }
        }
    }
}
