/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorColorEdit.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorViews.hpp>
#include <Luna/GUI/EditorWidgets.hpp>
#include <Luna/Runtime/StringUtils.hpp>

namespace Luna
{
    namespace GUI
    {
        static u64 hash_u64_local(u64 value, u64 h = 14695981039346656037ull)
        {
            const byte_t* p = (const byte_t*)&value;
            for(usize i = 0; i < sizeof(value); ++i)
            {
                h ^= (u64)p[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        static u64 hash_cstr_local(const c8* str, u64 h)
        {
            if(str)
            {
                while(*str)
                {
                    h ^= (u64)(byte_t)*str;
                    h *= 1099511628211ull;
                    ++str;
                }
            }
            return h;
        }

        static void assign_color_binding(ColorBinding& binding, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count)
        {
            binding.f32_value = f32_value;
            binding.u8_value = u8_value;
            binding.u32_value = u32_value;
            binding.type = type;
            binding.value_count = count;
        }

        static GUICore::LayoutInput fixed_layout(f32 width, f32 height)
        {
            GUICore::LayoutInput layout;
            if(width > 0.0f)
            {
                layout.width.kind = GUICore::SizeKind::pixels;
                layout.width.value = width;
            }
            if(height > 0.0f)
            {
                layout.height.kind = GUICore::SizeKind::pixels;
                layout.height.value = height;
            }
            return layout;
        }

        static void sync_color_picker_build_state(ColorPickerState& state, const Float4U& color)
        {
            ensure_color_picker_state_channels(state);
            state.color_picker_rgb[0] = (i32)color_channel_to_u8(color.x);
            state.color_picker_rgb[1] = (i32)color_channel_to_u8(color.y);
            state.color_picker_rgb[2] = (i32)color_channel_to_u8(color.z);
            state.color_picker_rgb[3] = (i32)color_channel_to_u8(color.w);
            f32 h = 0.0f;
            f32 s = 0.0f;
            f32 v = 0.0f;
            color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
            state.color_picker_hsv[0] = (i32)color_channel_to_u8(h);
            state.color_picker_hsv[1] = (i32)color_channel_to_u8(s);
            state.color_picker_hsv[2] = (i32)color_channel_to_u8(v);
        }

        static String color_edit_hex_text(const ColorBinding& binding)
        {
            Float4U color = read_color_value(binding);
            String hex;
            if(color_value_count(binding) > 3)
            {
                strprintf(hex, "#%02X%02X%02X%02X", color_channel_to_u8(color.x), color_channel_to_u8(color.y), color_channel_to_u8(color.z), color_channel_to_u8(color.w));
            }
            else
            {
                strprintf(hex, "#%02X%02X%02X", color_channel_to_u8(color.x), color_channel_to_u8(color.y), color_channel_to_u8(color.z));
            }
            return hex;
        }
        static GUICore::id_t core_derived_id(GUICore::id_t id, const c8* salt)
        {
            return hash_cstr_local(salt, hash_u64_local(id));
        }

        static GUICore::StyleValue core_style_value(GUICore::IContext* context, const Name& entry, const GUICore::StyleValue& default_value)
        {
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        template <typename _Ty>
        static Ref<_Ty> core_state(GUICore::IContext* context, GUICore::id_t owner_id, GUICore::StateLifetime lifetime)
        {
            id_t state_id = GUICore::make_state_id<_Ty>(owner_id);
            Ref<_Ty> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<_Ty>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), lifetime));
            return state;
        }

        static void core_set_layout_result(GUICore::IContext* context, const GUICore::ElementHandle& element, const RectF& rect)
        {
            GUICore::LayoutResult result;
            result.rect = rect;
            result.clip_rect = rect;
            result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(element, result);
        }

        static void core_draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
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

        static void core_draw_relative_gradient_rect(GUICore::IContext* context, const RectF& rect, const Float4U& top_left,
            const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::gradient_rect;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = top_left;
            command.color_top_right = top_right;
            command.color_bottom_right = bottom_right;
            command.color_bottom_left = bottom_left;
            context->draw(command);
        }

        static void core_draw_relative_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
            const Float4U& color, f32 width)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.color = color;
            command.line_width = width;
            context->draw(command);
        }

        static void core_draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static void core_set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            GUICore::Interactable interactable;
            interactable.hit_test = true;
            interactable.hoverable = true;
            interactable.activatable = true;
            interactable.focusable = true;
            context->set_interactable(element, interactable);
        }

        static bool core_rect_contains(const RectF& rect, const Float2U& p)
        {
            return p.x >= rect.offset_x && p.y >= rect.offset_y &&
                p.x <= rect.offset_x + rect.width && p.y <= rect.offset_y + rect.height;
        }

        static void core_draw_color_picker_swatch(GUICore::IContext* context, const RectF& rect, const Float4U& color)
        {
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, rect, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 3.0f);
            RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f), max(rect.height - 2.0f, 1.0f));
            f32 cell = 8.0f;
            u32 columns = max((u32)((inner.width + cell - 1.0f) / cell), 1u);
            u32 rows = max((u32)((inner.height + cell - 1.0f) / cell), 1u);
            for(u32 y = 0; y < rows; ++y)
            {
                for(u32 x = 0; x < columns; ++x)
                {
                    Float4U checker = ((x + y) & 1) ? Float4U(0.42f, 0.46f, 0.52f, 1.0f) : Float4U(0.20f, 0.23f, 0.28f, 1.0f);
                    RectF cell_rect(inner.offset_x + (f32)x * cell, inner.offset_y + (f32)y * cell,
                        min(cell, max(inner.offset_x + inner.width - (inner.offset_x + (f32)x * cell), 0.0f)),
                        min(cell, max(inner.offset_y + inner.height - (inner.offset_y + (f32)y * cell), 0.0f)));
                    core_draw_relative_rect(context, GUICore::DrawCommandType::rect, cell_rect, checker);
                }
            }
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, inner, color, 2.0f);
        }

        static void core_apply_picker_position(ColorBinding& binding, ColorPickerState& state, i32 axis, u32 part, const Float2U& position,
            const RectF& square, const RectF& bar)
        {
            Float4U color = read_color_value(binding);
            f32 x = 0.0f;
            f32 y = 0.0f;
            f32 picker_bar = 0.0f;
            color_picker_channels_from_color(axis, color, x, y, picker_bar);
            if(part == 1)
            {
                x = clamp((position.x - square.offset_x) / max(square.width, 1.0f), 0.0f, 1.0f);
                y = clamp(1.0f - (position.y - square.offset_y) / max(square.height, 1.0f), 0.0f, 1.0f);
            }
            else if(part == 2)
            {
                f32 t = clamp((position.y - bar.offset_y) / max(bar.height, 1.0f), 0.0f, 1.0f);
                picker_bar = axis == 0 ? t : 1.0f - t;
            }
            write_color_value(binding, color_from_picker_channels(axis, x, y, picker_bar, color.w));
            sync_color_picker_build_state(state, read_color_value(binding));
        }

        static void core_draw_color_picker(GUICore::IContext* context, GUICore::id_t id, ColorBinding& binding,
            ColorPickerState& state, u8 count)
        {
            Float4U color = read_color_value(binding);
            i32 axis = clamp(color_picker_axis_ref(state), 0, 5);
            f32 picker_x = 0.0f;
            f32 picker_y = 0.0f;
            f32 picker_bar = 0.0f;
            color_picker_channels_from_color(axis, color, picker_x, picker_y, picker_bar);

            RectF local(0.0f, 0.0f, 456.0f, 300.0f);
            RectF square = color_picker_square_rect(local);
            RectF bar = color_picker_bar_rect(local);
            RectF current_rect = color_picker_current_rect(local);
            RectF original_rect = color_picker_original_rect(local);

            GUICore::ElementHandle picker = context->begin_element(id, Name("color_picker"));
            core_set_layout_result(context, picker, RectF(10.0f, 10.0f, local.width, local.height));
            core_set_basic_interactable(context, picker);
            Ref<ColorPickerInteractionState> interaction_state = core_state<ColorPickerInteractionState>(context, id, GUICore::StateLifetime::next_frame);
            Span<const GUICore::RoutedInputEvent> events = context->get_routed_input_events(id);
            for(const GUICore::RoutedInputEvent& routed : events)
            {
                const GUICore::InputEvent& event = routed.event;
                if(event.type == GUICore::InputEventType::pointer_down && event.button == GUICore::PointerButton::left)
                {
                    if(core_rect_contains(square, routed.element_position))
                    {
                        interaction_state->active_color_part = 1;
                        core_apply_picker_position(binding, state, axis, interaction_state->active_color_part, routed.element_position, square, bar);
                    }
                    else if(core_rect_contains(bar, routed.element_position))
                    {
                        interaction_state->active_color_part = 2;
                        core_apply_picker_position(binding, state, axis, interaction_state->active_color_part, routed.element_position, square, bar);
                    }
                    else if(core_rect_contains(original_rect, routed.element_position) && state.color_picker_original_valid)
                    {
                        write_color_value(binding, state.color_picker_original);
                        sync_color_picker_build_state(state, read_color_value(binding));
                    }
                }
                else if(event.type == GUICore::InputEventType::pointer_move && interaction_state->active_color_part)
                {
                    core_apply_picker_position(binding, state, axis, interaction_state->active_color_part, routed.element_position, square, bar);
                }
                else if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::left)
                {
                    interaction_state->active_color_part = 0;
                }
            }

            color = read_color_value(binding);
            color_picker_channels_from_color(axis, color, picker_x, picker_y, picker_bar);
            if(axis == 0)
            {
                Float4U hue_color = color_hsv_to_rgb(picker_bar, 1.0f, 1.0f, 1.0f);
                core_draw_relative_gradient_rect(context, square, Float4U(1.0f), hue_color, hue_color, Float4U(1.0f));
                core_draw_relative_gradient_rect(context, square, Float4U(0.0f), Float4U(0.0f),
                    Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            }
            else if(axis == 1 || axis == 2)
            {
                for(u32 i = 0; i < 6; ++i)
                {
                    f32 x0 = (f32)i / 6.0f;
                    f32 x1 = (f32)(i + 1) / 6.0f;
                    RectF segment(square.offset_x + square.width * x0, square.offset_y, square.width * (x1 - x0) + 0.5f, square.height);
                    Float4U left = color_from_picker_channels(axis, x0, 1.0f, picker_bar, 1.0f);
                    Float4U right = color_from_picker_channels(axis, x1, 1.0f, picker_bar, 1.0f);
                    core_draw_relative_gradient_rect(context, segment, left, right, right, left);
                    if(axis == 1)
                    {
                        core_draw_relative_gradient_rect(context, segment, Float4U(0.0f), Float4U(0.0f),
                            Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                    }
                    else
                    {
                        Float4U gray(picker_bar, picker_bar, picker_bar, 1.0f);
                        core_draw_relative_gradient_rect(context, segment, Float4U(gray.x, gray.y, gray.z, 0.0f),
                            Float4U(gray.x, gray.y, gray.z, 0.0f), gray, gray);
                    }
                }
            }
            else
            {
                core_draw_relative_gradient_rect(context, square,
                    color_from_picker_channels(axis, 0.0f, 1.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 1.0f, 1.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 1.0f, 0.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 0.0f, 0.0f, picker_bar, 1.0f));
            }
            Float4U border = core_style_value(context, Name("gui.color_picker.border"), GUICore::style_f32x4(Float4U(0.24f, 0.29f, 0.36f, 1.0f))).number;
            core_draw_relative_line(context, Float2U(square.offset_x, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y), border, 1.0f);
            core_draw_relative_line(context, Float2U(square.offset_x + square.width, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y + square.height), border, 1.0f);
            core_draw_relative_line(context, Float2U(square.offset_x + square.width, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y + square.height), border, 1.0f);
            core_draw_relative_line(context, Float2U(square.offset_x, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y), border, 1.0f);

            if(axis == 0)
            {
                for(u32 i = 0; i < 6; ++i)
                {
                    f32 y0 = (f32)i / 6.0f;
                    f32 y1 = (f32)(i + 1) / 6.0f;
                    RectF segment(bar.offset_x, bar.offset_y + bar.height * y0, bar.width, bar.height * (y1 - y0) + 0.5f);
                    core_draw_relative_gradient_rect(context, segment, color_hsv_to_rgb(y0, 1.0f, 1.0f, 1.0f),
                        color_hsv_to_rgb(y0, 1.0f, 1.0f, 1.0f), color_hsv_to_rgb(y1, 1.0f, 1.0f, 1.0f),
                        color_hsv_to_rgb(y1, 1.0f, 1.0f, 1.0f));
                }
            }
            else
            {
                core_draw_relative_gradient_rect(context, bar,
                    color_from_picker_channels(axis, picker_x, picker_y, 1.0f, 1.0f),
                    color_from_picker_channels(axis, picker_x, picker_y, 1.0f, 1.0f),
                    color_from_picker_channels(axis, picker_x, picker_y, 0.0f, 1.0f),
                    color_from_picker_channels(axis, picker_x, picker_y, 0.0f, 1.0f));
            }

            f32 cursor_x = square.offset_x + picker_x * square.width;
            f32 cursor_y = square.offset_y + (1.0f - picker_y) * square.height;
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(cursor_x - 8.0f, cursor_y - 8.0f, 16.0f, 16.0f), Float4U(1.0f), 8.0f);
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(cursor_x - 5.0f, cursor_y - 5.0f, 10.0f, 10.0f), color, 5.0f);
            f32 bar_y = bar.offset_y + (axis == 0 ? picker_bar : (1.0f - picker_bar)) * bar.height;
            core_draw_relative_line(context, Float2U(bar.offset_x - 5.0f, bar_y), Float2U(bar.offset_x + bar.width + 5.0f, bar_y), Float4U(1.0f), 2.0f);

            Float4U text_color = core_style_value(context, Name("gui.editor.text.color"), GUICore::style_f32x4(Float4U(1.0f))).number;
            core_draw_relative_text(context, RectF(current_rect.offset_x, current_rect.offset_y - 26.0f, current_rect.width, 22.0f), "Current", text_color, 15.0f);
            core_draw_color_picker_swatch(context, current_rect, color);
            core_draw_relative_text(context, RectF(original_rect.offset_x, original_rect.offset_y - 26.0f, original_rect.width, 22.0f), "Original", text_color, 15.0f);
            core_draw_color_picker_swatch(context, original_rect, state.color_picker_original_valid ? state.color_picker_original : color);
            context->end_element();
            (void)count;
        }

        static GUICore::ElementHandle add_core_color_edit_view(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count, const GUICore::LayoutInput& layout)
        {
            luassert(context && id);
            ColorBinding binding;
            assign_color_binding(binding, f32_value, u8_value, u32_value, type, count);
            write_color_value(binding, read_color_value(binding));

            GUICore::ElementHandle preview = context->begin_element(id, label ? Name(label) : Name("color_edit"));
            context->set_layout(preview, layout);
            core_set_basic_interactable(context, preview);
            GUICore::InteractionState preview_interaction = context->get_interaction_state(id);
            Float4U background = core_style_value(context, preview_interaction.hovered ? Name("gui.editor.color_edit.background_hovered") :
                Name("gui.editor.color_edit.background"), preview_interaction.hovered ?
                GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f))).number;
            Float4U border = core_style_value(context, Name("gui.editor.color_edit.border"), GUICore::style_f32x4(Float4U(0.18f, 0.25f, 0.34f, 1.0f))).number;
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f), border, 4.0f);
            core_draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(1.0f, 1.0f, -2.0f, -2.0f), background, 3.0f);
            core_draw_color_picker_swatch(context, RectF(7.0f, 4.0f, 22.0f, 22.0f), read_color_value(binding));
            String hex = color_edit_hex_text(binding);
            core_draw_relative_text(context, RectF(38.0f, 0.0f, -46.0f, 0.0f), hex.c_str(),
                core_style_value(context, Name("gui.editor.text.color"), GUICore::style_f32x4(Float4U(0.86f, 0.90f, 0.96f, 1.0f))).number, 15.0f);
            context->end_element();

            GUICore::id_t popup_id = core_derived_id(id, "color_popup");
            if(preview_interaction.clicked)
            {
                if(is_popup_open(context, popup_id))
                {
                    close_popup(context, popup_id);
                }
                else
                {
                    open_popup(context, popup_id);
                    Ref<PopupAnchorState> anchor = core_state<PopupAnchorState>(context, popup_id, GUICore::StateLifetime::process);
                    anchor->popup_anchor_position = Float2U(preview_interaction.clicked_screen_position.x + 8.0f,
                        preview_interaction.clicked_screen_position.y + 8.0f);
                    anchor->popup_anchor_placement = PopupAnchorPlacement::pointer;
                    anchor->popup_anchor_valid = true;
                    Ref<ColorPickerState> state = core_state<ColorPickerState>(context, id, GUICore::StateLifetime::next_frame);
                    state->color_picker_original = read_color_value(binding);
                    state->color_picker_original_valid = true;
                }
            }

            Ref<ColorPickerState> state = core_state<ColorPickerState>(context, id, GUICore::StateLifetime::next_frame);
            ensure_color_picker_state_channels(*state);
            color_picker_axis_ref(*state) = clamp(color_picker_axis_ref(*state), 0, 5);
            sync_color_picker_build_state(*state, read_color_value(binding));

            Ref<PopupAnchorState> anchor = core_state<PopupAnchorState>(context, popup_id, GUICore::StateLifetime::process);
            constexpr f32 popup_width = 476.0f;
            f32 popup_height = count == 4 ? 470.0f : 432.0f;
            PopupDesc popup_desc;
            popup_desc.layout = fixed_layout(popup_width, popup_height);
            popup_desc.position = anchor->popup_anchor_valid ? anchor->popup_anchor_position : Float2U(40.0f, 40.0f);
            popup_desc.position.x = min(popup_desc.position.x, max(context->get_frame_desc().screen_size.x - popup_width, 0.0f));
            popup_desc.position.y = min(popup_desc.position.y, max(context->get_frame_desc().screen_size.y - popup_height, 0.0f));
            GUICore::ElementHandle popup;
            if(begin_popup(context, popup_id, popup_desc, &popup))
            {
                core_set_layout_result(context, popup, RectF(0.0f, 0.0f, popup_width, popup_height));
                core_draw_color_picker(context, core_derived_id(id, "picker"), binding, *state, count);

                const c8* axis_labels[] = { "H", "S", "V", "R", "G", "B" };
                f32 axis_y = 318.0f;
                f32 axis_w = 456.0f / 6.0f;
                for(u32 i = 0; i < 6; ++i)
                {
                    GUICore::id_t axis_id = core_derived_id(id, axis_labels[i]);
                    GUICore::ElementHandle axis_item = text_button(context, axis_id, axis_labels[i]);
                    core_set_layout_result(context, axis_item, RectF(10.0f + axis_w * (f32)i, axis_y, axis_w, 28.0f));
                    if(context->get_interaction_state(axis_id).clicked)
                    {
                        color_picker_axis_ref(*state) = (i32)i;
                    }
                }

                i32 old_rgb[4] = { state->color_picker_rgb[0], state->color_picker_rgb[1], state->color_picker_rgb[2], state->color_picker_rgb[3] };
                i32 old_hsv[3] = { state->color_picker_hsv[0], state->color_picker_hsv[1], state->color_picker_hsv[2] };
                f32 column_w = 112.0f;
                f32 label_w = 20.0f;
                f32 row_y[] = { 354.0f, 392.0f, 430.0f };
                for(u32 i = 0; i < 3; ++i)
                {
                    GUICore::id_t label_id = core_derived_id(id, i == 0 ? "r_label" : (i == 1 ? "g_label" : "b_label"));
                    GUICore::ElementHandle label_element = text(context, label_id, i == 0 ? "R" : (i == 1 ? "G" : "B"));
                    core_set_layout_result(context, label_element, RectF(10.0f + (label_w + column_w + 20.0f) * (f32)i, row_y[0], label_w, 30.0f));
                    GUICore::ElementHandle drag = drag_int(context, core_derived_id(id, i == 0 ? "r_drag" : (i == 1 ? "g_drag" : "b_drag")),
                        &state->color_picker_rgb[i], 1.0f, 0, 255);
                    core_set_layout_result(context, drag, RectF(34.0f + (label_w + column_w + 20.0f) * (f32)i, row_y[0], column_w, 30.0f));
                }
                for(u32 i = 0; i < 3; ++i)
                {
                    GUICore::id_t label_id = core_derived_id(id, i == 0 ? "h_label" : (i == 1 ? "s_label" : "v_label"));
                    GUICore::ElementHandle label_element = text(context, label_id, i == 0 ? "H" : (i == 1 ? "S" : "V"));
                    core_set_layout_result(context, label_element, RectF(10.0f + (label_w + column_w + 20.0f) * (f32)i, row_y[1], label_w, 30.0f));
                    GUICore::ElementHandle drag = drag_int(context, core_derived_id(id, i == 0 ? "h_drag" : (i == 1 ? "s_drag" : "v_drag")),
                        &state->color_picker_hsv[i], 1.0f, 0, 255);
                    core_set_layout_result(context, drag, RectF(34.0f + (label_w + column_w + 20.0f) * (f32)i, row_y[1], column_w, 30.0f));
                }
                if(count == 4)
                {
                    GUICore::ElementHandle label_element = text(context, core_derived_id(id, "a_label"), "A");
                    core_set_layout_result(context, label_element, RectF(10.0f, row_y[2], label_w, 30.0f));
                    GUICore::ElementHandle drag = drag_int(context, core_derived_id(id, "a_drag"), &state->color_picker_rgb[3], 1.0f, 0, 255);
                    core_set_layout_result(context, drag, RectF(34.0f, row_y[2], 422.0f, 30.0f));
                }

                bool rgb_changed = old_rgb[0] != state->color_picker_rgb[0] || old_rgb[1] != state->color_picker_rgb[1] ||
                    old_rgb[2] != state->color_picker_rgb[2] || old_rgb[3] != state->color_picker_rgb[3];
                bool hsv_changed = old_hsv[0] != state->color_picker_hsv[0] || old_hsv[1] != state->color_picker_hsv[1] ||
                    old_hsv[2] != state->color_picker_hsv[2];
                if(rgb_changed)
                {
                    write_color_value(binding, Float4U(color_u8_to_channel((u8)clamp(state->color_picker_rgb[0], 0, 255)),
                        color_u8_to_channel((u8)clamp(state->color_picker_rgb[1], 0, 255)),
                        color_u8_to_channel((u8)clamp(state->color_picker_rgb[2], 0, 255)),
                        color_u8_to_channel((u8)clamp(state->color_picker_rgb[3], 0, 255))));
                    sync_color_picker_build_state(*state, read_color_value(binding));
                }
                else if(hsv_changed)
                {
                    Float4U new_color = color_hsv_to_rgb(color_u8_to_channel((u8)clamp(state->color_picker_hsv[0], 0, 255)),
                        color_u8_to_channel((u8)clamp(state->color_picker_hsv[1], 0, 255)),
                        color_u8_to_channel((u8)clamp(state->color_picker_hsv[2], 0, 255)),
                        read_color_value(binding).w);
                    write_color_value(binding, new_color);
                    sync_color_picker_build_state(*state, read_color_value(binding));
                }
                context->end_element();
                context->pop_layer();
            }
            return preview;
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, f32* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, value, nullptr, nullptr, ColorValueType::f32, 3, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, f32* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, value, nullptr, nullptr, ColorValueType::f32, 4, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, u8* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, nullptr, value, nullptr, ColorValueType::u8, 3, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, u8* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, nullptr, value, nullptr, ColorValueType::u8, 4, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit3(GUICore::IContext* context, GUICore::id_t id, const c8* label, u32* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, nullptr, nullptr, value, ColorValueType::rgba8, 3, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle color_edit4(GUICore::IContext* context, GUICore::id_t id, const c8* label, u32* value,
            const GUICore::LayoutInput& layout)
        {
            return add_core_color_edit_view(context, id, label, nullptr, nullptr, value, ColorValueType::rgba8, 4, layout);
        }
    }
}
