/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ColorEdit.cpp
* @author JXMaster
* @date 2026/7/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cmath>
#include <cstdio>

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            struct ColorBinding
            {
                f32* f32_value = nullptr;
                u8* u8_value = nullptr;
                u32* rgba8_value = nullptr;
                ColorStorage storage = ColorStorage::f32;
                u8 value_count = 3;
            };

            struct PreviewData
            {
                ColorBinding binding;
                c8* hex = nullptr;
                bool enabled = true;
            };

            struct PickerData
            {
                ColorBinding binding;
                ColorEditState* state = nullptr;
            };

            struct PickerGeometry
            {
                RectF square = RectF(0.0f, 0.0f, 300.0f, 300.0f);
                RectF bar = RectF(310.0f, 0.0f, 26.0f, 300.0f);
                RectF current = RectF(350.0f, 30.0f, 100.0f, 72.0f);
                RectF original = RectF(350.0f, 170.0f, 100.0f, 72.0f);
            };

            static u8 color_to_u8(f32 value)
            {
                return (u8)clamp(value * 255.0f + 0.5f, 0.0f, 255.0f);
            }

            static f32 color_from_u8(u8 value)
            {
                return (f32)value / 255.0f;
            }

            static Float4U read_color(const ColorBinding& binding)
            {
                if(binding.storage == ColorStorage::u8 && binding.u8_value)
                {
                    return Float4U(color_from_u8(binding.u8_value[0]), color_from_u8(binding.u8_value[1]),
                        color_from_u8(binding.u8_value[2]), binding.value_count == 4 ?
                        color_from_u8(binding.u8_value[3]) : 1.0f);
                }
                if(binding.storage == ColorStorage::rgba8 && binding.rgba8_value)
                {
                    u32 value = *binding.rgba8_value;
                    return Float4U(color_from_u8((u8)(value & 0xffu)), color_from_u8((u8)((value >> 8) & 0xffu)),
                        color_from_u8((u8)((value >> 16) & 0xffu)), binding.value_count == 4 ?
                        color_from_u8((u8)((value >> 24) & 0xffu)) : 1.0f);
                }
                if(binding.f32_value)
                {
                    return Float4U(clamp(binding.f32_value[0], 0.0f, 1.0f),
                        clamp(binding.f32_value[1], 0.0f, 1.0f), clamp(binding.f32_value[2], 0.0f, 1.0f),
                        binding.value_count == 4 ? clamp(binding.f32_value[3], 0.0f, 1.0f) : 1.0f);
                }
                return Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            }

            static bool write_color(const ColorBinding& binding, Float4U color)
            {
                color.x = clamp(color.x, 0.0f, 1.0f);
                color.y = clamp(color.y, 0.0f, 1.0f);
                color.z = clamp(color.z, 0.0f, 1.0f);
                color.w = binding.value_count == 4 ? clamp(color.w, 0.0f, 1.0f) : 1.0f;
                Float4U old = read_color(binding);
                if(binding.storage == ColorStorage::u8 && binding.u8_value)
                {
                    binding.u8_value[0] = color_to_u8(color.x);
                    binding.u8_value[1] = color_to_u8(color.y);
                    binding.u8_value[2] = color_to_u8(color.z);
                    if(binding.value_count == 4) binding.u8_value[3] = color_to_u8(color.w);
                }
                else if(binding.storage == ColorStorage::rgba8 && binding.rgba8_value)
                {
                    u32 r = color_to_u8(color.x);
                    u32 g = color_to_u8(color.y);
                    u32 b = color_to_u8(color.z);
                    u32 a = binding.value_count == 4 ? color_to_u8(color.w) : 255u;
                    *binding.rgba8_value = r | (g << 8) | (b << 16) | (a << 24);
                }
                else if(binding.f32_value)
                {
                    binding.f32_value[0] = color.x;
                    binding.f32_value[1] = color.y;
                    binding.f32_value[2] = color.z;
                    if(binding.value_count == 4) binding.f32_value[3] = color.w;
                }
                return old.x != color.x || old.y != color.y || old.z != color.z || old.w != color.w;
            }

            static void rgb_to_hsv(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v)
            {
                f32 maximum = max(max(r, g), b);
                f32 minimum = min(min(r, g), b);
                f32 delta = maximum - minimum;
                v = maximum;
                s = maximum <= 0.0f ? 0.0f : delta / maximum;
                if(delta <= 0.000001f) h = 0.0f;
                else if(maximum == r)
                {
                    h = (g - b) / delta;
                    if(h < 0.0f) h += 6.0f;
                    h /= 6.0f;
                }
                else if(maximum == g) h = ((b - r) / delta + 2.0f) / 6.0f;
                else h = ((r - g) / delta + 4.0f) / 6.0f;
                h = clamp(h, 0.0f, 1.0f);
            }

            static Float4U hsv_to_rgb(f32 h, f32 s, f32 v, f32 a)
            {
                h = clamp(h, 0.0f, 1.0f);
                s = clamp(s, 0.0f, 1.0f);
                v = clamp(v, 0.0f, 1.0f);
                f32 r = v;
                f32 g = v;
                f32 b = v;
                if(s > 0.0f)
                {
                    f32 scaled = h * 6.0f;
                    i32 sector = (i32)floorf(scaled);
                    f32 fraction = scaled - (f32)sector;
                    f32 p = v * (1.0f - s);
                    f32 q = v * (1.0f - s * fraction);
                    f32 t = v * (1.0f - s * (1.0f - fraction));
                    switch(sector % 6)
                    {
                    case 0: r = v; g = t; b = p; break;
                    case 1: r = q; g = v; b = p; break;
                    case 2: r = p; g = v; b = t; break;
                    case 3: r = p; g = q; b = v; break;
                    case 4: r = t; g = p; b = v; break;
                    default: r = v; g = p; b = q; break;
                    }
                }
                return Float4U(r, g, b, clamp(a, 0.0f, 1.0f));
            }

            static void picker_channels(i32 axis, const Float4U& color, f32& x, f32& y, f32& bar)
            {
                axis = clamp(axis, 0, 5);
                if(axis < 3)
                {
                    f32 h = 0.0f, s = 0.0f, v = 0.0f;
                    rgb_to_hsv(color.x, color.y, color.z, h, s, v);
                    if(axis == 0) { x = s; y = v; bar = h; }
                    else if(axis == 1) { x = h; y = v; bar = s; }
                    else { x = h; y = s; bar = v; }
                }
                else if(axis == 3) { x = color.y; y = color.z; bar = color.x; }
                else if(axis == 4) { x = color.x; y = color.z; bar = color.y; }
                else { x = color.x; y = color.y; bar = color.z; }
            }

            static Float4U color_from_picker(i32 axis, f32 x, f32 y, f32 bar, f32 alpha)
            {
                axis = clamp(axis, 0, 5);
                x = clamp(x, 0.0f, 1.0f);
                y = clamp(y, 0.0f, 1.0f);
                bar = clamp(bar, 0.0f, 1.0f);
                if(axis == 0) return hsv_to_rgb(bar, x, y, alpha);
                if(axis == 1) return hsv_to_rgb(x, bar, y, alpha);
                if(axis == 2) return hsv_to_rgb(x, y, bar, alpha);
                if(axis == 3) return Float4U(bar, x, y, alpha);
                if(axis == 4) return Float4U(x, bar, y, alpha);
                return Float4U(x, y, bar, alpha);
            }

            static void ensure_color_state(ColorEditState& state)
            {
                if(state.rgb.size() != 4) state.rgb.resize(4, 0);
                if(state.hsv.size() != 3) state.hsv.resize(3, 0);
            }

            static void sync_color_state(ColorEditState& state, const Float4U& color)
            {
                ensure_color_state(state);
                state.rgb[0] = color_to_u8(color.x);
                state.rgb[1] = color_to_u8(color.y);
                state.rgb[2] = color_to_u8(color.z);
                state.rgb[3] = color_to_u8(color.w);
                f32 h = 0.0f, s = 0.0f, v = 0.0f;
                rgb_to_hsv(color.x, color.y, color.z, h, s, v);
                state.hsv[0] = color_to_u8(h);
                state.hsv[1] = color_to_u8(s);
                state.hsv[2] = color_to_u8(v);
            }

            static void draw_rect(GUI::IContext* context, GUI::DrawCommandType type, const RectF& rect,
                const Float4U& color, f32 radius = 0.0f)
            {
                GUI::DrawCommand command;
                command.type = type;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                context->draw(command);
            }

            static void draw_gradient(GUI::IContext* context, const RectF& rect, const Float4U& top_left,
                const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left)
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::gradient_rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = top_left;
                command.color_top_right = top_right;
                command.color_bottom_right = bottom_right;
                command.color_bottom_left = bottom_left;
                context->draw(command);
            }

            static void draw_outline(GUI::IContext* context, const RectF& rect, const Float4U& color)
            {
                const Float2U points[] =
                {
                    Float2U(rect.offset_x, rect.offset_y), Float2U(rect.offset_x + rect.width, rect.offset_y),
                    Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height),
                    Float2U(rect.offset_x, rect.offset_y + rect.height)
                };
                for(u32 i = 0; i < 4; ++i)
                {
                    GUI::DrawCommand command;
                    command.type = GUI::DrawCommandType::line;
                    command.rect_reference = GUI::DrawCommandRectReference::element;
                    command.rect = RectF(points[i].x, points[i].y, 0.0f, 0.0f);
                    command.point1 = points[(i + 1) & 3];
                    command.color = color;
                    command.line_width = 1.0f;
                    context->draw(command);
                }
            }

            static void draw_label(GUI::IContext* context, const RectF& rect, const c8* text,
                const Float4U& color, f32 font_size, VG::TextAlignment alignment = VG::TextAlignment::begin)
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::text;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.text = text ? text : "";
                command.font = style_name(context, GUI::ElementHandle(), "gui.font");
                command.font_size = font_size;
                command.color = color;
                command.horizontal_alignment = alignment;
                command.vertical_alignment = VG::TextAlignment::center;
                context->draw(command);
            }

            static void draw_swatch(GUI::IContext* context, const RectF& rect, const Float4U& color,
                const Float4U& border)
            {
                draw_rect(context, GUI::DrawCommandType::rounded_rect, rect, border, 3.0f);
                RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f),
                    max(rect.height - 2.0f, 1.0f));
                constexpr f32 cell_size = 8.0f;
                u32 columns = max((u32)ceilf(inner.width / cell_size), 1u);
                u32 rows = max((u32)ceilf(inner.height / cell_size), 1u);
                for(u32 y = 0; y < rows; ++y)
                {
                    for(u32 x = 0; x < columns; ++x)
                    {
                        RectF cell(inner.offset_x + (f32)x * cell_size, inner.offset_y + (f32)y * cell_size,
                            min(cell_size, inner.width - (f32)x * cell_size),
                            min(cell_size, inner.height - (f32)y * cell_size));
                        draw_rect(context, GUI::DrawCommandType::rect, cell,
                            (x + y) & 1 ? Float4U(0.20f, 0.23f, 0.28f, 1.0f) :
                            Float4U(0.40f, 0.44f, 0.50f, 1.0f));
                    }
                }
                draw_rect(context, GUI::DrawCommandType::rounded_rect, inner, color, 2.0f);
            }

            static bool rect_contains(const RectF& rect, const Float2U& point)
            {
                return point.x >= rect.offset_x && point.y >= rect.offset_y &&
                    point.x <= rect.offset_x + rect.width && point.y <= rect.offset_y + rect.height;
            }

            static GUI::MeasureResult measure_color_preview(GUI::IContext* context,
                const GUI::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 30.0f);
                GUI::MeasureResult result;
                result.minimum = Float2U(86.0f, height);
                result.desired = Float2U(150.0f, height);
                return result;
            }

            static RV draw_color_preview(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                PreviewData* data = (PreviewData*)userdata;
                if(!data) return ok;
                GUI::InteractionState interaction = context->get_interaction_state(element.id);
                Float4U border = style_color(context, element, "gui.color_edit.border", Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                Float4U background = style_color(context, element, data->enabled && interaction.hovered ?
                    "gui.color_edit.background_hovered" : "gui.color_edit.background", data->enabled && interaction.hovered ?
                    Float4U(0.13f, 0.19f, 0.27f, 1.0f) : Float4U(0.10f, 0.13f, 0.18f, 1.0f));
                f32 radius = style_scalar(context, element, "gui.color_edit.radius", 4.0f);
                draw_rect(context, GUI::DrawCommandType::rounded_rect, RectF(), border, radius);
                draw_rect(context, GUI::DrawCommandType::rounded_rect, RectF(1.0f, 1.0f, -2.0f, -2.0f), background,
                    max(radius - 1.0f, 0.0f));
                draw_swatch(context, RectF(6.0f, 4.0f, 22.0f, 22.0f), read_color(data->binding), border);
                draw_label(context, RectF(36.0f, 0.0f, -44.0f, 0.0f), data->hex,
                    style_color(context, element, data->enabled ? "gui.text.color" : "gui.text.disabled",
                    data->enabled ? Float4U(0.86f, 0.88f, 0.92f, 1.0f) : Float4U(0.48f, 0.52f, 0.58f, 1.0f)),
                    style_scalar(context, element, "gui.text.font_size", 15.0f));
                return ok;
            }

            static RV draw_color_picker(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                PickerData* data = (PickerData*)userdata;
                if(!data || !data->state) return ok;
                PickerGeometry geometry;
                Float4U color = read_color(data->binding);
                i32 axis = clamp(data->state->axis, 0, 5);
                f32 x = 0.0f, y = 0.0f, bar = 0.0f;
                picker_channels(axis, color, x, y, bar);
                if(axis == 0)
                {
                    Float4U hue = hsv_to_rgb(bar, 1.0f, 1.0f, 1.0f);
                    draw_gradient(context, geometry.square, Float4U(1.0f), hue, hue, Float4U(1.0f));
                    draw_gradient(context, geometry.square, Float4U(0.0f), Float4U(0.0f),
                        Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                }
                else if(axis < 3)
                {
                    for(u32 i = 0; i < 6; ++i)
                    {
                        f32 x0 = (f32)i / 6.0f;
                        f32 x1 = (f32)(i + 1) / 6.0f;
                        RectF segment(geometry.square.offset_x + geometry.square.width * x0, geometry.square.offset_y,
                            geometry.square.width * (x1 - x0) + 0.5f, geometry.square.height);
                        Float4U left = color_from_picker(axis, x0, 1.0f, bar, 1.0f);
                        Float4U right = color_from_picker(axis, x1, 1.0f, bar, 1.0f);
                        draw_gradient(context, segment, left, right, right, left);
                        if(axis == 1)
                        {
                            draw_gradient(context, segment, Float4U(0.0f), Float4U(0.0f),
                                Float4U(0.0f, 0.0f, 0.0f, 1.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f));
                        }
                        else
                        {
                            Float4U gray(bar, bar, bar, 1.0f);
                            draw_gradient(context, segment, Float4U(gray.x, gray.y, gray.z, 0.0f),
                                Float4U(gray.x, gray.y, gray.z, 0.0f), gray, gray);
                        }
                    }
                }
                else
                {
                    draw_gradient(context, geometry.square, color_from_picker(axis, 0.0f, 1.0f, bar, 1.0f),
                        color_from_picker(axis, 1.0f, 1.0f, bar, 1.0f), color_from_picker(axis, 1.0f, 0.0f, bar, 1.0f),
                        color_from_picker(axis, 0.0f, 0.0f, bar, 1.0f));
                }
                Float4U border = style_color(context, element, "gui.color_picker.border", Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                draw_outline(context, geometry.square, border);
                if(axis == 0)
                {
                    for(u32 i = 0; i < 6; ++i)
                    {
                        f32 y0 = (f32)i / 6.0f;
                        f32 y1 = (f32)(i + 1) / 6.0f;
                        RectF segment(geometry.bar.offset_x, geometry.bar.offset_y + geometry.bar.height * y0,
                            geometry.bar.width, geometry.bar.height * (y1 - y0) + 0.5f);
                        Float4U top = hsv_to_rgb(y0, 1.0f, 1.0f, 1.0f);
                        Float4U bottom = hsv_to_rgb(y1, 1.0f, 1.0f, 1.0f);
                        draw_gradient(context, segment, top, top, bottom, bottom);
                    }
                }
                else
                {
                    Float4U top = color_from_picker(axis, x, y, 1.0f, 1.0f);
                    Float4U bottom = color_from_picker(axis, x, y, 0.0f, 1.0f);
                    draw_gradient(context, geometry.bar, top, top, bottom, bottom);
                }
                draw_outline(context, geometry.bar, border);
                f32 cursor_x = geometry.square.offset_x + x * geometry.square.width;
                f32 cursor_y = geometry.square.offset_y + (1.0f - y) * geometry.square.height;
                Float4U cursor = style_color(context, element, "gui.color_picker.cursor", Float4U(1.0f));
                draw_rect(context, GUI::DrawCommandType::rounded_rect, RectF(cursor_x - 8.0f, cursor_y - 8.0f, 16.0f, 16.0f),
                    cursor, 8.0f);
                draw_rect(context, GUI::DrawCommandType::rounded_rect, RectF(cursor_x - 5.0f, cursor_y - 5.0f, 10.0f, 10.0f),
                    color, 5.0f);
                f32 bar_y = geometry.bar.offset_y + (axis == 0 ? bar : (1.0f - bar)) * geometry.bar.height;
                GUI::DrawCommand marker;
                marker.type = GUI::DrawCommandType::line;
                marker.rect_reference = GUI::DrawCommandRectReference::element;
                marker.rect = RectF(geometry.bar.offset_x - 5.0f, bar_y, 0.0f, 0.0f);
                marker.point1 = Float2U(geometry.bar.offset_x + geometry.bar.width + 5.0f, bar_y);
                marker.color = cursor;
                marker.line_width = 2.0f;
                context->draw(marker);
                Float4U text_color = style_color(context, element, "gui.text.color", Float4U(0.86f, 0.88f, 0.92f, 1.0f));
                f32 text_size = style_scalar(context, element, "gui.text.font_size", 15.0f);
                draw_label(context, RectF(geometry.current.offset_x, geometry.current.offset_y - 26.0f,
                    geometry.current.width, 22.0f), "Current", text_color, text_size);
                draw_swatch(context, geometry.current, color, border);
                draw_label(context, RectF(geometry.original.offset_x, geometry.original.offset_y - 26.0f,
                    geometry.original.width, 22.0f), "Original", text_color, text_size);
                draw_swatch(context, geometry.original, data->state->original_valid ? data->state->original : color, border);
                return ok;
            }

            static void build_channel_row(GUI::IContext* context, id_t id, const c8* const* labels,
                i32* values, u32 count)
            {
                GUI::LayoutConfig row_layout;
                row_layout.width.kind = GUI::SizeKind::fixed;
                row_layout.width.value = 456.0f;
                row_layout.height.kind = GUI::SizeKind::fixed;
                row_layout.height.value = 30.0f;
                GUI::ElementHandle row = begin_h_layout(context, id, "Color Channels", row_layout);
                for(u32 i = 0; i < count; ++i)
                {
                    GUI::LayoutConfig pair_layout;
                    pair_layout.width.kind = GUI::SizeKind::fixed;
                    pair_layout.width.value = count == 1 ? 456.0f : 146.0f;
                    pair_layout.height.kind = GUI::SizeKind::fixed;
                    pair_layout.height.value = 30.0f;
                    GUI::ElementHandle pair = begin_h_layout(context, GUI::make_scoped_id(id, (u64)i + 1),
                        "Color Channel", pair_layout);
                    GUI::LayoutConfig label_layout;
                    label_layout.width.kind = GUI::SizeKind::fixed;
                    label_layout.width.value = 20.0f;
                    label_layout.height.kind = GUI::SizeKind::fixed;
                    label_layout.height.value = 30.0f;
                    text(context, GUI::make_scoped_id(id, (u64)i + 11), labels[i], label_layout);
                    GUI::LayoutConfig drag_layout;
                    drag_layout.width.kind = GUI::SizeKind::fixed;
                    drag_layout.width.value = count == 1 ? 426.0f : 116.0f;
                    drag_layout.height.kind = GUI::SizeKind::fixed;
                    drag_layout.height.value = 30.0f;
                    DragDesc drag_desc;
                    drag_desc.speed = 1.0f;
                    drag_int(context, GUI::make_scoped_id(id, (u64)i + 31), &values[i], 0, 255, drag_layout, drag_desc);
                    GUI::FlexLayoutDesc flex;
                    flex.axis = GUI::LayoutAxis::x;
                    flex.cross_alignment = GUI::FlexAlignment::stretch;
                    flex.main_axis_gap = 4.0f;
                    end_h_layout(context, pair, flex);
                }
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.main_axis_gap = count == 1 ? 0.0f : 9.0f;
                flex.cross_alignment = GUI::FlexAlignment::stretch;
                end_h_layout(context, row, flex);
            }

            static void add_canvas_item(GUI::CanvasLayoutItem& item, id_t id, f32 x, f32 y)
            {
                item.element_id = id;
                item.anchor_min = Float2U(0.0f);
                item.anchor_max = Float2U(0.0f);
                item.offset = Float4U(x, y, 0.0f, 0.0f);
                item.pivot = Float2U(0.0f);
            }

            static void build_color_popup(GUI::IContext* context, id_t id, const ColorBinding& binding,
                ColorEditState* state, const ColorEditDesc& desc)
            {
                id_t popup_id = derived_id(id, "color_edit.popup");
                if(!is_popup_open(context, popup_id)) return;
                constexpr f32 default_width = 470.0f;
                f32 canvas_height = binding.value_count == 4 ? 454.0f : 416.0f;
                f32 popup_width = desc.popup_width > 0.0f ? max(desc.popup_width, default_width) : default_width;
                f32 popup_height = canvas_height + 14.0f;
                Float2U screen = context->get_frame_desc().screen_size;
                PopupDesc popup_desc;
                popup_desc.position.x = clamp(state->popup_position.x, 0.0f, max(screen.x - popup_width, 0.0f));
                popup_desc.position.y = clamp(state->popup_position.y, 0.0f, max(screen.y - popup_height, 0.0f));
                popup_desc.layout.width.kind = GUI::SizeKind::fixed;
                popup_desc.layout.width.value = popup_width;
                popup_desc.layout.height.kind = GUI::SizeKind::fixed;
                popup_desc.layout.height.value = popup_height;
                GUI::ElementHandle popup;
                if(!begin_popup(context, popup_id, popup_desc, &popup)) return;
                GUI::LayoutConfig canvas_layout;
                canvas_layout.width.kind = GUI::SizeKind::fixed;
                canvas_layout.width.value = 456.0f;
                canvas_layout.height.kind = GUI::SizeKind::fixed;
                canvas_layout.height.value = canvas_height;
                GUI::ElementHandle canvas = begin_canvas_layout(context, derived_id(id, "color_edit.canvas"),
                    "Color Edit Picker", canvas_layout);
                id_t picker_id = derived_id(id, "color_edit.picker");
                GUI::LayoutConfig picker_layout;
                picker_layout.width.kind = GUI::SizeKind::fixed;
                picker_layout.width.value = 456.0f;
                picker_layout.height.kind = GUI::SizeKind::fixed;
                picker_layout.height.value = 300.0f;
                GUI::ElementHandle picker = begin_element(context, picker_id, "Color Picker", picker_layout);
                set_interactable(context, picker, desc.enabled);
                PickerData* picker_data = allocate_frame<PickerData>(context);
                picker_data->binding = binding;
                picker_data->state = state;
                GUI::DrawConfig picker_draw;
                picker_draw.name = Name("gui.color_picker");
                picker_draw.callback = draw_color_picker;
                picker_draw.userdata = picker_data;
                context->set_draw_config(picker, picker_draw);
                context->end_element();

                const c8* axis_labels[] = { "H", "S", "V", "R", "G", "B" };
                id_t axis_id = derived_id(id, "color_edit.axis");
                GUI::LayoutConfig axis_layout;
                axis_layout.width.kind = GUI::SizeKind::fixed;
                axis_layout.width.value = 456.0f;
                axis_layout.height.kind = GUI::SizeKind::fixed;
                axis_layout.height.value = 30.0f;
                button_group(context, axis_id, Span<const c8*>(axis_labels, 6), &state->axis, axis_layout);

                const c8* rgb_labels[] = { "R", "G", "B" };
                const c8* hsv_labels[] = { "H", "S", "V" };
                id_t rgb_id = derived_id(id, "color_edit.rgb");
                id_t hsv_id = derived_id(id, "color_edit.hsv");
                build_channel_row(context, rgb_id, rgb_labels, state->rgb.data(), 3);
                build_channel_row(context, hsv_id, hsv_labels, state->hsv.data(), 3);
                id_t alpha_id = 0;
                if(binding.value_count == 4)
                {
                    const c8* alpha_labels[] = { "A" };
                    alpha_id = derived_id(id, "color_edit.alpha");
                    build_channel_row(context, alpha_id, alpha_labels, state->rgb.data() + 3, 1);
                }
                GUI::CanvasLayoutItem items[5];
                add_canvas_item(items[0], picker_id, 0.0f, 0.0f);
                add_canvas_item(items[1], axis_id, 0.0f, 310.0f);
                add_canvas_item(items[2], rgb_id, 0.0f, 348.0f);
                add_canvas_item(items[3], hsv_id, 0.0f, 386.0f);
                if(alpha_id) add_canvas_item(items[4], alpha_id, 0.0f, 424.0f);
                GUI::CanvasLayoutDesc canvas_desc;
                canvas_desc.items = Span<const GUI::CanvasLayoutItem>(items, alpha_id ? 5 : 4);
                canvas_desc.clip_children = false;
                end_canvas_layout(context, canvas, canvas_desc);
                lupanic_if_failed(end_popup(context, popup, RectF(0.0f, 0.0f, popup_width, popup_height)));
            }

            static void apply_picker_position(const ColorBinding& binding, ColorEditState& state, u32 part,
                const Float2U& position)
            {
                PickerGeometry geometry;
                Float4U color = read_color(binding);
                f32 x = 0.0f, y = 0.0f, bar = 0.0f;
                picker_channels(state.axis, color, x, y, bar);
                if(part == 1)
                {
                    x = clamp((position.x - geometry.square.offset_x) / max(geometry.square.width, 1.0f), 0.0f, 1.0f);
                    y = clamp(1.0f - (position.y - geometry.square.offset_y) / max(geometry.square.height, 1.0f), 0.0f, 1.0f);
                }
                else if(part == 2)
                {
                    f32 t = clamp((position.y - geometry.bar.offset_y) / max(geometry.bar.height, 1.0f), 0.0f, 1.0f);
                    bar = state.axis == 0 ? t : 1.0f - t;
                }
                write_color(binding, color_from_picker(state.axis, x, y, bar, color.w));
                sync_color_state(state, read_color(binding));
            }

            bool resolve_color_edit_action(GUI::IContext* context, ColorEditAction& action)
            {
                if(!action.state) return false;
                ColorBinding binding;
                binding.f32_value = action.f32_value;
                binding.u8_value = action.u8_value;
                binding.rgba8_value = action.rgba8_value;
                binding.storage = action.storage;
                binding.value_count = action.value_count;
                ColorEditState& state = *action.state;
                ensure_color_state(state);
                bool changed = false;
                GUI::InteractionState preview = context->get_interaction_state(action.id);
                if(action.enabled && preview.clicked)
                {
                    if(is_popup_open(context, action.popup_id)) close_popup(context, action.popup_id);
                    else
                    {
                        state.original = read_color(binding);
                        state.original_valid = true;
                        state.popup_position = Float2U(preview.clicked_screen_position.x - preview.clicked_element_position.x,
                            preview.clicked_screen_position.y - preview.clicked_element_position.y +
                            preview.clicked_element_rect.height + 6.0f);
                        open_popup(context, action.popup_id);
                    }
                }
                if(action.enabled)
                {
                    PickerGeometry geometry;
                    for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(action.picker_id))
                    {
                        if(!routed.has_element_position) continue;
                        if(routed.event.type == GUI::InputEventType::pointer_down &&
                            routed.event.button == GUI::PointerButton::left)
                        {
                            if(rect_contains(geometry.square, routed.element_position)) state.active_part = 1;
                            else if(rect_contains(geometry.bar, routed.element_position)) state.active_part = 2;
                            else if(rect_contains(geometry.original, routed.element_position) && state.original_valid)
                            {
                                changed |= write_color(binding, state.original);
                                sync_color_state(state, read_color(binding));
                                state.active_part = 0;
                            }
                            if(state.active_part)
                            {
                                Float4U before = read_color(binding);
                                apply_picker_position(binding, state, state.active_part, routed.element_position);
                                Float4U after = read_color(binding);
                                changed |= before.x != after.x || before.y != after.y || before.z != after.z || before.w != after.w;
                            }
                        }
                        else if(routed.event.type == GUI::InputEventType::pointer_move && state.active_part)
                        {
                            Float4U before = read_color(binding);
                            apply_picker_position(binding, state, state.active_part, routed.element_position);
                            Float4U after = read_color(binding);
                            changed |= before.x != after.x || before.y != after.y || before.z != after.z || before.w != after.w;
                        }
                        else if(routed.event.type == GUI::InputEventType::pointer_up &&
                            routed.event.button == GUI::PointerButton::left)
                        {
                            state.active_part = 0;
                        }
                    }
                    bool rgb_changed = false;
                    bool hsv_changed = false;
                    for(u32 i = 0; i < 4; ++i) rgb_changed |= state.rgb[i] != action.rgb_before[i];
                    for(u32 i = 0; i < 3; ++i) hsv_changed |= state.hsv[i] != action.hsv_before[i];
                    if(rgb_changed)
                    {
                        Float4U value(color_from_u8((u8)clamp(state.rgb[0], 0, 255)),
                            color_from_u8((u8)clamp(state.rgb[1], 0, 255)),
                            color_from_u8((u8)clamp(state.rgb[2], 0, 255)),
                            color_from_u8((u8)clamp(state.rgb[3], 0, 255)));
                        changed |= write_color(binding, value);
                        sync_color_state(state, read_color(binding));
                    }
                    else if(hsv_changed)
                    {
                        Float4U value = hsv_to_rgb(color_from_u8((u8)clamp(state.hsv[0], 0, 255)),
                            color_from_u8((u8)clamp(state.hsv[1], 0, 255)),
                            color_from_u8((u8)clamp(state.hsv[2], 0, 255)), read_color(binding).w);
                        changed |= write_color(binding, value);
                        sync_color_state(state, read_color(binding));
                    }
                }
                return changed;
            }

            static GUI::ElementHandle color_edit(GUI::IContext* context, id_t id, const c8* label,
                const ColorBinding& binding, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
            {
                luassert(context && id);
                id_t popup_id = derived_id(id, "color_edit.popup");
                Ref<ColorEditState> state = widget_state<ColorEditState>(context, id);
                ensure_color_state(*state);
                if(!state->initialized || !is_popup_open(context, popup_id))
                {
                    sync_color_state(*state, read_color(binding));
                    state->initialized = true;
                }
                state->axis = clamp(state->axis, 0, 5);
                GUI::ElementHandle preview = begin_element(context, id, label ? label : "Color Edit", layout);
                set_interactable(context, preview, desc.enabled);
                c8 hex[16];
                Float4U color = read_color(binding);
                if(binding.value_count == 4)
                {
                    snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X", color_to_u8(color.x), color_to_u8(color.y),
                        color_to_u8(color.z), color_to_u8(color.w));
                }
                else
                {
                    snprintf(hex, sizeof(hex), "#%02X%02X%02X", color_to_u8(color.x), color_to_u8(color.y), color_to_u8(color.z));
                }
                PreviewData* preview_data = allocate_frame<PreviewData>(context);
                preview_data->binding = binding;
                preview_data->hex = copy_frame_string(context, hex);
                preview_data->enabled = desc.enabled;
                GUI::LayoutCallbackConfig preview_layout;
                preview_layout.algorithm = Name("gui.color_edit");
                preview_layout.measure_callback = measure_color_preview;
                context->set_layout_callback_config(preview, preview_layout);
                GUI::DrawConfig preview_draw;
                preview_draw.name = Name("gui.color_edit");
                preview_draw.callback = draw_color_preview;
                preview_draw.userdata = preview_data;
                context->set_draw_config(preview, preview_draw);
                context->end_element();
                build_color_popup(context, id, binding, state.get(), desc);
                ColorEditAction* action = allocate_frame<ColorEditAction>(context);
                action->id = id;
                action->popup_id = popup_id;
                action->picker_id = derived_id(id, "color_edit.picker");
                action->f32_value = binding.f32_value;
                action->u8_value = binding.u8_value;
                action->rgba8_value = binding.rgba8_value;
                action->storage = binding.storage;
                action->value_count = binding.value_count;
                action->enabled = desc.enabled;
                for(u32 i = 0; i < 4; ++i) action->rgb_before[i] = state->rgb[i];
                for(u32 i = 0; i < 3; ++i) action->hsv_before[i] = state->hsv[i];
                action->state = state.get();
                add_action(context, ActionType::color_edit, id, action);
                return preview;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit3(GUI::IContext* context, id_t id, const c8* label,
            f32* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.f32_value = value;
            binding.storage = Internal::ColorStorage::f32;
            binding.value_count = 3;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit4(GUI::IContext* context, id_t id, const c8* label,
            f32* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.f32_value = value;
            binding.storage = Internal::ColorStorage::f32;
            binding.value_count = 4;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit3(GUI::IContext* context, id_t id, const c8* label,
            u8* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.u8_value = value;
            binding.storage = Internal::ColorStorage::u8;
            binding.value_count = 3;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit4(GUI::IContext* context, id_t id, const c8* label,
            u8* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.u8_value = value;
            binding.storage = Internal::ColorStorage::u8;
            binding.value_count = 4;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit3(GUI::IContext* context, id_t id, const c8* label,
            u32* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.rgba8_value = value;
            binding.storage = Internal::ColorStorage::rgba8;
            binding.value_count = 3;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle color_edit4(GUI::IContext* context, id_t id, const c8* label,
            u32* value, const GUI::LayoutConfig& layout, const ColorEditDesc& desc)
        {
            Internal::ColorBinding binding;
            binding.rgba8_value = value;
            binding.storage = Internal::ColorStorage::rgba8;
            binding.value_count = 4;
            return Internal::color_edit(context, id, label, binding, layout, desc);
        }
    }
}
