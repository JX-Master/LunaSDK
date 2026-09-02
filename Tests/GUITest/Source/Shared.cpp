/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Shared.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUITest.hpp"

namespace Luna::GUITest
{
    static void submit_draw_command(GUI::IContext* context, const GUI::DrawCommand& command,
        GUI::paint_order_id_t paint_order_id)
    {
        if(paint_order_id == GUI::INVALID_PAINT_ORDER_ID) context->draw(command);
        else context->draw(command, paint_order_id);
    }

    GUI::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUI::LayoutConfig layout;
        layout.width.kind = GUI::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUI::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    void set_canvas_layout(GUI::IContext* context, const GUI::ElementHandle& element,
        GUI::CanvasLayoutDesc* desc)
    {
        GUI::LayoutCallbackConfig config;
        config.algorithm = Name("gui.test.canvas");
        config.callback = GUI::layout_canvas;
        config.finalize_callback = nullptr;
        config.userdata = desc;
        context->set_layout_callback_config(element, config);
    }

    void add_canvas_item(Vector<GUI::CanvasLayoutItem>& items, GUI::id_t id, f32 x, f32 y)
    {
        GUI::CanvasLayoutItem item;
        item.element_id = id;
        item.anchor_min = Float2U(0.0f);
        item.anchor_max = Float2U(0.0f);
        item.offset = Float4U(x, y, 0.0f, 0.0f);
        items.push_back(item);
    }

    void draw_rect(GUI::IContext* context, const RectF& rect, const Float4U& color, f32 radius,
        GUI::paint_order_id_t paint_order_id)
    {
        GUI::DrawCommand command;
        command.type = radius > 0.0f ? GUI::DrawCommandType::rounded_rect : GUI::DrawCommandType::rect;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = color;
        command.radius = radius;
        submit_draw_command(context, command, paint_order_id);
    }

    void draw_shadow(GUI::IContext* context, const RectF& rect, const Float4U& color,
        f32 radius, const GUI::ShadowDesc& desc, GUI::paint_order_id_t paint_order_id)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::shadow;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = color;
        command.radius = radius;
        command.shadow = desc;
        submit_draw_command(context, command, paint_order_id);
    }

    void draw_gradient_rect(GUI::IContext* context, const RectF& rect, const Float4U& top_left,
        const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left,
        GUI::paint_order_id_t paint_order_id)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::gradient_rect;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = top_left;
        command.color_top_right = top_right;
        command.color_bottom_right = bottom_right;
        command.color_bottom_left = bottom_left;
        submit_draw_command(context, command, paint_order_id);
    }

    void draw_line(GUI::IContext* context, const Float2U& begin, const Float2U& end,
        const Float4U& color, f32 width, GUI::paint_order_id_t paint_order_id)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::line;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
        command.point1 = end;
        command.color = color;
        command.line_width = width;
        submit_draw_command(context, command, paint_order_id);
    }

    void draw_outline(GUI::IContext* context, const RectF& rect, const Float4U& color, f32 width,
        GUI::paint_order_id_t paint_order_id)
    {
        draw_line(context, Float2U(rect.offset_x, rect.offset_y),
            Float2U(rect.offset_x + rect.width, rect.offset_y), color, width, paint_order_id);
        draw_line(context, Float2U(rect.offset_x + rect.width, rect.offset_y),
            Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height), color, width, paint_order_id);
        draw_line(context, Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height),
            Float2U(rect.offset_x, rect.offset_y + rect.height), color, width, paint_order_id);
        draw_line(context, Float2U(rect.offset_x, rect.offset_y + rect.height),
            Float2U(rect.offset_x, rect.offset_y), color, width, paint_order_id);
    }

    void draw_text(GUI::IContext* context, const RectF& rect, const c8* text, f32 size,
        const Float4U& color, VG::TextAlignment alignment, GUI::paint_order_id_t paint_order_id)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::text;
        command.rect_reference = GUI::DrawCommandRectReference::element;
        command.rect = rect;
        command.color = color;
        command.font = Name("default");
        command.font_size = size;
        command.horizontal_alignment = alignment;
        command.vertical_alignment = VG::TextAlignment::begin;
        command.text = text ? text : "";
        submit_draw_command(context, command, paint_order_id);
    }

    void bullet(GUI::IContext* context, f32 x, f32 y, const c8* text)
    {
        draw_rect(context, RectF(x, y + 8.0f, 5.0f, 5.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 2.5f);
        draw_text(context, RectF(x + 16.0f, y, 520.0f, 28.0f), text, 19.0f, Float4U(0.05f, 0.05f, 0.05f, 1.0f));
    }

    GUI::ElementHandle begin_panel(GUI::IContext* context, GUI::id_t id, const c8* title,
        f32 width, f32 height)
    {
        GUI::ElementHandle panel = context->begin_element(id);
        context->set_layout_config(panel, fixed_layout(width, height));
        draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
        draw_line(context, Float2U(0.0f, 44.0f), Float2U(width, 44.0f), Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.25f);
        draw_outline(context, RectF(0.0f, 0.0f, width, height), Float4U(0.78f, 0.78f, 0.78f, 1.0f), 1.0f);
        draw_text(context, RectF(18.0f, 9.0f, width - 36.0f, 32.0f), title, 23.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        return panel;
    }

    void end_panel(GUI::IContext* context)
    {
        context->end_element();
    }

    void panel_label_value(GUI::IContext* context, f32 y, const c8* label, const c8* value)
    {
        draw_text(context, RectF(18.0f, y, 148.0f, 26.0f), label, 17.0f, Float4U(0.32f, 0.32f, 0.32f, 1.0f));
        draw_text(context, RectF(170.0f, y, 300.0f, 26.0f), value, 17.0f, Float4U(0.02f, 0.02f, 0.02f, 1.0f));
    }

    bool circle_hit_test(const GUI::IContext*, const GUI::ElementHitTestRequest& request, void*)
    {
        f32 radius = min(request.element_rect.width, request.element_rect.height) * 0.5f;
        Float2U center(request.element_rect.width * 0.5f, request.element_rect.height * 0.5f);
        Float2U delta(request.element_position.x - center.x, request.element_position.y - center.y);
        return delta.x * delta.x + delta.y * delta.y <= radius * radius;
    }

    void set_interactable(GUI::IContext* context, const GUI::ElementHandle& element,
        GUI::PointerHitBehavior hit_behavior, GUI::InteractableFlag flags)
    {
        GUI::Interactable interactable;
        interactable.pointer_hit_behavior = hit_behavior;
        interactable.flags = flags;
        context->set_interactable(element, interactable);
    }

    R<GUI::paint_order_id_t> draw_sheet_callback(GUI::IContext* context,
        const GUI::ElementHandle& element, GUI::DrawPhase phase,
        GUI::paint_order_id_t paint_order_id, void* userdata)
    {
        (void)element;
        (void)userdata;
        if(phase == GUI::DrawPhase::before_children)
        {
            GUI::ShadowDesc shadow;
            shadow.offset = Float2U(0.0f, 12.0f);
            shadow.softness = 18.0f;
            shadow.spread = 1.0f;
            draw_shadow(context, RectF(0.0f, 0.0f, SHEET_WIDTH, SHEET_HEIGHT),
                Float4U(0.0f, 0.0f, 0.0f, 0.24f), 2.0f, shadow, paint_order_id);
            draw_rect(context, RectF(0.0f, 0.0f, SHEET_WIDTH, SHEET_HEIGHT),
                Float4U(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, paint_order_id + 1);
            return paint_order_id + 1;
        }
        else
        {
            draw_outline(context, RectF(0.0f, 0.0f, SHEET_WIDTH, SHEET_HEIGHT),
                Float4U(0.74f, 0.74f, 0.74f, 1.0f), 1.0f, paint_order_id);
        }
        return paint_order_id;
    }
}
