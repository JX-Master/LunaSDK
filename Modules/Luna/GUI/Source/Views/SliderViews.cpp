/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static ItemHandle add_slider_float_with_input_component(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ItemHandle handle = begin_h_layout(context, label ? label : "SliderFloatWithInput", row);
            set_next_item_layout(context, LayoutStyle::fill());
            add_slider_float_node(context, label, value, 1, min_value, max_value);
            set_next_item_layout(context, LayoutStyle::fixed_width(72.0f));
            add_input_float_node(context, "", value, 1, min_value, max_value);
            end_h_layout(context);
            return handle;
        }

        static ItemHandle add_slider_int_with_input_component(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ItemHandle handle = begin_h_layout(context, label ? label : "SliderIntWithInput", row);
            set_next_item_layout(context, LayoutStyle::fill());
            add_slider_int_node(context, label, value, 1, min_value, max_value);
            set_next_item_layout(context, LayoutStyle::fixed_width(72.0f));
            add_input_int_node(context, "", value, 1, min_value, max_value);
            end_h_layout(context);
            return handle;
        }

        static ItemHandle add_slider_float_with_input_view(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            if(count <= 1) return add_slider_float_with_input_component(context, label, value, min_value, max_value);
            LayoutDesc column;
            column.gap = 4.0f;
            ItemHandle handle = begin_v_layout(context, label ? label : "SliderFloatWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                push_id(context, i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_float_with_input_component(context, component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                pop_id(context);
            }
            end_v_layout(context);
            return handle;
        }

        static ItemHandle add_slider_int_with_input_view(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            if(count <= 1) return add_slider_int_with_input_component(context, label, value, min_value, max_value);
            LayoutDesc column;
            column.gap = 4.0f;
            ItemHandle handle = begin_v_layout(context, label ? label : "SliderIntWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                push_id(context, i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_int_with_input_component(context, component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                pop_id(context);
            }
            end_v_layout(context);
            return handle;
        }

        LUNA_GUI_API ItemHandle slider_float_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_view(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float2_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_view(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float3_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_view(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float4_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_view(context, label, value, 4, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_view(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int2_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_view(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int3_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_view(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int4_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_view(context, label, value, 4, min_value, max_value);
        }
    }
}
