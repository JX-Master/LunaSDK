/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorInputText.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>
#include <Luna/Runtime/Unicode.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
            bool enabled = true, bool readonly = false)
        {
            GUICore::Interactable interactable;
            set_flags(interactable.flags, GUICore::InteractableFlag::hit_test);
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            set_flags(interactable.flags, GUICore::InteractableFlag::disabled, !enabled);
            set_flags(interactable.flags, GUICore::InteractableFlag::read_only, readonly);
            context->set_interactable(element, interactable);
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_relative_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
            const Float4U& color, f32 width, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.rect_layout_scale = scale;
            command.color = color;
            command.line_width = width;
            context->draw(command);
        }

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = VG::TextAlignment::begin;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static Ref<InputEditState> input_edit_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<InputEditState>(id);
            Ref<InputEditState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<InputEditState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static bool has_modifier(GUICore::KeyModifierFlag flags, GUICore::KeyModifierFlag flag)
        {
            return (((u8)flags) & ((u8)flag)) != 0;
        }

        static String filter_input_text(const String& text)
        {
            String filtered;
            usize offset = 0;
            const c8* src = text.c_str();
            while(offset < text.size() && src[offset])
            {
                usize len = min(utf8_charlen(src + offset), text.size() - offset);
                c32 ch = utf8_decode_char(src + offset);
                if(ch >= 0x20 && ch != 0x7F)
                {
                    filtered.append(src + offset, len);
                }
                offset += len;
            }
            return filtered;
        }

        static bool delete_input_text_selection(String& value, InputEditState& state)
        {
            if(!input_text_has_selection(value, state))
            {
                return false;
            }
            usize begin = 0;
            usize end = 0;
            input_text_selection_range(value, state, begin, end);
            value.erase(begin, end - begin);
            state.text_cursor = begin;
            input_text_clear_selection(state);
            return true;
        }

        static usize input_cursor_from_x(const String& value, f32 x, f32 font_size)
        {
            if(x <= 0.0f)
            {
                return 0;
            }
            f32 advance = max(font_size * 0.5f, 1.0f);
            usize byte_cursor = min((usize)((x + advance * 0.5f) / advance), value.size());
            return clamp_utf8_cursor(value, byte_cursor);
        }

        static f32 input_cursor_x(const String& value, usize cursor, f32 font_size)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            f32 advance = max(font_size * 0.5f, 1.0f);
            return (f32)cursor * advance;
        }

        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, GUICore::id_t id, String& value,
            const GUICore::LayoutInput& layout, bool enabled, bool readonly)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("input_text"));
            context->set_layout(element, layout);
            set_basic_interactable(context, element, enabled, readonly);

            Ref<InputEditState> state = input_edit_state(context, id);
            state->text_cursor = clamp_utf8_cursor(value, state->text_cursor);

            GUICore::InteractionState interaction = context->get_interaction_state(id);
            bool focused = enabled && (context->focused_element() == id || interaction.focused);
            f32 padding_x = style_value(context, Name("gui.editor.input_text.padding_x"), GUICore::style_f32(8.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.input_text.font_size"), GUICore::style_f32(16.0f)).number.x;
            if(interaction.clicked)
            {
                state->text_cursor = input_cursor_from_x(value, interaction.clicked_element_position.x - padding_x, font_size);
                state->text_cursor_blink_start = 0.0;
                input_text_clear_selection(*state);
            }
            if(enabled)
            {
                Span<const GUICore::RoutedInputEvent> events = context->get_routed_input_events(id);
                for(const GUICore::RoutedInputEvent& routed : events)
                {
                    const GUICore::InputEvent& event = routed.event;
                    if(event.type == GUICore::InputEventType::text_utf8 && !event.text.empty() && !readonly)
                    {
                        delete_input_text_selection(value, *state);
                        value.insert(state->text_cursor, event.text);
                        state->text_cursor = clamp_utf8_cursor(value, state->text_cursor + event.text.size());
                        input_text_clear_selection(*state);
                        state->text_cursor_blink_start = 0.0;
                    }
                    else if(event.type == GUICore::InputEventType::key_down)
                    {
                        bool shortcut = has_modifier(event.modifiers, GUICore::KeyModifierFlag::ctrl) ||
                            has_modifier(event.modifiers, GUICore::KeyModifierFlag::system);
                        if(shortcut && event.key == KeyCode::c)
                        {
                            GUICore::ClipboardIO clipboard = context->get_clipboard_io();
                            if(clipboard.set_text && input_text_has_selection(value, *state))
                            {
                                usize begin = 0;
                                usize end = 0;
                                input_text_selection_range(value, *state, begin, end);
                                String selected = value.substr(begin, end - begin);
                                (void)clipboard.set_text(selected.c_str(), selected.size(), clipboard.userdata);
                            }
                        }
                        else if(shortcut && event.key == KeyCode::v && !readonly)
                        {
                            GUICore::ClipboardIO clipboard = context->get_clipboard_io();
                            if(clipboard.get_text)
                            {
                                String clipboard_text;
                                if(succeeded(clipboard.get_text(clipboard_text, clipboard.userdata)))
                                {
                                    String filtered = filter_input_text(clipboard_text);
                                    if(!filtered.empty() || input_text_has_selection(value, *state))
                                    {
                                        delete_input_text_selection(value, *state);
                                        value.insert(state->text_cursor, filtered);
                                        state->text_cursor = clamp_utf8_cursor(value, state->text_cursor + filtered.size());
                                        input_text_clear_selection(*state);
                                        state->text_cursor_blink_start = 0.0;
                                    }
                                }
                            }
                        }
                        else if(event.key == KeyCode::backspace && !readonly)
                        {
                            if(!delete_input_text_selection(value, *state))
                            {
                                erase_previous_utf8_codepoint(value, state->text_cursor);
                            }
                            input_text_clear_selection(*state);
                            state->text_cursor_blink_start = 0.0;
                        }
                        else if(event.key == KeyCode::del && !readonly)
                        {
                            if(!delete_input_text_selection(value, *state))
                            {
                                erase_utf8_codepoint_at(value, state->text_cursor);
                            }
                            input_text_clear_selection(*state);
                            state->text_cursor_blink_start = 0.0;
                        }
                        else if(event.key == KeyCode::left)
                        {
                            state->text_cursor = previous_utf8_cursor(value, state->text_cursor);
                            input_text_clear_selection(*state);
                            state->text_cursor_blink_start = 0.0;
                        }
                        else if(event.key == KeyCode::right)
                        {
                            state->text_cursor = next_utf8_cursor(value, state->text_cursor);
                            input_text_clear_selection(*state);
                            state->text_cursor_blink_start = 0.0;
                        }
                        else if(event.key == KeyCode::enter || event.key == KeyCode::esc)
                        {
                            context->focus_element(0);
                            focused = false;
                            input_text_clear_selection(*state);
                        }
                    }
                }
            }

            Float4U background = style_value(context, focused ? Name("gui.editor.input_text.background_focused") :
                Name("gui.editor.input_text.background"), focused ?
                GUICore::style_f32x4(Float4U(0.12f, 0.16f, 0.22f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f))).number;
            Float4U border = style_value(context, focused ? Name("gui.editor.input_text.border_focused") :
                Name("gui.editor.input_text.border"), focused ?
                GUICore::style_f32x4(Float4U(0.16f, 0.55f, 0.86f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.18f, 0.24f, 0.32f, 1.0f))).number;
            Float4U text_color = style_value(context, Name("gui.editor.input_text.text_color"),
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f))).number;
            Float4U cursor_color = style_value(context, Name("gui.editor.input_text.cursor"),
                GUICore::style_f32x4(Float4U(0.75f, 0.88f, 1.0f, 1.0f))).number;
            f32 radius = style_value(context, Name("gui.editor.input_text.radius"), GUICore::style_f32(4.0f)).number.x;
            f32 border_size = style_value(context, Name("gui.editor.input_text.border_size"), GUICore::style_f32(1.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f), border, radius);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(border_size, border_size, -border_size * 2.0f, -border_size * 2.0f),
                background, max(radius - border_size, 0.0f));
            draw_relative_text(context, RectF(padding_x, 0.0f, -padding_x * 2.0f, 0.0f), value.c_str(),
                text_color, font_size);
            if(focused && !readonly)
            {
                context->request_text_input(element, (i32)state->text_cursor);
                f32 cursor_x = padding_x + input_cursor_x(value, state->text_cursor, font_size);
                draw_relative_line(context, Float2U(cursor_x, 5.0f), Float2U(cursor_x, -5.0f),
                    cursor_color, 1.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            }
            context->end_element();
            return element;
        }
    }
}
