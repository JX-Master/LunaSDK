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
    namespace GUITest
    {
        namespace
        {
            constexpr const c8* SECTION_LABELS[] = {
                "Overview", "Primitives", "Buttons", "Input", "Layouts",
                "Scroll Views", "Tables", "Overlay", "Workspace"
            };

            GUICore::LayoutConfig fixed(f32 width, f32 height)
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = width;
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
                layout.flex_shrink = 0.0f;
                return layout;
            }

            GUICore::LayoutConfig fill()
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUICore::SizeKind::percent;
                layout.height.value = 1.0f;
                return layout;
            }

            GUICore::LayoutConfig fill_width(f32 height)
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
                layout.flex_shrink = 0.0f;
                return layout;
            }

            GUICore::LayoutConfig grow()
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::fit;
                layout.height.kind = GUICore::SizeKind::fit;
                layout.flex_grow = 1.0f;
                return layout;
            }

            GUICore::LayoutConfig grow_x(f32 height = 0.0f)
            {
                GUICore::LayoutConfig layout = grow();
                if(height > 0.0f)
                {
                    layout.height.kind = GUICore::SizeKind::fixed;
                    layout.height.value = height;
                }
                else
                {
                    layout.height.kind = GUICore::SizeKind::percent;
                    layout.height.value = 1.0f;
                }
                return layout;
            }

            GUICore::LayoutConfig grow_y()
            {
                GUICore::LayoutConfig layout = grow();
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                return layout;
            }

            GUI::id_t id(GUICore::IContext* context, const c8* value)
            {
                return context->make_id(value);
            }

            Float4U style_color(GUICore::IContext* context, const c8* entry, const Float4U& fallback = Float4U(1.0f))
            {
                return context->get_style_value(Name(GUI::DEFAULT_STYLE_NAME), Name(entry),
                    GUICore::style_f32x4(fallback)).number;
            }

            f32 style_scalar(GUICore::IContext* context, const c8* entry, f32 fallback)
            {
                return context->get_style_value(Name(GUI::DEFAULT_STYLE_NAME), Name(entry),
                    GUICore::style_f32(fallback)).number.x;
            }

            void rounded_rect(GUICore::IContext* context, const Float4U& color, f32 radius,
                const RectF& rect = RectF())
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                context->draw(command);
            }

            void rounded_shadow(GUICore::IContext* context, const Float4U& color, f32 radius,
                const Float2U& offset, f32 softness, GUICore::ShadowMode mode,
                const RectF& rect = RectF())
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::shadow;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = rect;
                command.color = color;
                command.radius = radius;
                command.shadow.offset = offset;
                command.shadow.softness = softness;
                command.shadow.mode = mode;
                context->draw(command);
            }

            RV draw_sdf_swatch(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                const GUICore::Element* element_data = context->get_element(element.index);
                if(!element_data) return ok;
                const RectF& element_rect = element_data->layout_result.rect;
                f32 width = max(element_rect.width, 1.0f);
                f32 height = max(element_rect.height, 1.0f);
                u32 kind = (u32)(usize)userdata;
                Vector<f32> shape_floats;
                if(kind == 0)
                {
                    GUICore::sdf_shape_add_rounded_rectangle(shape_floats,
                        RectF(5.0f, 5.0f, width - 10.0f, height - 10.0f),
                        Float4U(18.0f, 8.0f, 18.0f, 8.0f));
                }
                else if(kind == 1)
                {
                    GUICore::sdf_shape_add_operation(shape_floats,
                        GUICore::SDFShapeInstruction::difference_op);
                    GUICore::sdf_shape_add_circle(shape_floats, Float2U(width * 0.5f, height * 0.5f),
                        min(width, height) * 0.42f);
                    GUICore::sdf_shape_add_circle(shape_floats, Float2U(width * 0.5f, height * 0.5f),
                        min(width, height) * 0.21f);
                }
                else
                {
                    GUICore::sdf_shape_add_capsule(shape_floats, Float2U(18.0f, height * 0.5f),
                        Float2U(width - 18.0f, height * 0.5f), min(height * 0.32f, 28.0f));
                }
                auto shape = context->append_sdf_shape_program(shape_floats.cspan());
                if(failed(shape)) return shape.errcode();

                Float4U accent = style_color(context, "gui.accent");
                Float4U highlight = style_color(context, "gui.shadow.light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.8f));
                Float4U active = style_color(context, "gui.accent.active", accent);
                GUICore::SDFGradientStop stops[] = {
                    {0.0f, highlight, 0.42f}, {0.48f, accent, 0.58f}, {1.0f, active, 0.5f}
                };

                auto submit_color = [&](Vector<f32>& color_floats) -> RV
                {
                    auto color = context->append_sdf_color_program(color_floats.cspan());
                    if(failed(color)) return color.errcode();
                    GUICore::DrawCommand command;
                    command.type = GUICore::DrawCommandType::sdf;
                    command.rect_reference = GUICore::DrawCommandRectReference::element;
                    command.sdf.shape = shape.get();
                    command.sdf.color = color.get();
                    context->draw(command);
                    return ok;
                };

                Vector<f32> color_floats;
                if(kind == 0)
                {
                    Vector<f32> shadow_floats;
                    GUICore::sdf_color_add_shadow(shadow_floats,
                        style_color(context, "gui.shadow.dark", Float4U(0.0f, 0.0f, 0.0f, 0.2f)),
                        Float2U(3.0f, 4.0f), 6.0f, 0.0f, GUICore::SDFClipDesc::inner(0.0f));
                    RV result = submit_color(shadow_floats);
                    if(failed(result)) return result;
                    GUICore::sdf_color_add_linear_gradient(color_floats, Float2U(0.0f, 0.0f),
                        Float2U(width, height), Span<const GUICore::SDFGradientStop>(stops, 3));
                }
                else if(kind == 1)
                {
                    GUICore::sdf_color_add_radial_gradient(color_floats,
                        Float2U(width * 0.42f, height * 0.38f), Float2U(width * 0.55f, height * 0.55f),
                        Span<const GUICore::SDFGradientStop>(stops, 3));
                }
                else
                {
                    GUICore::sdf_color_add_conic_gradient(color_floats,
                        Float2U(width * 0.5f, height * 0.5f), -PI_DIV_TWO,
                        Span<const GUICore::SDFGradientStop>(stops, 3));
                }
                return submit_color(color_floats);
            }

            GUICore::ElementHandle sdf_swatch(GUICore::IContext* context, GUI::id_t element_id,
                const c8* name, u32 kind)
            {
                GUICore::ElementHandle swatch = GUI::begin_v_layout(context, element_id, name, grow_x(112.0f));
                GUICore::DrawConfig draw;
                draw.name = Name(name);
                draw.callback = draw_sdf_swatch;
                draw.userdata = (void*)(usize)(u32)kind;
                context->set_draw_config(swatch, draw);
                GUI::end_v_layout(context, swatch);
                return swatch;
            }

            void surface(GUICore::IContext* context, const c8* background, f32 radius,
                bool raised = false, bool strong_elevation = false)
            {
                if(raised)
                {
                    GUICore::DrawCommand shadow;
                    shadow.type = GUICore::DrawCommandType::shadow;
                    shadow.rect_reference = GUICore::DrawCommandRectReference::element;
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

            GUI::TextDesc text_desc(GUICore::IContext* context, f32 size, const c8* color = "gui.text.color")
            {
                GUI::TextDesc desc;
                // The HTML reference uses Inter while GUITest deliberately uses the bundled Open Sans.
                // Compensate for Open Sans's smaller apparent x-height in explicitly-sized showcase copy.
                desc.font_size = size > 0.0f && size <= 13.0f ? size + 2.0f : size;
                desc.color = style_color(context, color);
                return desc;
            }

            GUICore::ElementHandle label(GUICore::IContext* context, GUI::id_t element_id, const c8* value,
                const GUICore::LayoutConfig& layout, f32 size = 0.0f, const c8* color = "gui.text.color")
            {
                return GUI::text(context, element_id, value, layout, text_desc(context, size, color));
            }

            GUICore::ElementHandle begin_panel(GUICore::IContext* context, GUI::id_t element_id,
                const c8* name, const GUICore::LayoutConfig& input, const c8* background = "gui.surface.1",
                f32 radius = 12.0f, bool raised = true)
            {
                GUICore::ElementHandle panel = GUI::begin_v_layout(context, element_id, name, input);
                surface(context, background, radius, raised);
                return panel;
            }

            void end_panel(GUICore::IContext* context, const GUICore::ElementHandle& panel, f32 gap = 8.0f)
            {
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::y;
                flex.main_axis_gap = gap;
                flex.cross_alignment = GUICore::FlexAlignment::stretch;
                GUI::end_v_layout(context, panel, flex);
            }

            void divider(GUICore::IContext* context, GUI::id_t element_id)
            {
                GUICore::ElementHandle line = GUI::begin_v_layout(context, element_id, "Divider", fill_width(1.0f));
                rounded_rect(context, style_color(context, "gui.border"), 0.0f);
                GUI::end_v_layout(context, line);
            }

            void led_label(GUICore::IContext* context, GUI::id_t element_id,
                const c8* value, const c8* color_key, f32 width)
            {
                GUICore::LayoutConfig layout = fixed(width, 32.0f);
                layout.padding = Float4U(9.0f, 5.0f, 9.0f, 5.0f);
                GUICore::ElementHandle pill = GUI::begin_h_layout(context, element_id, value, layout);
                surface(context, "gui.surface.1", 16.0f, true);
                GUICore::ElementHandle indicator = GUI::begin_v_layout(context,
                    GUICore::make_scoped_id(element_id, 1), "LED", fixed(10.0f, 10.0f));
                Float4U led_color = style_color(context, color_key);
                if(std::strcmp(color_key, "gui.status.off") != 0)
                {
                    Float4U glow_color = led_color;
                    glow_color.w *= 0.56f;
                    rounded_shadow(context, glow_color, 5.0f, Float2U(0.0f), 3.0f,
                        GUICore::ShadowMode::outer);
                }
                rounded_rect(context, style_color(context, "gui.border"), 5.0f);
                rounded_rect(context, led_color, 4.0f, RectF(1.0f, 1.0f, -2.0f, -2.0f));
                Float4U highlight = style_color(context, "gui.shadow.inset_light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.7f));
                highlight.w *= 0.55f;
                rounded_shadow(context, highlight, 4.0f, Float2U(0.0f, -1.0f), 1.0f,
                    GUICore::ShadowMode::inner, RectF(1.0f, 1.0f, -2.0f, -2.0f));
                GUI::end_v_layout(context, indicator);
                label(context, GUICore::make_scoped_id(element_id, 2), value, grow_x(), 11.0f, "gui.text.secondary");
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::center;
                flex.main_axis_gap = 9.0f;
                GUI::end_h_layout(context, pill, flex);
            }

            void section_heading(GUICore::IContext* context, const c8* prefix, const c8* eyebrow,
                const c8* title, const c8* description)
            {
                String row_name(prefix);
                row_name.append(".heading");
                GUICore::ElementHandle row = GUI::begin_h_layout(context, id(context, row_name.c_str()),
                    "Section heading", fill_width(92.0f));
                String left_name(prefix);
                left_name.append(".heading.left");
                GUICore::ElementHandle left = GUI::begin_v_layout(context, id(context, left_name.c_str()),
                    "Section title", fixed(450.0f, 92.0f));
                label(context, GUICore::make_scoped_id(left.id, 1), eyebrow, fill_width(22.0f), 10.0f, "gui.accent.active");
                label(context, GUICore::make_scoped_id(left.id, 2), title, fill_width(48.0f), 28.0f);
                end_panel(context, left, 2.0f);
                label(context, GUICore::make_scoped_id(row.id, 3), description, grow_x(), 13.0f, "gui.text.secondary");
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::center;
                flex.main_axis_gap = 24.0f;
                GUI::end_h_layout(context, row, flex);
            }

            void card_title(GUICore::IContext* context, GUI::id_t parent, const c8* title, const c8* note)
            {
                label(context, GUICore::make_scoped_id(parent, 1), title, fill_width(24.0f), 13.0f);
                label(context, GUICore::make_scoped_id(parent, 2), note, fill_width(20.0f), 10.0f, "gui.text.muted");
                divider(context, GUICore::make_scoped_id(parent, 3));
            }

            GUICore::ElementHandle begin_card(GUICore::IContext* context, const c8* name,
                const GUICore::LayoutConfig& input, const c8* title, const c8* note)
            {
                GUICore::LayoutConfig layout = input;
                layout.padding = Float4U(16.0f);
                GUICore::ElementHandle card = begin_panel(context, id(context, name), title, layout,
                    "gui.surface.1", style_scalar(context, "gui.radius.large", 14.0f), true);
                card_title(context, card.id, title, note);
                return card;
            }

            void page_two_columns(GUICore::IContext* context, GUI::id_t element_id,
                GUICore::ElementHandle& row, f32 height)
            {
                row = GUI::begin_h_layout(context, element_id, "Card row", fill_width(height));
            }

            void end_two_columns(GUICore::IContext* context, const GUICore::ElementHandle& row)
            {
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::stretch;
                flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                flex.clip_children = false;
                GUI::end_h_layout(context, row, flex);
            }

            void property_row(GUICore::IContext* context, GUI::id_t element_id, const c8* name,
                ShowcaseState& state, i32 kind)
            {
                const f32 height = style_scalar(context, "gui.control.height", 40.0f) + 4.0f;
                GUICore::ElementHandle row = GUI::begin_h_layout(context, element_id, name, fill_width(height));
                label(context, GUICore::make_scoped_id(element_id, 1), name, fixed(78.0f, height),
                    10.0f, "gui.text.secondary");
                if(kind == 0)
                {
                    const c8* items[] = { "PBR Metallic", "Unlit" };
                    GUI::combo(context, GUICore::make_scoped_id(element_id, 2), "Shader Model", &state.combo_item,
                        Span<const c8*>(items, 2), grow_x());
                }
                else if(kind == 1)
                {
                    GUI::slider_float(context, GUICore::make_scoped_id(element_id, 2), &state.roughness,
                        0.0f, 1.0f, grow_x());
                }
                else if(kind == 2)
                {
                    GUI::slider_float(context, GUICore::make_scoped_id(element_id, 2), &state.metallic,
                        0.0f, 1.0f, grow_x());
                }
                else if(kind == 3)
                {
                    GUI::toggle_switch(context, GUICore::make_scoped_id(element_id, 2), "Normal Map",
                        &state.normal_map, grow_x());
                }
                else
                {
                    GUI::checkbox(context, GUICore::make_scoped_id(element_id, 2), "Use Triplanar Mapping",
                        &state.triplanar, grow_x());
                }
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::center;
                flex.main_axis_gap = 8.0f;
                GUI::end_h_layout(context, row, flex);
            }

            RHI::ITexture* material_texture(ShowcaseState& state, i32 index)
            {
                if(index == 0) return state.material_sand;
                if(index == 2) return state.material_concrete;
                return state.material_rusted;
            }

            GUI::ImageDesc flipped_image(f32 left = 0.0f, f32 right = 1.0f,
                f32 top = 0.0f, f32 bottom = 1.0f)
            {
                GUI::ImageDesc desc;
                desc.min_texcoord = Float2U(left, bottom);
                desc.max_texcoord = Float2U(right, top);
                return desc;
            }

            void build_top_bar(GUICore::IContext* context, ShowcaseState& state)
            {
                const f32 height = state.density == 1 ? 72.0f : 56.0f;
                GUICore::LayoutConfig layout = fill_width(height);
                layout.padding = Float4U(12.0f, 8.0f, 12.0f, 8.0f);
                GUICore::ElementHandle top = GUI::begin_h_layout(context, id(context, "showcase.topbar"),
                    "Top bar", layout);
                rounded_rect(context, style_color(context, "gui.surface.1"), 0.0f);

                GUICore::ElementHandle brand = GUI::begin_h_layout(context, id(context, "showcase.brand"),
                    "Brand", fixed(245.0f, height - 16.0f));
                GUICore::ElementHandle logo = GUI::begin_v_layout(context, id(context, "showcase.logo"),
                    "Logo", fixed(40.0f, 40.0f));
                rounded_rect(context, style_color(context, "gui.accent"), 12.0f);
                label(context, id(context, "showcase.logo.text"), "L", fill(), 20.0f, "gui.accent.ink");
                GUI::end_v_layout(context, logo);
                GUICore::ElementHandle brand_text = GUI::begin_v_layout(context, id(context, "showcase.brand.text"),
                    "Brand text", grow_x());
                label(context, id(context, "showcase.brand.title"), "Luna GUI", fill_width(22.0f), 15.0f);
                label(context, id(context, "showcase.brand.subtitle"), "Design Language Lab", fill_width(18.0f),
                    11.0f, "gui.text.muted");
                end_panel(context, brand_text, 0.0f);
                GUICore::FlexLayoutDesc brand_flex;
                brand_flex.axis = GUICore::LayoutAxis::x;
                brand_flex.cross_alignment = GUICore::FlexAlignment::center;
                brand_flex.main_axis_gap = 11.0f;
                GUI::end_h_layout(context, brand, brand_flex);

                GUICore::ElementHandle status = GUI::begin_h_layout(context, id(context, "showcase.status"),
                    "System status", grow_x());
                led_label(context, id(context, "showcase.status.renderer"), "Renderer Online",
                    "gui.status.success", 128.0f);
                led_label(context, id(context, "showcase.status.bake"), "Bake Queue  2",
                    "gui.status.busy", 112.0f);
                led_label(context, id(context, "showcase.status.sync"), "Remote Sync",
                    "gui.status.off", 106.0f);
                led_label(context, id(context, "showcase.status.error"), "Error  1",
                    "gui.status.error", 86.0f);
                GUICore::FlexLayoutDesc status_flex;
                status_flex.axis = GUICore::LayoutAxis::x;
                status_flex.cross_alignment = GUICore::FlexAlignment::center;
                status_flex.main_alignment = GUICore::FlexAlignment::center;
                status_flex.main_axis_gap = 9.0f;
                GUI::end_h_layout(context, status, status_flex);

                GUICore::ElementHandle controls = GUI::begin_h_layout(context, id(context, "showcase.controls"),
                    "Style controls", fixed(430.0f, height - 16.0f));
                const c8* themes[] = { "Light", "Dark" };
                GUI::button_group(context, id(context, "showcase.theme"), Span<const c8*>(themes, 2),
                    &state.theme, fixed(126.0f, height - 22.0f));
                const c8* densities[] = { "Compact", "Touch" };
                GUI::button_group(context, id(context, "showcase.density"), Span<const c8*>(densities, 2),
                    &state.density, fixed(158.0f, height - 22.0f));
                GUICore::LayoutConfig accent_layout = grow_x(height - 22.0f);
                accent_layout.padding = Float4U(9.0f, 4.0f, 9.0f, 4.0f);
                GUICore::ElementHandle accent = GUI::begin_h_layout(context, id(context, "showcase.accent"),
                    "Accent", accent_layout);
                surface(context, "gui.surface.1", 18.0f, false);
                GUI::ShapeWidgetDesc accent_shape;
                accent_shape.tint = style_color(context, "gui.accent");
                GUI::shape(context, id(context, "showcase.accent.swatch"), state.circle,
                    fixed(24.0f, 24.0f), accent_shape);
                label(context, id(context, "showcase.accent.value"), "#E34F59", grow_x(), 10.0f, "gui.text.secondary");
                GUI::end_h_layout(context, accent, brand_flex);
                GUICore::FlexLayoutDesc controls_flex;
                controls_flex.axis = GUICore::LayoutAxis::x;
                controls_flex.cross_alignment = GUICore::FlexAlignment::center;
                controls_flex.main_axis_gap = 7.0f;
                GUI::end_h_layout(context, controls, controls_flex);

                GUICore::FlexLayoutDesc top_flex;
                top_flex.axis = GUICore::LayoutAxis::x;
                top_flex.cross_alignment = GUICore::FlexAlignment::center;
                top_flex.main_axis_gap = 12.0f;
                GUI::end_h_layout(context, top, top_flex);
            }

            void build_navigation(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUICore::LayoutConfig layout = grow_x();
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = 196.0f;
                layout.padding = Float4U(10.0f, 14.0f, 10.0f, 12.0f);
                GUICore::ElementHandle navigation = GUI::begin_v_layout(context, id(context, "showcase.navigation"),
                    "Navigation", layout);
                rounded_rect(context, style_color(context, "gui.surface.0"), 0.0f);
                label(context, id(context, "showcase.navigation.title"), "COMPONENTS        gui.editor",
                    fill_width(34.0f), 10.0f, "gui.text.muted");
                for(i32 i = 0; i < 9; ++i)
                {
                    handles.navigation[i] = GUI::selectable(context,
                        GUICore::make_scoped_id(navigation.id, (u64)i + 10), SECTION_LABELS[i], state.section == i,
                        fill_width(style_scalar(context, "gui.control.height", 40.0f)));
                }
                GUICore::ElementHandle spacer = GUI::begin_v_layout(context, id(context, "showcase.navigation.spacer"),
                    "Spacer", grow_y());
                GUI::end_v_layout(context, spacer);
                GUICore::LayoutConfig footer_layout = fill_width(92.0f);
                footer_layout.padding = Float4U(12.0f);
                GUICore::ElementHandle footer = begin_panel(context, id(context, "showcase.navigation.footer"),
                    "Active style", footer_layout, "gui.surface.1", 10.0f);
                label(context, id(context, "showcase.navigation.footer.label"), "ACTIVE LEAF STYLE",
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                const c8* style_name = state.theme == 0 ? (state.density == 0 ? "light.compact" : "light.touch") :
                    (state.density == 0 ? "dark.compact" : "dark.touch");
                label(context, id(context, "showcase.navigation.footer.value"), style_name,
                    fill_width(22.0f), 12.0f, "gui.accent.active");
                label(context, id(context, "showcase.navigation.footer.note"), "Same component tree - Style only",
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                end_panel(context, footer, 3.0f);
                end_panel(context, navigation, 4.0f);
            }

            GUICore::ElementHandle style_option(GUICore::IContext* context, GUI::id_t element_id,
                const c8* value, bool selected, const GUICore::LayoutConfig& layout)
            {
                GUICore::ElementHandle option = GUI::begin_button(context, element_id, value, layout);
                if(selected)
                {
                    rounded_rect(context, style_color(context, "gui.accent"), 9.0f);
                    rounded_rect(context, style_color(context, "gui.accent.subtle"), 8.0f,
                        RectF(1.0f, 1.0f, -2.0f, -2.0f));
                }
                label(context, GUICore::make_scoped_id(element_id, 1), value, grow_x(), 11.0f,
                    selected ? "gui.accent.active" : "gui.text.secondary");
                GUI::end_button(context);
                return option;
            }

            void build_style_matrix(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles,
                const GUICore::LayoutConfig& layout)
            {
                GUICore::ElementHandle matrix = GUI::begin_v_layout(context, id(context, "overview.matrix"),
                    "Style matrix", layout);
                const c8* labels[] = { "Light / Compact", "Light / Touch", "Dark / Compact", "Dark / Touch" };
                for(i32 row_index = 0; row_index < 2; ++row_index)
                {
                    GUICore::ElementHandle row = GUI::begin_h_layout(context,
                        GUICore::make_scoped_id(matrix.id, (u64)row_index + 1), "Style row", grow_y());
                    for(i32 column = 0; column < 2; ++column)
                    {
                        i32 option_index = row_index * 2 + column;
                        bool selected = option_index == state.theme * 2 + state.density;
                        handles.style_options[option_index] = style_option(context,
                            GUICore::make_scoped_id(row.id, (u64)column + 1), labels[option_index], selected, grow_x());
                    }
                    GUICore::FlexLayoutDesc row_flex;
                    row_flex.axis = GUICore::LayoutAxis::x;
                    row_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                    row_flex.main_axis_gap = 9.0f;
                    GUI::end_h_layout(context, row, row_flex);
                }
                end_panel(context, matrix, 9.0f);
            }

            void build_asset_row(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles,
                GUI::id_t parent, i32 index, const c8* name, const c8* meta)
            {
                GUICore::LayoutConfig layout = fill_width(54.0f);
                layout.padding = Float4U(5.0f);
                handles.assets[index] = GUI::begin_button(context, GUICore::make_scoped_id(parent, (u64)index + 20),
                    name, layout);
                if(state.selected_asset == index)
                {
                    rounded_rect(context, style_color(context, "gui.accent"), 8.0f);
                    rounded_rect(context, style_color(context, "gui.accent.subtle"), 7.0f,
                        RectF(1.0f, 1.0f, -2.0f, -2.0f));
                }
                GUI::image(context, GUICore::make_scoped_id(parent, (u64)index + 30), material_texture(state, index),
                    fixed(40.0f, 40.0f), flipped_image());
                GUICore::LayoutConfig text_layout = grow_x();
                text_layout.margin.x = 9.0f;
                GUICore::ElementHandle text_column = GUI::begin_v_layout(context,
                    GUICore::make_scoped_id(parent, (u64)index + 40), "Asset name", text_layout);
                label(context, GUICore::make_scoped_id(parent, (u64)index + 50), name,
                    fill_width(20.0f), 10.0f);
                label(context, GUICore::make_scoped_id(parent, (u64)index + 60), meta,
                    fill_width(18.0f), 9.0f, "gui.text.muted");
                end_panel(context, text_column, 0.0f);
                GUI::end_button(context);
            }

            void build_mini_editor(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUICore::LayoutConfig editor_layout = grow_y();
                editor_layout.padding = Float4U(state.density == 1 ? 12.0f : 10.0f);
                GUICore::ElementHandle editor = GUI::begin_h_layout(context, id(context, "overview.editor"),
                    "DCC editor composition", editor_layout);
                surface(context, "gui.surface.0", style_scalar(context, "gui.radius.large", 16.0f), true, true);

                GUICore::LayoutConfig library_layout = fixed(176.0f, 0.0f);
                library_layout.height.kind = GUICore::SizeKind::percent;
                library_layout.height.value = 1.0f;
                library_layout.padding = Float4U(10.0f);
                GUICore::ElementHandle library = begin_panel(context, id(context, "overview.library"),
                    "Library", library_layout, "gui.surface.1", 15.0f, false);
                label(context, id(context, "overview.library.title"), "Scene Hierarchy        +",
                    fill_width(34.0f), 11.0f);
                GUI::TextInputDesc search_desc;
                search_desc.placeholder = "Search scene...";
                GUI::input_text(context, id(context, "overview.library.search"), state.search_query,
                    fill_width(style_scalar(context, "gui.control.small_height", 32.0f)), search_desc);
                label(context, id(context, "overview.tree.root"), "v  Desert_Outpost", fill_width(28.0f),
                    10.0f, "gui.text.secondary");
                label(context, id(context, "overview.tree.environment"), "    v  Environment", fill_width(28.0f),
                    10.0f, "gui.text.secondary");
                label(context, id(context, "overview.tree.sun"), "        Sun_Light", fill_width(28.0f),
                    10.0f, "gui.text.muted");
                GUI::selectable(context, id(context, "overview.tree.ball"), "        Material_Ball", true,
                    fill_width(30.0f));
                divider(context, id(context, "overview.library.divider"));
                label(context, id(context, "overview.library.assets"), "Assets                              3 items",
                    fill_width(28.0f), 10.0f, "gui.text.secondary");
                build_asset_row(context, state, handles, library.id, 0, "M_Desert_Sand", "Material - 3.6 MB");
                build_asset_row(context, state, handles, library.id, 1, "M_Rusted_Metal", "Material - 2.1 MB");
                build_asset_row(context, state, handles, library.id, 2, "M_Concrete_Grime", "Material - 1.8 MB");
                end_panel(context, library, 5.0f);

                GUICore::ElementHandle viewport = GUI::begin_v_layout(context, id(context, "overview.viewport"),
                    "Viewport", grow_x());
                GUICore::ElementHandle toolbar = GUI::begin_h_layout(context, id(context, "overview.viewport.toolbar"),
                    "Viewport toolbar", fill_width(48.0f));
                rounded_rect(context, style_color(context, "gui.surface.1"), 0.0f);
                const c8* tools[] = { "Select", "Move", "Rotate", "Frame" };
                GUI::button_group(context, id(context, "overview.viewport.tools"), Span<const c8*>(tools, 4),
                    &state.selected_group, fixed(270.0f, 40.0f));
                const c8* spaces[] = { "Local", "World" };
                GUI::button_group(context, id(context, "overview.viewport.space"), Span<const c8*>(spaces, 2),
                    &state.space_mode, fixed(126.0f, 40.0f));
                GUI::text_button(context, id(context, "overview.viewport.more"), "...", fixed(40.0f, 40.0f));
                GUICore::FlexLayoutDesc toolbar_flex;
                toolbar_flex.axis = GUICore::LayoutAxis::x;
                toolbar_flex.cross_alignment = GUICore::FlexAlignment::center;
                toolbar_flex.main_alignment = GUICore::FlexAlignment::space_between;
                GUI::end_h_layout(context, toolbar, toolbar_flex);
                GUI::image(context, id(context, "overview.viewport.image"), state.material_preview,
                    fill_width(state.density == 1 ? 420.0f : 444.0f), flipped_image(0.14f, 0.86f));
                GUICore::LayoutConfig console_layout = fill_width(122.0f);
                console_layout.padding = Float4U(10.0f, 0.0f, 10.0f, 8.0f);
                GUICore::ElementHandle console = begin_panel(context, id(context, "overview.console"),
                    "Console", console_layout, "gui.surface.1", 0.0f, false);
                const c8* console_tabs[] = { "Console", "Bake Queue 2", "Messages" };
                GUI::button_group(context, id(context, "overview.console.tabs"),
                    Span<const c8*>(console_tabs, 3), &state.console_tab, fill_width(36.0f));
                label(context, id(context, "overview.console.line1"),
                    "[ready]  Renderer initialized successfully.                         10:23:45",
                    fill_width(24.0f), 9.0f, "gui.text.secondary");
                label(context, id(context, "overview.console.line2"),
                    "[busy]   Bake queue contains two materials.                         10:24:12",
                    fill_width(24.0f), 9.0f, "gui.text.secondary");
                end_panel(context, console, 2.0f);
                GUICore::FlexLayoutDesc viewport_flex;
                viewport_flex.axis = GUICore::LayoutAxis::y;
                viewport_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                GUI::end_v_layout(context, viewport, viewport_flex);

                GUICore::LayoutConfig inspector_layout = fixed(220.0f, 0.0f);
                inspector_layout.height.kind = GUICore::SizeKind::percent;
                inspector_layout.height.value = 1.0f;
                inspector_layout.padding = Float4U(12.0f);
                GUICore::ElementHandle inspector = begin_panel(context, id(context, "overview.inspector"),
                    "Inspector", inspector_layout, "gui.surface.1", 15.0f, false);
                GUICore::ElementHandle asset_header = GUI::begin_h_layout(context, id(context, "overview.inspector.asset"),
                    "Selected asset", fill_width(52.0f));
                GUI::image(context, id(context, "overview.inspector.thumb"),
                    material_texture(state, state.selected_asset), fixed(40.0f, 40.0f), flipped_image());
                const c8* asset_names[] = { "M_Desert_Sand", "M_Rusted_Metal", "M_Concrete_Grime" };
                label(context, id(context, "overview.inspector.name"), asset_names[state.selected_asset],
                    grow_x(), 10.0f);
                GUICore::FlexLayoutDesc asset_flex;
                asset_flex.axis = GUICore::LayoutAxis::x;
                asset_flex.cross_alignment = GUICore::FlexAlignment::center;
                asset_flex.main_axis_gap = 9.0f;
                GUI::end_h_layout(context, asset_header, asset_flex);
                const c8* inspector_tabs[] = { "Inspector", "Channels" };
                GUI::button_group(context, id(context, "overview.inspector.tabs"),
                    Span<const c8*>(inspector_tabs, 2), &state.inspector_tab, fill_width(40.0f));
                property_row(context, id(context, "overview.property.shader"), "Shader Model", state, 0);
                property_row(context, id(context, "overview.property.roughness"), "Roughness", state, 1);
                property_row(context, id(context, "overview.property.metallic"), "Metallic", state, 2);
                property_row(context, id(context, "overview.property.normal"), "Normal Map", state, 3);
                property_row(context, id(context, "overview.property.triplanar"), "Triplanar", state, 4);
                end_panel(context, inspector, 4.0f);

                GUICore::FlexLayoutDesc editor_flex;
                editor_flex.axis = GUICore::LayoutAxis::x;
                editor_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                editor_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                GUI::end_h_layout(context, editor, editor_flex);
            }

            void build_overview(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                section_heading(context, "overview", "SELECTED DIRECTION - SOFT WORKSHOP",
                    "A tactile workbench for serious tools",
                    "Warm machined surfaces, disciplined red accents, and instrument-like status semantics, scaled from compact desktop density to touch.");
                GUICore::LayoutConfig banner_layout = fill_width(194.0f);
                banner_layout.padding = Float4U(28.0f);
                GUICore::ElementHandle banner = GUI::begin_h_layout(context, id(context, "overview.banner"),
                    "Style summary", banner_layout);
                surface(context, "gui.surface.1", style_scalar(context, "gui.radius.large", 16.0f), true, true);
                GUICore::ElementHandle summary = GUI::begin_v_layout(context, id(context, "overview.banner.summary"),
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
                GUICore::FlexLayoutDesc banner_flex;
                banner_flex.axis = GUICore::LayoutAxis::x;
                banner_flex.cross_alignment = GUICore::FlexAlignment::center;
                banner_flex.main_axis_gap = 24.0f;
                GUI::end_h_layout(context, banner, banner_flex);
                build_mini_editor(context, state, handles);
            }

            void build_primitives(GUICore::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "primitives", "GUITEST - PRIMITIVES", "Type, imagery, progress, and system light",
                    "Foundational drawing and semantic feedback, including the LED vocabulary used by editor status surfaces.");
                GUICore::ElementHandle row;
                page_two_columns(context, id(context, "primitives.row1"), row, 280.0f);
                GUICore::ElementHandle typography = begin_card(context, "primitives.typography", grow_x(),
                    "Typography", "Default UI font and numeric hierarchy");
                label(context, id(context, "primitives.type.h1"), "Editor title", fill_width(44.0f), 24.0f);
                label(context, id(context, "primitives.type.h2"), "Panel heading", fill_width(34.0f), 17.0f);
                label(context, id(context, "primitives.type.body"),
                    "Primary body text keeps dense application copy readable.", fill_width(42.0f), 12.0f,
                    "gui.text.secondary");
                label(context, id(context, "primitives.type.code"), "Float3U(1.0f, 2.0f, 3.0f)",
                    fill_width(34.0f), 11.0f, "gui.accent.active");
                end_panel(context, typography);
                GUICore::ElementHandle progress = begin_card(context, "primitives.progress", grow_x(),
                    "Progress and LED", "Determinate work and semantic status");
                label(context, id(context, "primitives.progress.label1"), "Baking textures                         68%",
                    fill_width(24.0f), 11.0f, "gui.text.secondary");
                GUI::ProgressBarDesc progress_desc;
                progress_desc.show_overlay = false;
                GUI::progress_bar(context, id(context, "primitives.progress.1"), 0.68f, fill_width(10.0f),
                    progress_desc);
                label(context, id(context, "primitives.progress.label2"), "Compiling shaders                      42%",
                    fill_width(24.0f), 11.0f, "gui.text.secondary");
                GUI::progress_bar(context, id(context, "primitives.progress.2"), 0.42f, fill_width(10.0f),
                    progress_desc);
                led_label(context, id(context, "primitives.led.online"), "Online", "gui.status.success", 100.0f);
                led_label(context, id(context, "primitives.led.working"), "Working", "gui.status.busy", 100.0f);
                end_panel(context, progress);
                end_two_columns(context, row);
                GUICore::ElementHandle sdf_card = begin_card(context, "primitives.sdf", fill_width(220.0f),
                    "SDF shape and paint programs", "Linear, radial, conic, CSG, capsule, and analytic shadow");
                GUICore::ElementHandle sdf_row = GUI::begin_h_layout(context, id(context, "primitives.sdf.row"),
                    "SDF swatches", grow_y());
                sdf_swatch(context, id(context, "primitives.sdf.linear"), "Linear rounded rectangle", 0);
                sdf_swatch(context, id(context, "primitives.sdf.radial"), "Radial CSG ring", 1);
                sdf_swatch(context, id(context, "primitives.sdf.conic"), "Conic capsule", 2);
                GUICore::FlexLayoutDesc sdf_flex;
                sdf_flex.axis = GUICore::LayoutAxis::x;
                sdf_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                sdf_flex.main_axis_gap = 14.0f;
                GUI::end_h_layout(context, sdf_row, sdf_flex);
                end_panel(context, sdf_card);
                GUICore::ElementHandle image_card = begin_card(context, "primitives.image", fill_width(420.0f),
                    "Image and shape", "Raster material assets and vector primitives");
                GUI::image(context, id(context, "primitives.image.preview"), state.material_preview,
                    grow_y(), flipped_image(0.05f, 0.95f, 0.08f, 0.92f));
                end_panel(context, image_card);
            }

            void build_buttons(GUICore::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "buttons", "GUITEST - BUTTONS", "Actions and choices",
                    "Raised actions, recessed selection wells, explicit focus, and states that never depend on hover alone.");
                GUICore::ElementHandle row1;
                page_two_columns(context, id(context, "buttons.row1"), row1, 260.0f);
                GUICore::ElementHandle states = begin_card(context, "buttons.states", grow_x(),
                    "Button states", "Default, pressed, destructive, and disabled");
                GUI::text_button(context, id(context, "buttons.primary"), "Primary", fill_width(44.0f));
                GUI::text_button(context, id(context, "buttons.neutral"), "Neutral", fill_width(44.0f));
                GUI::ButtonDesc disabled;
                disabled.enabled = false;
                GUI::text_button(context, id(context, "buttons.disabled"), "Disabled", fill_width(44.0f), disabled);
                end_panel(context, states);
                GUICore::ElementHandle groups = begin_card(context, "buttons.groups", grow_x(),
                    "Button groups", "Single and multiple selection");
                const c8* items[] = { "Build", "Run", "Profile", "Ship" };
                GUI::button_group(context, id(context, "buttons.group.single"), Span<const c8*>(items, 4),
                    &state.selected_group, fill_width(48.0f));
                GUI::button_group(context, id(context, "buttons.group.multi"), Span<const c8*>(items, 4),
                    Span<bool>(state.selected_group_multi, 4), fill_width(48.0f));
                end_panel(context, groups);
                end_two_columns(context, row1);
                GUICore::ElementHandle row2;
                page_two_columns(context, id(context, "buttons.row2"), row2, 300.0f);
                GUICore::ElementHandle choices = begin_card(context, "buttons.choices", grow_x(),
                    "Selection controls", "Checkbox, radio, and switch");
                GUI::checkbox(context, id(context, "buttons.checkbox"), "Visible in render", &state.checkbox_value,
                    fill_width(48.0f));
                GUI::checkbox(context, id(context, "buttons.receive"), "Receive shadows", &state.receive_shadows,
                    fill_width(48.0f));
                GUI::radio_button(context, id(context, "buttons.radio.a"), "Radio A", &state.radio_value, 0,
                    fill_width(48.0f));
                GUI::radio_button(context, id(context, "buttons.radio.b"), "Radio B", &state.radio_value, 1,
                    fill_width(48.0f));
                GUI::toggle_switch(context, id(context, "buttons.toggle"), "Live preview", &state.live_preview,
                    fill_width(48.0f));
                end_panel(context, choices);
                GUICore::ElementHandle disclosure = begin_card(context, "buttons.disclosure", grow_x(),
                    "Disclosure and tree", "Expanded, selected, and leaf states");
                if(GUI::collapsing_header(context, id(context, "buttons.header"), "Transform", fill_width(48.0f)))
                {
                    label(context, id(context, "buttons.position"), "Position        0.00, 1.25, 0.00",
                        fill_width(34.0f), 10.0f, "gui.text.secondary");
                    label(context, id(context, "buttons.rotation"), "Rotation        0, 15, 0",
                        fill_width(34.0f), 10.0f, "gui.text.secondary");
                }
                GUI::tree_node(context, id(context, "buttons.tree.scene"), "Scene", GUI::TreeNodeFlag::none, 0,
                    fill_width(40.0f));
                GUI::tree_node(context, id(context, "buttons.tree.ball"), "Material Ball",
                    GUI::TreeNodeFlag::selected, 1, fill_width(40.0f));
                GUI::tree_node(context, id(context, "buttons.tree.camera"), "Camera",
                    GUI::TreeNodeFlag::leaf, 1, fill_width(40.0f));
                end_panel(context, disclosure);
                end_two_columns(context, row2);
            }

            void build_input(GUICore::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "input", "GUITEST - INPUT", "Text, numeric, slider, and color editing",
                    "Inset fields communicate editability while focus, read-only, disabled, and validation states remain distinct.");
                GUICore::ElementHandle row1;
                page_two_columns(context, id(context, "input.row1"), row1, 330.0f);
                GUICore::ElementHandle fields = begin_card(context, "input.fields", grow_x(),
                    "Text input", "Editable, read-only, and disabled");
                GUI::input_text(context, id(context, "input.asset"), state.asset_name, fill_width(48.0f));
                GUI::TextInputDesc read_only;
                read_only.read_only = true;
                GUI::input_text(context, id(context, "input.readonly"), state.readonly_value,
                    fill_width(48.0f), read_only);
                GUI::TextInputDesc disabled;
                disabled.enabled = false;
                GUI::input_text(context, id(context, "input.disabled"), state.readonly_value,
                    fill_width(48.0f), disabled);
                end_panel(context, fields);
                GUICore::ElementHandle sliders = begin_card(context, "input.sliders", grow_x(),
                    "Slider and drag editors", "Scalar, integer, and vector values");
                GUI::slider_float(context, id(context, "input.roughness"), &state.roughness, 0.0f, 1.0f,
                    fill_width(48.0f));
                GUI::slider_int(context, id(context, "input.subdivisions"), &state.subdivisions, 0, 100,
                    fill_width(48.0f));
                GUI::DragDesc drag_desc;
                drag_desc.speed = 0.05f;
                GUI::drag_float3(context, id(context, "input.position"), state.position, -100.0f, 100.0f,
                    fill_width(48.0f), drag_desc);
                end_panel(context, sliders);
                end_two_columns(context, row1);
                GUICore::ElementHandle row2;
                page_two_columns(context, id(context, "input.row2"), row2, 250.0f);
                GUICore::ElementHandle color = begin_card(context, "input.color", grow_x(),
                    "Color edit", "RGBA preview and numeric channels");
                GUI::color_edit4(context, id(context, "input.color.value"), "Preview color", state.preview_color,
                    fill_width(48.0f));
                end_panel(context, color);
                GUICore::ElementHandle combo = begin_card(context, "input.combo", grow_x(),
                    "Combo and search", "Popup-ready selector and filtered query");
                const c8* render_modes[] = { "PBR Metallic Roughness", "Subsurface", "Unlit" };
                GUI::combo(context, id(context, "input.combo.shader"), "Shader model", &state.combo_item,
                    Span<const c8*>(render_modes, 3), fill_width(48.0f));
                GUI::TextInputDesc search_desc;
                search_desc.placeholder = "Filter components...";
                GUI::input_text(context, id(context, "input.search"), state.search_query,
                    fill_width(48.0f), search_desc);
                end_panel(context, combo);
                end_two_columns(context, row2);
            }

            void build_layouts(GUICore::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "layouts", "GUITEST - LAYOUTS", "Flex, grid, canvas, focus scope, and tabs",
                    "The visual system scales without changing layout semantics or stable element identity.");
                GUICore::ElementHandle row1;
                page_two_columns(context, id(context, "layouts.row1"), row1, 300.0f);
                GUICore::ElementHandle flex_card = begin_card(context, "layouts.flex", grow_x(),
                    "Flex and grid", "Fixed, growing, and repeated cells");
                GUICore::ElementHandle flex_row = GUI::begin_h_layout(context, id(context, "layouts.flex.row"),
                    "Flex row", fill_width(48.0f));
                GUI::text_button(context, id(context, "layouts.flex.fixed1"), "Fixed", fixed(90.0f, 44.0f));
                GUI::text_button(context, id(context, "layouts.flex.grow"), "flex-grow: 1", grow_x(44.0f));
                GUI::text_button(context, id(context, "layouts.flex.fixed2"), "Fixed", fixed(90.0f, 44.0f));
                end_two_columns(context, flex_row);
                for(i32 row_index = 0; row_index < 2; ++row_index)
                {
                    GUICore::ElementHandle grid_row = GUI::begin_h_layout(context,
                        GUICore::make_scoped_id(flex_card.id, (u64)row_index + 20), "Grid row", fill_width(48.0f));
                    for(i32 column = 0; column < 4; ++column)
                    {
                        c8 text[16];
                        snprintf(text, sizeof(text), "Cell %d", row_index * 4 + column + 1);
                        GUI::text_button(context, GUICore::make_scoped_id(grid_row.id, (u64)column + 1), text,
                            grow_x(44.0f));
                    }
                    end_two_columns(context, grid_row);
                }
                end_panel(context, flex_card);
                GUICore::ElementHandle assets = begin_card(context, "layouts.assets", grow_x(),
                    "Asset grid", "Image, selection, and metadata");
                GUICore::ElementHandle asset_row = GUI::begin_h_layout(context, id(context, "layouts.assets.row"),
                    "Asset row", grow_y());
                for(i32 i = 0; i < 3; ++i)
                {
                    GUICore::ElementHandle item = GUI::begin_v_layout(context,
                        GUICore::make_scoped_id(asset_row.id, (u64)i + 1), "Asset", grow_x());
                    surface(context, i == state.selected_asset ? "gui.accent.subtle" : "gui.surface.2", 10.0f, false);
                    GUI::image(context, GUICore::make_scoped_id(item.id, 1), material_texture(state, i),
                        grow_y(), flipped_image());
                    const c8* names[] = { "Sand", "Rusted Metal", "Concrete" };
                    label(context, GUICore::make_scoped_id(item.id, 2), names[i], fill_width(28.0f), 10.0f);
                    end_panel(context, item, 5.0f);
                }
                end_two_columns(context, asset_row);
                end_panel(context, assets);
                end_two_columns(context, row1);
            }

            void build_scroll_views(GUICore::IContext* context)
            {
                section_heading(context, "scroll", "GUITEST - SCROLL VIEWS", "Overlay and reserved scrollbars",
                    "Compact pointer mode favors overlay scrollbars; touch mode keeps broad, visible affordances.");
                GUICore::ElementHandle row;
                page_two_columns(context, id(context, "scroll.row"), row, 620.0f);
                for(i32 column = 0; column < 2; ++column)
                {
                    const c8* card_name = column == 0 ? "scroll.overlay" : "scroll.visible";
                    GUICore::ElementHandle card = begin_card(context, card_name, grow_x(),
                        column == 0 ? "Dynamic overlay" : "Always visible",
                        column == 0 ? "Appears during interaction" : "Reserves viewport space");
                    GUI::ScrollViewDesc desc;
                    desc.horizontal = false;
                    desc.scrollbar_mode = column == 0 ? GUI::ScrollBarMode::dynamic_overlay :
                        GUI::ScrollBarMode::always_visible;
                    GUI::begin_scroll_view(context, GUICore::make_scoped_id(card.id, 10), "Rows", grow_y(), desc);
                    for(i32 i = 0; i < 16; ++i)
                    {
                        c8 value[32];
                        snprintf(value, sizeof(value), "%s row %02d", column == 0 ? "Overlay" : "Persistent", i + 1);
                        GUI::text_button(context, GUICore::make_scoped_id(card.id, (u64)i + 30), value,
                            fill_width(44.0f));
                    }
                    GUI::end_scroll_view(context);
                    end_panel(context, card);
                }
                end_two_columns(context, row);
            }

            void table_row(GUICore::IContext* context, GUI::id_t element_id, const c8* value, bool selected)
            {
                GUICore::LayoutConfig layout = fill_width(48.0f);
                layout.padding = Float4U(12.0f, 0.0f, 12.0f, 0.0f);
                GUICore::ElementHandle row = GUI::begin_h_layout(context, element_id, "Table row", layout);
                if(selected) rounded_rect(context, style_color(context, "gui.accent.subtle"), 0.0f);
                label(context, GUICore::make_scoped_id(element_id, 1), value, grow_x(), 11.0f,
                    selected ? "gui.accent.active" : "gui.text.secondary");
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::center;
                GUI::end_h_layout(context, row, flex);
            }

            void build_tables(GUICore::IContext* context)
            {
                section_heading(context, "tables", "GUITEST - TABLES", "Dense, resizable data",
                    "Readable rows, clear selection, subtle hierarchy, and generous splitter hit targets.");
                GUICore::ElementHandle card = begin_card(context, "tables.memory", fill_width(650.0f),
                    "Memory Profiler", "Visual row states and resize-handle targets");
                GUI::text_button(context, id(context, "tables.snapshot"), "Snapshot", fill_width(44.0f));
                table_row(context, id(context, "tables.header"),
                    "#       TYPE                         CATEGORY       SIZE        ALLOCATIONS       STATUS", false);
                table_row(context, id(context, "tables.row1"),
                    "0001    RHI::Texture                 Graphics       128.0 MB    12                Resident", true);
                table_row(context, id(context, "tables.row2"),
                    "0002    VG::ShapeBuffer              Vector         48.6 MB     64                Resident", false);
                table_row(context, id(context, "tables.row3"),
                    "0003    Asset::Texture               Content        32.1 MB     24                Streaming", false);
                table_row(context, id(context, "tables.row4"),
                    "0004    GUI::State                   Interface      2.8 MB      1264              Resident", false);
                table_row(context, id(context, "tables.row5"),
                    "0005    Shader Cache                 Graphics       1.4 MB      116               Offline", false);
                end_panel(context, card);
            }

            void build_overlay(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                section_heading(context, "overlay", "GUITEST - OVERLAY", "Menus, popup, tooltip, and dialog layers",
                    "Transient surfaces use stronger elevation and ordinary alpha while true backdrop blur remains deferred.");
                GUICore::ElementHandle row;
                page_two_columns(context, id(context, "overlay.row"), row, 420.0f);
                GUICore::ElementHandle menu_card = begin_card(context, "overlay.menu", grow_x(),
                    "Menu bar", "Shortcut, checked, disabled, and separator states");
                GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, id(context, "overlay.menu.bar"),
                    "Menu", fill_width(48.0f));
                if(GUI::begin_menu(context, id(context, "overlay.menu.file"), "File"))
                {
                    GUI::menu_item(context, id(context, "overlay.menu.new"), "New");
                    GUI::menu_item(context, id(context, "overlay.menu.save"), "Save All");
                    GUI::menu_separator(context, id(context, "overlay.menu.separator"));
                    GUI::menu_item(context, id(context, "overlay.menu.grid"), "Show Grid", &state.menu_grid);
                    lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 230.0f, 190.0f)));
                }
                GUI::end_menu_bar(context, menu_bar);
                end_panel(context, menu_card);
                GUICore::ElementHandle popup_card = begin_card(context, "overlay.popup", grow_x(),
                    "Popup and tooltip", "Independent GUICore layers with analytic shadow");
                handles.popup_button = GUI::text_button(context, id(context, "overlay.popup.open"), "Open Popup",
                    fill_width(48.0f));
                GUICore::ElementHandle tooltip_button = GUI::text_button(context, id(context, "overlay.tooltip"),
                    "Hover for tooltip", fill_width(48.0f));
                GUI::set_item_tooltip(context, id(context, "overlay.tooltip.layer"), tooltip_button,
                    "Available on hover or pointer focus.");
                led_label(context, id(context, "overlay.notification.success"), "Build complete",
                    "gui.status.success", 150.0f);
                led_label(context, id(context, "overlay.notification.warning"), "2 warnings",
                    "gui.status.warning", 150.0f);
                end_panel(context, popup_card);
                end_two_columns(context, row);

                GUI::PopupDesc popup_desc;
                popup_desc.position = Float2U(735.0f, 330.0f);
                GUICore::ElementHandle popup;
                if(GUI::begin_popup(context, id(context, "overlay.popup.layer"), popup_desc, &popup))
                {
                    label(context, id(context, "overlay.popup.title"), "Popup layer", fill_width(34.0f), 13.0f);
                    label(context, id(context, "overlay.popup.copy"), "Dismiss with Done or Escape.",
                        fill_width(34.0f), 11.0f, "gui.text.secondary");
                    GUI::text_button(context, id(context, "overlay.popup.done"), "Done", fill_width(44.0f));
                    lupanic_if_failed(GUI::end_popup(context, popup, RectF(0.0f, 0.0f, 260.0f, 154.0f)));
                }
            }

            void build_workspace(GUICore::IContext* context, ShowcaseState& state)
            {
                section_heading(context, "workspace", "GUITEST - WORKSPACE", "Docked editor composition",
                    "Tabs, splitters, viewport tools, console, and status feedback in one functional software surface.");
                GUICore::ElementHandle workspace = GUI::begin_v_layout(context, id(context, "workspace.demo"),
                    "Workspace", grow_y());
                surface(context, "gui.surface.0", style_scalar(context, "gui.radius.large", 16.0f), true, true);
                GUICore::ElementHandle menu = GUI::begin_h_layout(context, id(context, "workspace.menu"),
                    "Menu", fill_width(44.0f));
                label(context, id(context, "workspace.menu.items"),
                    "File     Edit     Create     Tools     Window     Help                  Untitled Scene.luna*",
                    grow_x(), 10.0f, "gui.text.secondary");
                GUI::end_h_layout(context, menu);
                GUICore::ElementHandle body = GUI::begin_h_layout(context, id(context, "workspace.body"),
                    "Workspace body", grow_y());
                GUICore::LayoutConfig hierarchy_layout = fixed(180.0f, 0.0f);
                hierarchy_layout.height.kind = GUICore::SizeKind::percent;
                hierarchy_layout.height.value = 1.0f;
                hierarchy_layout.padding = Float4U(10.0f);
                GUICore::ElementHandle hierarchy = begin_panel(context, id(context, "workspace.hierarchy"),
                    "Hierarchy", hierarchy_layout, "gui.surface.1", 0.0f, false);
                label(context, id(context, "workspace.hierarchy.title"), "Hierarchy", fill_width(40.0f), 12.0f);
                GUI::tree_node(context, id(context, "workspace.scene"), "Scene", GUI::TreeNodeFlag::none, 0,
                    fill_width(40.0f));
                GUI::tree_node(context, id(context, "workspace.camera"), "Camera", GUI::TreeNodeFlag::leaf, 1,
                    fill_width(40.0f));
                GUI::tree_node(context, id(context, "workspace.ball"), "Material Ball",
                    GUI::TreeNodeFlag::selected | GUI::TreeNodeFlag::leaf, 1, fill_width(40.0f));
                end_panel(context, hierarchy);
                GUICore::ElementHandle viewport = GUI::begin_v_layout(context, id(context, "workspace.viewport"),
                    "Viewport", grow_x());
                label(context, id(context, "workspace.tabs"), "Viewport       Material Editor       Shader Graph       +",
                    fill_width(44.0f), 11.0f, "gui.text.secondary");
                GUI::image(context, id(context, "workspace.image"), state.material_preview,
                    grow_y(), flipped_image());
                label(context, id(context, "workspace.console"),
                    "Console     Build\n[Info] Dock workspace initialized.\n[Warning] Texture mip chain is incomplete.",
                    fill_width(130.0f), 10.0f, "gui.text.secondary");
                GUI::end_v_layout(context, viewport);
                GUICore::LayoutConfig inspector_layout = fixed(220.0f, 0.0f);
                inspector_layout.height.kind = GUICore::SizeKind::percent;
                inspector_layout.height.value = 1.0f;
                inspector_layout.padding = Float4U(12.0f);
                GUICore::ElementHandle inspector = begin_panel(context, id(context, "workspace.inspector"),
                    "Inspector", inspector_layout, "gui.surface.1", 0.0f, false);
                label(context, id(context, "workspace.inspector.title"), "Inspector       Layers",
                    fill_width(44.0f), 11.0f);
                GUI::drag_float3(context, id(context, "workspace.position"), state.position, -100.0f, 100.0f,
                    fill_width(48.0f));
                GUI::checkbox(context, id(context, "workspace.visible"), "Visible", &state.checkbox_value,
                    fill_width(48.0f));
                GUI::slider_float(context, id(context, "workspace.roughness"), &state.roughness, 0.0f, 1.0f,
                    fill_width(48.0f));
                end_panel(context, inspector);
                GUICore::FlexLayoutDesc body_flex;
                body_flex.axis = GUICore::LayoutAxis::x;
                body_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                GUI::end_h_layout(context, body, body_flex);
                GUICore::FlexLayoutDesc workspace_flex;
                workspace_flex.axis = GUICore::LayoutAxis::y;
                workspace_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                GUI::end_v_layout(context, workspace, workspace_flex);
            }

            void build_content(GUICore::IContext* context, ShowcaseState& state, ShowcaseHandles& handles)
            {
                GUICore::LayoutConfig layout = grow_x();
                layout.padding = Float4U(18.0f);
                GUICore::ElementHandle content = GUI::begin_v_layout(context, id(context, "showcase.content"),
                    "Content", layout);
                rounded_rect(context, style_color(context, "gui.canvas"), 0.0f);
                if(state.section == 0)
                {
                    build_overview(context, state, handles);
                }
                else
                {
                    GUI::ScrollViewDesc desc;
                    desc.horizontal = false;
                    GUI::begin_scroll_view(context, id(context, "showcase.content.scroll"), "Content scroll", grow_y(), desc);
                    GUICore::ElementHandle page = GUI::begin_v_layout(context, id(context, "showcase.content.page"),
                        "Scrollable page", grow_y());
                    if(state.section == 1) build_primitives(context, state);
                    else if(state.section == 2) build_buttons(context, state);
                    else if(state.section == 3) build_input(context, state);
                    else if(state.section == 4) build_layouts(context, state);
                    else if(state.section == 5) build_scroll_views(context);
                    else if(state.section == 6) build_tables(context);
                    else if(state.section == 7) build_overlay(context, state, handles);
                    else build_workspace(context, state);
                    GUICore::FlexLayoutDesc page_flex;
                    page_flex.axis = GUICore::LayoutAxis::y;
                    page_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                    page_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                    page_flex.clip_children = false;
                    GUI::end_v_layout(context, page, page_flex);
                    GUI::end_scroll_view(context);
                }
                GUICore::FlexLayoutDesc content_flex;
                content_flex.axis = GUICore::LayoutAxis::y;
                content_flex.cross_alignment = GUICore::FlexAlignment::stretch;
                content_flex.main_axis_gap = style_scalar(context, "gui.section.gap", 16.0f);
                content_flex.clip_children = false;
                GUI::end_v_layout(context, content, content_flex);
            }

            void token_row(GUICore::IContext* context, ShowcaseState& state, GUI::id_t element_id,
                const c8* label_text, const c8* token, const c8* color_key)
            {
                GUICore::ElementHandle row = GUI::begin_h_layout(context, element_id, token, fill_width(28.0f));
                GUI::ShapeWidgetDesc swatch;
                swatch.tint = style_color(context, color_key);
                GUI::shape(context, GUICore::make_scoped_id(element_id, 1), state.circle,
                    fixed(17.0f, 17.0f), swatch);
                label(context, GUICore::make_scoped_id(element_id, 2), label_text, grow_x(), 10.0f,
                    "gui.text.secondary");
                label(context, GUICore::make_scoped_id(element_id, 3), token, fixed(108.0f, 24.0f),
                    8.0f, "gui.text.muted");
                GUICore::FlexLayoutDesc flex;
                flex.axis = GUICore::LayoutAxis::x;
                flex.cross_alignment = GUICore::FlexAlignment::center;
                flex.main_axis_gap = 8.0f;
                GUI::end_h_layout(context, row, flex);
            }

            void build_token_panel(GUICore::IContext* context, ShowcaseState& state)
            {
                GUICore::LayoutConfig layout = grow_x();
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = 248.0f;
                layout.padding = Float4U(14.0f);
                GUICore::ElementHandle panel = GUI::begin_v_layout(context, id(context, "showcase.tokens"),
                    "Style Inspector", layout);
                rounded_rect(context, style_color(context, "gui.surface.0"), 0.0f);
                label(context, id(context, "showcase.tokens.title"), "Style Inspector                         Settings",
                    fill_width(30.0f), 13.0f);
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

        GUICore::ElementHandle build_showcase(GUICore::IContext* context, ShowcaseState& state,
            ShowcaseHandles& handles)
        {
            context->push_layer(1, Float2U(0.0f));
            GUICore::ElementHandle root = GUI::begin_v_layout(context, id(context, "showcase.root"),
                "GUI Design Language Lab", fill());
            rounded_rect(context, style_color(context, "gui.canvas"), 0.0f);
            build_top_bar(context, state);
            GUICore::ElementHandle body = GUI::begin_h_layout(context, id(context, "showcase.body"),
                "Application body", grow_y());
            build_navigation(context, state, handles);
            build_content(context, state, handles);
            build_token_panel(context, state);
            GUICore::FlexLayoutDesc body_flex;
            body_flex.axis = GUICore::LayoutAxis::x;
            body_flex.cross_alignment = GUICore::FlexAlignment::stretch;
            GUI::end_h_layout(context, body, body_flex);
            GUICore::FlexLayoutDesc root_flex;
            root_flex.axis = GUICore::LayoutAxis::y;
            root_flex.cross_alignment = GUICore::FlexAlignment::stretch;
            GUI::end_v_layout(context, root, root_flex);
            context->pop_layer();
            return root;
        }

        void resolve_showcase(GUICore::IContext* context, ShowcaseState& state,
            const ShowcaseHandles& handles)
        {
                for(i32 i = 0; i < 9; ++i)
            {
                if(GUI::is_item_valid(context, handles.navigation[i]) &&
                    GUI::is_item_clicked(context, handles.navigation[i])) state.section = i;
            }
            for(i32 i = 0; i < 4; ++i)
            {
                if(GUI::is_item_valid(context, handles.style_options[i]) &&
                    GUI::is_item_clicked(context, handles.style_options[i]))
                {
                    state.theme = i / 2;
                    state.density = i % 2;
                }
            }
            for(i32 i = 0; i < 3; ++i)
            {
                if(GUI::is_item_valid(context, handles.assets[i]) && GUI::is_item_clicked(context, handles.assets[i]))
                {
                    state.selected_asset = i;
                }
            }
            if(GUI::is_item_valid(context, handles.popup_button) &&
                GUI::is_item_clicked(context, handles.popup_button))
            {
                GUI::open_popup(context, id(context, "overlay.popup.layer"));
            }
        }
    }
}
