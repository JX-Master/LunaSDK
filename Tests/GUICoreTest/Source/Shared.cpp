/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Shared.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    GUICore::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUICore::LayoutConfig layout;
        layout.width.kind = GUICore::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUICore::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    void set_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
        GUICore::CanvasLayoutDesc* desc)
    {
        const GUICore::Element* core_element = context->get_element(element.index);
        GUICore::LayoutConfig config = core_element ? core_element->layout : GUICore::LayoutConfig();
        config.name = Name("guicore.test.canvas");
        config.callback = GUICore::layout_canvas;
        config.finalize_callback = nullptr;
        config.userdata = desc;
        context->set_layout_config(element, config);
    }

    void add_canvas_item(Vector<GUICore::CanvasLayoutItem>& items, GUICore::id_t id, f32 x, f32 y)
    {
        GUICore::CanvasLayoutItem item;
        item.element_id = id;
        item.anchor_min = Float2U(0.0f);
        item.anchor_max = Float2U(0.0f);
        item.offset = Float4U(x, y, 0.0f, 0.0f);
        items.push_back(item);
    }

    void draw_rect(GUICore::IContext* context, const RectF& rect, const Float4U& color, f32 radius)
    {
        GUICore::DrawCommand command;
        command.type = radius > 0.0f ? GUICore::DrawCommandType::rounded_rect : GUICore::DrawCommandType::rect;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = color;
        command.radius = radius;
        context->draw(command);
    }

    void draw_gradient_rect(GUICore::IContext* context, const RectF& rect, const Float4U& top_left,
        const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left)
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::gradient_rect;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = top_left;
        command.color_top_right = top_right;
        command.color_bottom_right = bottom_right;
        command.color_bottom_left = bottom_left;
        context->draw(command);
    }

    void draw_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
        const Float4U& color, f32 width)
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::line;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
        command.point1 = end;
        command.color = color;
        command.line_width = width;
        context->draw(command);
    }

    void draw_outline(GUICore::IContext* context, const RectF& rect, const Float4U& color, f32 width)
    {
        draw_line(context, Float2U(rect.offset_x, rect.offset_y),
            Float2U(rect.offset_x + rect.width, rect.offset_y), color, width);
        draw_line(context, Float2U(rect.offset_x + rect.width, rect.offset_y),
            Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height), color, width);
        draw_line(context, Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height),
            Float2U(rect.offset_x, rect.offset_y + rect.height), color, width);
        draw_line(context, Float2U(rect.offset_x, rect.offset_y + rect.height),
            Float2U(rect.offset_x, rect.offset_y), color, width);
    }

    void draw_text(GUICore::IContext* context, const RectF& rect, const c8* text, f32 size,
        const Float4U& color, VG::TextAlignment alignment)
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::text;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = color;
        command.font = Name("default");
        command.font_size = size;
        command.horizontal_alignment = alignment;
        command.vertical_alignment = VG::TextAlignment::begin;
        command.text = text ? text : "";
        context->draw(command);
    }

    void bullet(GUICore::IContext* context, f32 x, f32 y, const c8* text)
    {
        draw_rect(context, RectF(x, y + 7.0f, 5.0f, 5.0f), Float4U(0.28f, 0.74f, 0.92f, 1.0f), 2.5f);
        draw_text(context, RectF(x + 14.0f, y, 360.0f, 24.0f), text, 16.0f, Float4U(0.78f, 0.84f, 0.90f, 1.0f));
    }

    GUICore::ElementHandle begin_panel(GUICore::IContext* context, GUICore::id_t id, const c8* title,
        f32 width, f32 height)
    {
        GUICore::ElementHandle panel = context->begin_element(id, Name(title));
        context->set_layout_config(panel, fixed_layout(width, height));
        draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.065f, 0.086f, 0.105f, 0.96f), 8.0f);
        draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 42.0f), Float4U(0.085f, 0.115f, 0.140f, 1.0f), 8.0f);
        draw_line(context, Float2U(0.0f, 42.0f), Float2U(width, 42.0f), Float4U(0.19f, 0.27f, 0.34f, 1.0f), 1.0f);
        draw_outline(context, RectF(0.0f, 0.0f, width, height), Float4U(0.15f, 0.23f, 0.30f, 1.0f), 1.0f);
        draw_text(context, RectF(18.0f, 11.0f, width - 36.0f, 28.0f), title, 20.0f, Float4U(0.94f, 0.96f, 0.98f, 1.0f));
        return panel;
    }

    void end_panel(GUICore::IContext* context)
    {
        context->end_element();
    }

    void panel_label_value(GUICore::IContext* context, f32 y, const c8* label, const c8* value)
    {
        draw_text(context, RectF(18.0f, y, 128.0f, 24.0f), label, 15.0f, Float4U(0.56f, 0.64f, 0.70f, 1.0f));
        draw_text(context, RectF(150.0f, y, 250.0f, 24.0f), value, 15.0f, Float4U(0.82f, 0.87f, 0.92f, 1.0f));
    }

    bool circle_hit_test(const GUICore::IContext*, const GUICore::ElementHitTestRequest& request, void*)
    {
        f32 radius = min(request.element_rect.width, request.element_rect.height) * 0.5f;
        Float2U center(request.element_rect.width * 0.5f, request.element_rect.height * 0.5f);
        Float2U delta(request.element_position.x - center.x, request.element_position.y - center.y);
        return delta.x * delta.x + delta.y * delta.y <= radius * radius;
    }

    void set_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
        GUICore::PointerHitBehavior hit_behavior, GUICore::InteractableFlag flags)
    {
        GUICore::Interactable interactable;
        interactable.pointer_hit_behavior = hit_behavior;
        interactable.flags = flags;
        context->set_interactable(element, interactable);
    }
}
