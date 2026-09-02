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
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace EditorGUI
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

            static RV draw_button_group_item_surface(GUI::IContext* context,
                const GUI::ElementHandle& element, const RectF& rect, const Float4U& rect_layout_scale,
                f32 radius, const Float4U& color, bool selected,
                GUI::paint_order_id_t paint_order_id)
            {
                RoundedRectEffect effects[3];
                u32 num_effects = 0;
                if(selected)
                {
                    RoundedRectEffect& shadow = effects[num_effects++];
                    shadow.shadow = true;
                    shadow.color = color;
                    shadow.color.w *= 0.24f;
                    shadow.shadow_desc.offset = Float2U(0.0f,
                        style_vector2(context, element, "gui.shadow.offset", Float2U(3.0f)).y);
                    shadow.shadow_desc.softness =
                        style_scalar(context, element, "gui.shadow.softness", 5.0f) * 0.8f;
                }
                effects[num_effects++].color = color;
                if(selected)
                {
                    RoundedRectEffect& highlight = effects[num_effects++];
                    highlight.shadow = true;
                    highlight.color = Float4U(1.0f, 1.0f, 1.0f, 0.34f);
                    highlight.shadow_desc.offset = Float2U(0.0f, 1.0f);
                    highlight.shadow_desc.mode = GUI::ShadowMode::inner;
                }
                return draw_rounded_rect_effects(context, element, rect, rect_layout_scale,
                    radius, Span<const RoundedRectEffect>(effects, num_effects), paint_order_id);
            }

            static R<GUI::paint_order_id_t> draw_button_group(GUI::IContext* context,
                const GUI::ElementHandle& element, GUI::DrawPhase,
                GUI::paint_order_id_t paint_order_id, void* userdata)
            {
                ButtonGroupData* data = (ButtonGroupData*)userdata;
                if(!data || !data->item_count) return paint_order_id;
                f32 radius = style_scalar(context, element, "gui.group.radius", 5.0f);
                f32 item_inset = style_scalar(context, element, "gui.group.padding", 2.0f) + 1.0f;
                f32 item_radius = style_scalar(context, element, "gui.group.selected_radius",
                    max(radius - item_inset - 1.0f, 0.0f));
                f32 shadow_softness = style_scalar(context, element, "gui.shadow.softness", 5.0f);
                RoundedRectEffect border;
                border.color = style_color(context, element, "gui.group.border",
                    Float4U(0.24f, 0.30f, 0.38f, 1.0f));
                if(RV result = draw_rounded_rect_effects(context, element, RectF(), Float4U(), radius,
                    Span<const RoundedRectEffect>(&border, 1), paint_order_id); failed(result))
                {
                    return result.errcode();
                }

                RoundedRectEffect inner_effects[2];
                inner_effects[0].color = style_color(context, element, "gui.group.background",
                    Float4U(0.08f, 0.10f, 0.13f, 1.0f));
                inner_effects[1].shadow = true;
                inner_effects[1].color = style_color(context, element, "gui.shadow.inset",
                    Float4U(0.0f, 0.0f, 0.0f, 0.18f));
                inner_effects[1].shadow_desc.offset = Float2U(1.0f, 1.0f);
                inner_effects[1].shadow_desc.softness = shadow_softness * 0.65f;
                inner_effects[1].shadow_desc.mode = GUI::ShadowMode::inner;
                if(RV result = draw_rounded_rect_effects(context, element,
                    RectF(1.0f, 1.0f, -2.0f, -2.0f),
                    Float4U(), max(radius - 1.0f, 0.0f),
                    Span<const RoundedRectEffect>(inner_effects, 2), paint_order_id + 1); failed(result))
                {
                    return result.errcode();
                }

                auto draw_item_surface = [&](f32 position, const Float4U& color, bool selected) -> RV
                {
                    f32 item_width = 1.0f / (f32)data->item_count;
                    RectF item_rect(item_inset - position * item_width * item_inset * 2.0f, item_inset,
                        -item_inset * item_width * 2.0f, -item_inset * 2.0f);
                    Float4U item_scale(position * item_width, 0.0f, item_width, 0.0f);
                    return draw_button_group_item_surface(context, element, item_rect, item_scale,
                        item_radius, color, selected, paint_order_id + 2);
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
                    if(RV result = draw_item_surface((f32)hovered_item,
                        style_color(context, element, "gui.group.hovered",
                            Float4U(0.13f, 0.19f, 0.27f, 1.0f)), false); failed(result))
                    {
                        return result.errcode();
                    }
                }

                Float4U selected_color = style_color(context, element, "gui.group.selected",
                    Float4U(0.16f, 0.35f, 0.58f, 1.0f));
                if(data->state)
                {
                    if(RV result = draw_item_surface(data->state->animated_index,
                        selected_color, true); failed(result))
                    {
                        return result.errcode();
                    }
                }
                else if(data->selected_items)
                {
                    for(usize i = 0; i < data->item_count; ++i)
                    {
                        if(data->selected_items[i])
                        {
                            if(RV result = draw_item_surface((f32)i, selected_color, true); failed(result))
                            {
                                return result.errcode();
                            }
                        }
                    }
                }
                return paint_order_id + 2;
            }

            static R<GUI::paint_order_id_t> draw_button_group_item(GUI::IContext* context,
                const GUI::ElementHandle& element, GUI::DrawPhase,
                GUI::paint_order_id_t paint_order_id, void* userdata)
            {
                ButtonGroupItemData* data = (ButtonGroupItemData*)userdata;
                if(!data) return paint_order_id;
                bool selected = data->selected ? *data->selected :
                    (data->selected_index && *data->selected_index == (i32)data->item_index);
                GUI::DrawCommand text;
                text.type = GUI::DrawCommandType::text;
                text.rect_reference = GUI::DrawCommandRectReference::element;
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
                context->draw(text, paint_order_id);
                return paint_order_id;
            }

            bool resolve_button_group_multi_action(GUI::IContext* context, ButtonGroupMultiAction& action)
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

        LUNA_EDITOR_GUI_API GUI::ElementHandle button_group(GUI::IContext* context, id_t id,
            Span<const c8*> items, i32* selected_index, const GUI::LayoutConfig& layout,
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
            GUI::ElementHandle group = Internal::begin_element(context, id, "Button Group", layout);
            GUI::FlexLayoutDesc* flex = Internal::allocate_frame<GUI::FlexLayoutDesc>(context);
            flex->axis = GUI::LayoutAxis::x;
            flex->cross_alignment = GUI::FlexAlignment::stretch;
            GUI::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.button_group");
            layout_callbacks.measure_callback = GUI::measure_flex;
            layout_callbacks.callback = GUI::layout_flex;
            layout_callbacks.userdata = flex;
            context->set_layout_callback_config(group, layout_callbacks);
            context->set_child_paint_order_mode(group, GUI::ChildPaintOrderMode::shared);

            Ref<Internal::ButtonGroupState> state = Internal::widget_state<Internal::ButtonGroupState>(context, id);
            Internal::ButtonGroupData* group_data = Internal::allocate_frame<Internal::ButtonGroupData>(context);
            group_data->selected_index = selected_index;
            group_data->item_count = items.size();
            group_data->enabled = desc.enabled;
            group_data->state = state.get();
            GUI::DrawConfig group_draw;
            group_draw.name = Name("gui.button_group");
            group_draw.callback = Internal::draw_button_group;
            group_draw.userdata = group_data;
            context->set_draw_config(group, group_draw);

            id_t* item_ids = Internal::allocate_frame_array<id_t>(context, items.size());
            group_data->item_ids = item_ids;
            for(usize i = 0; i < items.size(); ++i)
            {
                id_t item_id = GUI::make_scoped_id(id, (id_t)i + 1);
                item_ids[i] = item_id;
                GUI::LayoutConfig item_layout;
                item_layout.width.kind = GUI::SizeKind::fit;
                item_layout.width.min = desc.item_min_width;
                item_layout.height.kind = GUI::SizeKind::fixed;
                item_layout.height.value = 32.0f;
                item_layout.flex_grow = 1.0f;
                GUI::ElementHandle item = Internal::begin_element(context, item_id, items[i], item_layout);
                Internal::set_interactable(context, item, desc.enabled);
                Internal::ButtonGroupItemData* item_data = Internal::allocate_frame<Internal::ButtonGroupItemData>(context);
                item_data->text = Internal::copy_frame_string(context, items[i]);
                item_data->enabled = desc.enabled;
                item_data->selected_index = selected_index;
                item_data->item_index = i;
                GUI::DrawConfig item_draw;
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
            action->animation_duration = desc.animation_duration;
            action->state = state.get();
            Internal::add_action(context, Internal::ActionType::button_group, id, action);
            return group;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle button_group(GUI::IContext* context, id_t id,
            Span<const c8*> items, Span<bool> selected, const GUI::LayoutConfig& layout,
            const ButtonGroupDesc& desc)
        {
            luassert(context && id && items.size() == selected.size());
            GUI::ElementHandle group = Internal::begin_element(context, id, "Multi Button Group", layout);
            GUI::FlexLayoutDesc* flex = Internal::allocate_frame<GUI::FlexLayoutDesc>(context);
            flex->axis = GUI::LayoutAxis::x;
            flex->cross_alignment = GUI::FlexAlignment::stretch;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.button_group.multi");
            callbacks.measure_callback = GUI::measure_flex;
            callbacks.callback = GUI::layout_flex;
            callbacks.userdata = flex;
            context->set_layout_callback_config(group, callbacks);
            context->set_child_paint_order_mode(group, GUI::ChildPaintOrderMode::shared);

            Internal::ButtonGroupData* group_data = Internal::allocate_frame<Internal::ButtonGroupData>(context);
            group_data->item_count = items.size();
            group_data->enabled = desc.enabled;
            GUI::DrawConfig group_draw;
            group_draw.name = Name("gui.button_group.multi");
            group_draw.callback = Internal::draw_button_group;
            group_draw.userdata = group_data;
            context->set_draw_config(group, group_draw);

            id_t* item_ids = Internal::allocate_frame_array<id_t>(context, items.size());
            group_data->item_ids = item_ids;
            group_data->selected_items = selected.data();
            for(usize i = 0; i < items.size(); ++i)
            {
                id_t item_id = GUI::make_scoped_id(id, (id_t)i + 1);
                item_ids[i] = item_id;
                GUI::LayoutConfig item_layout;
                item_layout.width.kind = GUI::SizeKind::fit;
                item_layout.width.min = desc.item_min_width;
                item_layout.height.kind = GUI::SizeKind::fixed;
                item_layout.height.value = 32.0f;
                item_layout.flex_grow = 1.0f;
                GUI::ElementHandle item = Internal::begin_element(context, item_id, items[i], item_layout);
                Internal::set_interactable(context, item, desc.enabled);
                Internal::ButtonGroupItemData* item_data =
                    Internal::allocate_frame<Internal::ButtonGroupItemData>(context);
                item_data->text = Internal::copy_frame_string(context, items[i]);
                item_data->enabled = desc.enabled;
                item_data->selected = selected.data() + i;
                item_data->item_index = i;
                GUI::DrawConfig item_draw;
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
