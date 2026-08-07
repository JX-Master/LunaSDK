/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InputSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUITest.hpp"
#include <cstdio>

namespace Luna::GUITest
{
    namespace
    {
        void input_sample(GUI::IContext* context, GUI::id_t id, const c8* label,
            const Float4U& color, GUI::PointerHitBehavior behavior, bool circle = false)
        {
            GUI::ElementHandle element = context->begin_element(id);
            context->set_layout_config(element, fixed_layout(circle ? 132.0f : 220.0f, circle ? 132.0f : 92.0f));
            set_interactable(context, element, behavior,
                GUI::InteractableFlag::hoverable | GUI::InteractableFlag::activatable |
                GUI::InteractableFlag::focusable);
            if(circle)
            {
                GUI::ElementHitTestConfig config;
                config.mode = GUI::ElementHitTestMode::callback;
                config.callback = circle_hit_test;
                context->set_hit_test_config(element, config);
            }
            GUI::InteractionState state = context->get_interaction_state(id);
            Float4U fill = state.hovered ? Float4U(0.02f, 0.02f, 0.02f, 1.0f) : color;
            Float4U text = state.hovered ? Float4U(1.0f, 1.0f, 1.0f, 1.0f) : Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            if(circle)
            {
                draw_rect(context, RectF(0.0f, 0.0f, 132.0f, 132.0f), fill, 66.0f);
                draw_text(context, RectF(10.0f, 52.0f, 112.0f, 28.0f), label, 20.0f, text, VG::TextAlignment::center);
            }
            else
            {
                draw_rect(context, RectF(0.0f, 0.0f, 220.0f, 92.0f), fill, 4.0f);
                draw_outline(context, RectF(0.0f, 0.0f, 220.0f, 92.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
                draw_text(context, RectF(14.0f, 32.0f, 192.0f, 30.0f), label, 21.0f, text, VG::TextAlignment::center);
            }
            context->end_element();
        }

        const c8* key_name(KeyCode key)
        {
            switch(key)
            {
            case KeyCode::esc: return "Esc";
            case KeyCode::f1: return "F1";
            case KeyCode::f2: return "F2";
            case KeyCode::f3: return "F3";
            case KeyCode::f4: return "F4";
            case KeyCode::f5: return "F5";
            case KeyCode::f6: return "F6";
            case KeyCode::f7: return "F7";
            case KeyCode::f8: return "F8";
            case KeyCode::f9: return "F9";
            case KeyCode::f10: return "F10";
            case KeyCode::f11: return "F11";
            case KeyCode::f12: return "F12";
            case KeyCode::grave: return "Grave";
            case KeyCode::num0: return "0";
            case KeyCode::num1: return "1";
            case KeyCode::num2: return "2";
            case KeyCode::num3: return "3";
            case KeyCode::num4: return "4";
            case KeyCode::num5: return "5";
            case KeyCode::num6: return "6";
            case KeyCode::num7: return "7";
            case KeyCode::num8: return "8";
            case KeyCode::num9: return "9";
            case KeyCode::equal: return "Equal";
            case KeyCode::minus: return "Minus";
            case KeyCode::backspace: return "Backspace";
            case KeyCode::a: return "A";
            case KeyCode::b: return "B";
            case KeyCode::c: return "C";
            case KeyCode::d: return "D";
            case KeyCode::e: return "E";
            case KeyCode::f: return "F";
            case KeyCode::g: return "G";
            case KeyCode::h: return "H";
            case KeyCode::i: return "I";
            case KeyCode::j: return "J";
            case KeyCode::k: return "K";
            case KeyCode::l: return "L";
            case KeyCode::m: return "M";
            case KeyCode::n: return "N";
            case KeyCode::o: return "O";
            case KeyCode::p: return "P";
            case KeyCode::q: return "Q";
            case KeyCode::r: return "R";
            case KeyCode::s: return "S";
            case KeyCode::t: return "T";
            case KeyCode::u: return "U";
            case KeyCode::v: return "V";
            case KeyCode::w: return "W";
            case KeyCode::x: return "X";
            case KeyCode::y: return "Y";
            case KeyCode::z: return "Z";
            case KeyCode::tab: return "Tab";
            case KeyCode::caps_lock: return "Caps Lock";
            case KeyCode::enter: return "Enter";
            case KeyCode::ctrl: return "Ctrl";
            case KeyCode::l_ctrl: return "Left Ctrl";
            case KeyCode::r_ctrl: return "Right Ctrl";
            case KeyCode::shift: return "Shift";
            case KeyCode::l_shift: return "Left Shift";
            case KeyCode::r_shift: return "Right Shift";
            case KeyCode::menu: return "Alt";
            case KeyCode::l_menu: return "Left Alt";
            case KeyCode::r_menu: return "Right Alt";
            case KeyCode::system: return "System";
            case KeyCode::l_system: return "Left System";
            case KeyCode::r_system: return "Right System";
            case KeyCode::apps: return "Apps";
            case KeyCode::spacebar: return "Space";
            case KeyCode::l_branket: return "Left Bracket";
            case KeyCode::r_branket: return "Right Bracket";
            case KeyCode::backslash: return "Backslash";
            case KeyCode::semicolon: return "Semicolon";
            case KeyCode::quote: return "Quote";
            case KeyCode::comma: return "Comma";
            case KeyCode::period: return "Period";
            case KeyCode::slash: return "Slash";
            case KeyCode::print_screen: return "Print Screen";
            case KeyCode::scroll_lock: return "Scroll Lock";
            case KeyCode::pause: return "Pause";
            case KeyCode::insert: return "Insert";
            case KeyCode::home: return "Home";
            case KeyCode::page_up: return "Page Up";
            case KeyCode::page_down: return "Page Down";
            case KeyCode::del: return "Delete";
            case KeyCode::end: return "End";
            case KeyCode::left: return "Left";
            case KeyCode::up: return "Up";
            case KeyCode::right: return "Right";
            case KeyCode::down: return "Down";
            case KeyCode::num_lock: return "Num Lock";
            case KeyCode::numpad0: return "Numpad 0";
            case KeyCode::numpad1: return "Numpad 1";
            case KeyCode::numpad2: return "Numpad 2";
            case KeyCode::numpad3: return "Numpad 3";
            case KeyCode::numpad4: return "Numpad 4";
            case KeyCode::numpad5: return "Numpad 5";
            case KeyCode::numpad6: return "Numpad 6";
            case KeyCode::numpad7: return "Numpad 7";
            case KeyCode::numpad8: return "Numpad 8";
            case KeyCode::numpad9: return "Numpad 9";
            case KeyCode::numpad_decimal: return "Numpad Decimal";
            case KeyCode::numpad_add: return "Numpad Add";
            case KeyCode::numpad_subtract: return "Numpad Subtract";
            case KeyCode::numpad_multiply: return "Numpad Multiply";
            case KeyCode::numpad_divide: return "Numpad Divide";
            case KeyCode::numpad_equal: return "Numpad Equal";
            case KeyCode::numpad_enter: return "Numpad Enter";
            default: return nullptr;
            }
        }

        void key_chip(GUI::IContext* context, f32 x, f32 y, const c8* label)
        {
            draw_rect(context, RectF(x, y, 172.0f, 42.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 4.0f);
            draw_text(context, RectF(x + 12.0f, y + 9.0f, 148.0f, 24.0f), label, 16.0f,
                Float4U(1.0f, 1.0f, 1.0f, 1.0f), VG::TextAlignment::center);
        }

        usize previous_utf8_cursor(const String& value, usize cursor)
        {
            if(cursor == USIZE_MAX || cursor > value.size())
            {
                cursor = value.size();
            }
            if(!cursor)
            {
                return 0;
            }
            --cursor;
            while(cursor > 0 && (((u8)value[cursor]) & 0xC0) == 0x80)
            {
                --cursor;
            }
            return cursor;
        }

        void erase_previous_utf8_codepoint(String& value)
        {
            usize end = value.size();
            usize begin = previous_utf8_cursor(value, end);
            if(begin != end)
            {
                value.erase(begin, end - begin);
            }
        }

        void ime_input_element(GUI::IContext* context, SheetState& state)
        {
            GUI::ElementHandle element = context->begin_element(ID_IME_INPUT);
            context->set_layout_config(element, fixed_layout(560.0f, 68.0f));
            set_interactable(context, element, GUI::PointerHitBehavior::target,
                GUI::InteractableFlag::hoverable | GUI::InteractableFlag::activatable |
                GUI::InteractableFlag::focusable);

            Span<const GUI::RoutedInputEvent> events = context->get_routed_input_events(ID_IME_INPUT);
            for(const GUI::RoutedInputEvent& routed : events)
            {
                const GUI::InputEvent& event = routed.event;
                if(event.type == GUI::InputEventType::text_utf8 && !event.text.empty())
                {
                    state.ime_text.append(event.text.c_str(), event.text.size());
                }
                else if(event.type == GUI::InputEventType::key_down && event.key == KeyCode::backspace)
                {
                    erase_previous_utf8_codepoint(state.ime_text);
                }
            }

            GUI::InteractionState interaction = context->get_interaction_state(ID_IME_INPUT);
            bool focused = context->focused_element() == ID_IME_INPUT || interaction.focused;
            Float4U border = focused ? Float4U(0.0f, 0.0f, 0.0f, 1.0f) : Float4U(0.32f, 0.32f, 0.32f, 1.0f);
            Float4U background = interaction.hovered ? Float4U(0.94f, 0.94f, 0.94f, 1.0f) :
                Float4U(0.98f, 0.98f, 0.98f, 1.0f);
            draw_rect(context, RectF(0.0f, 0.0f, 560.0f, 68.0f), background, 4.0f);
            draw_outline(context, RectF(0.0f, 0.0f, 560.0f, 68.0f), border, focused ? 2.5f : 1.25f);
            const c8* text = state.ime_text.empty() ? "Click here, then type with IME..." : state.ime_text.c_str();
            Float4U text_color = state.ime_text.empty() ? Float4U(0.45f, 0.45f, 0.45f, 1.0f) :
                Float4U(0.02f, 0.02f, 0.02f, 1.0f);
            draw_text(context, RectF(18.0f, 20.0f, 524.0f, 28.0f), text, 20.0f, text_color);
            if(focused)
            {
                context->request_text_input(element, (i32)state.ime_text.size());
            }
            context->end_element();
        }

        void narrow_bullet(GUI::IContext* context, f32 x, f32 y, const c8* text)
        {
            draw_rect(context, RectF(x, y + 8.0f, 5.0f, 5.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 2.5f);
            draw_text(context, RectF(x + 16.0f, y, 350.0f, 28.0f), text, 18.0f, Float4U(0.05f, 0.05f, 0.05f, 1.0f));
        }

        bool open_submenu_callback(GUI::IContext*, const GUI::NavigationRequest& request, void* userdata)
        {
            if(request.event_type != GUI::InputEventType::navigation_confirm || !userdata)
            {
                return true;
            }
            SheetState* state = (SheetState*)userdata;
            state->navigation_submenu_open = true;
            state->navigation_pending_focus = ID_NAV_SUBMENU_BASE;
            return true;
        }

        bool close_submenu_callback(GUI::IContext*, const GUI::NavigationRequest& request, void* userdata)
        {
            if(request.event_type != GUI::InputEventType::navigation_back || !userdata)
            {
                return true;
            }
            SheetState* state = (SheetState*)userdata;
            state->navigation_submenu_open = false;
            state->navigation_pending_focus = ID_NAV_GRID_BASE;
            return true;
        }

        void navigation_sample(GUI::IContext* context, GUI::id_t id, const c8* label, SheetState& sheet_state,
            bool submenu_item = false)
        {
            GUI::ElementHandle element = context->begin_element(id);
            context->set_layout_config(element, fixed_layout(submenu_item ? 260.0f : 154.0f, submenu_item ? 74.0f : 82.0f));
            set_interactable(context, element, GUI::PointerHitBehavior::target,
                GUI::InteractableFlag::hoverable | GUI::InteractableFlag::activatable |
                GUI::InteractableFlag::focusable);
            GUI::NavigationConfig navigation;
            if(submenu_item)
            {
                navigation.back = GUI::NavigationMode::callback;
                navigation.callback = close_submenu_callback;
                navigation.userdata = &sheet_state;
            }
            else
            {
                navigation.confirm = GUI::NavigationMode::callback;
                navigation.callback = open_submenu_callback;
                navigation.userdata = &sheet_state;
            }
            context->set_navigation_config(element, navigation);
            GUI::InteractionState interaction = context->get_interaction_state(id);
            Float4U fill = interaction.focused ? Float4U(0.0f, 0.0f, 0.0f, 1.0f) :
                (interaction.hovered ? Float4U(0.84f, 0.84f, 0.84f, 1.0f) : Float4U(0.96f, 0.96f, 0.96f, 1.0f));
            Float4U text = interaction.focused ? Float4U(1.0f, 1.0f, 1.0f, 1.0f) : Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            f32 width = submenu_item ? 260.0f : 154.0f;
            f32 height = submenu_item ? 74.0f : 82.0f;
            draw_rect(context, RectF(0.0f, 0.0f, width, height), fill, 4.0f);
            draw_outline(context, RectF(0.0f, 0.0f, width, height), Float4U(0.0f, 0.0f, 0.0f, 1.0f), interaction.focused ? 3.0f : 1.25f);
            draw_text(context, RectF(10.0f, height * 0.5f - 14.0f, width - 20.0f, 30.0f), label, 22.0f, text, VG::TextAlignment::center);
            context->end_element();
        }

        bool id_in_range(GUI::id_t id, GUI::id_t begin, u32 count)
        {
            return id >= begin && id < begin + count;
        }

        void pointer_label(GUI::IContext* context, f32 x, f32 y, const c8* line1, const c8* line2,
            const c8* line3 = nullptr, const Float4U& color = Float4U(0.0f, 0.0f, 0.0f, 1.0f),
            f32 size = 23.0f)
        {
            draw_text(context, RectF(x, y, 420.0f, 30.0f), line1, size, color);
            draw_text(context, RectF(x, y + 30.0f, 420.0f, 30.0f), line2, size, color);
            if(line3)
            {
                draw_text(context, RectF(x, y + 60.0f, 420.0f, 30.0f), line3, size, color);
            }
        }

        void pointer_demo_element(GUI::IContext* context, GUI::id_t id, f32 width, f32 height,
            const c8* name, const c8* behavior, GUI::PointerHitBehavior hit_behavior, const c8* note = nullptr)
        {
            GUI::ElementHandle element = context->begin_element(id);
            context->set_layout_config(element, fixed_layout(width, height));
            if(hit_behavior != GUI::PointerHitBehavior::none)
            {
                GUI::InteractableFlag flags = hit_behavior == GUI::PointerHitBehavior::block ?
                    GUI::InteractableFlag::none :
                    (GUI::InteractableFlag::hoverable | GUI::InteractableFlag::activatable |
                        GUI::InteractableFlag::focusable);
                set_interactable(context, element, hit_behavior, flags);
            }
            GUI::InteractionState interaction = context->get_interaction_state(id);
            bool highlighted = interaction.hovered;
            Float4U fill = highlighted ? Float4U(0.0f, 0.0f, 0.0f, 1.0f) : Float4U(0.83f, 0.83f, 0.83f, 1.0f);
            Float4U text = highlighted ? Float4U(1.0f, 1.0f, 1.0f, 1.0f) : Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            draw_rect(context, RectF(0.0f, 0.0f, width, height), fill, 0.0f);
            pointer_label(context, 26.0f, note ? 16.0f : 14.0f, name, behavior, note, text, 23.0f);
            context->end_element();
        }
    }

    void build_pointer_input_slice(GUI::IContext* context, SheetState& state)
    {
        GUI::ElementHandle panel = context->begin_element(ID_INPUT);
        context->set_layout_config(panel, fixed_layout(SHEET_WIDTH - 128.0f, 528.0f));
        state.pointer_items.clear();
        add_canvas_item(state.pointer_items, ID_POINTER_BASE_LAYER, 0.0f, 0.0f);
        add_canvas_item(state.pointer_items, ID_POINTER_TOP_LAYER, 520.0f, 48.0f);
        add_canvas_item(state.pointer_items, ID_POINTER_C, 570.0f, 144.0f);
        add_canvas_item(state.pointer_items, ID_POINTER_E, 792.0f, 144.0f);
        add_canvas_item(state.pointer_items, ID_POINTER_D, 570.0f, 238.0f);
        state.pointer_base_items.clear();
        add_canvas_item(state.pointer_base_items, ID_POINTER_A, 62.0f, 104.0f);
        add_canvas_item(state.pointer_base_items, ID_POINTER_B, 320.0f, 104.0f);

        GUI::ElementHandle base = context->begin_element(ID_POINTER_BASE_LAYER);
        context->set_layout_config(base, fixed_layout(1160.0f, 290.0f));
        draw_rect(context, RectF(0.0f, 0.0f, 1160.0f, 290.0f), Float4U(0.05f, 0.92f, 0.48f, 1.0f), 0.0f);
        pointer_label(context, 28.0f, 26.0f, "Bottom Layer Element", "PHB: none");
        pointer_demo_element(context, ID_POINTER_A, 190.0f, 108.0f, "Element A", "PHB: target",
            GUI::PointerHitBehavior::target);
        pointer_demo_element(context, ID_POINTER_B, 808.0f, 214.0f, "Element B", "PHB: target",
            GUI::PointerHitBehavior::target, "Partly covered");
        state.pointer_base_canvas.items = Span<const GUI::CanvasLayoutItem>(state.pointer_base_items.data(), state.pointer_base_items.size());
        state.pointer_base_canvas.default_item = GUI::CanvasLayoutItem();
        state.pointer_base_canvas.clip_children = true;
        set_canvas_layout(context, base, &state.pointer_base_canvas);
        context->end_element();

        GUI::ElementHandle top = context->begin_element(ID_POINTER_TOP_LAYER);
        context->set_layout_config(top, fixed_layout(608.0f, 264.0f));
        draw_rect(context, RectF(0.0f, 0.0f, 608.0f, 264.0f), Float4U(1.0f, 0.52f, 0.52f, 1.0f), 0.0f);
        pointer_label(context, 32.0f, 28.0f, "Top Layer Element", "PHB: none");
        context->end_element();

        pointer_demo_element(context, ID_POINTER_C, 206.0f, 76.0f, "Element C", "PHB: block",
            GUI::PointerHitBehavior::block);
        pointer_demo_element(context, ID_POINTER_E, 326.0f, 76.0f, "Element E", "PHB: pass_through",
            GUI::PointerHitBehavior::pass_through);
        pointer_demo_element(context, ID_POINTER_D, 206.0f, 76.0f, "Element D", "PHB: target",
            GUI::PointerHitBehavior::target);

        GUI::id_t focused = context->focused_element();
        char focused_text[96];
        snprintf(focused_text, sizeof(focused_text), "focused element: %llu", (unsigned long long)focused);
        draw_text(context, RectF(0.0f, 304.0f, 420.0f, 26.0f), focused_text, 17.0f,
            Float4U(0.22f, 0.22f, 0.22f, 1.0f));

        draw_text(context, RectF(0.0f, 340.0f, 520.0f, 28.0f), "PHB (Pointer Hit Behavior):", 22.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_text(context, RectF(0.0f, 380.0f, 1160.0f, 26.0f),
            "none: This element itself does not participate in hit testing.", 17.0f,
            Float4U(0.05f, 0.05f, 0.05f, 1.0f));
        draw_text(context, RectF(0.0f, 414.0f, 1160.0f, 26.0f),
            "target: If hit-testing is passed, this element receives pointer input events and stops hit testing.",
            17.0f, Float4U(0.05f, 0.05f, 0.05f, 1.0f));
        draw_text(context, RectF(0.0f, 448.0f, 1160.0f, 26.0f),
            "block: If hit-testing is passed, this element does not receive pointer input events, but stops hit testing.",
            17.0f, Float4U(0.05f, 0.05f, 0.05f, 1.0f));
        draw_text(context, RectF(0.0f, 482.0f, 1160.0f, 26.0f),
            "pass_through: If hit-testing is passed, this element receives pointer input events and hit testing continues.",
            17.0f, Float4U(0.05f, 0.05f, 0.05f, 1.0f));

        state.pointer_canvas.items = Span<const GUI::CanvasLayoutItem>(state.pointer_items.data(), state.pointer_items.size());
        state.pointer_canvas.default_item = GUI::CanvasLayoutItem();
        state.pointer_canvas.clip_children = false;
        set_canvas_layout(context, panel, &state.pointer_canvas);
        context->end_element();
    }

    void build_keyboard_input_slice(GUI::IContext* context, SheetState& state)
    {
        GUI::ElementHandle panel = context->begin_element(ID_KEYBOARD);
        context->set_layout_config(panel, fixed_layout(SHEET_WIDTH - 128.0f, 500.0f));
        state.keyboard_items.clear();
        add_canvas_item(state.keyboard_items, ID_IME_INPUT, 704.0f, 286.0f);

        draw_text(context, RectF(0.0f, 0.0f, 620.0f, 46.0f), "Keyboard state", 32.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        bullet(context, 2.0f, 68.0f, "Host adapters translate OS key events into GUI input events.");
        bullet(context, 2.0f, 112.0f, "The context stores current key-down state for frame-local queries.");
        bullet(context, 2.0f, 156.0f, "Text input is separate from raw key state, so IME/text composition can remain host-driven.");
        bullet(context, 2.0f, 200.0f, "Keyboard events are delivered to the focused element after pointer routing.");

        draw_line(context, Float2U(0.0f, 250.0f), Float2U(620.0f, 250.0f),
            Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        draw_text(context, RectF(0.0f, 286.0f, 620.0f, 34.0f), "Currently pressed keys", 27.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        u32 pressed_count = 0;
        for(u32 i = (u32)KeyCode::esc; i <= (u32)KeyCode::numpad_enter; ++i)
        {
            KeyCode key = (KeyCode)i;
            if(!context->is_key_down(key))
            {
                continue;
            }
            const c8* name = key_name(key);
            if(!name)
            {
                continue;
            }
            f32 x = (f32)(pressed_count % 3) * 186.0f;
            f32 y = 342.0f + (f32)(pressed_count / 3) * 50.0f;
            key_chip(context, x, y, name);
            ++pressed_count;
        }
        if(!pressed_count)
        {
            draw_text(context, RectF(0.0f, 344.0f, 560.0f, 30.0f), "No key is currently pressed.", 20.0f,
                Float4U(0.32f, 0.32f, 0.32f, 1.0f));
        }

        draw_line(context, Float2U(660.0f, 4.0f), Float2U(660.0f, 486.0f),
            Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        draw_text(context, RectF(704.0f, 0.0f, 520.0f, 46.0f), "IME input sample", 32.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        bullet(context, 706.0f, 68.0f, "Click the field below to focus it.");
        bullet(context, 706.0f, 112.0f, "Committed IME text is appended to the element text.");
        bullet(context, 706.0f, 156.0f, "Backspace removes the previous UTF-8 code point.");
        bullet(context, 706.0f, 200.0f, "This sample intentionally omits cursor, selection and clipboard behavior.");
        ime_input_element(context, state);

        state.keyboard_canvas.items = Span<const GUI::CanvasLayoutItem>(state.keyboard_items.data(), state.keyboard_items.size());
        state.keyboard_canvas.default_item = GUI::CanvasLayoutItem();
        state.keyboard_canvas.clip_children = false;
        set_canvas_layout(context, panel, &state.keyboard_canvas);
        context->end_element();
    }

    void build_navigation_input_slice(GUI::IContext* context, SheetState& state)
    {
        GUI::ElementHandle panel = context->begin_element(ID_NAVIGATION);
        context->set_layout_config(panel, fixed_layout(SHEET_WIDTH - 128.0f, 500.0f));
        draw_text(context, RectF(0.0f, 0.0f, 620.0f, 46.0f), "Navigation demo", 32.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        narrow_bullet(context, 2.0f, 68.0f, "Arrow keys send D-pad navigation.");
        narrow_bullet(context, 2.0f, 112.0f, "Tab and Shift+Tab walk focus order.");
        narrow_bullet(context, 2.0f, 156.0f, "Enter opens a nested menu.");
        narrow_bullet(context, 2.0f, 200.0f, "Esc returns from the nested menu.");

        draw_text(context, RectF(0.0f, 314.0f, 460.0f, 34.0f), "Current mode", 27.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_text(context, RectF(0.0f, 370.0f, 420.0f, 30.0f),
            state.navigation_submenu_open ? "Nested menu: press Esc to return." : "Main grid: press Enter to open nested menu.",
            21.0f, Float4U(0.08f, 0.08f, 0.08f, 1.0f));

        draw_line(context, Float2U(420.0f, 4.0f), Float2U(420.0f, 486.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        draw_text(context, RectF(468.0f, 0.0f, 600.0f, 46.0f), "Canvas-positioned interactive area", 32.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        GUI::id_t focused = context->focused_element();
        char focused_text[96];
        snprintf(focused_text, sizeof(focused_text), "focused element: %llu", (unsigned long long)focused);
        draw_text(context, RectF(470.0f, 56.0f, 420.0f, 28.0f), focused_text, 18.0f,
            Float4U(0.22f, 0.22f, 0.22f, 1.0f));

        state.navigation_items.clear();
        if(state.navigation_submenu_open)
        {
            draw_text(context, RectF(760.0f, 96.0f, 300.0f, 34.0f), "Nested menu", 28.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(760.0f, 392.0f, 330.0f, 26.0f), "Esc closes this menu and restores the grid.", 18.0f,
                Float4U(0.22f, 0.22f, 0.22f, 1.0f));
            const c8* labels[] = { "Menu Action 1", "Menu Action 2", "Menu Action 3" };
            for(u32 i = 0; i < 3; ++i)
            {
                GUI::id_t id = ID_NAV_SUBMENU_BASE + i;
                add_canvas_item(state.navigation_items, id, 760.0f, 152.0f + (f32)i * 92.0f);
                navigation_sample(context, id, labels[i], state, true);
            }
            if(!id_in_range(context->focused_element(), ID_NAV_SUBMENU_BASE, 3))
            {
                state.navigation_pending_focus = ID_NAV_SUBMENU_BASE;
            }
        }
        else
        {
            draw_text(context, RectF(608.0f, 96.0f, 260.0f, 34.0f), "Main grid", 28.0f,
                Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            draw_text(context, RectF(500.0f, 450.0f, 560.0f, 26.0f), "D-pad follows position; Tab follows row-major creation order.", 18.0f,
                Float4U(0.22f, 0.22f, 0.22f, 1.0f));
            for(u32 row = 0; row < 3; ++row)
            {
                for(u32 column = 0; column < 3; ++column)
                {
                    u32 index = row * 3 + column;
                    GUI::id_t id = ID_NAV_GRID_BASE + index;
                    char label[32];
                    snprintf(label, sizeof(label), "Cell %u", index + 1);
                    add_canvas_item(state.navigation_items, id, 500.0f + (f32)column * 180.0f, 144.0f + (f32)row * 104.0f);
                    navigation_sample(context, id, label, state);
                }
            }
            if(!id_in_range(context->focused_element(), ID_NAV_GRID_BASE, 9))
            {
                state.navigation_pending_focus = ID_NAV_GRID_BASE;
            }
        }
        if(state.navigation_pending_focus)
        {
            context->focus_element(state.navigation_pending_focus);
            state.navigation_pending_focus = 0;
        }
        state.navigation_canvas.items = Span<const GUI::CanvasLayoutItem>(state.navigation_items.data(), state.navigation_items.size());
        state.navigation_canvas.default_item = GUI::CanvasLayoutItem();
        state.navigation_canvas.clip_children = false;
        set_canvas_layout(context, panel, &state.navigation_canvas);
        context->end_element();
    }
}
