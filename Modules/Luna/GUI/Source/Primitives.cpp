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
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/VG/TextArranger.hpp>
#include <cstdio>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct TextData
            {
                c8* text = nullptr;
                usize text_size = 0;
                TextDesc desc;
            };

            static GUICore::FontDesc resolve_font(GUICore::IContext* context,
                const GUICore::ElementHandle& element, const TextDesc& desc)
            {
                Name font_id = desc.font.empty() ? style_name(context, element, "gui.font") : desc.font;
                GUICore::FontDesc font = context->get_font(font_id);
                if(!font.font)
                {
                    font.font = Font::get_default_font();
                    font.font_index = 0;
                }
                return font;
            }

            static f32 resolve_font_size(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const TextDesc& desc)
            {
                return desc.font_size > 0.0f ? desc.font_size : style_scalar(context, element, "gui.text.font_size", 16.0f);
            }

            static GUICore::MeasureResult measure_text(GUICore::IContext* context,
                const GUICore::ElementHandle& element, const Float2U& available_size, void* userdata)
            {
                GUICore::MeasureResult result;
                TextData* data = (TextData*)userdata;
                if(!data || !data->text_size)
                {
                    return result;
                }
                GUICore::FontDesc font = resolve_font(context, element, data->desc);
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

            static RV draw_text(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                TextData* data = (TextData*)userdata;
                if(!data)
                {
                    return ok;
                }
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::text;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                command.text = data->text ? data->text : "";
                command.font = data->desc.font.empty() ? style_name(context, element, "gui.font") : data->desc.font;
                command.font_size = resolve_font_size(context, element, data->desc);
                command.color = data->desc.color.w >= 0.0f ? data->desc.color :
                    style_color(context, element, "gui.text.color", Float4U(0.86f, 0.88f, 0.92f, 1.0f));
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

            static GUICore::MeasureResult measure_image(GUICore::IContext*, const GUICore::ElementHandle&,
                const Float2U&, void* userdata)
            {
                GUICore::MeasureResult result;
                ImageData* data = (ImageData*)userdata;
                if(data && data->texture)
                {
                    RHI::TextureDesc desc = data->texture->get_desc();
                    result.desired = Float2U((f32)desc.width, (f32)desc.height);
                }
                return result;
            }

            static RV draw_image(GUICore::IContext* context, const GUICore::ElementHandle&,
                GUICore::DrawPhase, void* userdata)
            {
                ImageData* data = (ImageData*)userdata;
                if(!data || !data->texture)
                {
                    return ok;
                }
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::image;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
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
                GUICore::ShapeDesc shape;
                ShapeWidgetDesc desc;
            };

            static GUICore::MeasureResult measure_shape(GUICore::IContext*, const GUICore::ElementHandle&,
                const Float2U&, void* userdata)
            {
                GUICore::MeasureResult result;
                ShapeData* data = (ShapeData*)userdata;
                if(data)
                {
                    result.desired = Float2U(max(data->shape.bounds.width, 0.0f),
                        max(data->shape.bounds.height, 0.0f));
                }
                return result;
            }

            static RV draw_shape(GUICore::IContext* context, const GUICore::ElementHandle&,
                GUICore::DrawPhase, void* userdata)
            {
                ShapeData* data = (ShapeData*)userdata;
                if(!data || !data->shape.buffer || !data->shape.num_commands) return ok;
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::shape;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
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

            static GUICore::MeasureResult measure_progress(GUICore::IContext*, const GUICore::ElementHandle&,
                const Float2U&, void*)
            {
                GUICore::MeasureResult result;
                result.minimum = Float2U(48.0f, 18.0f);
                result.desired = Float2U(160.0f, 22.0f);
                return result;
            }

            static RV draw_progress(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ProgressData* data = (ProgressData*)userdata;
                if(!data)
                {
                    return ok;
                }
                f32 radius = style_scalar(context, element, "gui.progress.radius", 4.0f);
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.color = style_color(context, element, "gui.progress.border", Float4U(0.24f, 0.29f, 0.35f, 1.0f));
                command.radius = radius;
                context->draw(command);
                command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                command.color = style_color(context, element, "gui.progress.background", Float4U(0.07f, 0.08f, 0.10f, 1.0f));
                command.radius = max(radius - 1.0f, 0.0f);
                context->draw(command);
                if(data->fraction > 0.0f)
                {
                    command.rect = RectF(1.0f, 1.0f, -2.0f * data->fraction, -2.0f);
                    command.rect_layout_scale = Float4U(0.0f, 0.0f, data->fraction, 0.0f);
                    command.color = style_color(context, element, "gui.progress.fill", Float4U(0.15f, 0.46f, 0.76f, 1.0f));
                    context->draw(command);
                }
                if(data->show_overlay)
                {
                    GUICore::DrawCommand text_command;
                    text_command.type = GUICore::DrawCommandType::text;
                    text_command.rect_reference = GUICore::DrawCommandRectReference::element;
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

        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout, const TextDesc& desc)
        {
            GUICore::ElementHandle element = Internal::begin_element(context, id, value ? value : "Text", layout);
            Internal::TextData* data = Internal::allocate_frame<Internal::TextData>(context);
            data->text = Internal::copy_frame_string(context, value, &data->text_size);
            data->desc = desc;
            GUICore::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.text");
            layout_callbacks.measure_callback = Internal::measure_text;
            layout_callbacks.userdata = data;
            context->set_layout_callback_config(element, layout_callbacks);
            GUICore::DrawConfig draw;
            draw.name = Name("gui.text");
            draw.callback = Internal::draw_text;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout, const ImageDesc& desc)
        {
            GUICore::ElementHandle element = Internal::begin_element(context, id, "Image", layout);
            Internal::ImageData* data = Internal::allocate_frame<Internal::ImageData>(context);
            data->texture = texture;
            data->desc = desc;
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.image");
            callbacks.measure_callback = Internal::measure_image;
            callbacks.userdata = data;
            context->set_layout_callback_config(element, callbacks);
            GUICore::DrawConfig draw;
            draw.name = Name("gui.image");
            draw.callback = Internal::draw_image;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, id_t id,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout, const ShapeWidgetDesc& desc)
        {
            GUICore::ElementHandle element = Internal::begin_element(context, id, "Shape", layout);
            Internal::ShapeData* data = Internal::allocate_frame<Internal::ShapeData>(context);
            data->shape = value;
            data->desc = desc;
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.shape");
            callbacks.measure_callback = Internal::measure_shape;
            callbacks.userdata = data;
            context->set_layout_callback_config(element, callbacks);
            GUICore::DrawConfig draw;
            draw.name = Name("gui.shape");
            draw.callback = Internal::draw_shape;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout, const HitBoxDesc& desc)
        {
            GUICore::ElementHandle element = Internal::begin_element(context, id, "Hit Box", layout);
            Internal::set_interactable(context, element, desc.enabled);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, id_t id, f32 fraction,
            const GUICore::LayoutConfig& layout, const ProgressBarDesc& desc)
        {
            GUICore::ElementHandle element = Internal::begin_element(context, id, "Progress Bar", layout);
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
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.progress_bar");
            callbacks.measure_callback = Internal::measure_progress;
            context->set_layout_callback_config(element, callbacks);
            GUICore::DrawConfig draw;
            draw.name = Name("gui.progress_bar");
            draw.callback = Internal::draw_progress;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }
    }
}
