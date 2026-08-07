/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Showcase.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include "Showcase.hpp"
#include <Luna/VG/ShapeBuffer.hpp>
#include <cstdio>
#include <cstring>

namespace Luna
{
    namespace EditorGUITest
    {
        namespace
        {
            constexpr const c8* SECTION_LABELS[] = {
                "Overview", "Primitives", "Buttons", "Input", "Layouts",
                "Scroll Views", "Tables", "Overlay", "Workspace"
            };

            constexpr EditorGUI::IconName SECTION_ICONS[] = {
                EditorGUI::IconName::squares_four,
                EditorGUI::IconName::stack,
                EditorGUI::IconName::cursor_click,
                EditorGUI::IconName::sliders_horizontal,
                EditorGUI::IconName::grid_four,
                EditorGUI::IconName::columns,
                EditorGUI::IconName::list_bullets,
                EditorGUI::IconName::bell,
                EditorGUI::IconName::sidebar
            };

            GUI::LayoutConfig fixed(f32 width, f32 height)
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::fixed;
                layout.width.value = width;
                layout.height.kind = GUI::SizeKind::fixed;
                layout.height.value = height;
                layout.flex_shrink = 0.0f;
                return layout;
            }

            GUI::LayoutConfig fill()
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUI::SizeKind::percent;
                layout.height.value = 1.0f;
                return layout;
            }

            GUI::LayoutConfig fill_width(f32 height)
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUI::SizeKind::fixed;
                layout.height.value = height;
                layout.flex_shrink = 0.0f;
                return layout;
            }

            GUI::LayoutConfig grow()
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::fit;
                layout.height.kind = GUI::SizeKind::fit;
                layout.flex_grow = 1.0f;
                return layout;
            }

            GUI::LayoutConfig grow_x(f32 height = 0.0f)
            {
                GUI::LayoutConfig layout = grow();
                if(height > 0.0f)
                {
                    layout.height.kind = GUI::SizeKind::fixed;
                    layout.height.value = height;
                }
                else
                {
                    layout.height.kind = GUI::SizeKind::percent;
                    layout.height.value = 1.0f;
                }
                return layout;
            }

            GUI::LayoutConfig grow_y()
            {
                GUI::LayoutConfig layout = grow();
                layout.width.kind = GUI::SizeKind::percent;
                layout.width.value = 1.0f;
                return layout;
            }

            EditorGUI::id_t id(GUI::IContext* context, const c8* value)
            {
                return context->make_id(value);
            }

            Float4U style_color(GUI::IContext* context, const c8* entry, const Float4U& fallback = Float4U(1.0f))
            {
                return context->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name(entry),
                    GUI::style_f32x4(fallback)).number;
            }

            f32 style_scalar(GUI::IContext* context, const c8* entry, f32 fallback)
            {
                return context->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name(entry),
                    GUI::style_f32(fallback)).number.x;
            }

            void rounded_rect(GUI::IContext* context, const Float4U& color, f32 radius,
                const RectF& rect = RectF())
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::rounded_rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                context->draw(command);
            }

            void rounded_shadow(GUI::IContext* context, const Float4U& color, f32 radius,
                const Float2U& offset, f32 softness, GUI::ShadowMode mode,
                const RectF& rect = RectF())
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::shadow;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                command.shadow.offset = offset;
                command.shadow.softness = softness;
                command.shadow.mode = mode;
                context->draw(command);
            }

            RV draw_sdf_swatch(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                const GUI::Element* element_data = context->get_element(element.index);
                if(!element_data) return ok;
                const RectF& element_rect = element_data->layout_result.rect;
                f32 width = max(element_rect.width, 1.0f);
                f32 height = max(element_rect.height, 1.0f);
                u32 kind = (u32)(usize)userdata;
                Vector<f32> shape_floats;
                if(kind == 0)
                {
                    GUI::sdf_shape_add_rounded_rectangle(shape_floats,
                        RectF(5.0f, 5.0f, width - 10.0f, height - 10.0f),
                        Float4U(18.0f, 8.0f, 18.0f, 8.0f));
                }
                else if(kind == 1)
                {
                    GUI::sdf_shape_add_operation(shape_floats,
                        GUI::SDFShapeInstruction::difference_op);
                    GUI::sdf_shape_add_circle(shape_floats, Float2U(width * 0.5f, height * 0.5f),
                        min(width, height) * 0.42f);
                    GUI::sdf_shape_add_circle(shape_floats, Float2U(width * 0.5f, height * 0.5f),
                        min(width, height) * 0.21f);
                }
                else
                {
                    GUI::sdf_shape_add_capsule(shape_floats, Float2U(18.0f, height * 0.5f),
                        Float2U(width - 18.0f, height * 0.5f), min(height * 0.32f, 28.0f));
                }
                auto shape = context->append_sdf_shape_program(shape_floats.cspan());
                if(failed(shape)) return shape.errcode();

                Float4U accent = style_color(context, "gui.accent");
                Float4U highlight = style_color(context, "gui.shadow.light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.8f));
                Float4U active = style_color(context, "gui.accent.active", accent);
                GUI::SDFGradientStop stops[] = {
                    {0.0f, highlight, 0.42f}, {0.48f, accent, 0.58f}, {1.0f, active, 0.5f}
                };

                auto submit_color = [&](Vector<f32>& color_floats) -> RV
                {
                    auto color = context->append_sdf_color_program(color_floats.cspan());
                    if(failed(color)) return color.errcode();
                    GUI::DrawCommand command;
                    command.type = GUI::DrawCommandType::sdf;
                    command.rect_reference = GUI::DrawCommandRectReference::element;
                    command.sdf.shape = shape.get();
                    command.sdf.color = color.get();
                    context->draw(command);
                    return ok;
                };

                Vector<f32> color_floats;
                if(kind == 0)
                {
                    GUI::sdf_color_add_shadow(color_floats,
                        style_color(context, "gui.shadow.dark", Float4U(0.0f, 0.0f, 0.0f, 0.2f)),
                        Float2U(3.0f, 4.0f), 6.0f, 0.0f, GUI::SDFClipDesc::inner(0.0f));
                    GUI::sdf_color_add_linear_gradient(color_floats, Float2U(0.0f, 0.0f),
                        Float2U(width, height), Span<const GUI::SDFGradientStop>(stops, 3));
                }
                else if(kind == 1)
                {
                    GUI::sdf_color_add_radial_gradient(color_floats,
                        Float2U(width * 0.42f, height * 0.38f), Float2U(width * 0.55f, height * 0.55f),
                        Span<const GUI::SDFGradientStop>(stops, 3));
                }
                else
                {
                    GUI::sdf_color_add_conic_gradient(color_floats,
                        Float2U(width * 0.5f, height * 0.5f), -PI_DIV_TWO,
                        Span<const GUI::SDFGradientStop>(stops, 3));
                }
                return submit_color(color_floats);
            }

            RV finish_rounded_children_clip(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                f32 radius = (f32)(usize)userdata;
                GUI::DrawCommand pop_clip;
                pop_clip.type = GUI::DrawCommandType::pop_clip;
                context->draw(pop_clip);

                const GUI::Element* element_data = context->get_element(element.index);
                if(!element_data) return ok;
                const RectF& rect = element_data->layout_result.rect;
                f32 width = max(rect.width, 1.0f);
                f32 height = max(rect.height, 1.0f);

                auto submit = [&](Vector<f32>& shape_floats, Vector<f32>& color_floats) -> RV
                {
                    auto shape = context->append_sdf_shape_program(shape_floats.cspan());
                    if(failed(shape)) return shape.errcode();
                    auto color = context->append_sdf_color_program(color_floats.cspan());
                    if(failed(color)) return color.errcode();
                    GUI::DrawCommand command;
                    command.type = GUI::DrawCommandType::sdf;
                    command.rect_reference = GUI::DrawCommandRectReference::element;
                    command.sdf.shape = shape.get();
                    command.sdf.color = color.get();
                    context->draw(command);
                    return ok;
                };

                Vector<f32> border_shape;
                GUI::sdf_shape_add_rounded_rectangle(border_shape, RectF(0.5f, 0.5f,
                    max(width - 1.0f, 0.0f), max(height - 1.0f, 0.0f)),
                    Float4U(max(radius - 0.5f, 0.0f)));
                Vector<f32> border_color;
                GUI::sdf_color_add_solid(border_color, style_color(context, "gui.border.strong"),
                    GUI::SDFClipDesc::stroke(1.0f));
                return submit(border_shape, border_color);
            }

            GUI::ElementHandle sdf_swatch(GUI::IContext* context, EditorGUI::id_t element_id,
                const c8* name, u32 kind)
            {
                GUI::ElementHandle swatch = EditorGUI::begin_v_layout(context, element_id, name, grow_x(112.0f));
                GUI::DrawConfig draw;
                draw.name = Name(name);
                draw.callback = draw_sdf_swatch;
                draw.userdata = (void*)(usize)(u32)kind;
                context->set_draw_config(swatch, draw);
                EditorGUI::end_v_layout(context, swatch);
                return swatch;
            }

            void surface(GUI::IContext* context, const c8* background, f32 radius,
                bool raised = false, bool strong_elevation = false)
            {
                if(raised)
                {
                    GUI::DrawCommand shadow;
                    shadow.type = GUI::DrawCommandType::shadow;
                    shadow.rect_reference = GUI::DrawCommandRectReference::element;
                    shadow.radius = radius;
                    shadow.color = style_color(context, "gui.shadow.dark", Float4U(0.0f, 0.0f, 0.0f, 0.20f));
                    if(!strong_elevation) shadow.color.w *= 0.70f;
                    shadow.shadow.offset = strong_elevation ? Float2U(7.0f, 7.0f) : Float2U(4.0f, 5.0f);
                    shadow.shadow.softness = strong_elevation ? 9.0f : 6.0f;
                    context->draw(shadow);
                    shadow.color = style_color(context, "gui.shadow.light", Float4U(1.0f, 1.0f, 1.0f, 0.75f));
                    if(!strong_elevation) shadow.color.w *= 0.78f;
                    shadow.shadow.offset = strong_elevation ? Float2U(-7.0f, -7.0f) : Float2U(-4.0f, -4.0f);
                    shadow.shadow.softness = strong_elevation ? 9.0f : 5.0f;
                    context->draw(shadow);
                }
                rounded_rect(context, style_color(context, "gui.border"), radius);
                rounded_rect(context, style_color(context, background), max(radius - 1.0f, 0.0f),
                    RectF(1.0f, 1.0f, -2.0f, -2.0f));
            }

            EditorGUI::TextDesc text_desc(GUI::IContext* context, f32 size, const c8* color = "gui.text.color")
            {
                EditorGUI::TextDesc desc;
                // The HTML reference uses Inter while EditorGUITest deliberately uses the bundled Open Sans.
                // Compensate for Open Sans's smaller apparent x-height in explicitly-sized showcase copy.
                desc.font_size = size > 0.0f && size <= 13.0f ? size + 2.0f : size;
                desc.color = style_color(context, color);
                return desc;
            }

            GUI::ElementHandle label(GUI::IContext* context, EditorGUI::id_t element_id, const c8* value,
                const GUI::LayoutConfig& layout, f32 size = 0.0f, const c8* color = "gui.text.color")
            {
                return EditorGUI::text(context, element_id, value, layout, text_desc(context, size, color));
            }

            GUI::ElementHandle typography_label(GUI::IContext* context, EditorGUI::id_t element_id,
                const c8* value, const GUI::LayoutConfig& layout, EditorGUI::TypographyRole typography)
            {
                EditorGUI::TextDesc desc;
                desc.typography = typography;
                return EditorGUI::text(context, element_id, value, layout, desc);
            }

            EditorGUI::IconDesc icon_desc(GUI::IContext* context, const c8* color,
                EditorGUI::IconWeight weight = EditorGUI::IconWeight::regular, f32 size = 0.0f)
            {
                EditorGUI::IconDesc desc;
                desc.weight = weight;
                desc.tint = style_color(context, color);
                desc.size = size;
                return desc;
            }

            void selected_button_surface(GUI::IContext* context)
            {
                const f32 radius = style_scalar(context, "gui.button.radius",
                    style_scalar(context, "gui.radius.medium", 9.0f));
                rounded_rect(context, style_color(context, "gui.accent"), radius);
                rounded_rect(context, style_color(context, "gui.accent.subtle"),
                    max(radius - 1.0f, 0.0f), RectF(1.0f, 1.0f, -2.0f, -2.0f));
            }

            void icon_label(GUI::IContext* context, EditorGUI::id_t element_id, EditorGUI::IconName icon_name,
                const c8* value, const GUI::LayoutConfig& layout, f32 icon_size = 18.0f,
                f32 text_size = 0.0f, const c8* color = "gui.text.color",
                EditorGUI::IconWeight weight = EditorGUI::IconWeight::regular)
            {
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, element_id, value, layout);
                EditorGUI::icon(context, GUI::make_scoped_id(element_id, "icon"), icon_name,
                    fixed(icon_size, icon_size), icon_desc(context, color, weight, icon_size));
                label(context, GUI::make_scoped_id(element_id, "text"), value, grow_x(), text_size, color);
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = style_scalar(context, "gui.control.content_gap", 8.0f);
                EditorGUI::end_h_layout(context, row, flex);
            }

            void tree_label(GUI::IContext* context, EditorGUI::id_t element_id, EditorGUI::IconName icon_name,
                const c8* value, i32 depth, bool expanded, bool has_children, bool selected = false)
            {
                GUI::LayoutConfig layout = fill_width(28.0f);
                layout.padding.x = 4.0f + (f32)depth * 14.0f;
                layout.padding.z = 4.0f;
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, element_id, value, layout);
                if(selected)
                {
                    rounded_rect(context, style_color(context, "gui.accent.subtle"), 6.0f);
                }
                if(has_children)
                {
                    EditorGUI::icon(context, GUI::make_scoped_id(element_id, "caret"),
                        expanded ? EditorGUI::IconName::caret_down : EditorGUI::IconName::caret_right,
                        fixed(13.0f, 13.0f), icon_desc(context, "gui.text.muted", EditorGUI::IconWeight::bold, 13.0f));
                }
                else
                {
                    GUI::ElementHandle spacer = EditorGUI::begin_v_layout(context,
                        GUI::make_scoped_id(element_id, "spacer"), "Tree spacer", fixed(13.0f, 13.0f));
                    EditorGUI::end_v_layout(context, spacer);
                }
                const c8* color = selected ? "gui.accent.active" : "gui.text.secondary";
                EditorGUI::icon(context, GUI::make_scoped_id(element_id, "icon"), icon_name,
                    fixed(16.0f, 16.0f), icon_desc(context, color,
                        selected ? EditorGUI::IconWeight::fill : EditorGUI::IconWeight::regular, 16.0f));
                label(context, GUI::make_scoped_id(element_id, "text"), value, grow_x(), 10.0f, color);
                if(selected)
                {
                    EditorGUI::icon(context, GUI::make_scoped_id(element_id, "visible"), EditorGUI::IconName::eye,
                        fixed(15.0f, 15.0f), icon_desc(context, "gui.text.muted", EditorGUI::IconWeight::regular, 15.0f));
                }
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = 5.0f;
                EditorGUI::end_h_layout(context, row, flex);
            }

            GUI::ElementHandle icon_button(GUI::IContext* context, EditorGUI::id_t element_id,
                EditorGUI::IconName icon_name, const c8* value, const GUI::LayoutConfig& layout,
                const EditorGUI::ButtonDesc& desc = EditorGUI::ButtonDesc(), bool show_text = true,
                EditorGUI::IconWeight weight = EditorGUI::IconWeight::regular, bool selected = false)
            {
                GUI::LayoutConfig resolved_layout = layout;
                if(!show_text && resolved_layout.padding.x == 0.0f && resolved_layout.padding.y == 0.0f &&
                    resolved_layout.padding.z == 0.0f && resolved_layout.padding.w == 0.0f)
                {
                    // A non-zero symmetric value opts out of the text-button default padding while keeping
                    // the icon centered in the complete button rectangle.
                    resolved_layout.padding = Float4U(0.01f);
                }
                GUI::ElementHandle button = EditorGUI::begin_button(context, element_id, value, resolved_layout, desc);
                if(selected)
                {
                    selected_button_surface(context);
                }
                const c8* color = !desc.enabled ? "gui.text.disabled" :
                    (selected ? "gui.accent.active" : "gui.button.text");
                EditorGUI::icon(context, GUI::make_scoped_id(element_id, "icon"), icon_name,
                    fixed(18.0f, 18.0f), icon_desc(context, color, weight, 18.0f));
                if(show_text)
                {
                    GUI::LayoutConfig text_layout;
                    text_layout.width.kind = GUI::SizeKind::fit;
                    text_layout.height.kind = GUI::SizeKind::fit;
                    EditorGUI::TextDesc desc;
                    desc.color = style_color(context, color);
                    EditorGUI::text(context, GUI::make_scoped_id(element_id, "text"), value, text_layout, desc);
                }
                EditorGUI::end_button(context);
                return button;
            }

            GUI::ElementHandle icon_menu_item(GUI::IContext* context, EditorGUI::id_t element_id,
                EditorGUI::IconName icon_name, const c8* value, const EditorGUI::MenuItemDesc& desc = EditorGUI::MenuItemDesc())
            {
                GUI::ElementHandle item = EditorGUI::begin_menu_item(context, element_id, value, desc);
                const c8* color = desc.enabled ? "gui.menu_item.text" : "gui.menu_item.text_disabled";
                EditorGUI::icon(context, GUI::make_scoped_id(element_id, "icon"), icon_name,
                    fixed(16.0f, 16.0f), icon_desc(context, color, EditorGUI::IconWeight::regular, 16.0f));
                label(context, GUI::make_scoped_id(element_id, "text"), value, grow_x(), 0.0f, color);
                if(desc.shortcut && desc.shortcut[0])
                {
                    label(context, GUI::make_scoped_id(element_id, "shortcut"), desc.shortcut,
                        fixed(72.0f, 20.0f), 0.0f, color);
                }
                EditorGUI::end_menu_item(context);
                return item;
            }

            GUI::ElementHandle begin_panel(GUI::IContext* context, EditorGUI::id_t element_id,
                const c8* name, const GUI::LayoutConfig& input, const c8* background = "gui.surface.1",
                f32 radius = 12.0f, bool raised = true)
            {
                GUI::ElementHandle panel = EditorGUI::begin_v_layout(context, element_id, name, input);
                surface(context, background, radius, raised);
                return panel;
            }

            void end_panel(GUI::IContext* context, const GUI::ElementHandle& panel, f32 gap = 8.0f)
            {
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::y;
                flex.main_axis_gap = gap;
                flex.cross_alignment = GUI::FlexAlignment::stretch;
                EditorGUI::end_v_layout(context, panel, flex);
            }

            void divider(GUI::IContext* context, EditorGUI::id_t element_id)
            {
                GUI::ElementHandle line = EditorGUI::begin_v_layout(context, element_id, "Divider", fill_width(1.0f));
                rounded_rect(context, style_color(context, "gui.border"), 0.0f);
                EditorGUI::end_v_layout(context, line);
            }

            void led_label(GUI::IContext* context, EditorGUI::id_t element_id,
                const c8* value, const c8* color_key, f32 width)
            {
                GUI::LayoutConfig layout = fixed(width, 32.0f);
                layout.padding = Float4U(9.0f, 5.0f, 9.0f, 5.0f);
                GUI::ElementHandle pill = EditorGUI::begin_h_layout(context, element_id, value, layout);
                surface(context, "gui.surface.1", 16.0f, true);
                GUI::ElementHandle indicator = EditorGUI::begin_v_layout(context,
                    GUI::make_scoped_id(element_id, 1), "LED", fixed(10.0f, 10.0f));
                Float4U led_color = style_color(context, color_key);
                if(std::strcmp(color_key, "gui.status.off") != 0)
                {
                    Float4U glow_color = led_color;
                    glow_color.w *= 0.56f;
                    rounded_shadow(context, glow_color, 5.0f, Float2U(0.0f), 3.0f,
                        GUI::ShadowMode::outer);
                }
                rounded_rect(context, style_color(context, "gui.border"), 5.0f);
                rounded_rect(context, led_color, 4.0f, RectF(1.0f, 1.0f, -2.0f, -2.0f));
                Float4U highlight = style_color(context, "gui.shadow.inset_light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.7f));
                highlight.w *= 0.55f;
                rounded_shadow(context, highlight, 4.0f, Float2U(0.0f, -1.0f), 1.0f,
                    GUI::ShadowMode::inner, RectF(1.0f, 1.0f, -2.0f, -2.0f));
                EditorGUI::end_v_layout(context, indicator);
                label(context, GUI::make_scoped_id(element_id, 2), value, grow_x(), 11.0f, "gui.text.secondary");
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = 9.0f;
                EditorGUI::end_h_layout(context, pill, flex);
            }

            void section_heading(GUI::IContext* context, const c8* prefix, const c8* eyebrow,
                const c8* title, const c8* description, EditorGUI::IconName section_icon)
            {
                String row_name(prefix);
                row_name.append(".heading");
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, id(context, row_name.c_str()),
                    "Section heading", fill_width(92.0f));
                String left_name(prefix);
                left_name.append(".heading.left");
                GUI::ElementHandle left = EditorGUI::begin_v_layout(context, id(context, left_name.c_str()),
                    "Section title", fixed(450.0f, 92.0f));
                label(context, GUI::make_scoped_id(left.id, 1), eyebrow, fill_width(22.0f), 10.0f, "gui.accent.active");
                icon_label(context, GUI::make_scoped_id(left.id, 2), section_icon, title,
                    fill_width(48.0f), 24.0f, 28.0f, "gui.text.color", EditorGUI::IconWeight::duotone);
                end_panel(context, left, 2.0f);
                label(context, GUI::make_scoped_id(row.id, 3), description, grow_x(), 13.0f, "gui.text.secondary");
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = 24.0f;
                EditorGUI::end_h_layout(context, row, flex);
            }

            void card_title(GUI::IContext* context, EditorGUI::id_t parent, const c8* title, const c8* note)
            {
                label(context, GUI::make_scoped_id(parent, 1), title, fill_width(24.0f), 13.0f);
                label(context, GUI::make_scoped_id(parent, 2), note, fill_width(20.0f), 10.0f, "gui.text.muted");
                divider(context, GUI::make_scoped_id(parent, 3));
            }

            GUI::ElementHandle begin_card(GUI::IContext* context, const c8* name,
                const GUI::LayoutConfig& input, const c8* title, const c8* note)
            {
                GUI::LayoutConfig layout = input;
                layout.padding = Float4U(16.0f);
                GUI::ElementHandle card = begin_panel(context, id(context, name), title, layout,
                    "gui.surface.1", style_scalar(context, "gui.radius.large", 14.0f), true);
                card_title(context, card.id, title, note);
                return card;
            }

            void page_two_columns(GUI::IContext* context, EditorGUI::id_t element_id,
                GUI::ElementHandle& row, f32 height)
            {
                row = EditorGUI::begin_h_layout(context, element_id, "Card row", fill_width(height));
            }

            void page_two_columns_fit(GUI::IContext* context, EditorGUI::id_t element_id,
                GUI::ElementHandle& row)
            {
                GUI::LayoutConfig layout;
                layout.width.kind = GUI::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUI::SizeKind::fit;
                layout.flex_shrink = 0.0f;
                row = EditorGUI::begin_h_layout(context, element_id, "Content-sized card row", layout);
            }

            void end_two_columns(GUI::IContext* context, const GUI::ElementHandle& row)
            {
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::stretch;
                flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                flex.clip_children = false;
                EditorGUI::end_h_layout(context, row, flex);
            }

            void property_row(GUI::IContext* context, EditorGUI::id_t element_id, const c8* name,
                ShowcaseState& state, i32 kind)
            {
                const f32 height = style_scalar(context, "gui.control.height", 40.0f) + 4.0f;
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, element_id, name, fill_width(height));
                label(context, GUI::make_scoped_id(element_id, 1), name, fixed(78.0f, height),
                    10.0f, "gui.text.secondary");
                if(kind == 0)
                {
                    const c8* items[] = { "PBR Metallic", "Unlit" };
                    EditorGUI::combo(context, GUI::make_scoped_id(element_id, 2), "Shader Model", &state.combo_item,
                        Span<const c8*>(items, 2), grow_x());
                }
                else if(kind == 1)
                {
                    EditorGUI::slider_float(context, GUI::make_scoped_id(element_id, 2), &state.roughness,
                        0.0f, 1.0f, grow_x());
                }
                else if(kind == 2)
                {
                    EditorGUI::slider_float(context, GUI::make_scoped_id(element_id, 2), &state.metallic,
                        0.0f, 1.0f, grow_x());
                }
                else if(kind == 3)
                {
                    EditorGUI::toggle_switch(context, GUI::make_scoped_id(element_id, 2), "Normal Map",
                        &state.normal_map, grow_x());
                }
                else
                {
                    EditorGUI::checkbox(context, GUI::make_scoped_id(element_id, 2), "Use Triplanar Mapping",
                        &state.triplanar, grow_x());
                }
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = 8.0f;
                EditorGUI::end_h_layout(context, row, flex);
            }

            RHI::ITexture* material_texture(ShowcaseState& state, i32 index)
            {
                if(index == 0) return state.material_sand;
                if(index == 2) return state.material_concrete;
                return state.material_rusted;
            }

            EditorGUI::ImageDesc flipped_image(f32 left = 0.0f, f32 right = 1.0f,
                f32 top = 0.0f, f32 bottom = 1.0f)
            {
                EditorGUI::ImageDesc desc;
                desc.min_texcoord = Float2U(left, bottom);
                desc.max_texcoord = Float2U(right, top);
                return desc;
            }

            void build_top_bar(GUI::IContext* context, ShowcaseState& state)
            {
                const f32 height = state.density == 1 ? 72.0f : 56.0f;
                GUI::LayoutConfig layout = fill_width(height);
                layout.padding = Float4U(12.0f, 8.0f, 12.0f, 8.0f);
                GUI::ElementHandle top = EditorGUI::begin_h_layout(context, id(context, "showcase.topbar"),
                    "Top bar", layout);
                rounded_rect(context, style_color(context, "gui.surface.1"), 0.0f);

                GUI::ElementHandle brand = EditorGUI::begin_h_layout(context, id(context, "showcase.brand"),
                    "Brand", fixed(245.0f, height - 16.0f));
                GUI::ElementHandle logo = EditorGUI::begin_v_layout(context, id(context, "showcase.logo"),
                    "Logo", fixed(40.0f, 40.0f));
                rounded_rect(context, style_color(context, "gui.accent"), 12.0f);
                EditorGUI::icon(context, id(context, "showcase.logo.icon"), EditorGUI::IconName::package, fill(),
                    icon_desc(context, "gui.accent.ink", EditorGUI::IconWeight::duotone, 24.0f));
                EditorGUI::end_v_layout(context, logo);
                GUI::ElementHandle brand_text = EditorGUI::begin_v_layout(context, id(context, "showcase.brand.text"),
                    "Brand text", grow_x());
                label(context, id(context, "showcase.brand.title"), "Luna GUI", fill_width(22.0f), 15.0f);
                label(context, id(context, "showcase.brand.subtitle"), "Design Language Lab", fill_width(18.0f),
                    11.0f, "gui.text.muted");
                end_panel(context, brand_text, 0.0f);
                GUI::FlexLayoutDesc brand_flex;
                brand_flex.axis = GUI::LayoutAxis::x;
                brand_flex.cross_alignment = GUI::FlexAlignment::center;
                brand_flex.main_axis_gap = 11.0f;
                EditorGUI::end_h_layout(context, brand, brand_flex);

                GUI::ElementHandle status = EditorGUI::begin_h_layout(context, id(context, "showcase.status"),
                    "System status", grow_x());
                led_label(context, id(context, "showcase.status.renderer"), "Renderer Online",
                    "gui.status.success", 128.0f);
                led_label(context, id(context, "showcase.status.bake"), "Bake Queue  2",
                    "gui.status.busy", 112.0f);
                led_label(context, id(context, "showcase.status.sync"), "Remote Sync",
                    "gui.status.off", 106.0f);
                led_label(context, id(context, "showcase.status.error"), "Error  1",
                    "gui.status.error", 86.0f);
                GUI::FlexLayoutDesc status_flex;
                status_flex.axis = GUI::LayoutAxis::x;
                status_flex.cross_alignment = GUI::FlexAlignment::center;
                status_flex.main_alignment = GUI::FlexAlignment::center;
                status_flex.main_axis_gap = 9.0f;
                EditorGUI::end_h_layout(context, status, status_flex);

                GUI::ElementHandle controls = EditorGUI::begin_h_layout(context, id(context, "showcase.controls"),
                    "Style controls", fixed(474.0f, height - 16.0f));
                EditorGUI::icon(context, id(context, "showcase.theme.icon"),
                    state.theme == 0 ? EditorGUI::IconName::sun : EditorGUI::IconName::moon, fixed(18.0f, 18.0f),
                    icon_desc(context, "gui.text.secondary", EditorGUI::IconWeight::regular, 18.0f));
                const c8* themes[] = { "Light", "Dark" };
                EditorGUI::button_group(context, id(context, "showcase.theme"), Span<const c8*>(themes, 2),
                    &state.theme, fixed(126.0f, height - 22.0f));
                EditorGUI::icon(context, id(context, "showcase.density.icon"),
                    state.density == 0 ? EditorGUI::IconName::mouse : EditorGUI::IconName::hand_tap, fixed(18.0f, 18.0f),
                    icon_desc(context, "gui.text.secondary", EditorGUI::IconWeight::regular, 18.0f));
                const c8* densities[] = { "Compact", "Touch" };
                EditorGUI::button_group(context, id(context, "showcase.density"), Span<const c8*>(densities, 2),
                    &state.density, fixed(158.0f, height - 22.0f));
                GUI::LayoutConfig accent_layout = grow_x(height - 22.0f);
                accent_layout.padding = Float4U(9.0f, 4.0f, 9.0f, 4.0f);
                GUI::ElementHandle accent = EditorGUI::begin_h_layout(context, id(context, "showcase.accent"),
                    "Accent", accent_layout);
                surface(context, "gui.surface.1", 18.0f, false);
                EditorGUI::ShapeWidgetDesc accent_shape;
                accent_shape.tint = style_color(context, "gui.accent");
                EditorGUI::shape(context, id(context, "showcase.accent.swatch"), state.circle,
                    fixed(24.0f, 24.0f), accent_shape);
                label(context, id(context, "showcase.accent.value"), "#E34F59", grow_x(), 10.0f, "gui.text.secondary");
                EditorGUI::end_h_layout(context, accent, brand_flex);
                GUI::FlexLayoutDesc controls_flex;
                controls_flex.axis = GUI::LayoutAxis::x;
                controls_flex.cross_alignment = GUI::FlexAlignment::center;
                controls_flex.main_axis_gap = 7.0f;
                EditorGUI::end_h_layout(context, controls, controls_flex);

                GUI::FlexLayoutDesc top_flex;
                top_flex.axis = GUI::LayoutAxis::x;
                top_flex.cross_alignment = GUI::FlexAlignment::center;
                top_flex.main_axis_gap = 12.0f;
                EditorGUI::end_h_layout(context, top, top_flex);
            }

            void build_navigation(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUI::LayoutConfig layout = grow_x();
                layout.width.kind = GUI::SizeKind::fixed;
                layout.width.value = 196.0f;
                layout.padding = Float4U(10.0f, 14.0f, 10.0f, 12.0f);
                GUI::ElementHandle navigation = EditorGUI::begin_v_layout(context, id(context, "showcase.navigation"),
                    "Navigation", layout);
                rounded_rect(context, style_color(context, "gui.surface.0"), 0.0f);
                icon_label(context, id(context, "showcase.navigation.title"), EditorGUI::IconName::tree_structure,
                    "COMPONENTS", fill_width(34.0f), 16.0f, 10.0f, "gui.text.muted",
                    EditorGUI::IconWeight::bold);
                for(i32 i = 0; i < 9; ++i)
                {
                    EditorGUI::id_t item_id = GUI::make_scoped_id(navigation.id, (u64)i + 10);
                    handles.navigation[i] = EditorGUI::begin_button(context, item_id, SECTION_LABELS[i],
                        fill_width(style_scalar(context, "gui.control.height", 40.0f)));
                    if(state.section == i)
                    {
                        selected_button_surface(context);
                        rounded_rect(context, style_color(context, "gui.accent"), 1.5f,
                            RectF(3.0f, 8.0f, 3.0f, -16.0f));
                    }
                    const c8* item_color = state.section == i ? "gui.accent.active" : "gui.text.secondary";
                    EditorGUI::icon(context, GUI::make_scoped_id(item_id, "icon"), SECTION_ICONS[i],
                        fixed(18.0f, 18.0f), icon_desc(context, item_color,
                            state.section == i ? EditorGUI::IconWeight::fill : EditorGUI::IconWeight::regular, 18.0f));
                    label(context, GUI::make_scoped_id(item_id, "text"), SECTION_LABELS[i],
                        grow_x(), 11.0f, item_color);
                    EditorGUI::end_button(context);
                }
                GUI::ElementHandle spacer = EditorGUI::begin_v_layout(context, id(context, "showcase.navigation.spacer"),
                    "Spacer", grow_y());
                EditorGUI::end_v_layout(context, spacer);
                GUI::LayoutConfig footer_layout = fill_width(92.0f);
                footer_layout.padding = Float4U(12.0f);
                GUI::ElementHandle footer = begin_panel(context, id(context, "showcase.navigation.footer"),
                    "Active style", footer_layout, "gui.surface.1", 10.0f);
                label(context, id(context, "showcase.navigation.footer.label"), "ACTIVE LEAF STYLE",
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                const c8* style_name = state.theme == 0 ? (state.density == 0 ? "light.compact" : "light.touch") :
                    (state.density == 0 ? "dark.compact" : "dark.touch");
                icon_label(context, id(context, "showcase.navigation.footer.value"), EditorGUI::IconName::check_circle,
                    style_name, fill_width(22.0f), 15.0f, 12.0f, "gui.accent.active", EditorGUI::IconWeight::fill);
                label(context, id(context, "showcase.navigation.footer.note"), "Same component tree - Style only",
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                end_panel(context, footer, 3.0f);
                end_panel(context, navigation, 4.0f);
            }

            GUI::ElementHandle style_option(GUI::IContext* context, EditorGUI::id_t element_id,
                const c8* value, bool selected, const GUI::LayoutConfig& layout)
            {
                GUI::ElementHandle option = EditorGUI::begin_button(context, element_id, value, layout);
                if(selected)
                {
                    rounded_rect(context, style_color(context, "gui.accent"), 9.0f);
                    rounded_rect(context, style_color(context, "gui.accent.subtle"), 8.0f,
                        RectF(1.0f, 1.0f, -2.0f, -2.0f));
                }
                label(context, GUI::make_scoped_id(element_id, 1), value, grow_x(), 11.0f,
                    selected ? "gui.accent.active" : "gui.text.secondary");
                EditorGUI::end_button(context);
                return option;
            }

            void build_style_matrix(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles,
                const GUI::LayoutConfig& layout)
            {
                GUI::ElementHandle matrix = EditorGUI::begin_v_layout(context, id(context, "overview.matrix"),
                    "Style matrix", layout);
                const c8* labels[] = { "Light / Compact", "Light / Touch", "Dark / Compact", "Dark / Touch" };
                for(i32 row_index = 0; row_index < 2; ++row_index)
                {
                    GUI::ElementHandle row = EditorGUI::begin_h_layout(context,
                        GUI::make_scoped_id(matrix.id, (u64)row_index + 1), "Style row", grow_y());
                    for(i32 column = 0; column < 2; ++column)
                    {
                        i32 option_index = row_index * 2 + column;
                        bool selected = option_index == state.theme * 2 + state.density;
                        handles.style_options[option_index] = style_option(context,
                            GUI::make_scoped_id(row.id, (u64)column + 1), labels[option_index], selected, grow_x());
                    }
                    GUI::FlexLayoutDesc row_flex;
                    row_flex.axis = GUI::LayoutAxis::x;
                    row_flex.cross_alignment = GUI::FlexAlignment::stretch;
                    row_flex.main_axis_gap = 9.0f;
                    EditorGUI::end_h_layout(context, row, row_flex);
                }
                end_panel(context, matrix, 9.0f);
            }

            void build_asset_row(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles,
                EditorGUI::id_t parent, i32 index, const c8* name, const c8* meta)
            {
                GUI::LayoutConfig layout = fill_width(54.0f);
                layout.padding = Float4U(5.0f);
                handles.assets[index] = EditorGUI::begin_button(context, GUI::make_scoped_id(parent, (u64)index + 20),
                    name, layout);
                if(state.selected_asset == index)
                {
                    rounded_rect(context, style_color(context, "gui.accent"), 8.0f);
                    rounded_rect(context, style_color(context, "gui.accent.subtle"), 7.0f,
                        RectF(1.0f, 1.0f, -2.0f, -2.0f));
                }
                EditorGUI::image(context, GUI::make_scoped_id(parent, (u64)index + 30), material_texture(state, index),
                    fixed(40.0f, 40.0f), flipped_image());
                GUI::LayoutConfig text_layout = grow_x();
                text_layout.margin.x = 9.0f;
                GUI::ElementHandle text_column = EditorGUI::begin_v_layout(context,
                    GUI::make_scoped_id(parent, (u64)index + 40), "Asset name", text_layout);
                label(context, GUI::make_scoped_id(parent, (u64)index + 50), name,
                    fill_width(20.0f), 10.0f);
                label(context, GUI::make_scoped_id(parent, (u64)index + 60), meta,
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                end_panel(context, text_column, 0.0f);
                EditorGUI::end_button(context);
            }

            void build_mini_editor(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUI::LayoutConfig editor_layout = grow_y();
                editor_layout.padding = Float4U(state.density == 1 ? 12.0f : 10.0f);
                GUI::ElementHandle editor = EditorGUI::begin_h_layout(context, id(context, "overview.editor"),
                    "DCC editor composition", editor_layout);
                surface(context, "gui.surface.0", style_scalar(context, "gui.radius.large", 16.0f), true, true);

                GUI::LayoutConfig library_layout = fixed(176.0f, 0.0f);
                library_layout.height.kind = GUI::SizeKind::percent;
                library_layout.height.value = 1.0f;
                library_layout.padding = Float4U(10.0f);
                GUI::ElementHandle library = begin_panel(context, id(context, "overview.library"),
                    "Library", library_layout, "gui.surface.1", 15.0f, false);
                GUI::ElementHandle library_title = EditorGUI::begin_h_layout(context,
                    id(context, "overview.library.title"), "Scene Hierarchy", fill_width(34.0f));
                EditorGUI::icon(context, id(context, "overview.library.title.icon"), EditorGUI::IconName::tree_structure,
                    fixed(17.0f, 17.0f), icon_desc(context, "gui.text.secondary", EditorGUI::IconWeight::regular, 17.0f));
                label(context, id(context, "overview.library.title.text"), "Scene Hierarchy",
                    grow_x(), 11.0f);
                icon_button(context, id(context, "overview.library.add"), EditorGUI::IconName::plus, "Add object",
                    fixed(28.0f, 28.0f), EditorGUI::ButtonDesc(), false, EditorGUI::IconWeight::bold);
                GUI::FlexLayoutDesc library_title_flex;
                library_title_flex.axis = GUI::LayoutAxis::x;
                library_title_flex.cross_alignment = GUI::FlexAlignment::center;
                library_title_flex.main_axis_gap = 6.0f;
                EditorGUI::end_h_layout(context, library_title, library_title_flex);
                GUI::ElementHandle search_row = EditorGUI::begin_h_layout(context,
                    id(context, "overview.library.search.row"), "Search scene", fill_width(
                        style_scalar(context, "gui.control.small_height", 32.0f)));
                EditorGUI::icon(context, id(context, "overview.library.search.icon"), EditorGUI::IconName::magnifying_glass,
                    fixed(16.0f, 16.0f), icon_desc(context, "gui.text.muted", EditorGUI::IconWeight::regular, 16.0f));
                EditorGUI::TextInputDesc search_desc;
                search_desc.placeholder = "Search scene...";
                EditorGUI::input_text(context, id(context, "overview.library.search"), state.search_query,
                    grow_x(), search_desc);
                GUI::FlexLayoutDesc search_flex;
                search_flex.axis = GUI::LayoutAxis::x;
                search_flex.cross_alignment = GUI::FlexAlignment::center;
                search_flex.main_axis_gap = 6.0f;
                EditorGUI::end_h_layout(context, search_row, search_flex);
                tree_label(context, id(context, "overview.tree.root"), EditorGUI::IconName::package,
                    "Desert_Outpost", 0, true, true);
                tree_label(context, id(context, "overview.tree.environment"), EditorGUI::IconName::folder_open,
                    "Environment", 1, true, true);
                tree_label(context, id(context, "overview.tree.sun"), EditorGUI::IconName::sun,
                    "Sun_Light", 2, false, false);
                tree_label(context, id(context, "overview.tree.ball"), EditorGUI::IconName::bounding_box,
                    "Material_Ball", 2, false, false, true);
                divider(context, id(context, "overview.library.divider"));
                icon_label(context, id(context, "overview.library.assets"), EditorGUI::IconName::folder_open,
                    "Assets  -  3 items", fill_width(28.0f), 16.0f, 10.0f, "gui.text.secondary");
                build_asset_row(context, state, handles, library.id, 0, "M_Desert_Sand", "Material - 3.6 MB");
                build_asset_row(context, state, handles, library.id, 1, "M_Rusted_Metal", "Material - 2.1 MB");
                build_asset_row(context, state, handles, library.id, 2, "M_Concrete_Grime", "Material - 1.8 MB");
                end_panel(context, library, 5.0f);

                GUI::ElementHandle viewport = EditorGUI::begin_v_layout(context, id(context, "overview.viewport"),
                    "Viewport", grow_x());
                GUI::ElementHandle toolbar = EditorGUI::begin_h_layout(context, id(context, "overview.viewport.toolbar"),
                    "Viewport toolbar", fill_width(48.0f));
                rounded_rect(context, style_color(context, "gui.surface.1"), 0.0f);
                GUI::ElementHandle tool_row = EditorGUI::begin_h_layout(context,
                    id(context, "overview.viewport.tools"), "Viewport tools", fixed(184.0f, 40.0f));
                const EditorGUI::IconName tool_icons[] = {
                    EditorGUI::IconName::selection, EditorGUI::IconName::arrows_out,
                    EditorGUI::IconName::arrows_clockwise, EditorGUI::IconName::bounding_box
                };
                const c8* tool_labels[] = { "Select", "Move", "Rotate", "Frame" };
                for(i32 tool_index = 0; tool_index < 4; ++tool_index)
                {
                    EditorGUI::id_t tool_id = GUI::make_scoped_id(tool_row.id, (u64)tool_index + 1);
                    GUI::ElementHandle tool = icon_button(context, tool_id, tool_icons[tool_index],
                        tool_labels[tool_index], fixed(40.0f, 40.0f), EditorGUI::ButtonDesc(), false,
                        tool_index == state.selected_group ? EditorGUI::IconWeight::bold : EditorGUI::IconWeight::regular,
                        tool_index == state.selected_group);
                    if(EditorGUI::is_item_valid(context, tool) && EditorGUI::is_item_clicked(context, tool))
                    {
                        state.selected_group = tool_index;
                    }
                }
                GUI::FlexLayoutDesc tool_flex;
                tool_flex.axis = GUI::LayoutAxis::x;
                tool_flex.cross_alignment = GUI::FlexAlignment::center;
                tool_flex.main_axis_gap = 6.0f;
                EditorGUI::end_h_layout(context, tool_row, tool_flex);
                const c8* spaces[] = { "Local", "World" };
                EditorGUI::button_group(context, id(context, "overview.viewport.space"), Span<const c8*>(spaces, 2),
                    &state.space_mode, fixed(126.0f, 40.0f));
                icon_button(context, id(context, "overview.viewport.more"), EditorGUI::IconName::dots_three,
                    "More viewport options", fixed(40.0f, 40.0f), EditorGUI::ButtonDesc(), false,
                    EditorGUI::IconWeight::bold);
                GUI::FlexLayoutDesc toolbar_flex;
                toolbar_flex.axis = GUI::LayoutAxis::x;
                toolbar_flex.cross_alignment = GUI::FlexAlignment::center;
                toolbar_flex.main_alignment = GUI::FlexAlignment::space_between;
                EditorGUI::end_h_layout(context, toolbar, toolbar_flex);
                EditorGUI::image(context, id(context, "overview.viewport.image"), state.material_preview,
                    grow_y(), flipped_image(0.14f, 0.86f));
                GUI::LayoutConfig console_layout = fill_width(104.0f);
                console_layout.padding = Float4U(10.0f, 0.0f, 10.0f, 8.0f);
                GUI::ElementHandle console = begin_panel(context, id(context, "overview.console"),
                    "Console", console_layout, "gui.surface.1", 0.0f, false);
                const c8* console_tabs[] = { "Console", "Bake Queue 2", "Messages" };
                EditorGUI::button_group(context, id(context, "overview.console.tabs"),
                    Span<const c8*>(console_tabs, 3), &state.console_tab, fill_width(36.0f));
                icon_label(context, id(context, "overview.console.line1"), EditorGUI::IconName::check_circle,
                    "[ready]  Renderer initialized successfully.                         10:23:45",
                    fill_width(24.0f), 14.0f, 9.0f, "gui.status.success", EditorGUI::IconWeight::fill);
                icon_label(context, id(context, "overview.console.line2"), EditorGUI::IconName::spinner_gap,
                    "[busy]   Bake queue contains two materials.                         10:24:12",
                    fill_width(24.0f), 14.0f, 9.0f, "gui.status.busy", EditorGUI::IconWeight::bold);
                end_panel(context, console, 2.0f);
                GUI::FlexLayoutDesc viewport_flex;
                viewport_flex.axis = GUI::LayoutAxis::y;
                viewport_flex.cross_alignment = GUI::FlexAlignment::stretch;
                EditorGUI::end_v_layout(context, viewport, viewport_flex);

                GUI::LayoutConfig inspector_layout = fixed(220.0f, 0.0f);
                inspector_layout.height.kind = GUI::SizeKind::percent;
                inspector_layout.height.value = 1.0f;
                inspector_layout.padding = Float4U(12.0f);
                GUI::ElementHandle inspector = begin_panel(context, id(context, "overview.inspector"),
                    "Inspector", inspector_layout, "gui.surface.1", 15.0f, false);
                GUI::ElementHandle asset_header = EditorGUI::begin_h_layout(context, id(context, "overview.inspector.asset"),
                    "Selected asset", fill_width(52.0f));
                EditorGUI::image(context, id(context, "overview.inspector.thumb"),
                    material_texture(state, state.selected_asset), fixed(40.0f, 40.0f), flipped_image());
                const c8* asset_names[] = { "M_Desert_Sand", "M_Rusted_Metal", "M_Concrete_Grime" };
                label(context, id(context, "overview.inspector.name"), asset_names[state.selected_asset],
                    grow_x(), 10.0f);
                GUI::FlexLayoutDesc asset_flex;
                asset_flex.axis = GUI::LayoutAxis::x;
                asset_flex.cross_alignment = GUI::FlexAlignment::center;
                asset_flex.main_axis_gap = 9.0f;
                EditorGUI::end_h_layout(context, asset_header, asset_flex);
                const c8* inspector_tabs[] = { "Inspector", "Channels" };
                EditorGUI::button_group(context, id(context, "overview.inspector.tabs"),
                    Span<const c8*>(inspector_tabs, 2), &state.inspector_tab, fill_width(40.0f));
                property_row(context, id(context, "overview.property.shader"), "Shader Model", state, 0);
                property_row(context, id(context, "overview.property.roughness"), "Roughness", state, 1);
                property_row(context, id(context, "overview.property.metallic"), "Metallic", state, 2);
                property_row(context, id(context, "overview.property.normal"), "Normal Map", state, 3);
                property_row(context, id(context, "overview.property.triplanar"), "Triplanar", state, 4);
                end_panel(context, inspector, 4.0f);

                GUI::FlexLayoutDesc editor_flex;
                editor_flex.axis = GUI::LayoutAxis::x;
                editor_flex.cross_alignment = GUI::FlexAlignment::stretch;
                editor_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                EditorGUI::end_h_layout(context, editor, editor_flex);
            }

            void build_overview(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                section_heading(context, "overview", "SELECTED DIRECTION - SOFT WORKSHOP",
                    "A tactile workbench for serious tools",
                    "Warm machined surfaces, disciplined red accents, and instrument-like status semantics, scaled from compact desktop density to touch.",
                    EditorGUI::IconName::squares_four);
                GUI::LayoutConfig banner_layout = fill_width(194.0f);
                banner_layout.padding = Float4U(28.0f);
                GUI::ElementHandle banner = EditorGUI::begin_h_layout(context, id(context, "overview.banner"),
                    "Style summary", banner_layout);
                surface(context, "gui.surface.1", style_scalar(context, "gui.radius.large", 16.0f), true, true);
                GUI::ElementHandle summary = EditorGUI::begin_v_layout(context, id(context, "overview.banner.summary"),
                    "Summary", grow_x(130.0f));
                label(context, id(context, "overview.banner.active"), "READY  STYLE ACTIVE", fill_width(22.0f),
                    10.0f, "gui.text.secondary");
                const c8* heading = state.theme == 0 ? (state.density == 0 ? "Light - Compact" : "Light - Touch") :
                    (state.density == 0 ? "Dark - Compact" : "Dark - Touch");
                label(context, id(context, "overview.banner.heading"), heading, fill_width(58.0f), 42.0f);
                label(context, id(context, "overview.banner.copy"),
                    "Four concrete leaf Styles map palette and input density onto Luna::GUI.",
                    fill_width(38.0f), 12.0f, "gui.text.secondary");
                end_panel(context, summary, 4.0f);
                build_style_matrix(context, state, handles, fixed(384.0f, 130.0f));
                GUI::FlexLayoutDesc banner_flex;
                banner_flex.axis = GUI::LayoutAxis::x;
                banner_flex.cross_alignment = GUI::FlexAlignment::center;
                banner_flex.main_axis_gap = 24.0f;
                EditorGUI::end_h_layout(context, banner, banner_flex);
                build_mini_editor(context, state, handles);
            }

            void build_primitives(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "primitives", "GUITEST - PRIMITIVES", "Type, imagery, progress, and system light",
                    "Foundational drawing and semantic feedback, including the LED vocabulary used by editor status surfaces.",
                    EditorGUI::IconName::stack);
                GUI::ElementHandle row;
                page_two_columns(context, id(context, "primitives.row1"), row, 430.0f);
                GUI::ElementHandle typography = begin_card(context, "primitives.typography", grow_x(),
                    "Typography", "Semantic roles resolved from the active Style");
                typography_label(context, id(context, "primitives.type.h1"), "H1  Editor title",
                    fill_width(42.0f), EditorGUI::TypographyRole::heading1);
                typography_label(context, id(context, "primitives.type.h2"), "H2  Workspace title",
                    fill_width(36.0f), EditorGUI::TypographyRole::heading2);
                typography_label(context, id(context, "primitives.type.h3"), "H3  Panel heading",
                    fill_width(32.0f), EditorGUI::TypographyRole::heading3);
                typography_label(context, id(context, "primitives.type.h4"), "H4  Property group",
                    fill_width(28.0f), EditorGUI::TypographyRole::heading4);
                typography_label(context, id(context, "primitives.type.h5"), "H5  Subsection",
                    fill_width(26.0f), EditorGUI::TypographyRole::heading5);
                typography_label(context, id(context, "primitives.type.h6"), "H6  Dense heading",
                    fill_width(24.0f), EditorGUI::TypographyRole::heading6);
                typography_label(context, id(context, "primitives.type.body"),
                    "Body  Primary application copy remains readable.", fill_width(26.0f),
                    EditorGUI::TypographyRole::body);
                typography_label(context, id(context, "primitives.type.cite"),
                    "Cite  Supporting context and attribution.", fill_width(24.0f),
                    EditorGUI::TypographyRole::cite);
                typography_label(context, id(context, "primitives.type.code"),
                    "Code  Float3U(1.0f, 2.0f, 3.0f)", fill_width(24.0f), EditorGUI::TypographyRole::code);
                typography_label(context, id(context, "primitives.type.caption"),
                    "Caption  Secondary metadata - 12:45:08", fill_width(22.0f),
                    EditorGUI::TypographyRole::caption);
                end_panel(context, typography, 4.0f);
                GUI::ElementHandle progress = begin_card(context, "primitives.progress", grow_x(),
                    "Progress and LED", "Determinate work and semantic status");
                label(context, id(context, "primitives.progress.label1"), "Baking textures                         68%",
                    fill_width(24.0f), 11.0f, "gui.text.secondary");
                EditorGUI::ProgressBarDesc progress_desc;
                progress_desc.show_overlay = false;
                EditorGUI::progress_bar(context, id(context, "primitives.progress.1"), 0.68f, fill_width(10.0f),
                    progress_desc);
                label(context, id(context, "primitives.progress.label2"), "Compiling shaders                      42%",
                    fill_width(24.0f), 11.0f, "gui.text.secondary");
                EditorGUI::progress_bar(context, id(context, "primitives.progress.2"), 0.42f, fill_width(10.0f),
                    progress_desc);
                led_label(context, id(context, "primitives.led.online"), "Online", "gui.status.success", 100.0f);
                led_label(context, id(context, "primitives.led.working"), "Working", "gui.status.busy", 100.0f);
                end_panel(context, progress);
                end_two_columns(context, row);
                GUI::ElementHandle sdf_card = begin_card(context, "primitives.sdf", fill_width(220.0f),
                    "SDF shape and paint programs", "Linear, radial, conic, CSG, capsule, and analytic shadow");
                GUI::ElementHandle sdf_row = EditorGUI::begin_h_layout(context, id(context, "primitives.sdf.row"),
                    "SDF swatches", grow_y());
                sdf_swatch(context, id(context, "primitives.sdf.linear"), "Linear rounded rectangle", 0);
                sdf_swatch(context, id(context, "primitives.sdf.radial"), "Radial CSG ring", 1);
                sdf_swatch(context, id(context, "primitives.sdf.conic"), "Conic capsule", 2);
                GUI::FlexLayoutDesc sdf_flex;
                sdf_flex.axis = GUI::LayoutAxis::x;
                sdf_flex.cross_alignment = GUI::FlexAlignment::stretch;
                sdf_flex.main_axis_gap = 14.0f;
                EditorGUI::end_h_layout(context, sdf_row, sdf_flex);
                end_panel(context, sdf_card);
                GUI::ElementHandle icon_card = begin_card(context, "primitives.icons", fill_width(220.0f),
                    "Built-in icons", "Phosphor geometry rendered through the shared VG shape buffer");
                struct IconSample
                {
                    EditorGUI::IconName name;
                    EditorGUI::IconWeight weight;
                };
                const IconSample icon_samples[][7] = {
                    {
                        { EditorGUI::IconName::folder, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::floppy_disk, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::magnifying_glass, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::gear_six, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::trash, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::camera, EditorGUI::IconWeight::regular },
                        { EditorGUI::IconName::play, EditorGUI::IconWeight::regular }
                    },
                    {
                        { EditorGUI::IconName::caret_left, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::caret_right, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::plus, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::minus, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::check, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::x, EditorGUI::IconWeight::bold },
                        { EditorGUI::IconName::dots_three, EditorGUI::IconWeight::bold }
                    },
                    {
                        { EditorGUI::IconName::check_circle, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::warning_circle, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::bell, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::heart, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::star, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::folder_open, EditorGUI::IconWeight::fill },
                        { EditorGUI::IconName::package, EditorGUI::IconWeight::duotone }
                    }
                };
                for(u32 row_index = 0; row_index < 3; ++row_index)
                {
                    EditorGUI::id_t icon_row_id = GUI::make_scoped_id(
                        id(context, "primitives.icons.row"), (u64)row_index + 1);
                    GUI::ElementHandle icon_row = EditorGUI::begin_h_layout(context,
                        icon_row_id, "Icon weight row", fill_width(42.0f));
                    for(u32 icon_index = 0; icon_index < 7; ++icon_index)
                    {
                        EditorGUI::IconDesc icon_desc;
                        icon_desc.weight = icon_samples[row_index][icon_index].weight;
                        icon_desc.tint = row_index == 2 ? style_color(context, "gui.accent") :
                            style_color(context, "gui.icon.color");
                        EditorGUI::icon(context, GUI::make_scoped_id(icon_row_id, (u64)icon_index + 1),
                            icon_samples[row_index][icon_index].name, fixed(28.0f, 28.0f), icon_desc);
                    }
                    GUI::FlexLayoutDesc icon_flex;
                    icon_flex.axis = GUI::LayoutAxis::x;
                    icon_flex.main_axis_gap = 16.0f;
                    icon_flex.cross_alignment = GUI::FlexAlignment::center;
                    EditorGUI::end_h_layout(context, icon_row, icon_flex);
                }
                end_panel(context, icon_card);
                GUI::ElementHandle image_card = begin_card(context, "primitives.image", fill_width(420.0f),
                    "Image and shape", "Raster material assets and vector primitives");
                EditorGUI::image(context, id(context, "primitives.image.preview"), state.material_preview,
                    grow_y(), flipped_image(0.05f, 0.95f, 0.08f, 0.92f));
                end_panel(context, image_card);
            }

            void build_buttons(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "buttons", "GUITEST - BUTTONS", "Actions and choices",
                    "Raised actions, recessed selection wells, explicit focus, and states that never depend on hover alone.",
                    EditorGUI::IconName::cursor_click);
                GUI::ElementHandle row1;
                page_two_columns_fit(context, id(context, "buttons.row1"), row1);
                GUI::ElementHandle states = begin_card(context, "buttons.states", grow(),
                    "Button states", "Default, pressed, destructive, and disabled");
                icon_button(context, id(context, "buttons.primary"), EditorGUI::IconName::lightning,
                    "Primary", fill_width(44.0f), EditorGUI::ButtonDesc(), true, EditorGUI::IconWeight::fill);
                icon_button(context, id(context, "buttons.save"), EditorGUI::IconName::floppy_disk,
                    "Save", fill_width(44.0f));
                icon_button(context, id(context, "buttons.upload"), EditorGUI::IconName::upload_simple,
                    "Upload", fill_width(44.0f));
                EditorGUI::ButtonDesc disabled;
                disabled.enabled = false;
                icon_button(context, id(context, "buttons.disabled"), EditorGUI::IconName::gear_six,
                    "Disabled", fill_width(44.0f), disabled);
                end_panel(context, states);
                GUI::ElementHandle groups = begin_card(context, "buttons.groups", grow(),
                    "Button groups", "Single and multiple selection");
                const c8* items[] = { "Build", "Run", "Profile", "Ship" };
                EditorGUI::button_group(context, id(context, "buttons.group.single"), Span<const c8*>(items, 4),
                    &state.selected_group, fill_width(48.0f));
                EditorGUI::button_group(context, id(context, "buttons.group.multi"), Span<const c8*>(items, 4),
                    Span<bool>(state.selected_group_multi, 4), fill_width(48.0f));
                end_panel(context, groups);
                end_two_columns(context, row1);
                GUI::ElementHandle row2;
                page_two_columns(context, id(context, "buttons.row2"), row2, 380.0f);
                GUI::ElementHandle choices = begin_card(context, "buttons.choices", grow_x(),
                    "Selection controls", "Checkbox, radio, and switch");
                EditorGUI::checkbox(context, id(context, "buttons.checkbox"), "Visible in render", &state.checkbox_value,
                    fill_width(48.0f));
                EditorGUI::checkbox(context, id(context, "buttons.receive"), "Receive shadows", &state.receive_shadows,
                    fill_width(48.0f));
                EditorGUI::radio_button(context, id(context, "buttons.radio.a"), "Radio A", &state.radio_value, 0,
                    fill_width(48.0f));
                EditorGUI::radio_button(context, id(context, "buttons.radio.b"), "Radio B", &state.radio_value, 1,
                    fill_width(48.0f));
                EditorGUI::toggle_switch(context, id(context, "buttons.toggle"), "Live preview", &state.live_preview,
                    fill_width(48.0f));
                end_panel(context, choices);
                GUI::ElementHandle disclosure = begin_card(context, "buttons.disclosure", grow_x(),
                    "Disclosure and tree", "Expanded, selected, and leaf states");
                EditorGUI::ScrollViewDesc disclosure_scroll_desc;
                disclosure_scroll_desc.horizontal = false;
                disclosure_scroll_desc.scrollbar_mode = EditorGUI::ScrollBarMode::always_visible;
                EditorGUI::begin_scroll_view(context, id(context, "buttons.disclosure.scroll"),
                    "Disclosure content", grow_y(), disclosure_scroll_desc);
                GUI::LayoutConfig disclosure_content_layout = grow_y();
                disclosure_content_layout.flex_grow = 0.0f;
                disclosure_content_layout.flex_shrink = 0.0f;
                GUI::ElementHandle disclosure_content = EditorGUI::begin_v_layout(context,
                    id(context, "buttons.disclosure.content"), "Disclosure controls", disclosure_content_layout);
                if(EditorGUI::collapsing_header(context, id(context, "buttons.header"), "Transform", fill_width(48.0f)))
                {
                    label(context, id(context, "buttons.position"), "Position        0.00, 1.25, 0.00",
                        fill_width(34.0f), 10.0f, "gui.text.secondary");
                    label(context, id(context, "buttons.rotation"), "Rotation        0, 15, 0",
                        fill_width(34.0f), 10.0f, "gui.text.secondary");
                }
                if(EditorGUI::tree_node(context, id(context, "buttons.tree.scene"), "Scene",
                    EditorGUI::TreeNodeFlag::none, 0, fill_width(40.0f)))
                {
                    EditorGUI::tree_node(context, id(context, "buttons.tree.ball"), "Material Ball",
                        EditorGUI::TreeNodeFlag::selected, 1, fill_width(40.0f));
                    EditorGUI::tree_node(context, id(context, "buttons.tree.camera"), "Camera",
                        EditorGUI::TreeNodeFlag::leaf, 1, fill_width(40.0f));
                }
                end_panel(context, disclosure_content);
                EditorGUI::end_scroll_view(context);
                end_panel(context, disclosure);
                end_two_columns(context, row2);
            }

            void build_input(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "input", "GUITEST - INPUT", "Text, numeric, slider, and color editing",
                    "Inset fields communicate editability while focus, read-only, disabled, and validation states remain distinct.",
                    EditorGUI::IconName::sliders_horizontal);
                GUI::ElementHandle row1;
                page_two_columns(context, id(context, "input.row1"), row1, 330.0f);
                GUI::ElementHandle fields = begin_card(context, "input.fields", grow_x(),
                    "Text input", "Editable, read-only, and disabled");
                EditorGUI::input_text(context, id(context, "input.asset"), state.asset_name, fill_width(48.0f));
                EditorGUI::TextInputDesc read_only;
                read_only.read_only = true;
                EditorGUI::input_text(context, id(context, "input.readonly"), state.readonly_value,
                    fill_width(48.0f), read_only);
                EditorGUI::TextInputDesc disabled;
                disabled.enabled = false;
                EditorGUI::input_text(context, id(context, "input.disabled"), state.readonly_value,
                    fill_width(48.0f), disabled);
                end_panel(context, fields);
                GUI::ElementHandle sliders = begin_card(context, "input.sliders", grow_x(),
                    "Slider and drag editors", "Scalar, integer, and vector values");
                EditorGUI::slider_float(context, id(context, "input.roughness"), &state.roughness, 0.0f, 1.0f,
                    fill_width(48.0f));
                EditorGUI::slider_int(context, id(context, "input.subdivisions"), &state.subdivisions, 0, 100,
                    fill_width(48.0f));
                EditorGUI::DragDesc drag_desc;
                drag_desc.speed = 0.05f;
                EditorGUI::drag_float3(context, id(context, "input.position"), state.position, -100.0f, 100.0f,
                    fill_width(48.0f), drag_desc);
                end_panel(context, sliders);
                end_two_columns(context, row1);
                GUI::ElementHandle row2;
                page_two_columns(context, id(context, "input.row2"), row2, 250.0f);
                GUI::ElementHandle color = begin_card(context, "input.color", grow_x(),
                    "Color edit", "RGBA preview and numeric channels");
                EditorGUI::color_edit4(context, id(context, "input.color.value"), "Preview color", state.preview_color,
                    fill_width(48.0f));
                end_panel(context, color);
                GUI::ElementHandle combo = begin_card(context, "input.combo", grow_x(),
                    "Combo and search", "Popup-ready selector and filtered query");
                const c8* render_modes[] = { "PBR Metallic Roughness", "Subsurface", "Unlit" };
                GUI::ElementHandle combo_row = EditorGUI::begin_h_layout(context, id(context, "input.combo.row"),
                    "Shader model", fill_width(48.0f));
                EditorGUI::icon(context, id(context, "input.combo.icon"), EditorGUI::IconName::sliders_horizontal,
                    fixed(18.0f, 18.0f), icon_desc(context, "gui.text.secondary", EditorGUI::IconWeight::regular, 18.0f));
                EditorGUI::combo(context, id(context, "input.combo.shader"), "Shader model", &state.combo_item,
                    Span<const c8*>(render_modes, 3), grow_x());
                GUI::FlexLayoutDesc input_row_flex;
                input_row_flex.axis = GUI::LayoutAxis::x;
                input_row_flex.cross_alignment = GUI::FlexAlignment::center;
                input_row_flex.main_axis_gap = 8.0f;
                EditorGUI::end_h_layout(context, combo_row, input_row_flex);
                GUI::ElementHandle search_row = EditorGUI::begin_h_layout(context, id(context, "input.search.row"),
                    "Filter components", fill_width(48.0f));
                EditorGUI::icon(context, id(context, "input.search.icon"), EditorGUI::IconName::magnifying_glass,
                    fixed(18.0f, 18.0f), icon_desc(context, "gui.text.muted", EditorGUI::IconWeight::regular, 18.0f));
                EditorGUI::TextInputDesc search_desc;
                search_desc.placeholder = "Filter components...";
                EditorGUI::input_text(context, id(context, "input.search"), state.search_query,
                    grow_x(), search_desc);
                EditorGUI::end_h_layout(context, search_row, input_row_flex);
                end_panel(context, combo);
                end_two_columns(context, row2);
            }

            void build_layouts(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "layouts", "GUITEST - LAYOUTS", "Flex, grid, canvas, focus scope, and tabs",
                    "The visual system scales without changing layout semantics or stable element identity.",
                    EditorGUI::IconName::grid_four);
                GUI::ElementHandle row1;
                page_two_columns(context, id(context, "layouts.row1"), row1, 300.0f);
                GUI::ElementHandle flex_card = begin_card(context, "layouts.flex", grow_x(),
                    "Flex and grid", "Fixed, growing, and repeated cells");
                GUI::ElementHandle flex_row = EditorGUI::begin_h_layout(context, id(context, "layouts.flex.row"),
                    "Flex row", fill_width(48.0f));
                EditorGUI::text_button(context, id(context, "layouts.flex.fixed1"), "Fixed", fixed(90.0f, 44.0f));
                EditorGUI::text_button(context, id(context, "layouts.flex.grow"), "flex-grow: 1", grow_x(44.0f));
                EditorGUI::text_button(context, id(context, "layouts.flex.fixed2"), "Fixed", fixed(90.0f, 44.0f));
                end_two_columns(context, flex_row);
                for(i32 row_index = 0; row_index < 2; ++row_index)
                {
                    GUI::ElementHandle grid_row = EditorGUI::begin_h_layout(context,
                        GUI::make_scoped_id(flex_card.id, (u64)row_index + 20), "Grid row", fill_width(48.0f));
                    for(i32 column = 0; column < 4; ++column)
                    {
                        c8 text[16];
                        snprintf(text, sizeof(text), "Cell %d", row_index * 4 + column + 1);
                        EditorGUI::text_button(context, GUI::make_scoped_id(grid_row.id, (u64)column + 1), text,
                            grow_x(44.0f));
                    }
                    end_two_columns(context, grid_row);
                }
                end_panel(context, flex_card);
                GUI::ElementHandle assets = begin_card(context, "layouts.assets", grow_x(),
                    "Asset grid", "Image, selection, and metadata");
                GUI::ElementHandle asset_row = EditorGUI::begin_h_layout(context, id(context, "layouts.assets.row"),
                    "Asset row", grow_y());
                for(i32 i = 0; i < 3; ++i)
                {
                    GUI::ElementHandle item = EditorGUI::begin_v_layout(context,
                        GUI::make_scoped_id(asset_row.id, (u64)i + 1), "Asset", grow_x());
                    surface(context, i == state.selected_asset ? "gui.accent.subtle" : "gui.surface.2", 10.0f, false);
                    EditorGUI::image(context, GUI::make_scoped_id(item.id, 1), material_texture(state, i),
                        grow_y(), flipped_image());
                    const c8* names[] = { "Sand", "Rusted Metal", "Concrete" };
                    label(context, GUI::make_scoped_id(item.id, 2), names[i], fill_width(28.0f), 10.0f);
                    end_panel(context, item, 5.0f);
                }
                end_two_columns(context, asset_row);
                end_panel(context, assets);
                end_two_columns(context, row1);
            }

            void build_scroll_views(GUI::IContext* context)
            {
                section_heading(context, "scroll", "GUITEST - SCROLL VIEWS", "Overlay and reserved scrollbars",
                    "Compact pointer mode favors overlay scrollbars; touch mode keeps broad, visible affordances.",
                    EditorGUI::IconName::columns);
                GUI::ElementHandle row;
                page_two_columns(context, id(context, "scroll.row"), row, 620.0f);
                for(i32 column = 0; column < 2; ++column)
                {
                    const c8* card_name = column == 0 ? "scroll.overlay" : "scroll.visible";
                    GUI::ElementHandle card = begin_card(context, card_name, grow_x(),
                        column == 0 ? "Dynamic overlay" : "Always visible",
                        column == 0 ? "Appears during interaction" : "Reserves viewport space");
                    EditorGUI::ScrollViewDesc desc;
                    desc.horizontal = false;
                    desc.scrollbar_mode = column == 0 ? EditorGUI::ScrollBarMode::dynamic_overlay :
                        EditorGUI::ScrollBarMode::always_visible;
                    EditorGUI::begin_scroll_view(context, GUI::make_scoped_id(card.id, 10), "Rows", grow_y(), desc);
                    for(i32 i = 0; i < 16; ++i)
                    {
                        c8 value[32];
                        snprintf(value, sizeof(value), "%s row %02d", column == 0 ? "Overlay" : "Persistent", i + 1);
                        EditorGUI::text_button(context, GUI::make_scoped_id(card.id, (u64)i + 30), value,
                            fill_width(44.0f));
                    }
                    EditorGUI::end_scroll_view(context);
                    end_panel(context, card);
                }
                end_two_columns(context, row);
            }

            struct TableRecord
            {
                const c8* cells[6];
                const c8* status_color;
            };

            void table_cell(GUI::IContext* context, EditorGUI::id_t element_id, const c8* value,
                bool header, bool status, const c8* status_color)
            {
                GUI::LayoutConfig layout;
                layout.padding = Float4U(12.0f, 0.0f, 12.0f, 0.0f);
                GUI::ElementHandle cell = EditorGUI::begin_h_layout(context, element_id,
                    header ? "Table header cell" : "Table cell", layout);
                if(status)
                {
                    GUI::ElementHandle indicator = EditorGUI::begin_v_layout(context,
                        GUI::make_scoped_id(element_id, 1), "Status LED", fixed(10.0f, 10.0f));
                    Float4U led = style_color(context, status_color);
                    Float4U glow = led;
                    glow.w *= 0.45f;
                    rounded_shadow(context, glow, 5.0f, Float2U(), 3.0f, GUI::ShadowMode::outer);
                    rounded_rect(context, style_color(context, "gui.border"), 5.0f);
                    rounded_rect(context, led, 4.0f, RectF(1.0f, 1.0f, -2.0f, -2.0f));
                    EditorGUI::end_v_layout(context, indicator);
                }
                typography_label(context, GUI::make_scoped_id(element_id, 2), value,
                    grow_x(), header ? EditorGUI::TypographyRole::caption : EditorGUI::TypographyRole::cite);
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = status ? 8.0f : 0.0f;
                EditorGUI::end_h_layout(context, cell, flex);
            }

            void build_tables(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "tables", "GUITEST - TABLES", "Dense, resizable data",
                    "Readable rows, clear selection, subtle hierarchy, and generous splitter hit targets.",
                    EditorGUI::IconName::list_bullets);
                GUI::ElementHandle card = begin_card(context, "tables.memory", fill_width(470.0f),
                    "Memory Profiler", "Visual row states and resize-handle targets");
                GUI::ElementHandle toolbar = EditorGUI::begin_h_layout(context, id(context, "tables.toolbar"),
                    "Table toolbar", fill_width(44.0f));
                EditorGUI::icon(context, id(context, "tables.filter.icon"), EditorGUI::IconName::magnifying_glass,
                    fixed(18.0f, 18.0f), icon_desc(context, "gui.text.muted", EditorGUI::IconWeight::regular, 18.0f));
                EditorGUI::TextInputDesc filter_desc;
                filter_desc.placeholder = "Filter allocations...";
                EditorGUI::input_text(context, id(context, "tables.filter"), state.table_filter, grow_x(), filter_desc);
                icon_button(context, id(context, "tables.snapshot"), EditorGUI::IconName::camera,
                    "Snapshot", fixed(130.0f, 44.0f));
                icon_button(context, id(context, "tables.settings"), EditorGUI::IconName::gear_six,
                    "Table settings", fixed(44.0f, 44.0f), EditorGUI::ButtonDesc(), false);
                GUI::FlexLayoutDesc toolbar_flex;
                toolbar_flex.axis = GUI::LayoutAxis::x;
                toolbar_flex.cross_alignment = GUI::FlexAlignment::stretch;
                toolbar_flex.main_axis_gap = 8.0f;
                EditorGUI::end_h_layout(context, toolbar, toolbar_flex);

                GUI::LayoutConfig table_layout = fill_width(294.0f);
                table_layout.padding = Float4U(1.0f);
                EditorGUI::TableDesc table_desc;
                table_desc.gap = Float2U(1.0f);
                table_desc.clip_children = true;
                table_desc.fixed_row_height_mode = true;
                table_desc.fixed_row_height = 48.0f;
                GUI::ElementHandle table = EditorGUI::begin_table_layout(context, id(context, "tables.grid"),
                    "Memory allocation table", table_layout, table_desc);
                rounded_rect(context, style_color(context, "gui.border"),
                    style_scalar(context, "gui.radius.medium", 9.0f));
                rounded_rect(context, style_color(context, "gui.surface.1"),
                    max(style_scalar(context, "gui.radius.medium", 9.0f) - 1.0f, 0.0f),
                    RectF(1.0f, 1.0f, -2.0f, -2.0f));
                rounded_rect(context, style_color(context, "gui.surface.2"), 0.0f,
                    RectF(1.0f, 1.0f, -2.0f, 48.0f));

                static const f32 COLUMN_WIDTHS[] = { 0.06f, 0.23f, 0.16f, 0.13f, 0.16f, 0.26f };
                GUI::TableTrackDesc columns[6];
                for(usize i = 0; i < 6; ++i)
                {
                    columns[i].kind = GUI::TableTrackSizeKind::percent;
                    columns[i].value = COLUMN_WIDTHS[i];
                }
                EditorGUI::set_table_columns(context, Span<const GUI::TableTrackDesc>(columns, 6));

                static const c8* HEADERS[] = { "#", "TYPE", "CATEGORY", "SIZE", "ALLOCATIONS", "STATUS" };
                static const TableRecord RECORDS[] = {
                    { { "0001", "RHI::Texture", "Graphics", "128.0 MB", "12", "Resident" }, "gui.status.success" },
                    { { "0002", "VG::ShapeBuffer", "Vector", "48.6 MB", "64", "Resident" }, "gui.status.success" },
                    { { "0003", "Asset::Texture", "Content", "32.1 MB", "24", "Streaming" }, "gui.status.warning" },
                    { { "0004", "EditorGUI::State", "Interface", "2.8 MB", "1,264", "Resident" }, "gui.status.success" },
                    { { "0005", "Shader Cache", "Graphics", "1.4 MB", "116", "Offline" }, "gui.status.off" }
                };
                EditorGUI::begin_table_row(context);
                for(usize column = 0; column < 6; ++column)
                {
                    table_cell(context, GUI::make_scoped_id(table.id, (u64)column + 1),
                        HEADERS[column], true, false, nullptr);
                }
                EditorGUI::end_table_row(context);
                for(usize row = 0; row < 5; ++row)
                {
                    f32 row_y = 50.0f + (f32)row * 49.0f;
                    if((i32)row == state.table_selection)
                    {
                        rounded_rect(context, style_color(context, "gui.accent.subtle"), 0.0f,
                            RectF(1.0f, row_y, -2.0f, 48.0f));
                        rounded_rect(context, style_color(context, "gui.accent"), 0.0f,
                            RectF(1.0f, row_y, 3.0f, 48.0f));
                    }
                    else if(row & 1)
                    {
                        Float4U zebra = style_color(context, "gui.surface.2");
                        zebra.w *= 0.52f;
                        rounded_rect(context, zebra, 0.0f, RectF(1.0f, row_y, -2.0f, 48.0f));
                    }
                    EditorGUI::begin_table_row(context);
                    for(usize column = 0; column < 6; ++column)
                    {
                        EditorGUI::id_t cell_id = GUI::make_scoped_id(
                            GUI::make_scoped_id(table.id, (u64)row + 10), (u64)column + 1);
                        table_cell(context, cell_id, RECORDS[row].cells[column], false,
                            column == 5, RECORDS[row].status_color);
                    }
                    EditorGUI::end_table_row(context);
                }
                EditorGUI::end_table_layout(context, table);
                end_panel(context, card);
            }

            void build_overlay(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                section_heading(context, "overlay", "GUITEST - OVERLAY", "Menus, popup, tooltip, and dialog layers",
                    "Transient surfaces use painter-ordered backdrop capture, separable blur, and translucent tint.",
                    EditorGUI::IconName::bell);
                GUI::ElementHandle row;
                page_two_columns(context, id(context, "overlay.row"), row, 420.0f);
                GUI::ElementHandle menu_card = begin_card(context, "overlay.menu", grow_x(),
                    "Menu bar", "Shortcut, checked, disabled, and separator states");
                GUI::ElementHandle menu_bar = EditorGUI::begin_menu_bar(context, id(context, "overlay.menu.bar"),
                    "Menu", fill_width(style_scalar(context, "gui.control.height", 32.0f)));
                if(EditorGUI::begin_menu(context, id(context, "overlay.menu.file"), "File"))
                {
                    icon_menu_item(context, id(context, "overlay.menu.new"), EditorGUI::IconName::file_plus, "New");
                    icon_menu_item(context, id(context, "overlay.menu.save"), EditorGUI::IconName::floppy_disk, "Save All");
                    EditorGUI::menu_separator(context, id(context, "overlay.menu.separator"));
                    GUI::ElementHandle grid_item = icon_menu_item(context, id(context, "overlay.menu.grid"),
                        state.menu_grid ? EditorGUI::IconName::check_circle : EditorGUI::IconName::grid_four, "Show Grid");
                    if(EditorGUI::is_item_valid(context, grid_item) && EditorGUI::is_item_clicked(context, grid_item))
                    {
                        state.menu_grid = !state.menu_grid;
                    }
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 230.0f, 190.0f)));
                }
                EditorGUI::end_menu_bar(context, menu_bar);
                end_panel(context, menu_card);
                GUI::ElementHandle popup_card = begin_card(context, "overlay.popup", grow_x(),
                    "Popup and tooltip", "Independent GUI layers with captured backdrop blur");
                handles.popup_button = icon_button(context, id(context, "overlay.popup.open"),
                    EditorGUI::IconName::squares_four, "Open Popup", fill_width(48.0f));
                GUI::ElementHandle tooltip_button = icon_button(context, id(context, "overlay.tooltip"),
                    EditorGUI::IconName::info, "Hover for tooltip", fill_width(48.0f));
                EditorGUI::set_item_tooltip(context, id(context, "overlay.tooltip.layer"), tooltip_button,
                    "Available on hover or pointer focus.");
                GUI::ElementHandle backdrop_source = EditorGUI::begin_h_layout(context,
                    id(context, "overlay.popup.backdrop_source"), "Backdrop source", fill_width(56.0f));
                const c8* backdrop_colors[] = {
                    "gui.accent", "gui.status.busy", "gui.status.success", "gui.status.warning"
                };
                for(usize i = 0; i < 4; ++i)
                {
                    GUI::ElementHandle sample = EditorGUI::begin_v_layout(context,
                        GUI::make_scoped_id(backdrop_source.id, (u64)i + 1),
                        "Backdrop color sample", grow_x(56.0f));
                    rounded_rect(context, style_color(context, backdrop_colors[i]), 9.0f);
                    EditorGUI::end_v_layout(context, sample);
                }
                GUI::FlexLayoutDesc backdrop_flex;
                backdrop_flex.axis = GUI::LayoutAxis::x;
                backdrop_flex.cross_alignment = GUI::FlexAlignment::stretch;
                backdrop_flex.main_axis_gap = 6.0f;
                EditorGUI::end_h_layout(context, backdrop_source, backdrop_flex);
                led_label(context, id(context, "overlay.notification.success"), "Build complete",
                    "gui.status.success", 150.0f);
                led_label(context, id(context, "overlay.notification.warning"), "2 warnings",
                    "gui.status.warning", 150.0f);
                end_panel(context, popup_card);
                end_two_columns(context, row);

                EditorGUI::PopupDesc popup_desc;
                popup_desc.position = Float2U(735.0f, 330.0f);
                GUI::ElementHandle popup;
                if(EditorGUI::begin_popup(context, id(context, "overlay.popup.layer"), popup_desc, &popup))
                {
                    label(context, id(context, "overlay.popup.title"), "Popup layer", fill_width(34.0f), 13.0f);
                    label(context, id(context, "overlay.popup.copy"), "Dismiss with Done or Escape.",
                        fill_width(34.0f), 11.0f, "gui.text.secondary");
                    icon_button(context, id(context, "overlay.popup.done"), EditorGUI::IconName::check,
                        "Done", fill_width(44.0f), EditorGUI::ButtonDesc(), true, EditorGUI::IconWeight::bold);
                    lupanic_if_failed(EditorGUI::end_popup(context, popup, RectF(0.0f, 0.0f, 260.0f, 154.0f)));
                }
            }

            void build_workspace(GUI::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "workspace", "GUITEST - WORKSPACE", "Docked editor composition",
                    "Tabs, splitters, viewport tools, console, and status feedback in one functional software surface.",
                    EditorGUI::IconName::sidebar);
                EditorGUI::id_t dock_space_id = id(context, "workspace.dockspace");
                EditorGUI::id_t hierarchy_id = id(context, "workspace.panel.hierarchy");
                EditorGUI::id_t viewport_id = id(context, "workspace.panel.viewport");
                EditorGUI::id_t material_editor_id = id(context, "workspace.panel.material_editor");
                EditorGUI::id_t shader_graph_id = id(context, "workspace.panel.shader_graph");
                EditorGUI::id_t inspector_id = id(context, "workspace.panel.inspector");
                EditorGUI::id_t layers_id = id(context, "workspace.panel.layers");
                EditorGUI::id_t console_id = id(context, "workspace.panel.console");
                EditorGUI::id_t build_id = id(context, "workspace.panel.build");
                EditorGUI::id_t pinned_inspector_id = id(context, "workspace.panel.pinned_inspector");

                GUI::ElementHandle workspace = EditorGUI::begin_v_layout(context, id(context, "workspace.demo"),
                    "Workspace", fill_width(660.0f));
                f32 workspace_radius = style_scalar(context, "gui.radius.large", 16.0f);
                surface(context, "gui.surface.0", workspace_radius, true, true);
                GUI::DrawCommand push_workspace_clip;
                push_workspace_clip.type = GUI::DrawCommandType::push_clip;
                push_workspace_clip.rect_reference = GUI::DrawCommandRectReference::element;
                push_workspace_clip.radius = workspace_radius;
                context->draw(push_workspace_clip);
                GUI::DrawConfig workspace_clip;
                workspace_clip.name = Name("guitest.workspace.rounded_clip");
                workspace_clip.callback = finish_rounded_children_clip;
                workspace_clip.userdata = (void*)(usize)(u32)workspace_radius;
                workspace_clip.phases = GUI::DrawPhaseFlag::after_children;
                context->set_draw_config(workspace, workspace_clip);

                GUI::ElementHandle reset_layout_item;
                GUI::ElementHandle menu_bar = EditorGUI::begin_menu_bar(context, id(context, "workspace.menu"),
                    "Workspace menu", fill_width(style_scalar(context, "gui.control.height", 32.0f)));
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.file"), "File"))
                {
                    icon_menu_item(context, id(context, "workspace.menu.file.new"),
                        EditorGUI::IconName::file_plus, "New Scene");
                    icon_menu_item(context, id(context, "workspace.menu.file.open"),
                        EditorGUI::IconName::folder_open, "Open...");
                    EditorGUI::menu_separator(context, id(context, "workspace.menu.file.separator"));
                    EditorGUI::MenuItemDesc save_desc;
                    save_desc.shortcut = "Ctrl+S";
                    icon_menu_item(context, id(context, "workspace.menu.file.save"),
                        EditorGUI::IconName::floppy_disk, "Save", save_desc);
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 142.0f)));
                }
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.edit"), "Edit"))
                {
                    EditorGUI::MenuItemDesc undo_desc;
                    undo_desc.shortcut = "Ctrl+Z";
                    icon_menu_item(context, id(context, "workspace.menu.edit.undo"),
                        EditorGUI::IconName::arrow_counter_clockwise, "Undo", undo_desc);
                    EditorGUI::MenuItemDesc redo_desc;
                    redo_desc.shortcut = "Ctrl+Y";
                    icon_menu_item(context, id(context, "workspace.menu.edit.redo"),
                        EditorGUI::IconName::arrow_clockwise, "Redo", redo_desc);
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 86.0f)));
                }
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.create"), "Create"))
                {
                    icon_menu_item(context, id(context, "workspace.menu.create.material"),
                        EditorGUI::IconName::palette, "Material");
                    icon_menu_item(context, id(context, "workspace.menu.create.light"),
                        EditorGUI::IconName::lightning, "Light");
                    icon_menu_item(context, id(context, "workspace.menu.create.camera"),
                        EditorGUI::IconName::camera, "Camera");
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 190.0f, 118.0f)));
                }
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.tools"), "Tools"))
                {
                    icon_menu_item(context, id(context, "workspace.menu.tools.bake"),
                        EditorGUI::IconName::spinner_gap, "Bake Materials");
                    icon_menu_item(context, id(context, "workspace.menu.tools.profiler"),
                        EditorGUI::IconName::database, "Memory Profiler");
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 210.0f, 86.0f)));
                }
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.window"), "Window"))
                {
                    GUI::ElementHandle grid_item = icon_menu_item(context,
                        id(context, "workspace.menu.window.grid"),
                        state.menu_grid ? EditorGUI::IconName::check_circle : EditorGUI::IconName::grid_four, "Show Grid");
                    if(EditorGUI::is_item_valid(context, grid_item) && EditorGUI::is_item_clicked(context, grid_item))
                    {
                        state.menu_grid = !state.menu_grid;
                    }
                    reset_layout_item = icon_menu_item(context, id(context, "workspace.menu.window.reset"),
                        EditorGUI::IconName::arrows_counter_clockwise, "Reset Dock Layout");
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 86.0f)));
                }
                if(EditorGUI::begin_menu(context, id(context, "workspace.menu.help"), "Help"))
                {
                    icon_menu_item(context, id(context, "workspace.menu.help.docs"),
                        EditorGUI::IconName::file_text, "Documentation");
                    icon_menu_item(context, id(context, "workspace.menu.help.about"),
                        EditorGUI::IconName::info, "About Luna GUI");
                    lupanic_if_failed(EditorGUI::end_menu(context, RectF(0.0f, 0.0f, 210.0f, 86.0f)));
                }
                icon_label(context, id(context, "workspace.menu.document"), EditorGUI::IconName::file_code,
                    "Untitled Scene.luna*", grow_x(), 16.0f, 10.0f, "gui.text.secondary");
                EditorGUI::end_menu_bar(context, menu_bar);

                if(EditorGUI::is_item_valid(context, reset_layout_item) &&
                    EditorGUI::is_item_clicked(context, reset_layout_item))
                {
                    state.workspace_layout_initialized = false;
                }
                if(!state.workspace_layout_initialized)
                {
                    EditorGUI::DockSpaceLayoutDesc layout;
                    layout.root_node = 0;
                    layout.nodes.resize(7);
                    layout.nodes[0].split = true;
                    layout.nodes[0].split_axis = EditorGUI::DockSplitAxis::x;
                    layout.nodes[0].split_ratio = 0.15f;
                    layout.nodes[0].child0 = 1;
                    layout.nodes[0].child1 = 2;
                    layout.nodes[1].tabs.push_back(hierarchy_id);
                    layout.nodes[1].selected_tab = hierarchy_id;
                    layout.nodes[2].split = true;
                    layout.nodes[2].split_axis = EditorGUI::DockSplitAxis::x;
                    layout.nodes[2].split_ratio = 0.82f;
                    layout.nodes[2].child0 = 3;
                    layout.nodes[2].child1 = 4;
                    layout.nodes[3].split = true;
                    layout.nodes[3].split_axis = EditorGUI::DockSplitAxis::y;
                    layout.nodes[3].split_ratio = 0.76f;
                    layout.nodes[3].child0 = 5;
                    layout.nodes[3].child1 = 6;
                    layout.nodes[4].tabs.push_back(inspector_id);
                    layout.nodes[4].tabs.push_back(layers_id);
                    layout.nodes[4].selected_tab = inspector_id;
                    layout.nodes[5].tabs.push_back(viewport_id);
                    layout.nodes[5].tabs.push_back(material_editor_id);
                    layout.nodes[5].tabs.push_back(shader_graph_id);
                    layout.nodes[5].selected_tab = viewport_id;
                    layout.nodes[6].tabs.push_back(console_id);
                    layout.nodes[6].tabs.push_back(build_id);
                    layout.nodes[6].selected_tab = console_id;
                    EditorGUI::DockSpaceFloatingPanelDesc floating;
                    floating.panel = pinned_inspector_id;
                    floating.rect = RectF(610.0f, 118.0f, 380.0f, 238.0f);
                    floating.z_order = 1;
                    layout.floating_panels.push_back(floating);
                    EditorGUI::set_dockspace_layout(context, dock_space_id, layout);
                    state.workspace_layout_initialized = true;
                }

                EditorGUI::DockSpaceDesc dock_space_desc;
                dock_space_desc.splitter_size = 6.0f;
                dock_space_desc.splitter_visual_size = 1.0f;
                dock_space_desc.splitter_color = style_color(context, "gui.border.strong");
                dock_space_desc.docking_indicator_color = style_color(context, "gui.accent");
                EditorGUI::begin_dock_space(context, dock_space_id, "Workspace dock space", grow_y(), dock_space_desc);

                EditorGUI::DockPanelDesc panel_desc;
                panel_desc.close_button = false;
                panel_desc.title_bar_height = style_scalar(context, "gui.control.height", 40.0f);
                panel_desc.title_bar_color = style_color(context, "gui.surface.1");
                panel_desc.border_color = style_color(context, "gui.border.strong");

                if(EditorGUI::begin_dock_panel(context, hierarchy_id, "Hierarchy", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(10.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.hierarchy.content"), "Hierarchy content", panel_layout);
                    tree_label(context, id(context, "workspace.scene"), EditorGUI::IconName::folder_open,
                        "Scene", 0, true, true);
                    tree_label(context, id(context, "workspace.camera"), EditorGUI::IconName::camera,
                        "Camera", 1, false, false);
                    tree_label(context, id(context, "workspace.ball"), EditorGUI::IconName::bounding_box,
                        "Material Ball", 1, false, false, true);
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }

                if(EditorGUI::begin_dock_panel(context, viewport_id, "Viewport", nullptr, panel_desc))
                {
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.viewport.content"), "Viewport content", grow_y());
                    GUI::ElementHandle tools = EditorGUI::begin_h_layout(context,
                        id(context, "workspace.viewport.tools"), "Viewport tools", fill_width(44.0f));
                    const EditorGUI::IconName tool_icons[] = {
                        EditorGUI::IconName::selection, EditorGUI::IconName::arrows_out, EditorGUI::IconName::bounding_box
                    };
                    const c8* tool_labels[] = { "Select", "Move", "Frame" };
                    for(i32 tool_index = 0; tool_index < 3; ++tool_index)
                    {
                        EditorGUI::id_t tool_id = GUI::make_scoped_id(tools.id, (u64)tool_index + 1);
                        GUI::ElementHandle tool = icon_button(context, tool_id, tool_icons[tool_index],
                            tool_labels[tool_index], fixed(40.0f, 40.0f), EditorGUI::ButtonDesc(), false,
                            tool_index == state.space_mode ? EditorGUI::IconWeight::bold : EditorGUI::IconWeight::regular,
                            tool_index == state.space_mode);
                        if(EditorGUI::is_item_valid(context, tool) && EditorGUI::is_item_clicked(context, tool))
                        {
                            state.space_mode = tool_index;
                        }
                    }
                    GUI::FlexLayoutDesc tool_flex;
                    tool_flex.axis = GUI::LayoutAxis::x;
                    tool_flex.cross_alignment = GUI::FlexAlignment::center;
                    tool_flex.main_axis_gap = 6.0f;
                    EditorGUI::end_h_layout(context, tools, tool_flex);
                    EditorGUI::image(context, id(context, "workspace.image"), state.material_preview,
                        grow_y(), flipped_image());
                    end_panel(context, panel, 0.0f);
                    EditorGUI::end_dock_panel(context);
                }
                if(EditorGUI::begin_dock_panel(context, material_editor_id, "Material Editor", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(12.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.material.content"), "Material editor content", panel_layout);
                    icon_label(context, id(context, "workspace.material.title"), EditorGUI::IconName::palette,
                        "M_Rusted_Metal", fill_width(34.0f), 18.0f, 13.0f);
                    EditorGUI::slider_float(context, id(context, "workspace.material.roughness"), &state.roughness,
                        0.0f, 1.0f, fill_width(48.0f));
                    EditorGUI::slider_float(context, id(context, "workspace.material.metallic"), &state.metallic,
                        0.0f, 1.0f, fill_width(48.0f));
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }
                if(EditorGUI::begin_dock_panel(context, shader_graph_id, "Shader Graph", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(12.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.shader.content"), "Shader graph content", panel_layout);
                    icon_label(context, id(context, "workspace.shader.title"), EditorGUI::IconName::code,
                        "Surface to PBR Output", fill_width(40.0f), 18.0f, 13.0f);
                    label(context, id(context, "workspace.shader.note"),
                        "Drag this tab to create a floating shader workspace.", fill_width(40.0f), 10.0f,
                        "gui.text.secondary");
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }

                if(EditorGUI::begin_dock_panel(context, inspector_id, "Inspector", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(12.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.inspector.content"), "Inspector content", panel_layout);
                    icon_label(context, id(context, "workspace.inspector.transform"), EditorGUI::IconName::bounding_box,
                        "Transform", fill_width(34.0f), 18.0f, 12.0f);
                    EditorGUI::drag_float3(context, id(context, "workspace.position"), state.position, -100.0f, 100.0f,
                        fill_width(48.0f));
                    EditorGUI::checkbox(context, id(context, "workspace.visible"), "Visible", &state.checkbox_value,
                        fill_width(48.0f));
                    EditorGUI::slider_float(context, id(context, "workspace.roughness"), &state.roughness, 0.0f, 1.0f,
                        fill_width(48.0f));
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }
                if(EditorGUI::begin_dock_panel(context, layers_id, "Layers", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(12.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.layers.content"), "Layers content", panel_layout);
                    EditorGUI::checkbox(context, id(context, "workspace.layers.geometry"), "Geometry",
                        &state.checkbox_value, fill_width(40.0f));
                    EditorGUI::checkbox(context, id(context, "workspace.layers.lighting"), "Lighting",
                        &state.live_preview, fill_width(40.0f));
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }

                if(EditorGUI::begin_dock_panel(context, console_id, "Console", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(10.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.console.content"), "Console content", panel_layout);
                    icon_label(context, id(context, "workspace.console.info"), EditorGUI::IconName::info,
                        "Dock workspace initialized.", fill_width(30.0f), 15.0f, 10.0f, "gui.text.secondary");
                    icon_label(context, id(context, "workspace.console.warning"), EditorGUI::IconName::warning,
                        "Texture mip chain is incomplete.", fill_width(30.0f), 15.0f, 10.0f,
                        "gui.status.warning", EditorGUI::IconWeight::fill);
                    end_panel(context, panel, 2.0f);
                    EditorGUI::end_dock_panel(context);
                }
                if(EditorGUI::begin_dock_panel(context, build_id, "Build", nullptr, panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(10.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.build.content"), "Build content", panel_layout);
                    icon_label(context, id(context, "workspace.build.title"), EditorGUI::IconName::spinner_gap,
                        "Material bake queue", fill_width(30.0f), 15.0f, 10.0f,
                        "gui.text.secondary", EditorGUI::IconWeight::bold);
                    EditorGUI::ProgressBarDesc progress_desc;
                    progress_desc.show_overlay = false;
                    EditorGUI::progress_bar(context, id(context, "workspace.build.progress"), 0.68f,
                        fill_width(10.0f), progress_desc);
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }

                EditorGUI::DockPanelDesc floating_panel_desc = panel_desc;
                floating_panel_desc.close_button = true;
                if(EditorGUI::begin_dock_panel(context, pinned_inspector_id, "Inspector (Pinned)",
                    &state.pinned_inspector_open, floating_panel_desc))
                {
                    GUI::LayoutConfig panel_layout = grow_y();
                    panel_layout.padding = Float4U(12.0f);
                    GUI::ElementHandle panel = EditorGUI::begin_v_layout(context,
                        id(context, "workspace.pinned.content"), "Pinned inspector content", panel_layout);
                    label(context, id(context, "workspace.pinned.scale"), "Scale",
                        fill_width(30.0f), 11.0f, "gui.text.secondary");
                    EditorGUI::drag_float3(context, id(context, "workspace.pinned.position"), state.position,
                        -100.0f, 100.0f, fill_width(48.0f));
                    EditorGUI::checkbox(context, id(context, "workspace.pinned.visible"), "Visible",
                        &state.checkbox_value, fill_width(48.0f));
                    end_panel(context, panel);
                    EditorGUI::end_dock_panel(context);
                }
                EditorGUI::end_dock_space(context);

                GUI::FlexLayoutDesc workspace_flex;
                workspace_flex.axis = GUI::LayoutAxis::y;
                workspace_flex.cross_alignment = GUI::FlexAlignment::stretch;
                EditorGUI::end_v_layout(context, workspace, workspace_flex);
            }

            void build_content(GUI::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUI::LayoutConfig layout = grow_x();
                layout.padding = state.section == 0 ? Float4U(18.0f) : Float4U(0.0f);
                GUI::ElementHandle content = EditorGUI::begin_v_layout(context, id(context, "showcase.content"),
                    "Content", layout);
                rounded_rect(context, style_color(context, "gui.canvas"), 0.0f);
                if(state.section == 0)
                {
                    build_overview(context, state, handles);
                }
                else
                {
                    EditorGUI::ScrollViewDesc desc;
                    desc.horizontal = false;
                    EditorGUI::begin_scroll_view(context, id(context, "showcase.content.scroll"), "Content scroll", grow_y(), desc);
                    GUI::LayoutConfig page_layout = grow_y();
                    page_layout.padding = Float4U(18.0f);
                    GUI::ElementHandle page = EditorGUI::begin_v_layout(context,
                        id(context, "showcase.content.page"), "Scrollable page", page_layout);
                    if(state.section == 1) build_primitives(context, state);
                    else if(state.section == 2) build_buttons(context, state);
                    else if(state.section == 3) build_input(context, state);
                    else if(state.section == 4) build_layouts(context, state);
                    else if(state.section == 5) build_scroll_views(context);
                    else if(state.section == 6) build_tables(context, state);
                    else if(state.section == 7) build_overlay(context, state, handles);
                    else build_workspace(context, state);
                    GUI::FlexLayoutDesc page_flex;
                    page_flex.axis = GUI::LayoutAxis::y;
                    page_flex.cross_alignment = GUI::FlexAlignment::stretch;
                    page_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                    page_flex.clip_children = false;
                    EditorGUI::end_v_layout(context, page, page_flex);
                    EditorGUI::end_scroll_view(context);
                }
                GUI::FlexLayoutDesc content_flex;
                content_flex.axis = GUI::LayoutAxis::y;
                content_flex.cross_alignment = GUI::FlexAlignment::stretch;
                content_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                content_flex.clip_children = false;
                EditorGUI::end_v_layout(context, content, content_flex);
            }

            void token_row(GUI::IContext* context, ShowcaseState& state, EditorGUI::id_t element_id,
                const c8* label_text, const c8* token, const c8* color_key)
            {
                GUI::ElementHandle row = EditorGUI::begin_h_layout(context, element_id, token, fill_width(28.0f));
                EditorGUI::ShapeWidgetDesc swatch;
                swatch.tint = style_color(context, color_key);
                EditorGUI::shape(context, GUI::make_scoped_id(element_id, 1), state.circle,
                    fixed(17.0f, 17.0f), swatch);
                label(context, GUI::make_scoped_id(element_id, 2), label_text, fixed(68.0f, 24.0f), 10.0f,
                    "gui.text.secondary");
                label(context, GUI::make_scoped_id(element_id, 3), token, grow_x(24.0f),
                    8.0f, "gui.text.muted");
                GUI::FlexLayoutDesc flex;
                flex.axis = GUI::LayoutAxis::x;
                flex.cross_alignment = GUI::FlexAlignment::center;
                flex.main_axis_gap = 6.0f;
                EditorGUI::end_h_layout(context, row, flex);
            }

            void build_token_panel(GUI::IContext* context, ShowcaseState& state)
            {
                GUI::LayoutConfig layout = grow_x();
                layout.width.kind = GUI::SizeKind::fixed;
                layout.width.value = 248.0f;
                layout.padding = Float4U(14.0f);
                GUI::ElementHandle panel = EditorGUI::begin_v_layout(context, id(context, "showcase.tokens"),
                    "Style Inspector", layout);
                rounded_rect(context, style_color(context, "gui.surface.0"), 0.0f);
                GUI::ElementHandle title = EditorGUI::begin_h_layout(context,
                    id(context, "showcase.tokens.title"), "Style Inspector", fill_width(34.0f));
                EditorGUI::icon(context, id(context, "showcase.tokens.title.icon"), EditorGUI::IconName::palette,
                    fixed(20.0f, 20.0f), icon_desc(context, "gui.text.color", EditorGUI::IconWeight::duotone, 20.0f));
                label(context, id(context, "showcase.tokens.title.text"), "Style Inspector", grow_x(), 13.0f);
                icon_button(context, id(context, "showcase.tokens.settings"), EditorGUI::IconName::gear_six,
                    "Style settings", fixed(30.0f, 30.0f), EditorGUI::ButtonDesc(), false);
                GUI::FlexLayoutDesc title_flex;
                title_flex.axis = GUI::LayoutAxis::x;
                title_flex.cross_alignment = GUI::FlexAlignment::center;
                title_flex.main_axis_gap = 8.0f;
                EditorGUI::end_h_layout(context, title, title_flex);
                const c8* style_name = state.theme == 0 ? (state.density == 0 ? "light.compact" : "light.touch") :
                    (state.density == 0 ? "dark.compact" : "dark.touch");
                label(context, id(context, "showcase.tokens.style"), style_name, fill_width(20.0f),
                    9.0f, "gui.text.muted");
                divider(context, id(context, "showcase.tokens.divider1"));
                label(context, id(context, "showcase.tokens.accent.title"), "ACCENT STATES",
                    fill_width(24.0f), 9.0f, "gui.text.muted");
                token_row(context, state, id(context, "showcase.tokens.accent"), "Accent", "--accent", "gui.accent");
                token_row(context, state, id(context, "showcase.tokens.hover"), "Hover", "--accent-hover", "gui.accent.hovered");
                token_row(context, state, id(context, "showcase.tokens.pressed"), "Pressed", "--accent-active", "gui.accent.active");
                token_row(context, state, id(context, "showcase.tokens.subtle"), "Subtle", "--accent-soft", "gui.accent.subtle");
                token_row(context, state, id(context, "showcase.tokens.disabled"), "Disabled", "--accent-disabled", "gui.accent.disabled");
                token_row(context, state, id(context, "showcase.tokens.focus"), "Focus", "--focus", "gui.focus");
                divider(context, id(context, "showcase.tokens.divider2"));
                label(context, id(context, "showcase.tokens.gray.title"), "GRAY HIERARCHY",
                    fill_width(24.0f), 9.0f, "gui.text.muted");
                token_row(context, state, id(context, "showcase.tokens.surface0"), "Surface 0", "--surface-0", "gui.surface.0");
                token_row(context, state, id(context, "showcase.tokens.surface1"), "Surface 1", "--surface-1", "gui.surface.1");
                token_row(context, state, id(context, "showcase.tokens.surface2"), "Surface 2", "--surface-2", "gui.surface.2");
                token_row(context, state, id(context, "showcase.tokens.surface3"), "Surface 3", "--surface-3", "gui.surface.3");
                token_row(context, state, id(context, "showcase.tokens.surface4"), "Surface 4", "--surface-4", "gui.surface.4");
                token_row(context, state, id(context, "showcase.tokens.surface5"), "Surface 5", "--surface-5", "gui.surface.5");
                divider(context, id(context, "showcase.tokens.divider3"));
                label(context, id(context, "showcase.tokens.density.title"), "DENSITY",
                    fill_width(24.0f), 9.0f, "gui.text.muted");
                c8 density_text[96];
                snprintf(density_text, sizeof(density_text), "Control height                     %d lu",
                    state.density == 1 ? 48 : 32);
                label(context, id(context, "showcase.tokens.density.control"), density_text,
                    fill_width(28.0f), 10.0f, "gui.text.secondary");
                snprintf(density_text, sizeof(density_text), "Hit target                         %d lu",
                    state.density == 1 ? 48 : 32);
                label(context, id(context, "showcase.tokens.density.hit"), density_text,
                    fill_width(28.0f), 10.0f, "gui.text.secondary");
                snprintf(density_text, sizeof(density_text), "Section gap                        %d lu",
                    state.density == 1 ? 16 : 10);
                label(context, id(context, "showcase.tokens.density.gap"), density_text,
                    fill_width(28.0f), 10.0f, "gui.text.secondary");
                divider(context, id(context, "showcase.tokens.divider4"));
                label(context, id(context, "showcase.tokens.note"),
                    "Soft shadow is rendered analytically. Blur surfaces use ordinary alpha in this phase.",
                    fill_width(72.0f), 10.0f, "gui.text.secondary");
                end_panel(context, panel, 5.0f);
            }
        }

        GUI::ElementHandle build_showcase(GUI::IContext* context, ShowcaseState& state,
            ShowcaseHandles& handles)
        {
            context->push_layer(1, Float2U(0.0f));
            GUI::ElementHandle root = EditorGUI::begin_v_layout(context, id(context, "showcase.root"),
                "GUI Design Language Lab", fill());
            rounded_rect(context, style_color(context, "gui.canvas"), 0.0f);
            build_top_bar(context, state);
            GUI::ElementHandle body = EditorGUI::begin_h_layout(context, id(context, "showcase.body"),
                "Application body", grow_y());
            build_navigation(context, state, handles);
            build_content(context, state, handles);
            build_token_panel(context, state);
            GUI::FlexLayoutDesc body_flex;
            body_flex.axis = GUI::LayoutAxis::x;
            body_flex.cross_alignment = GUI::FlexAlignment::stretch;
            EditorGUI::end_h_layout(context, body, body_flex);
            GUI::FlexLayoutDesc root_flex;
            root_flex.axis = GUI::LayoutAxis::y;
            root_flex.cross_alignment = GUI::FlexAlignment::stretch;
            EditorGUI::end_v_layout(context, root, root_flex);
            context->pop_layer();
            return root;
        }

        void resolve_showcase(GUI::IContext* context, ShowcaseState& state,
            const ShowcaseHandles& handles)
        {
                for(i32 i = 0; i < 9; ++i)
            {
                if(EditorGUI::is_item_valid(context, handles.navigation[i]) &&
                    EditorGUI::is_item_clicked(context, handles.navigation[i])) state.section = i;
            }
            for(i32 i = 0; i < 4; ++i)
            {
                if(EditorGUI::is_item_valid(context, handles.style_options[i]) &&
                    EditorGUI::is_item_clicked(context, handles.style_options[i]))
                {
                    state.theme = i / 2;
                    state.density = i % 2;
                }
            }
            for(i32 i = 0; i < 3; ++i)
            {
                if(EditorGUI::is_item_valid(context, handles.assets[i]) && EditorGUI::is_item_clicked(context, handles.assets[i]))
                {
                    state.selected_asset = i;
                }
            }
            if(EditorGUI::is_item_valid(context, handles.popup_button) &&
                EditorGUI::is_item_clicked(context, handles.popup_button))
            {
                EditorGUI::open_popup(context, id(context, "overlay.popup.layer"));
            }
        }
    }
}
