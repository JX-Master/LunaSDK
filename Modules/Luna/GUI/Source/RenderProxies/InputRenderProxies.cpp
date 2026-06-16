/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "InputRenderProxies.hpp"
#include "../Nodes/InputNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
#define LUNA_GUI_STYLE_F32(entry_name, value, category) {Name(entry_name), StyleValueType::f32_1, StyleValue::f32_1(value), nullptr, category, nullptr}
#define LUNA_GUI_STYLE_F32X4(entry_name, value, category) {Name(entry_name), StyleValueType::f32_4, StyleValue::f32_4(value), nullptr, category, nullptr}
#define LUNA_GUI_STYLE_NAME(entry_name, value, category) {Name(entry_name), StyleValueType::name, StyleValue::name(Name(value)), nullptr, category, nullptr}

        static void draw_text_selection(NodeRenderContext& ctx, const RectF& text_rect, const RectF& text_clip,
            const String& value, const InputEditState& state, f32 font_size, const Float4U& color)
        {
            usize selection_begin = 0;
            usize selection_end = 0;
            input_text_selection_range(value, state, selection_begin, selection_end);
            f32 selection_x0 = text_rect.offset_x + ctx.text_cursor_x(value, selection_begin, font_size);
            f32 selection_x1 = text_rect.offset_x + ctx.text_cursor_x(value, selection_end, font_size);
            ctx.draw_rect(RectF(selection_x0, text_rect.offset_y + 4.0f, max(selection_x1 - selection_x0, 1.0f),
                max(text_rect.height - 8.0f, 1.0f)), text_clip, color, 2.0f);
        }

        static void draw_text_cursor(NodeRenderContext& ctx, const RectF& text_rect, const RectF& text_clip,
            const String& value, const InputEditState& state, f32 font_size, f64 time, const Float4U& color)
        {
            f64 blink_time = max(time - state.text_cursor_blink_start, 0.0);
            bool cursor_visible = (((u64)(blink_time / 0.5)) & 1) == 0;
            if(!cursor_visible) return;
            f32 cursor_x = text_rect.offset_x + ctx.text_cursor_x(value, state.text_cursor, font_size);
            if(cursor_x >= text_clip.offset_x && cursor_x <= text_clip.offset_x + text_clip.width)
            {
                ctx.draw_rect(RectF(cursor_x, text_rect.offset_y + 5.0f, 1.0f, max(text_rect.height - 10.0f, 1.0f)),
                    text_clip, color, 0.0f);
            }
        }

        static void draw_default_input_text(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& render_state, void*)
        {
            f32 font_size = style_f32(ctx, node, Name("gui.input_text.font_size"), 16.0f);
            ctx.draw_rect(rect, clip_rect, render_state.focused ?
                style_f32x4(ctx, node, Name("gui.input_text.background_focused"), Float4U(0.12f, 0.16f, 0.22f, 1.0f)) :
                style_f32x4(ctx, node, Name("gui.input_text.background"), Float4U(0.08f, 0.10f, 0.13f, 1.0f)),
                style_f32(ctx, node, Name("gui.input_text.radius"), 4.0f));
            const String* string_value = input_text_value(node);
            if(!string_value) return;
            RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
            RectF text_clip = intersect_rect(clip_rect, text_rect);
            f32 text_width = ctx.text_cursor_x(*string_value, string_value->size(), font_size);
            RectF arrange_rect(text_rect.offset_x, text_rect.offset_y, max(text_rect.width, text_width + 4.0f), text_rect.height);
            Ref<InputEditState> state_ref = ctx.get_or_create_widget_state<InputEditState>(node.id);
            InputEditState& state = *state_ref;
            state.text_cursor = clamp_utf8_cursor(*string_value, state.text_cursor);
            if(render_state.focused && input_text_has_selection(*string_value, state))
            {
                draw_text_selection(ctx, text_rect, text_clip, *string_value, state, font_size,
                    style_f32x4(ctx, node, Name("gui.input_text.selection"), Float4U(0.25f, 0.45f, 0.78f, 0.80f)));
            }
            ctx.draw_text(arrange_rect, text_clip, string_value->c_str(), font_size,
                style_f32x4(ctx, node, Name("gui.input_text.text_color"), Float4U(1.0f)), TextAlignment::begin);
            if(render_state.focused && !input_text_has_selection(*string_value, state))
            {
                draw_text_cursor(ctx, text_rect, text_clip, *string_value, state, font_size, render_state.time,
                    style_f32x4(ctx, node, Name("gui.input_text.cursor"), Float4U(1.0f)));
            }
        }

        static void draw_default_numeric(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& render_state, void*)
        {
            const NumericBinding* binding = numeric_binding(node);
            luassert(binding);
            f32 label_w = numeric_label_width(node, rect);
            u32 value_count = numeric_value_count(node);
            NumericInteractionState* numeric_state = ctx.get_widget_state<NumericInteractionState>(0);
            u32 active_component_index = numeric_state ? numeric_state->active_float_component : U32_MAX;
            if(numeric_slider(node))
            {
                for(u32 i = 0; i < value_count; ++i)
                {
                    f32* f32_values = binding->f32_value;
                    i32* i32_values = binding->i32_value;
                    f32 value = numeric_value_f32(node) ? (f32_values ? f32_values[i] : 0.0f) : (i32_values ? (f32)i32_values[i] : 0.0f);
                    RectF component_rect = numeric_component_rect(node, rect, i);
                    bool active_component = render_state.active && (active_component_index == U32_MAX || active_component_index == i);
                    f32 denom = max(binding->max_value - binding->min_value, 0.0001f);
                    f32 t = clamp((value - binding->min_value) / denom, 0.0f, 1.0f);
                    f32 track_pad = min(8.0f, component_rect.width * 0.20f);
                    f32 track_x0 = component_rect.offset_x + track_pad;
                    f32 track_x1 = component_rect.offset_x + max(component_rect.width - track_pad, track_pad);
                    f32 track_y = component_rect.offset_y + component_rect.height * 0.5f;
                    Float4U track_color = render_state.hovered ?
                        style_f32x4(ctx, node, Name("gui.numeric.slider_track_hovered"), Float4U(0.12f, 0.16f, 0.22f, 1.0f)) :
                        style_f32x4(ctx, node, Name("gui.numeric.slider_track"), Float4U(0.07f, 0.08f, 0.10f, 1.0f));
                    Float4U fill_color = active_component ?
                        style_f32x4(ctx, node, Name("gui.numeric.slider_fill_active"), Float4U(0.26f, 0.43f, 0.72f, 1.0f)) :
                        (render_state.hovered ?
                            style_f32x4(ctx, node, Name("gui.numeric.slider_fill_hovered"), Float4U(0.22f, 0.38f, 0.64f, 1.0f)) :
                            style_f32x4(ctx, node, Name("gui.numeric.slider_fill"), Float4U(0.20f, 0.36f, 0.62f, 1.0f)));
                    f32 knob_x = track_x0 + (track_x1 - track_x0) * t;
                    f32 track_width = active_component ? 3.0f : 2.0f;
                    ctx.draw_line(Float2U(track_x0, track_y), Float2U(track_x1, track_y), clip_rect, track_color, track_width);
                    ctx.draw_line(Float2U(track_x0, track_y), Float2U(knob_x, track_y), clip_rect, fill_color, track_width);
                    f32 knob_radius = active_component ? 6.5f : 5.5f;
                    ctx.draw_circle(RectF(knob_x - knob_radius, track_y - knob_radius, knob_radius * 2.0f, knob_radius * 2.0f), clip_rect, fill_color);
                }
                if(label_w > 0.0f)
                {
                    ctx.draw_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip_rect, node.text.c_str(), 16.0f,
                        style_f32x4(ctx, node, Name("gui.numeric.label_color"), Float4U(1.0f)), TextAlignment::begin);
                }
                return;
            }

            Ref<InputEditState> state_ref = ctx.get_or_create_widget_state<InputEditState>(node.id);
            InputEditState& state = *state_ref;
            for(u32 i = 0; i < value_count; ++i)
            {
                f32* f32_values = binding->f32_value;
                i32* i32_values = binding->i32_value;
                f32 value = numeric_value_f32(node) ? (f32_values ? f32_values[i] : 0.0f) : (i32_values ? (f32)i32_values[i] : 0.0f);
                RectF component_rect = numeric_component_rect(node, rect, i);
                bool editing_component = numeric_text_editable(node) && render_state.focused && state.numeric_editing && state.numeric_edit_component == i;
                Float4U bg = binding->f32_color ? Float4U(
                    i == 0 ? value : 0.10f,
                    i == 1 ? value : 0.10f,
                    i == 2 ? value : 0.10f,
                    1.0f) : style_f32x4(ctx, node, Name("gui.numeric.background"), Float4U(0.12f, 0.16f, 0.22f, 1.0f));
                bool active_component = render_state.active && (active_component_index == U32_MAX || active_component_index == i);
                ctx.draw_rect(component_rect, clip_rect, (active_component || editing_component) ?
                    style_f32x4(ctx, node, Name("gui.numeric.background_active"), Float4U(0.18f, 0.29f, 0.44f, 1.0f)) : bg, 4.0f);
                if(!editing_component && numeric_drag(node) && binding->max_value > binding->min_value)
                {
                    f32 denom = max(binding->max_value - binding->min_value, 0.0001f);
                    f32 t = clamp((value - binding->min_value) / denom, 0.0f, 1.0f);
                    ctx.draw_rect(RectF(component_rect.offset_x, component_rect.offset_y + component_rect.height - 3.0f,
                        component_rect.width * t, 3.0f), clip_rect,
                        style_f32x4(ctx, node, Name("gui.numeric.drag_fill"), Float4U(0.30f, 0.56f, 0.88f, 1.0f)), 1.5f);
                }
                String value_text = editing_component ? state.numeric_edit_text : numeric_value_text(node, i);
                RectF text_rect(component_rect.offset_x + 6.0f, component_rect.offset_y, max(component_rect.width - 12.0f, 1.0f), component_rect.height);
                RectF text_clip = intersect_rect(clip_rect, text_rect);
                f32 font_size = style_f32(ctx, node, Name("gui.numeric.font_size"), 15.0f);
                if(editing_component && input_text_has_selection(value_text, state))
                {
                    draw_text_selection(ctx, text_rect, text_clip, value_text, state, font_size,
                        style_f32x4(ctx, node, Name("gui.numeric.selection"), Float4U(0.25f, 0.45f, 0.78f, 0.80f)));
                }
                ctx.draw_text(text_rect, text_clip, value_text.c_str(), font_size,
                    style_f32x4(ctx, node, Name("gui.numeric.text_color"), Float4U(1.0f)), TextAlignment::begin);
                if(editing_component && !input_text_has_selection(value_text, state))
                {
                    draw_text_cursor(ctx, text_rect, text_clip, value_text, state, font_size, render_state.time,
                        style_f32x4(ctx, node, Name("gui.numeric.cursor"), Float4U(1.0f)));
                }
            }
            ctx.draw_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip_rect, node.text.c_str(), 16.0f,
                style_f32x4(ctx, node, Name("gui.numeric.label_color"), Float4U(1.0f)), TextAlignment::begin);
        }

        RenderProxyDesc default_input_text_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.input_text.font_size", 16.0f, "InputText"),
                LUNA_GUI_STYLE_F32X4("gui.input_text.background_focused", Float4U(0.12f, 0.16f, 0.22f, 1.0f), "InputText"),
                LUNA_GUI_STYLE_F32X4("gui.input_text.background", Float4U(0.08f, 0.10f, 0.13f, 1.0f), "InputText"),
                LUNA_GUI_STYLE_F32("gui.input_text.radius", 4.0f, "InputText"),
                LUNA_GUI_STYLE_F32X4("gui.input_text.selection", Float4U(0.25f, 0.45f, 0.78f, 0.80f), "InputText"),
                LUNA_GUI_STYLE_F32X4("gui.input_text.text_color", Float4U(1.0f), "InputText"),
                LUNA_GUI_STYLE_F32X4("gui.input_text.cursor", Float4U(1.0f), "InputText")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_input_text;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_numeric_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.slider_track_hovered", Float4U(0.12f, 0.16f, 0.22f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.slider_track", Float4U(0.07f, 0.08f, 0.10f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.slider_fill_active", Float4U(0.26f, 0.43f, 0.72f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.slider_fill_hovered", Float4U(0.22f, 0.38f, 0.64f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.slider_fill", Float4U(0.20f, 0.36f, 0.62f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.label_color", Float4U(1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.background", Float4U(0.12f, 0.16f, 0.22f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.background_active", Float4U(0.18f, 0.29f, 0.44f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.drag_fill", Float4U(0.30f, 0.56f, 0.88f, 1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32("gui.numeric.font_size", 15.0f, "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.selection", Float4U(0.25f, 0.45f, 0.78f, 0.80f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.text_color", Float4U(1.0f), "Numeric"),
                LUNA_GUI_STYLE_F32X4("gui.numeric.cursor", Float4U(1.0f), "Numeric")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_numeric;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

#undef LUNA_GUI_STYLE_F32
#undef LUNA_GUI_STYLE_F32X4
#undef LUNA_GUI_STYLE_NAME
    }
}
