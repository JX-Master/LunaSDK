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
        static void draw_combo_rect(GUI::IContext* context, const Float4U& color, f32 radius,
            const RectF& rect = RectF(0.0f, 0.0f, 0.0f, 0.0f))
        {
            GUI::DrawCommand command;
            command.type = GUI::DrawCommandType::rounded_rect;
            command.rect_reference = GUI::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_combo_text(GUI::IContext* context, const RectF& rect, const c8* value,
            const Name& font, const Float4U& color, f32 font_size,
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
            context->draw(command);
        }

        static void draw_combo_chevron(GUI::IContext* context, const Float4U& color, bool open)
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
            context->draw(line);
            line.rect = RectF(-18.0f, center_y, 0.0f, 0.0f);
            line.point1 = Float2U(-13.0f, outer_y);
            context->draw(line);
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
            draw_combo_rect(context, border, radius);
            draw_combo_rect(context, background, max(radius - 1.0f, 0.0f), RectF(1.0f, 1.0f, -2.0f, -2.0f));
            const c8* selected = "";
            if(current_item && *current_item >= 0 && (usize)*current_item < items.size()) selected = items[(usize)*current_item];
            draw_combo_text(context, RectF(12.0f, 0.0f, -44.0f, 0.0f), selected,
                Internal::style_name(context, preview, "gui.font"), text_color, font_size);
            draw_combo_chevron(context, text_color, open);
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
                    lupanic_if_failed(end_popup(context, popup, RectF(0.0f, 0.0f, width, height)));
                }
            }
            return preview;
        }
    }
}
