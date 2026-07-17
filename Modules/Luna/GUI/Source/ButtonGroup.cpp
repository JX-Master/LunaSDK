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
                bool* selected_items = nullptr;
            };

            struct ButtonGroupItemData
            {
                c8* text = nullptr;
                bool enabled = true;
                bool* selected = nullptr;
                i32* selected_index = nullptr;
                usize item_index = 0;
            };

            static void draw_button_group_item_surface(GUICore::IContext* context,
                const GUICore::ElementHandle& element, const RectF& rect, const Float4U& rect_layout_scale,
                f32 radius, const Float4U& color, bool selected)
            {
                GUICore::DrawCommand command;
                if(selected)
                {
                    command.type = GUICore::DrawCommandType::shadow;
                    command.rect_reference = GUICore::DrawCommandRectReference::element;
                    command.rect = rect;
                    command.rect_layout_scale = rect_layout_scale;
                    command.color = color;
                    command.color.w *= 0.24f;
                    command.radius = radius;
                    command.shadow.offset = Float2U(0.0f,
                        style_vector2(context, element, "gui.shadow.offset", Float2U(3.0f)).y);
                    command.shadow.softness =
                        style_scalar(context, element, "gui.shadow.softness", 5.0f) * 0.8f;
                    context->draw(command);
                }

                command = GUICore::DrawCommand();
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = rect;
                command.rect_layout_scale = rect_layout_scale;
                command.color = color;
                command.radius = radius;
                context->draw(command);

                if(selected)
                {
                    command = GUICore::DrawCommand();
                    command.type = GUICore::DrawCommandType::shadow;
                    command.rect_reference = GUICore::DrawCommandRectReference::element;
                    command.rect = rect;
                    command.rect_layout_scale = rect_layout_scale;
                    command.color = Float4U(1.0f, 1.0f, 1.0f, 0.34f);
                    command.radius = radius;
                    command.shadow.offset = Float2U(0.0f, 1.0f);
                    command.shadow.mode = GUICore::ShadowMode::inner;
                    context->draw(command);
                }
            }

            static RV draw_button_group(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ButtonGroupData* data = (ButtonGroupData*)userdata;
                if(!data || !data->item_count) return ok;
                f32 radius = style_scalar(context, element, "gui.group.radius", 5.0f);
                f32 item_inset = style_scalar(context, element, "gui.group.padding", 2.0f) + 1.0f;
                f32 item_radius = style_scalar(context, element, "gui.group.selected_radius",
                    max(radius - item_inset - 1.0f, 0.0f));
                f32 shadow_softness = style_scalar(context, element, "gui.shadow.softness", 5.0f);
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

                command = GUICore::DrawCommand();
                command.type = GUICore::DrawCommandType::shadow;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                command.color = style_color(context, element, "gui.shadow.inset_light",
                    Float4U(1.0f, 1.0f, 1.0f, 0.90f));
                command.radius = max(radius - 1.0f, 0.0f);
                command.shadow.offset = Float2U(-2.0f, -2.0f);
                command.shadow.softness = shadow_softness * 0.4f;
                command.shadow.mode = GUICore::ShadowMode::inner;
                context->draw(command);

                command = GUICore::DrawCommand();
                command.type = GUICore::DrawCommandType::shadow;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                command.color = style_color(context, element, "gui.shadow.inset",
                    Float4U(0.0f, 0.0f, 0.0f, 0.18f));
                command.radius = max(radius - 1.0f, 0.0f);
                command.shadow.offset = Float2U(2.0f, 2.0f);
                command.shadow.softness = shadow_softness * 0.5f;
                command.shadow.mode = GUICore::ShadowMode::inner;
                context->draw(command);

                auto draw_item_surface = [&](f32 position, const Float4U& color, bool selected)
                {
                    f32 item_width = 1.0f / (f32)data->item_count;
                    RectF item_rect(item_inset - position * item_width * item_inset * 2.0f, item_inset,
                        -item_inset * item_width * 2.0f, -item_inset * 2.0f);
                    Float4U item_scale(position * item_width, 0.0f, item_width, 0.0f);
                    draw_button_group_item_surface(context, element, item_rect, item_scale,
                        item_radius, color, selected);
                };

                i32 hovered_item = -1;
                if(data->enabled && data->item_ids)
                {
                    for(usize i = 0; i < data->item_count; ++i)
                    {
                        if(context->get_interaction_state(data->item_ids[i]).hovered)
                        {
                            hovered_item = (i32)i;
                            break;
                        }
                    }
                }
                bool hovered_selected = hovered_item >= 0 &&
                    ((data->selected_index && *data->selected_index == hovered_item) ||
                        (data->selected_items && data->selected_items[hovered_item]));
                if(hovered_item >= 0 && !hovered_selected)
                {
                    draw_item_surface((f32)hovered_item,
                        style_color(context, element, "gui.group.hovered",
                            Float4U(0.13f, 0.19f, 0.27f, 1.0f)), false);
                }

                Float4U selected_color = style_color(context, element, "gui.group.selected",
                    Float4U(0.16f, 0.35f, 0.58f, 1.0f));
                if(data->state)
                {
                    draw_item_surface(data->state->animated_index, selected_color, true);
                }
                else if(data->selected_items)
                {
                    for(usize i = 0; i < data->item_count; ++i)
                    {
                        if(data->selected_items[i]) draw_item_surface((f32)i, selected_color, true);
                    }
                }
                return ok;
            }

            static RV draw_button_group_item(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ButtonGroupItemData* data = (ButtonGroupItemData*)userdata;
                if(!data) return ok;
                bool selected = data->selected ? *data->selected :
                    (data->selected_index && *data->selected_index == (i32)data->item_index);
                GUICore::DrawCommand text;
                text.type = GUICore::DrawCommandType::text;
                text.rect_reference = GUICore::DrawCommandRectReference::element;
                text.rect = RectF(6.0f, 0.0f, -12.0f, 0.0f);
                text.text = data->text ? data->text : "";
                text.font = style_name(context, element, "gui.font");
                text.font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                text.color = !data->enabled ?
                    style_color(context, element, "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f)) :
                    (selected ? style_color(context, element, "gui.accent.ink", Float4U(0.08f, 0.09f, 0.09f, 1.0f)) :
                        style_color(context, element, "gui.button.text", Float4U(1.0f)));
                text.horizontal_alignment = VG::TextAlignment::center;
                text.vertical_alignment = VG::TextAlignment::center;
                context->draw(text);
                return ok;
            }

            bool resolve_button_group_multi_action(GUICore::IContext* context, ButtonGroupMultiAction& action)
            {
                if(!action.enabled || !action.selected || !action.item_ids) return false;
                bool changed = false;
                for(usize i = 0; i < action.item_count; ++i)
                {
                    if(context->get_interaction_state(action.item_ids[i]).clicked)
                    {
                        action.selected[i] = !action.selected[i];
                        changed = true;
                    }
                }
                return changed;
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
                item_data->selected_index = selected_index;
                item_data->item_index = i;
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

        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, id_t id,
            Span<const c8*> items, Span<bool> selected, const GUICore::LayoutConfig& layout,
            const ButtonGroupDesc& desc)
        {
            luassert(context && id && items.size() == selected.size());
            GUICore::ElementHandle group = Internal::begin_element(context, id, "Multi Button Group", layout);
            GUICore::FlexLayoutDesc* flex = Internal::allocate_frame<GUICore::FlexLayoutDesc>(context);
            flex->axis = GUICore::LayoutAxis::x;
            flex->cross_alignment = GUICore::FlexAlignment::stretch;
            GUICore::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.button_group.multi");
            callbacks.measure_callback = GUICore::measure_flex;
            callbacks.callback = GUICore::layout_flex;
            callbacks.userdata = flex;
            context->set_layout_callback_config(group, callbacks);

            Internal::ButtonGroupData* group_data = Internal::allocate_frame<Internal::ButtonGroupData>(context);
            group_data->item_count = items.size();
            group_data->enabled = desc.enabled;
            GUICore::DrawConfig group_draw;
            group_draw.name = Name("gui.button_group.multi");
            group_draw.callback = Internal::draw_button_group;
            group_draw.userdata = group_data;
            context->set_draw_config(group, group_draw);

            id_t* item_ids = Internal::allocate_frame_array<id_t>(context, items.size());
            group_data->item_ids = item_ids;
            group_data->selected_items = selected.data();
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
                Internal::ButtonGroupItemData* item_data =
                    Internal::allocate_frame<Internal::ButtonGroupItemData>(context);
                item_data->text = Internal::copy_frame_string(context, items[i]);
                item_data->enabled = desc.enabled;
                item_data->selected = selected.data() + i;
                item_data->item_index = i;
                GUICore::DrawConfig item_draw;
                item_draw.name = Name("gui.button_group.multi.item");
                item_draw.callback = Internal::draw_button_group_item;
                item_draw.userdata = item_data;
                context->set_draw_config(item, item_draw);
                context->end_element();
            }
            context->end_element();
            Internal::ButtonGroupMultiAction* action =
                Internal::allocate_frame<Internal::ButtonGroupMultiAction>(context);
            action->selected = selected.data();
            action->item_ids = item_ids;
            action->item_count = items.size();
            action->enabled = desc.enabled;
            Internal::add_action(context, Internal::ActionType::button_group_multi, id, action);
            return group;
        }
    }
}
