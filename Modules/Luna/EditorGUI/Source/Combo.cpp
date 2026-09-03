/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Combo.cpp
* @author JXMaster
* @date 2026/7/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        struct ComboDrawData
        {
            c8* selected = nullptr;
            Float4U background;
            Float4U border;
            Float4U text_color;
            f32 radius = 0.0f;
            f32 font_size = 0.0f;
            bool open = false;
        };

        static void draw_combo_rect(GUI::IContext* context, const Float4U& color, f32 radius,
            GUI::paint_order_id_t paint_order_id,
            const RectF& rect = RectF(0.0f, 0.0f, 0.0f, 0.0f))
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::rounded_rect;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
            context->draw(command, paint_order_id);
        }

        static void draw_combo_text(GUI::IContext* context, const RectF& rect, const c8* value,
            const Name& font, const Float4U& color, f32 font_size,
            GUI::paint_order_id_t paint_order_id,
            VG::TextAlignment alignment = VG::TextAlignment::begin)
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::text;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font = font;
            command.font_size = font_size;
            command.horizontal_alignment = alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = value ? value : "";
            context->draw(command, paint_order_id);
        }

        static void draw_combo_chevron(GUI::IContext* context, const Float4U& color, bool open,
            GUI::paint_order_id_t paint_order_id)
        {
            const f32 outer_y = open ? 2.5f : -2.5f;
            const f32 center_y = -outer_y;
            GUI::DrawCommand line;
            line.type = GUI::DrawCommandType::line;
            line.rect_reference = GUI::DrawCommandRectReference::element;
            line.rect = RectF(-23.0f, outer_y, 0.0f, 0.0f);
            line.point1 = Float2U(-18.0f, center_y);
            line.rect_layout_scale = Float4U(1.0f, 0.5f, 1.0f, 0.5f);
            line.color = color;
            line.line_width = 1.6f;
            context->draw(line, paint_order_id);
            line.rect = RectF(-18.0f, center_y, 0.0f, 0.0f);
            line.point1 = Float2U(-13.0f, outer_y);
            context->draw(line, paint_order_id);
        }

        static R<GUI::paint_order_id_t> draw_combo(GUI::IContext* context,
            const GUI::ElementHandle& element, GUI::DrawPhase, GUI::paint_order_id_t paint_order_id,
            void* userdata)
        {
            ComboDrawData* data = (ComboDrawData*)userdata;
            if(!data) return paint_order_id;
            draw_combo_rect(context, data->border, data->radius, paint_order_id);
            draw_combo_rect(context, data->background, max(data->radius - 1.0f, 0.0f),
                paint_order_id + 1, RectF(1.0f, 1.0f, -2.0f, -2.0f));
            Name font = Internal::style_name(context, element, "gui.font");
            draw_combo_text(context, RectF(12.0f, 0.0f, -44.0f, 0.0f), data->selected,
                font, data->text_color, data->font_size, paint_order_id + 2);
            draw_combo_chevron(context, data->text_color, data->open, paint_order_id + 2);
            return paint_order_id + 2;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle combo(GUI::IContext* context, id_t id, const c8* label,
            i32* current_item, Span<const c8*> items, const GUI::LayoutConfig& layout, const ComboDesc& desc)
        {
            luassert(context && id);
            if(current_item && !items.empty()) *current_item = clamp(*current_item, 0, (i32)items.size() - 1);
            id_t popup_id = Internal::derived_id(id, "combo.popup");
            Ref<Internal::PopupPlacementState> state = Internal::widget_state<Internal::PopupPlacementState>(context, id);
            for(usize i = 0; i < items.size(); ++i)
            {
                id_t item_id = GUI::make_scoped_id(Internal::derived_id(id, "combo.item"), (u64)i + 1);
                if(desc.enabled && context->get_interaction_state(item_id).clicked)
                {
                    if(current_item) *current_item = (i32)i;
                    close_popup(context, popup_id);
                }
            }
            GUI::InteractionState interaction = context->get_interaction_state(id);
            if(desc.enabled && interaction.clicked && current_item && !items.empty())
            {
                if(is_popup_open(context, popup_id)) close_popup(context, popup_id);
                else
                {
                    state->position.x = interaction.clicked_screen_position.x - interaction.clicked_element_position.x;
                    state->position.y = interaction.clicked_screen_position.y - interaction.clicked_element_position.y +
                        interaction.clicked_element_rect.height;
                    open_popup(context, popup_id);
                }
            }
            bool open = is_popup_open(context, popup_id);
            GUI::ElementHandle preview = Internal::begin_element(context, id, label ? label : "Combo", layout);
            Internal::set_interactable(context, preview, desc.enabled);
            Float4U background = Internal::style_color(context, preview,
                open ? "gui.combo.background_open" : (interaction.hovered ? "gui.combo.background_hovered" :
                "gui.combo.background"), open ? Float4U(0.16f, 0.25f, 0.38f, 1.0f) :
                (interaction.hovered ? Float4U(0.13f, 0.18f, 0.26f, 1.0f) : Float4U(0.10f, 0.13f, 0.18f, 1.0f)));
            Float4U border = Internal::style_color(context, preview, "gui.combo.border",
                Float4U(0.24f, 0.30f, 0.38f, 1.0f));
            Float4U text_color = Internal::style_color(context, preview, "gui.combo.text",
                Float4U(0.95f, 0.96f, 0.98f, 1.0f));
            f32 radius = Internal::style_scalar(context, preview, "gui.combo.radius", 4.0f);
            f32 font_size = Internal::style_scalar(context, preview, "gui.combo.font_size", 15.0f);
            const c8* selected = "";
            if(current_item && *current_item >= 0 && (usize)*current_item < items.size()) selected = items[(usize)*current_item];
            ComboDrawData* draw_data = Internal::allocate_frame<ComboDrawData>(context);
            draw_data->selected = Internal::copy_frame_string(context, selected);
            draw_data->background = background;
            draw_data->border = border;
            draw_data->text_color = text_color;
            draw_data->radius = radius;
            draw_data->font_size = font_size;
            draw_data->open = open;
            GUI::DrawConfig draw;
            draw.name = Name("gui.combo");
            draw.callback = draw_combo;
            draw.userdata = draw_data;
            context->set_draw_config(preview, draw);
            context->end_element();

            if(open)
            {
                f32 width = desc.popup_width > 0.0f ? desc.popup_width : 220.0f;
                f32 height = max(40.0f, min((f32)items.size() * 28.0f + 14.0f,
                    max(desc.popup_max_height, 40.0f)));
                PopupDesc popup_desc;
                popup_desc.position = state->position;
                popup_desc.layout.width.kind = GUI::SizeKind::fixed;
                popup_desc.layout.width.value = width;
                popup_desc.layout.height.kind = GUI::SizeKind::fixed;
                popup_desc.layout.height.value = height;
                GUI::ElementHandle popup;
                if(begin_popup(context, popup_id, popup_desc, &popup))
                {
                    GUI::LayoutConfig list_layout;
                    list_layout.width.kind = GUI::SizeKind::percent;
                    list_layout.width.value = 1.0f;
                    GUI::ElementHandle list = begin_v_layout(context,
                        Internal::derived_id(id, "combo.items"), "Combo Items", list_layout);
                    // Every package-owned item is clipped to one non-overlapping list row.
                    context->set_child_paint_order_mode(list, GUI::ChildPaintOrderMode::shared);
                    GUI::LayoutConfig item_layout;
                    item_layout.width.kind = GUI::SizeKind::percent;
                    item_layout.width.value = 1.0f;
                    item_layout.height.kind = GUI::SizeKind::fixed;
                    item_layout.height.value = 26.0f;
                    for(usize i = 0; i < items.size(); ++i)
                    {
                        id_t item_id = GUI::make_scoped_id(Internal::derived_id(id, "combo.item"), (u64)i + 1);
                        selectable(context, item_id, items[i] ? items[i] : "",
                            current_item && *current_item == (i32)i, item_layout);
                    }
                    GUI::FlexLayoutDesc list_flex;
                    list_flex.main_axis_gap = max(Internal::style_scalar(context, popup,
                        "gui.popup.gap", 4.0f), 0.0f);
                    list_flex.clip_children = true;
                    end_v_layout(context, list, list_flex);
                    lupanic_if_failed(end_popup(context, popup, RectF(0.0f, 0.0f, width, height)));
                }
            }
            return preview;
        }
    }
}
