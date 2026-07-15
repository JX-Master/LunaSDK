/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InputText.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/Runtime/Unicode.hpp>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct TextInputData
            {
                String* value = nullptr;
                c8* placeholder = nullptr;
                bool enabled = true;
                bool read_only = false;
                GUICore::FontDesc font;
                TextInputState* state = nullptr;
            };

            struct TextInputMetrics
            {
                VG::TextArrangeResult arrangement;

                TextInputMetrics(const String& value, const GUICore::FontDesc& font, f32 font_size)
                {
                    if(value.empty() || !font.font || font_size <= 0.0f) return;
                    VG::TextArrangeSection section;
                    section.font_file = font.font;
                    section.font_index = font.font_index;
                    section.font_size = font_size;
                    section.num_chars = value.size();
                    arrangement = VG::arrange_text(value.c_str(), value.size(),
                        Span<const VG::TextArrangeSection>(&section, 1), RectF(0.0f, 0.0f, F32_MAX, F32_MAX),
                        VG::TextAlignment::begin, VG::TextAlignment::begin);
                }
            };

            static usize clamp_cursor(const String& value, usize cursor)
            {
                cursor = min(cursor, value.size());
                while(cursor > 0 && cursor < value.size() && (((u8)value[cursor]) & 0xC0) == 0x80) --cursor;
                return cursor;
            }

            static usize previous_cursor(const String& value, usize cursor)
            {
                cursor = clamp_cursor(value, cursor);
                if(!cursor) return 0;
                --cursor;
                while(cursor > 0 && (((u8)value[cursor]) & 0xC0) == 0x80) --cursor;
                return cursor;
            }

            static usize next_cursor(const String& value, usize cursor)
            {
                cursor = clamp_cursor(value, cursor);
                if(cursor >= value.size()) return value.size();
                return min(cursor + utf8_charlen(value.c_str() + cursor), value.size());
            }

            static GUICore::FontDesc resolve_font(GUICore::IContext* context,
                const GUICore::ElementHandle& element)
            {
                GUICore::FontDesc font = context->get_font(style_name(context, element, "gui.font"));
                if(!font.font)
                {
                    font.font = Font::get_default_font();
                    font.font_index = 0;
                }
                return font;
            }

            static f32 cursor_x(const String& value, usize cursor, const TextInputMetrics& metrics)
            {
                cursor = clamp_cursor(value, cursor);
                if(!cursor || metrics.arrangement.lines.empty()) return 0.0f;
                const VG::TextLineArrangeResult& line = metrics.arrangement.lines[0];
                if(cursor >= value.size()) return line.bounding_rect.width;
                for(const VG::TextGlyphArrangeResult& glyph : line.glyphs)
                {
                    if(glyph.index == cursor) return glyph.origin_offset;
                }
                return line.bounding_rect.width;
            }

            static usize cursor_from_x(const String& value, f32 x, const TextInputMetrics& metrics)
            {
                if(x <= 0.0f || metrics.arrangement.lines.empty()) return 0;
                const VG::TextLineArrangeResult& line = metrics.arrangement.lines[0];
                for(usize i = 0; i < line.glyphs.size(); ++i)
                {
                    const VG::TextGlyphArrangeResult& glyph = line.glyphs[i];
                    f32 next_x = i + 1 < line.glyphs.size() ? line.glyphs[i + 1].origin_offset :
                        line.bounding_rect.width;
                    if(x < (glyph.origin_offset + next_x) * 0.5f) return glyph.index;
                }
                return value.size();
            }

            static void selection_range(const String& value, const TextInputState& state, usize& begin, usize& end)
            {
                usize cursor = clamp_cursor(value, state.cursor);
                usize anchor = state.selection_anchor == USIZE_MAX ? cursor : clamp_cursor(value, state.selection_anchor);
                begin = min(cursor, anchor);
                end = max(cursor, anchor);
            }

            static bool delete_selection(String& value, TextInputState& state)
            {
                usize begin = 0;
                usize end = 0;
                selection_range(value, state, begin, end);
                if(begin == end) return false;
                value.erase(begin, end - begin);
                state.cursor = begin;
                state.selection_anchor = USIZE_MAX;
                return true;
            }

            static bool has_modifier(GUICore::KeyModifierFlag value, GUICore::KeyModifierFlag flag)
            {
                return test_flags(value, flag);
            }

            static String filter_single_line_text(const String& value)
            {
                String result;
                usize offset = 0;
                while(offset < value.size())
                {
                    usize length = min(utf8_charlen(value.c_str() + offset), value.size() - offset);
                    c32 character = utf8_decode_char(value.c_str() + offset);
                    if(character >= 0x20 && character != 0x7F)
                    {
                        result.append(value.c_str() + offset, length);
                    }
                    offset += length;
                }
                return result;
            }

            static GUICore::MeasureResult measure_input_text(GUICore::IContext*, const GUICore::ElementHandle&,
                const Float2U&, void*)
            {
                GUICore::MeasureResult result;
                result.minimum = Float2U(48.0f, 28.0f);
                result.desired = Float2U(160.0f, 30.0f);
                return result;
            }

            static RV draw_input_text(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                TextInputData* data = (TextInputData*)userdata;
                if(!data || !data->value || !data->state) return ok;
                bool focused = context->focused_element() == element.id;
                f32 radius = style_scalar(context, element, "gui.input.radius", 4.0f);
                f32 padding = style_scalar(context, element, "gui.input.padding_x", 8.0f);
                f32 font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                TextInputMetrics metrics(*data->value, data->font, font_size);
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.color = style_color(context, element, focused ? "gui.input.border_focused" : "gui.input.border",
                    focused ? Float4U(0.12f, 0.55f, 0.86f, 1.0f) : Float4U(0.20f, 0.27f, 0.36f, 1.0f));
                command.radius = radius;
                context->draw(command);
                command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                command.color = style_color(context, element, focused ? "gui.input.background_focused" :
                    "gui.input.background", focused ? Float4U(0.11f, 0.15f, 0.21f, 1.0f) :
                    Float4U(0.08f, 0.10f, 0.13f, 1.0f));
                command.radius = max(radius - 1.0f, 0.0f);
                context->draw(command);

                usize selection_begin = 0;
                usize selection_end = 0;
                selection_range(*data->value, *data->state, selection_begin, selection_end);
                if(focused && selection_begin != selection_end)
                {
                    f32 begin_x = padding + cursor_x(*data->value, selection_begin, metrics) - data->state->scroll_x;
                    f32 end_x = padding + cursor_x(*data->value, selection_end, metrics) - data->state->scroll_x;
                    command.type = GUICore::DrawCommandType::rect;
                    command.rect = RectF(begin_x, 4.0f, end_x - begin_x, -8.0f);
                    command.color = style_color(context, element, "gui.input.selection",
                        Float4U(0.16f, 0.42f, 0.70f, 0.75f));
                    context->draw(command);
                }

                command = GUICore::DrawCommand();
                command.type = GUICore::DrawCommandType::text;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(padding - data->state->scroll_x, 0.0f, -padding + data->state->scroll_x, 0.0f);
                command.text = data->value->empty() && data->placeholder ? data->placeholder : data->value->c_str();
                command.font = style_name(context, element, "gui.font");
                command.font_size = font_size;
                command.color = data->enabled ? style_color(context, element, "gui.text.color", Float4U(0.86f, 0.88f, 0.92f, 1.0f)) :
                    style_color(context, element, "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                if(data->value->empty() && data->placeholder) command.color.w *= 0.55f;
                command.vertical_alignment = VG::TextAlignment::center;
                context->draw(command);

                if(focused && !data->read_only && fmod(data->state->blink_time, 1.0f) < 0.55f)
                {
                    f32 x = padding + cursor_x(*data->value, data->state->cursor, metrics) - data->state->scroll_x;
                    command = GUICore::DrawCommand();
                    command.type = GUICore::DrawCommandType::line;
                    command.rect_reference = GUICore::DrawCommandRectReference::element;
                    command.rect = RectF(x, 5.0f, 0.0f, 0.0f);
                    command.point1 = Float2U(x, -5.0f);
                    command.rect_layout_scale = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
                    command.color = style_color(context, element, "gui.input.cursor", Float4U(0.80f, 0.92f, 1.0f, 1.0f));
                    context->draw(command);
                }
                return ok;
            }

            bool resolve_input_text_action(GUICore::IContext* context, TextInputAction& action)
            {
                if(!action.value || !action.state) return false;
                String& value = *action.value;
                TextInputState& state = *action.state;
                state.cursor = clamp_cursor(value, state.cursor);
                bool changed = false;
                bool focused = context->focused_element() == action.id;
                for(const GUICore::RoutedInputEvent& routed : context->get_routed_input_events(action.id))
                {
                    const GUICore::InputEvent& event = routed.event;
                    if(event.type == GUICore::InputEventType::pointer_down && event.button == GUICore::PointerButton::left &&
                        routed.has_element_position)
                    {
                        TextInputMetrics metrics(value, action.font, action.font_size);
                        usize cursor = cursor_from_x(value, routed.element_position.x - action.padding_x + state.scroll_x,
                            metrics);
                        if(!has_modifier(event.modifiers, GUICore::KeyModifierFlag::shift)) state.selection_anchor = cursor;
                        state.cursor = cursor;
                        state.selecting = true;
                        state.blink_time = 0.0f;
                    }
                    else if(event.type == GUICore::InputEventType::pointer_move && state.selecting && routed.has_element_position)
                    {
                        TextInputMetrics metrics(value, action.font, action.font_size);
                        state.cursor = cursor_from_x(value, routed.element_position.x - action.padding_x + state.scroll_x,
                            metrics);
                        state.blink_time = 0.0f;
                    }
                    else if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::left)
                    {
                        state.selecting = false;
                        if(state.selection_anchor == state.cursor) state.selection_anchor = USIZE_MAX;
                    }
                    else if(focused && event.type == GUICore::InputEventType::text_utf8 && action.enabled &&
                        !action.read_only && !event.text.empty())
                    {
                        delete_selection(value, state);
                        String filtered = filter_single_line_text(event.text);
                        value.insert(state.cursor, filtered);
                        state.cursor = clamp_cursor(value, state.cursor + filtered.size());
                        state.selection_anchor = USIZE_MAX;
                        state.blink_time = 0.0f;
                        changed = true;
                    }
                    else if(focused && event.type == GUICore::InputEventType::key_down)
                    {
                        bool shortcut = has_modifier(event.modifiers, GUICore::KeyModifierFlag::ctrl) ||
                            has_modifier(event.modifiers, GUICore::KeyModifierFlag::system);
                        if(shortcut && event.key == KeyCode::a)
                        {
                            state.selection_anchor = 0;
                            state.cursor = value.size();
                        }
                        else if(shortcut && event.key == KeyCode::c)
                        {
                            usize begin = 0, end = 0;
                            selection_range(value, state, begin, end);
                            GUICore::ClipboardIO clipboard = context->get_clipboard_io();
                            if(begin != end && clipboard.set_text)
                                (void)clipboard.set_text(value.c_str() + begin, end - begin, clipboard.userdata);
                        }
                        else if(shortcut && event.key == KeyCode::x && !action.read_only)
                        {
                            usize begin = 0, end = 0;
                            selection_range(value, state, begin, end);
                            GUICore::ClipboardIO clipboard = context->get_clipboard_io();
                            if(begin != end && clipboard.set_text)
                                (void)clipboard.set_text(value.c_str() + begin, end - begin, clipboard.userdata);
                            changed |= delete_selection(value, state);
                        }
                        else if(shortcut && event.key == KeyCode::v && !action.read_only)
                        {
                            GUICore::ClipboardIO clipboard = context->get_clipboard_io();
                            String text;
                            if(clipboard.get_text && succeeded(clipboard.get_text(text, clipboard.userdata)))
                            {
                                delete_selection(value, state);
                                text = filter_single_line_text(text);
                                value.insert(state.cursor, text);
                                state.cursor += text.size();
                                state.selection_anchor = USIZE_MAX;
                                changed = true;
                            }
                        }
                        else if(event.key == KeyCode::backspace && !action.read_only)
                        {
                            if(!delete_selection(value, state))
                            {
                                usize begin = previous_cursor(value, state.cursor);
                                if(begin != state.cursor)
                                {
                                    value.erase(begin, state.cursor - begin);
                                    state.cursor = begin;
                                    changed = true;
                                }
                            }
                            else changed = true;
                        }
                        else if(event.key == KeyCode::del && !action.read_only)
                        {
                            if(!delete_selection(value, state))
                            {
                                usize end = next_cursor(value, state.cursor);
                                if(end != state.cursor)
                                {
                                    value.erase(state.cursor, end - state.cursor);
                                    changed = true;
                                }
                            }
                            else changed = true;
                        }
                        else if(event.key == KeyCode::left || event.key == KeyCode::right)
                        {
                            if(!has_modifier(event.modifiers, GUICore::KeyModifierFlag::shift)) state.selection_anchor = USIZE_MAX;
                            else if(state.selection_anchor == USIZE_MAX) state.selection_anchor = state.cursor;
                            state.cursor = event.key == KeyCode::left ? previous_cursor(value, state.cursor) :
                                next_cursor(value, state.cursor);
                        }
                        else if(event.key == KeyCode::enter || event.key == KeyCode::esc)
                        {
                            context->focus_element(0);
                            state.selecting = false;
                        }
                        state.blink_time = 0.0f;
                    }
                }
                if(focused)
                {
                    context->request_text_input(context->find_element_handle(action.id), (i32)state.cursor);
                    state.blink_time += max(context->get_frame_desc().delta_time, 0.0f);
                    const GUICore::Element* element = context->find_element(action.id);
                    f32 available = element ? max(element->layout_result.rect.width - action.padding_x * 2.0f, 1.0f) : 1.0f;
                    TextInputMetrics updated_metrics(value, action.font, action.font_size);
                    f32 x = cursor_x(value, state.cursor, updated_metrics);
                    if(x - state.scroll_x > available) state.scroll_x = x - available;
                    if(x < state.scroll_x) state.scroll_x = x;
                }
                else
                {
                    state.selecting = false;
                    state.scroll_x = 0.0f;
                }
                return changed;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle input_text(GUICore::IContext* context, id_t id, String& value,
            const GUICore::LayoutConfig& layout, const TextInputDesc& desc)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, "Text Input", layout);
            Internal::set_interactable(context, element, desc.enabled, desc.read_only);
            Ref<Internal::TextInputState> state = Internal::widget_state<Internal::TextInputState>(context, id);
            Internal::TextInputData* data = Internal::allocate_frame<Internal::TextInputData>(context);
            data->value = &value;
            data->placeholder = desc.placeholder ? Internal::copy_frame_string(context, desc.placeholder) : nullptr;
            data->enabled = desc.enabled;
            data->read_only = desc.read_only;
            data->font = Internal::resolve_font(context, element);
            data->state = state.get();
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.input_text");
            callbacks.measure_callback = Internal::measure_input_text;
            context->set_layout_callback_config(element, callbacks);
            GUICore::DrawConfig draw;
            draw.name = Name("gui.input_text");
            draw.callback = Internal::draw_input_text;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            Internal::TextInputAction* action = Internal::allocate_frame<Internal::TextInputAction>(context);
            action->id = id;
            action->value = &value;
            action->enabled = desc.enabled;
            action->read_only = desc.read_only;
            action->font = data->font;
            action->font_size = Internal::style_scalar(context, element, "gui.text.font_size", 16.0f);
            action->padding_x = Internal::style_scalar(context, element, "gui.input.padding_x", 8.0f);
            action->state = state.get();
            Internal::add_action(context, Internal::ActionType::input_text, id, action);
            return element;
        }
    }
}
