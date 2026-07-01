/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDrawing.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static VG::TextAlignment to_core_text_alignment(TextAlignment alignment)
        {
            switch(alignment)
            {
                case TextAlignment::begin: return VG::TextAlignment::begin;
                case TextAlignment::center: return VG::TextAlignment::center;
                case TextAlignment::end: return VG::TextAlignment::end;
                default: return VG::TextAlignment::begin;
            }
        }

        static void set_absolute_layout_result(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const RectF& rect)
        {
            GUICore::LayoutResult layout;
            layout.rect = rect;
            layout.clip_rect = rect;
            layout.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(element, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle draw_rect(GUICore::IContext* context, GUICore::id_t id,
            const RectF& rect, const Float4U& color, f32 radius)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("draw_rect"));
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = max(rect.width, 1.0f);
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = max(rect.height, 1.0f);
            context->set_layout_config(element, layout);
            set_absolute_layout_result(context, element,
                RectF(rect.offset_x, rect.offset_y, max(rect.width, 1.0f), max(rect.height, 1.0f)));

            GUICore::DrawCommand command;
            command.type = radius > 0.0f ? GUICore::DrawCommandType::rounded_rect : GUICore::DrawCommandType::rect;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle draw_circle(GUICore::IContext* context, GUICore::id_t id,
            const Float2U& center, f32 radius, const Float4U& color)
        {
            luassert(context && id);
            f32 r = max(radius, 0.5f);
            RectF rect(center.x - r, center.y - r, r * 2.0f, r * 2.0f);
            GUICore::ElementHandle element = context->begin_element(id, Name("draw_circle"));
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = rect.width;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = rect.height;
            context->set_layout_config(element, layout);
            set_absolute_layout_result(context, element, rect);

            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::rounded_rect;
            command.rect = rect;
            command.color = color;
            command.radius = r;
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle draw_line(GUICore::IContext* context, GUICore::id_t id,
            const Float2U& begin, const Float2U& end, const Float4U& color, f32 width)
        {
            luassert(context && id);
            f32 line_width = max(width, 1.0f);
            f32 half_width = line_width * 0.5f;
            f32 min_x = min(begin.x, end.x) - half_width;
            f32 min_y = min(begin.y, end.y) - half_width;
            f32 max_x = max(begin.x, end.x) + half_width;
            f32 max_y = max(begin.y, end.y) + half_width;
            RectF bounds(min_x, min_y, max(max_x - min_x, 1.0f), max(max_y - min_y, 1.0f));
            GUICore::ElementHandle element = context->begin_element(id, Name("draw_line"));
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = bounds.width;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = bounds.height;
            context->set_layout_config(element, layout);
            set_absolute_layout_result(context, element, bounds);

            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.color = color;
            command.line_width = line_width;
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle draw_text(GUICore::IContext* context, GUICore::id_t id,
            const RectF& rect, const c8* text, const Float4U& color, f32 font_size,
            TextAlignment horizontal_alignment, TextAlignment vertical_alignment)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, text ? Name(text) : Name("draw_text"));
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = max(rect.width, 1.0f);
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = max(rect.height, 1.0f);
            context->set_layout_config(element, layout);
            set_absolute_layout_result(context, element,
                RectF(rect.offset_x, rect.offset_y, max(rect.width, 1.0f), max(rect.height, 1.0f)));

            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = to_core_text_alignment(horizontal_alignment);
            command.vertical_alignment = to_core_text_alignment(vertical_alignment);
            command.text = text ? text : "";
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle draw_image(GUICore::IContext* context, GUICore::id_t id,
            RHI::ITexture* texture, const RectF& rect, const Float4U& color, ImageFlag flags)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("draw_image"));
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = max(rect.width, 1.0f);
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = max(rect.height, 1.0f);
            context->set_layout_config(element, layout);
            set_absolute_layout_result(context, element,
                RectF(rect.offset_x, rect.offset_y, max(rect.width, 1.0f), max(rect.height, 1.0f)));

            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::image;
            command.rect = rect;
            command.texture = texture;
            command.color = color;
            command.min_texcoord = Float2U(0.0f, test_flags(flags, ImageFlag::flip_y) ? 1.0f : 0.0f);
            command.max_texcoord = Float2U(1.0f, test_flags(flags, ImageFlag::flip_y) ? 0.0f : 1.0f);
            command.nearest_sampler = test_flags(flags, ImageFlag::nearest);
            context->draw(command);
            context->end_element();
            return element;
        }
    }
}
