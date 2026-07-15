/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ButtonGroup.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct ButtonGroupData
            {
                i32* selected_index = nullptr;
                id_t* item_ids = nullptr;
                usize item_count = 0;
                bool enabled = true;
                ButtonGroupState* state = nullptr;
            };

            struct ButtonGroupItemData
            {
                c8* text = nullptr;
                bool enabled = true;
            };

            static RV draw_button_group(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ButtonGroupData* data = (ButtonGroupData*)userdata;
                if(!data || !data->item_count) return ok;
                f32 radius = style_scalar(context, element, "gui.group.radius", 5.0f);
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.color = style_color(context, element, "gui.group.border", Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                command.radius = radius;
                context->draw(command);
                command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                command.color = style_color(context, element, "gui.group.background", Float4U(0.08f, 0.10f, 0.13f, 1.0f));
                command.radius = max(radius - 1.0f, 0.0f);
                context->draw(command);
                if(data->state)
                {
                    f32 item_width = 1.0f / (f32)data->item_count;
                    command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                    command.rect_layout_scale = Float4U(data->state->animated_index * item_width, 0.0f,
                        item_width, 0.0f);
                    command.color = style_color(context, element, "gui.group.selected",
                        Float4U(0.16f, 0.35f, 0.58f, 1.0f));
                    command.radius = max(radius - 1.0f, 0.0f);
                    context->draw(command);
                }
                if(data->enabled && data->item_ids)
                {
                    i32 hovered_item = -1;
                    for(usize i = 0; i < data->item_count; ++i)
                    {
                        if(context->get_interaction_state(data->item_ids[i]).hovered)
                        {
                            hovered_item = (i32)i;
                            break;
                        }
                    }
                    if(hovered_item >= 0 && (!data->selected_index || *data->selected_index != hovered_item))
                    {
                        f32 item_width = 1.0f / (f32)data->item_count;
                        command.type = GUICore::DrawCommandType::rounded_rect;
                        command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                        command.rect_layout_scale = Float4U((f32)hovered_item * item_width, 0.0f,
                            item_width, 0.0f);
                        command.color = style_color(context, element, "gui.group.hovered",
                            Float4U(0.13f, 0.19f, 0.27f, 1.0f));
                        command.radius = max(radius - 1.0f, 0.0f);
                        context->draw(command);
                    }
                }
                return ok;
            }

            static RV draw_button_group_item(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ButtonGroupItemData* data = (ButtonGroupItemData*)userdata;
                if(!data) return ok;
                GUICore::DrawCommand text;
                text.type = GUICore::DrawCommandType::text;
                text.rect_reference = GUICore::DrawCommandRectReference::element;
                text.rect = RectF(6.0f, 0.0f, -12.0f, 0.0f);
                text.text = data->text ? data->text : "";
                text.font = style_name(context, element, "gui.font");
                text.font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                text.color = data->enabled ? style_color(context, element, "gui.button.text", Float4U(1.0f)) :
                    style_color(context, element, "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                text.horizontal_alignment = VG::TextAlignment::center;
                text.vertical_alignment = VG::TextAlignment::center;
                context->draw(text);
                return ok;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, i32* selected_index, const GUICore::LayoutConfig& layout,
            const ButtonGroupDesc& desc)
        {
            luassert(context && id && selected_index);
            if(items.empty())
            {
                *selected_index = -1;
            }
            else
            {
                *selected_index = clamp(*selected_index, 0, (i32)items.size() - 1);
            }
            GUICore::ElementHandle group = Internal::begin_element(context, id, "Button Group", layout);
            GUICore::FlexLayoutDesc* flex = Internal::allocate_frame<GUICore::FlexLayoutDesc>(context);
            flex->axis = GUICore::LayoutAxis::x;
            flex->cross_alignment = GUICore::FlexAlignment::stretch;
            GUICore::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.button_group");
            layout_callbacks.measure_callback = GUICore::measure_flex;
            layout_callbacks.callback = GUICore::layout_flex;
            layout_callbacks.userdata = flex;
            context->set_layout_callback_config(group, layout_callbacks);

            Ref<Internal::ButtonGroupState> state = Internal::widget_state<Internal::ButtonGroupState>(context, id);
            Internal::ButtonGroupData* group_data = Internal::allocate_frame<Internal::ButtonGroupData>(context);
            group_data->selected_index = selected_index;
            group_data->item_count = items.size();
            group_data->enabled = desc.enabled;
            group_data->state = state.get();
            GUICore::DrawConfig group_draw;
            group_draw.name = Name("gui.button_group");
            group_draw.callback = Internal::draw_button_group;
            group_draw.userdata = group_data;
            context->set_draw_config(group, group_draw);

            id_t* item_ids = Internal::allocate_frame_array<id_t>(context, items.size());
            group_data->item_ids = item_ids;
            for(usize i = 0; i < items.size(); ++i)
            {
                id_t item_id = GUICore::make_scoped_id(id, (id_t)i + 1);
                item_ids[i] = item_id;
                GUICore::LayoutConfig item_layout;
                item_layout.width.kind = GUICore::SizeKind::fit;
                item_layout.width.min = desc.item_min_width;
                item_layout.height.kind = GUICore::SizeKind::fixed;
                item_layout.height.value = 32.0f;
                item_layout.flex_grow = 1.0f;
                GUICore::ElementHandle item = Internal::begin_element(context, item_id, items[i], item_layout);
                Internal::set_interactable(context, item, desc.enabled);
                Internal::ButtonGroupItemData* item_data = Internal::allocate_frame<Internal::ButtonGroupItemData>(context);
                item_data->text = Internal::copy_frame_string(context, items[i]);
                item_data->enabled = desc.enabled;
                GUICore::DrawConfig item_draw;
                item_draw.name = Name("gui.button_group.item");
                item_draw.callback = Internal::draw_button_group_item;
                item_draw.userdata = item_data;
                context->set_draw_config(item, item_draw);
                context->end_element();
            }
            context->end_element();

            Internal::ButtonGroupAction* action = Internal::allocate_frame<Internal::ButtonGroupAction>(context);
            action->selected_index = selected_index;
            action->item_ids = item_ids;
            action->item_count = items.size();
            action->enabled = desc.enabled;
            action->state = state.get();
            Internal::add_action(context, Internal::ActionType::button_group, id, action);
            return group;
        }
    }
}
