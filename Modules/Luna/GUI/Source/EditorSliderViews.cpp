/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorSliderViews.cpp
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

        static GUICore::id_t core_slider_with_input_id(GUICore::id_t id, const c8* salt, u32 index = 0)
        {
            return hash_u64_local(index, hash_cstr_local(salt, hash_u64_local(id)));
        }

        static Ref<SliderWithInputState> core_slider_with_input_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<SliderWithInputState>(id);
            Ref<SliderWithInputState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<SliderWithInputState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static void core_resize_slider_with_input_state(SliderWithInputState& state, u8 count, bool is_float)
        {
            state.texts.resize(count);
            if(is_float)
            {
                state.last_f32_values.resize(count);
                state.last_i32_values.clear();
            }
            else
            {
                state.last_i32_values.resize(count);
                state.last_f32_values.clear();
            }
        }

        static bool core_parse_f32_text(const String& text, f32& out_value)
        {
            if(text.empty()) return false;
            c8* end = nullptr;
            f32 value = strtof32(text.c_str(), &end);
            if(end == text.c_str() || (end && *end)) return false;
            out_value = value;
            return true;
        }

        static bool core_parse_i32_text(const String& text, i32& out_value)
        {
            if(text.empty()) return false;
            c8* end = nullptr;
            i64 value = strtoi64(text.c_str(), &end, 10);
            if(end == text.c_str() || (end && *end)) return false;
            out_value = clamp((i32)value, I32_MIN, I32_MAX);
            return true;
        }

        static void core_format_f32_text(String& text, f32 value)
        {
            strprintf(text, "%.3f", value);
        }

        static void core_format_i32_text(String& text, i32 value)
        {
            strprintf(text, "%d", value);
        }

        static GUICore::LayoutInput core_slider_with_input_text_layout();

        static void core_sync_float_input_text(GUICore::IContext* context, SliderWithInputState& state,
            GUICore::id_t input_id, f32* value, u32 index, f32 min_value, f32 max_value)
        {
            f32 current_value = value ? value[index] : 0.0f;
            bool focused = context->focused_element() == input_id || context->get_interaction_state(input_id).focused;
            if(!focused && (state.texts[index].empty() || state.last_f32_values[index] != current_value))
            {
                core_format_f32_text(state.texts[index], current_value);
                state.last_f32_values[index] = current_value;
            }
            GUI::input_text(context, input_id, state.texts[index], core_slider_with_input_text_layout());
            f32 parsed_value = 0.0f;
            if(value && core_parse_f32_text(state.texts[index], parsed_value))
            {
                value[index] = clamp(parsed_value, min_value, max_value);
                state.last_f32_values[index] = value[index];
            }
            else if(!focused)
            {
                core_format_f32_text(state.texts[index], current_value);
            }
        }

        static void core_sync_int_input_text(GUICore::IContext* context, SliderWithInputState& state,
            GUICore::id_t input_id, i32* value, u32 index, i32 min_value, i32 max_value)
        {
            i32 current_value = value ? value[index] : 0;
            bool focused = context->focused_element() == input_id || context->get_interaction_state(input_id).focused;
            if(!focused && (state.texts[index].empty() || state.last_i32_values[index] != current_value))
            {
                core_format_i32_text(state.texts[index], current_value);
                state.last_i32_values[index] = current_value;
            }
            GUI::input_text(context, input_id, state.texts[index], core_slider_with_input_text_layout());
            i32 parsed_value = 0;
            if(value && core_parse_i32_text(state.texts[index], parsed_value))
            {
                value[index] = clamp(parsed_value, min_value, max_value);
                state.last_i32_values[index] = value[index];
            }
            else if(!focused)
            {
                core_format_i32_text(state.texts[index], current_value);
            }
        }

        static GUICore::LayoutInput core_slider_with_input_row_layout()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::ratio;
            layout.height.value = 1.0f;
            return layout;
        }

        static GUICore::LayoutInput core_slider_with_input_slider_layout()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::ratio;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }

        static GUICore::LayoutInput core_slider_with_input_text_layout()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::pixels;
            layout.width.value = 72.0f;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }

        static GUICore::ElementHandle core_slider_float_with_input_component(GUICore::IContext* context,
            GUICore::id_t id, const c8* label, SliderWithInputState& state, f32* value, u32 index,
            f32 min_value, f32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label ? label : "slider_float_with_input", layout);
            GUI::slider_float(context, core_slider_with_input_id(id, "slider"), value ? value + index : nullptr,
                min_value, max_value, core_slider_with_input_slider_layout());
            GUICore::id_t input_id = core_slider_with_input_id(id, "input");
            core_sync_float_input_text(context, state, input_id, value, index, min_value, max_value);
            GUICore::LinearLayoutDesc row_desc;
            row_desc.gap = 8.0f;
            lupanic_if_failed(GUI::end_h_layout(context, row, rect, row_desc));
            return row;
        }

        static GUICore::ElementHandle core_slider_int_with_input_component(GUICore::IContext* context,
            GUICore::id_t id, const c8* label, SliderWithInputState& state, i32* value, u32 index,
            i32 min_value, i32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label ? label : "slider_int_with_input", layout);
            GUI::slider_int(context, core_slider_with_input_id(id, "slider"), value ? value + index : nullptr,
                min_value, max_value, core_slider_with_input_slider_layout());
            GUICore::id_t input_id = core_slider_with_input_id(id, "input");
            core_sync_int_input_text(context, state, input_id, value, index, min_value, max_value);
            GUICore::LinearLayoutDesc row_desc;
            row_desc.gap = 8.0f;
            lupanic_if_failed(GUI::end_h_layout(context, row, rect, row_desc));
            return row;
        }

        static GUICore::ElementHandle core_slider_float_with_input_view(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, u8 count, f32 min_value, f32 max_value, const RectF& rect,
            const GUICore::LayoutInput& layout)
        {
            luassert(context && id && count);
            Ref<SliderWithInputState> state = core_slider_with_input_state(context, id);
            core_resize_slider_with_input_state(*state, count, true);
            if(count <= 1)
            {
                return core_slider_float_with_input_component(context, id, label, *state, value, 0, min_value, max_value,
                    rect, layout);
            }
            GUICore::ElementHandle column = GUI::begin_v_layout(context, id, label ? label : "slider_float_with_input", layout);
            const c8* components[] = { "X", "Y", "Z", "W" };
            f32 gap = 4.0f;
            f32 row_height = count ? max((rect.height - gap * ((f32)count - 1.0f)) / (f32)count, 0.0f) : 0.0f;
            for(u32 i = 0; i < count; ++i)
            {
                GUICore::id_t row_id = core_slider_with_input_id(id, "row", i);
                RectF row_rect(rect.offset_x, rect.offset_y + (row_height + gap) * (f32)i, rect.width, row_height);
                core_slider_float_with_input_component(context, row_id, components[i], *state, value, i,
                    min_value, max_value, row_rect, core_slider_with_input_row_layout());
            }
            GUICore::LayoutResult result;
            result.rect = rect;
            result.clip_rect = rect;
            result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(column, result);
            context->end_element();
            return column;
        }

        static GUICore::ElementHandle core_slider_int_with_input_view(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, u8 count, i32 min_value, i32 max_value, const RectF& rect,
            const GUICore::LayoutInput& layout)
        {
            luassert(context && id && count);
            Ref<SliderWithInputState> state = core_slider_with_input_state(context, id);
            core_resize_slider_with_input_state(*state, count, false);
            if(count <= 1)
            {
                return core_slider_int_with_input_component(context, id, label, *state, value, 0, min_value, max_value,
                    rect, layout);
            }
            GUICore::ElementHandle column = GUI::begin_v_layout(context, id, label ? label : "slider_int_with_input", layout);
            const c8* components[] = { "X", "Y", "Z", "W" };
            f32 gap = 4.0f;
            f32 row_height = count ? max((rect.height - gap * ((f32)count - 1.0f)) / (f32)count, 0.0f) : 0.0f;
            for(u32 i = 0; i < count; ++i)
            {
                GUICore::id_t row_id = core_slider_with_input_id(id, "row", i);
                RectF row_rect(rect.offset_x, rect.offset_y + (row_height + gap) * (f32)i, rect.width, row_height);
                core_slider_int_with_input_component(context, row_id, components[i], *state, value, i,
                    min_value, max_value, row_rect, core_slider_with_input_row_layout());
            }
            GUICore::LayoutResult result;
            result.rect = rect;
            result.clip_rect = rect;
            result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(column, result);
            context->end_element();
            return column;
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_float_with_input_view(context, id, label, value, 1, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float2_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_float_with_input_view(context, id, label, value, 2, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float3_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_float_with_input_view(context, id, label, value, 3, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float4_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, f32* value, f32 min_value, f32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_float_with_input_view(context, id, label, value, 4, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_int_with_input_view(context, id, label, value, 1, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int2_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_int_with_input_view(context, id, label, value, 2, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int3_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_int_with_input_view(context, id, label, value, 3, min_value, max_value, rect, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int4_with_input(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, i32* value, i32 min_value, i32 max_value, const RectF& rect, const GUICore::LayoutInput& layout)
        {
            return core_slider_int_with_input_view(context, id, label, value, 4, min_value, max_value, rect, layout);
        }
    }
}
