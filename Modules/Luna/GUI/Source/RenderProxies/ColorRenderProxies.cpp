/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "ColorRenderProxies.hpp"
#include "../Nodes/ColorNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static void draw_color_swatch(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius)
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.24f, 0.29f, 0.36f, 1.0f), radius);
            RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f), max(rect.height - 2.0f, 1.0f));
            f32 inner_radius = max(radius - 1.0f, 0.0f);
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
                    ctx.draw_rect(cell_rect, clip_rect, checker, 0.0f);
                }
            }
            ctx.draw_rect(inner, clip_rect, color, inner_radius);
        }

        static void draw_square_border(NodeRenderContext& ctx, const RectF& square, const RectF& clip_rect, const Float4U& color)
        {
            ctx.draw_line(Float2U(square.offset_x, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y), clip_rect, color, 1.0f);
            ctx.draw_line(Float2U(square.offset_x + square.width, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y + square.height), clip_rect, color, 1.0f);
            ctx.draw_line(Float2U(square.offset_x + square.width, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y + square.height), clip_rect, color, 1.0f);
            ctx.draw_line(Float2U(square.offset_x, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y), clip_rect, color, 1.0f);
        }

        static void draw_default_color_picker(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            Float4U color = read_color_value(node);
            const ColorBinding* color_picker_binding = color_binding(node);
            id_t owner_id = color_picker_binding && color_picker_binding->owner_id ? color_picker_binding->owner_id : node.id;
            Ref<ColorPickerState> state_ref = ctx.get_or_create_widget_state<ColorPickerState>(owner_id);
            ColorPickerState& state = *state_ref;
            i32 axis = clamp(color_picker_axis_ref(state), 0, 5);
            f32 picker_x = 0.0f;
            f32 picker_y = 0.0f;
            f32 picker_bar = 0.0f;
            color_picker_channels_from_color(axis, color, picker_x, picker_y, picker_bar);

            RectF square = color_picker_square_rect(rect);
            RectF bar = color_picker_bar_rect(rect);
            RectF current_rect = color_picker_current_rect(rect);
            RectF original_rect = color_picker_original_rect(rect);

            if(axis == 0)
            {
                Float4U hue_color = color_hsv_to_rgb(picker_bar, 1.0f, 1.0f, 1.0f);
                ctx.draw_gradient_rect(square, clip_rect, Float4U(1.0f), hue_color, hue_color, Float4U(1.0f));
                ctx.draw_gradient_rect(square, clip_rect, Float4U(0.0f), Float4U(0.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
            }
            else if(axis == 1)
            {
                for(u32 i = 0; i < 6; ++i)
                {
                    f32 x0 = (f32)i / 6.0f;
                    f32 x1 = (f32)(i + 1) / 6.0f;
                    RectF segment(square.offset_x + square.width * x0, square.offset_y, square.width * (x1 - x0) + 0.5f, square.height);
                    Float4U left = color_from_picker_channels(axis, x0, 1.0f, picker_bar, 1.0f);
                    Float4U right = color_from_picker_channels(axis, x1, 1.0f, picker_bar, 1.0f);
                    ctx.draw_gradient_rect(segment, clip_rect, left, right, right, left);
                    ctx.draw_gradient_rect(segment, clip_rect, Float4U(0.0f), Float4U(0.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                }
            }
            else if(axis == 2)
            {
                Float4U gray(picker_bar, picker_bar, picker_bar, 1.0f);
                for(u32 i = 0; i < 6; ++i)
                {
                    f32 x0 = (f32)i / 6.0f;
                    f32 x1 = (f32)(i + 1) / 6.0f;
                    RectF segment(square.offset_x + square.width * x0, square.offset_y, square.width * (x1 - x0) + 0.5f, square.height);
                    Float4U left = color_from_picker_channels(axis, x0, 1.0f, picker_bar, 1.0f);
                    Float4U right = color_from_picker_channels(axis, x1, 1.0f, picker_bar, 1.0f);
                    ctx.draw_gradient_rect(segment, clip_rect, left, right, right, left);
                    ctx.draw_gradient_rect(segment, clip_rect, Float4U(gray.x, gray.y, gray.z, 0.0f), Float4U(gray.x, gray.y, gray.z, 0.0f), gray, gray);
                }
            }
            else
            {
                ctx.draw_gradient_rect(square, clip_rect,
                    color_from_picker_channels(axis, 0.0f, 1.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 1.0f, 1.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 1.0f, 0.0f, picker_bar, 1.0f),
                    color_from_picker_channels(axis, 0.0f, 0.0f, picker_bar, 1.0f));
            }
            draw_square_border(ctx, square, clip_rect, style_f32x4(ctx, node, Name("gui.color_picker.border"), Float4U(0.24f, 0.29f, 0.36f, 1.0f)));

            if(axis == 0)
            {
                for(u32 i = 0; i < 6; ++i)
                {
                    f32 y0 = (f32)i / 6.0f;
                    f32 y1 = (f32)(i + 1) / 6.0f;
                    RectF segment(bar.offset_x, bar.offset_y + bar.height * y0, bar.width, bar.height * (y1 - y0) + 0.5f);
                    Float4U top_color = color_hsv_to_rgb(y0, 1.0f, 1.0f, 1.0f);
                    Float4U bottom_color = color_hsv_to_rgb(y1, 1.0f, 1.0f, 1.0f);
                    ctx.draw_gradient_rect(segment, clip_rect, top_color, top_color, bottom_color, bottom_color);
                }
            }
            else
            {
                Float4U top_color = color_from_picker_channels(axis, picker_x, picker_y, 1.0f, 1.0f);
                Float4U bottom_color = color_from_picker_channels(axis, picker_x, picker_y, 0.0f, 1.0f);
                ctx.draw_gradient_rect(bar, clip_rect, top_color, top_color, bottom_color, bottom_color);
            }

            f32 cursor_x = square.offset_x + picker_x * square.width;
            f32 cursor_y = square.offset_y + (1.0f - picker_y) * square.height;
            ctx.draw_circle(RectF(cursor_x - 8.0f, cursor_y - 8.0f, 16.0f, 16.0f), clip_rect, Float4U(1.0f));
            ctx.draw_circle(RectF(cursor_x - 5.0f, cursor_y - 5.0f, 10.0f, 10.0f), clip_rect, color);
            f32 bar_y = bar.offset_y + (axis == 0 ? picker_bar : (1.0f - picker_bar)) * bar.height;
            ctx.draw_line(Float2U(bar.offset_x - 5.0f, bar_y), Float2U(bar.offset_x + bar.width + 5.0f, bar_y), clip_rect, Float4U(1.0f), 2.0f);

            ctx.draw_text(RectF(current_rect.offset_x, current_rect.offset_y - 26.0f, current_rect.width, 22.0f), clip_rect, "Current", 15.0f, Float4U(1.0f), TextAlignment::begin);
            draw_color_swatch(ctx, current_rect, clip_rect, color, 3.0f);
            ctx.draw_text(RectF(original_rect.offset_x, original_rect.offset_y - 26.0f, original_rect.width, 22.0f), clip_rect, "Original", 15.0f, Float4U(1.0f), TextAlignment::begin);
            draw_color_swatch(ctx, original_rect, clip_rect, state.color_picker_original_valid ? state.color_picker_original : color, 3.0f);
        }

        RenderProxyDesc default_color_picker_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_color_picker;
            return desc;
        }
    }
}
