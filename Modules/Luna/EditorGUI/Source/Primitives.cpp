/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Primitives.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/VG/TextArranger.hpp>
#include <cstdio>

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            struct TextData
            {
                c8* text = nullptr;
                usize text_size = 0;
                TextDesc desc;
            };

            struct TypographyEntries
            {
                const c8* font;
                const c8* font_size;
                const c8* color;
            };

            static const TypographyEntries& typography_entries(TypographyRole role)
            {
                static const TypographyEntries entries[] = {
                    { "gui.typography.heading1.font", "gui.typography.heading1.font_size", "gui.typography.heading1.color" },
                    { "gui.typography.heading2.font", "gui.typography.heading2.font_size", "gui.typography.heading2.color" },
                    { "gui.typography.heading3.font", "gui.typography.heading3.font_size", "gui.typography.heading3.color" },
                    { "gui.typography.heading4.font", "gui.typography.heading4.font_size", "gui.typography.heading4.color" },
                    { "gui.typography.heading5.font", "gui.typography.heading5.font_size", "gui.typography.heading5.color" },
                    { "gui.typography.heading6.font", "gui.typography.heading6.font_size", "gui.typography.heading6.color" },
                    { "gui.typography.body.font", "gui.typography.body.font_size", "gui.typography.body.color" },
                    { "gui.typography.cite.font", "gui.typography.cite.font_size", "gui.typography.cite.color" },
                    { "gui.typography.code.font", "gui.typography.code.font_size", "gui.typography.code.color" },
                    { "gui.typography.caption.font", "gui.typography.caption.font_size", "gui.typography.caption.color" }
                };
                usize index = (usize)role;
                if(index >= sizeof(entries) / sizeof(entries[0])) index = (usize)TypographyRole::body;
                return entries[index];
            }

            static Name resolve_font_id(GUI::IContext* context,
                const GUI::ElementHandle& element, const TextDesc& desc)
            {
                const TypographyEntries& entries = typography_entries(desc.typography);
                Name default_font = style_name(context, element, "gui.font");
                return desc.font.empty() ? style_name(context, element, entries.font, default_font) : desc.font;
            }

            static GUI::FontDesc resolve_font(GUI::IContext* context,
                const GUI::ElementHandle& element, const TextDesc& desc)
            {
                Name font_id = resolve_font_id(context, element, desc);
                GUI::FontDesc font = context->get_font(font_id);
                if(!font.font)
                {
                    font.font = Font::get_default_font();
                    font.font_index = 0;
                }
                return font;
            }

            static f32 resolve_font_size(GUI::IContext* context, const GUI::ElementHandle& element,
                const TextDesc& desc)
            {
                if(desc.font_size > 0.0f) return desc.font_size;
                const TypographyEntries& entries = typography_entries(desc.typography);
                f32 fallback = style_scalar(context, element, "gui.text.font_size", 16.0f);
                return style_scalar(context, element, entries.font_size, fallback);
            }

            static Float4U resolve_text_color(GUI::IContext* context,
                const GUI::ElementHandle& element, const TextDesc& desc)
            {
                if(desc.color.w >= 0.0f) return desc.color;
                const TypographyEntries& entries = typography_entries(desc.typography);
                Float4U fallback = style_color(context, element, "gui.text.color",
                    Float4U(0.86f, 0.88f, 0.92f, 1.0f));
                return style_color(context, element, entries.color, fallback);
            }

            static GUI::MeasureResult measure_text(GUI::IContext* context,
                const GUI::ElementHandle& element, const Float2U& available_size, void* userdata)
            {
                GUI::MeasureResult result;
                TextData* data = (TextData*)userdata;
                if(!data || !data->text_size)
                {
                    return result;
                }
                GUI::FontDesc font = resolve_font(context, element, data->desc);
                if(!font.font)
                {
                    return result;
                }
                f32 font_size = resolve_font_size(context, element, data->desc);
                f32 width = available_size.x >= F32_MAX * 0.5f ? 1000000.0f : max(available_size.x, 1.0f);
                f32 height = available_size.y >= F32_MAX * 0.5f ? 1000000.0f : max(available_size.y, font_size * 2.0f);
                VG::TextArrangeSection section;
                section.font_file = font.font;
                section.font_index = font.font_index;
                section.font_size = font_size;
                section.color = Float4U(1.0f);
                section.num_chars = data->text_size;
                VG::TextArrangeResult arranged = VG::arrange_text(data->text, data->text_size,
                    Span<const VG::TextArrangeSection>(&section, 1), RectF(0.0f, 0.0f, width, height),
                    text_alignment(data->desc.horizontal_alignment), VG::TextAlignment::begin);
                result.desired = Float2U(arranged.bounding_rect.width, arranged.bounding_rect.height);
                return result;
            }

            static RV draw_text(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                TextData* data = (TextData*)userdata;
                if(!data)
                {
                    return ok;
                }
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::text;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                command.text = data->text ? data->text : "";
                command.font = resolve_font_id(context, element, data->desc);
                command.font_size = resolve_font_size(context, element, data->desc);
                command.color = resolve_text_color(context, element, data->desc);
                command.horizontal_alignment = text_alignment(data->desc.horizontal_alignment);
                command.vertical_alignment = text_alignment(data->desc.vertical_alignment);
                context->draw(command);
                return ok;
            }

            struct ImageData
            {
                RHI::ITexture* texture = nullptr;
                ImageDesc desc;
            };

            static GUI::MeasureResult measure_image(GUI::IContext*, const GUI::ElementHandle&,
                const Float2U&, void* userdata)
            {
                GUI::MeasureResult result;
                ImageData* data = (ImageData*)userdata;
                if(data && data->texture)
                {
                    RHI::TextureDesc desc = data->texture->get_desc();
                    result.desired = Float2U((f32)desc.width, (f32)desc.height);
                }
                return result;
            }

            static RV draw_image(GUI::IContext* context, const GUI::ElementHandle&,
                GUI::DrawPhase, void* userdata)
            {
                ImageData* data = (ImageData*)userdata;
                if(!data || !data->texture)
                {
                    return ok;
                }
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::image;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.texture = data->texture;
                command.color = data->desc.tint;
                command.min_texcoord = data->desc.min_texcoord;
                command.max_texcoord = data->desc.max_texcoord;
                if(test_flags(data->desc.flags, ImageFlag::flip_y))
                {
                    swap(command.min_texcoord.y, command.max_texcoord.y);
                }
                command.nearest_sampler = test_flags(data->desc.flags, ImageFlag::nearest);
                context->draw(command);
                return ok;
            }

            struct ShapeData
            {
                GUI::ShapeDesc shape;
                ShapeWidgetDesc desc;
            };

            static GUI::MeasureResult measure_shape(GUI::IContext*, const GUI::ElementHandle&,
                const Float2U&, void* userdata)
            {
                GUI::MeasureResult result;
                ShapeData* data = (ShapeData*)userdata;
                if(data)
                {
                    result.desired = Float2U(max(data->shape.bounds.width, 0.0f),
                        max(data->shape.bounds.height, 0.0f));
                }
                return result;
            }

            static RV draw_shape(GUI::IContext* context, const GUI::ElementHandle&,
                GUI::DrawPhase, void* userdata)
            {
                ShapeData* data = (ShapeData*)userdata;
                if(!data || !data->shape.buffer || !data->shape.num_commands) return ok;
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::shape;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.color = data->desc.tint;
                command.shape = data->shape;
                context->draw(command);
                return ok;
            }

            struct ProgressData
            {
                f32 fraction = 0.0f;
                c8* overlay = nullptr;
                bool show_overlay = true;
            };

            static GUI::MeasureResult measure_progress(GUI::IContext*, const GUI::ElementHandle&,
                const Float2U&, void* userdata)
            {
                ProgressData* data = (ProgressData*)userdata;
                const bool show_overlay = !data || data->show_overlay;
                GUI::MeasureResult result;
                result.minimum = Float2U(48.0f, show_overlay ? 18.0f : 8.0f);
                result.desired = Float2U(160.0f, show_overlay ? 22.0f : 10.0f);
                return result;
            }

            static RV draw_progress(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                ProgressData* data = (ProgressData*)userdata;
                if(!data)
                {
                    return ok;
                }
                f32 radius = style_scalar(context, element, "gui.progress.radius", 4.0f);
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::rounded_rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.color = style_color(context, element, "gui.progress.border", Float4U(0.24f, 0.29f, 0.35f, 1.0f));
                command.radius = radius;
                context->draw(command);
                const RectF inner_rect(1.0f, 1.0f, -2.0f, -2.0f);
                const f32 inner_radius = max(radius - 1.0f, 0.0f);
                const f32 softness = style_scalar(context, element, "gui.shadow.softness", 5.0f);
                RoundedRectEffect track_effects[3];
                track_effects[0].color = style_color(context, element, "gui.progress.background",
                    Float4U(0.07f, 0.08f, 0.10f, 1.0f));
                track_effects[1].shadow = true;
                track_effects[1].color = style_color(context, element, "gui.shadow.inset",
                    Float4U(0.0f, 0.0f, 0.0f, 0.18f));
                track_effects[1].shadow_desc.offset = Float2U(2.0f, 2.0f);
                track_effects[1].shadow_desc.softness = softness * 0.5f;
                track_effects[1].shadow_desc.mode = GUI::ShadowMode::inner;
                track_effects[2].shadow = true;
                track_effects[2].color = style_color(context, element, "gui.shadow.inset_light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.65f));
                track_effects[2].shadow_desc.offset = Float2U(-2.0f, -2.0f);
                track_effects[2].shadow_desc.softness = softness * 0.4f;
                track_effects[2].shadow_desc.mode = GUI::ShadowMode::inner;
                if(RV result = draw_rounded_rect_effects(context, element, inner_rect, Float4U(),
                    inner_radius, Span<const RoundedRectEffect>(track_effects, 3)); failed(result))
                {
                    return result;
                }
                if(data->fraction > 0.0f)
                {
                    RectF fill_rect(1.0f, 1.0f, -2.0f * data->fraction, -2.0f);
                    Float4U fill_scale(0.0f, 0.0f, data->fraction, 0.0f);
                    RoundedRectEffect fill_effects[2];
                    fill_effects[0].color = style_color(context, element, "gui.progress.fill",
                        Float4U(0.15f, 0.46f, 0.76f, 1.0f));
                    Float4U highlight = style_color(context, element, "gui.shadow.inset_light",
                        Float4U(1.0f, 1.0f, 1.0f, 0.65f));
                    highlight.w *= 0.4f;
                    fill_effects[1].shadow = true;
                    fill_effects[1].color = highlight;
                    fill_effects[1].shadow_desc.offset = Float2U(0.0f, -1.0f);
                    fill_effects[1].shadow_desc.softness = 1.0f;
                    fill_effects[1].shadow_desc.mode = GUI::ShadowMode::inner;
                    if(RV result = draw_rounded_rect_effects(context, element, fill_rect, fill_scale,
                        inner_radius, Span<const RoundedRectEffect>(fill_effects, 2)); failed(result))
                    {
                        return result;
                    }
                }
                if(data->show_overlay)
                {
                    GUI::DrawCommand text_command;
                    text_command.type = GUI::DrawCommandType::text;
                    text_command.rect_reference = GUI::DrawCommandRectReference::element;
                    text_command.color = style_color(context, element, "gui.text.color", Float4U(1.0f));
                    text_command.font = style_name(context, element, "gui.font");
                    text_command.font_size = style_scalar(context, element, "gui.text.font_size", 14.0f);
                    text_command.horizontal_alignment = VG::TextAlignment::center;
                    text_command.vertical_alignment = VG::TextAlignment::center;
                    text_command.text = data->overlay ? data->overlay : "";
                    context->draw(text_command);
                }
                return ok;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle text(GUI::IContext* context, id_t id, const c8* value,
            const GUI::LayoutConfig& layout, const TextDesc& desc)
        {
            GUI::ElementHandle element = Internal::begin_element(context, id, value ? value : "Text", layout);
            Internal::TextData* data = Internal::allocate_frame<Internal::TextData>(context);
            data->text = Internal::copy_frame_string(context, value, &data->text_size);
            data->desc = desc;
            GUI::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.text");
            layout_callbacks.measure_callback = Internal::measure_text;
            layout_callbacks.userdata = data;
            context->set_layout_callback_config(element, layout_callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.text");
            draw.callback = Internal::draw_text;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle image(GUI::IContext* context, id_t id, RHI::ITexture* texture,
            const GUI::LayoutConfig& layout, const ImageDesc& desc)
        {
            GUI::ElementHandle element = Internal::begin_element(context, id, "Image", layout);
            Internal::ImageData* data = Internal::allocate_frame<Internal::ImageData>(context);
            data->texture = texture;
            data->desc = desc;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.image");
            callbacks.measure_callback = Internal::measure_image;
            callbacks.userdata = data;
            context->set_layout_callback_config(element, callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.image");
            draw.callback = Internal::draw_image;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle shape(GUI::IContext* context, id_t id,
            const GUI::ShapeDesc& value, const GUI::LayoutConfig& layout, const ShapeWidgetDesc& desc)
        {
            GUI::ElementHandle element = Internal::begin_element(context, id, "Shape", layout);
            Internal::ShapeData* data = Internal::allocate_frame<Internal::ShapeData>(context);
            data->shape = value;
            data->desc = desc;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.shape");
            callbacks.measure_callback = Internal::measure_shape;
            callbacks.userdata = data;
            context->set_layout_callback_config(element, callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.shape");
            draw.callback = Internal::draw_shape;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle hit_box(GUI::IContext* context, id_t id,
            const GUI::LayoutConfig& layout, const HitBoxDesc& desc)
        {
            GUI::ElementHandle element = Internal::begin_element(context, id, "Hit Box", layout);
            Internal::set_interactable(context, element, desc.enabled);
            context->end_element();
            return element;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle progress_bar(GUI::IContext* context, id_t id, f32 fraction,
            const GUI::LayoutConfig& layout, const ProgressBarDesc& desc)
        {
            GUI::ElementHandle element = Internal::begin_element(context, id, "Progress Bar", layout);
            Internal::ProgressData* data = Internal::allocate_frame<Internal::ProgressData>(context);
            data->fraction = clamp(fraction, 0.0f, 1.0f);
            data->show_overlay = desc.show_overlay;
            if(desc.show_overlay)
            {
                if(desc.overlay)
                {
                    data->overlay = Internal::copy_frame_string(context, desc.overlay);
                }
                else
                {
                    c8 buffer[32];
                    snprintf(buffer, sizeof(buffer), "%.0f%%", data->fraction * 100.0f);
                    data->overlay = Internal::copy_frame_string(context, buffer);
                }
            }
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.progress_bar");
            callbacks.measure_callback = Internal::measure_progress;
            context->set_layout_callback_config(element, callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.progress_bar");
            draw.callback = Internal::draw_progress;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }
    }
}
