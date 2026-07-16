/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Runtime.cpp
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
        static i32 popup_layer_at_position(GUICore::IContext* context, const Internal::FrameState& frame,
            const Float2U& position)
        {
            Span<const GUICore::Layer> layers = context->get_layers();
            i32 result = -1;
            for(const Internal::Action& record : frame.actions)
            {
                if(record.type != Internal::ActionType::popup) continue;
                Internal::PopupAction* action = (Internal::PopupAction*)record.data;
                const GUICore::Element* element = action ? context->get_element(action->root.index) : nullptr;
                if(!element || element->layer >= layers.size()) continue;
                const GUICore::Layer& layer = layers[element->layer];
                const RectF& rect = element->layout_result.rect;
                f32 x = layer.screen_position.x + rect.offset_x;
                f32 y = layer.screen_position.y + rect.offset_y;
                if(position.x >= x && position.y >= y &&
                    position.x < x + rect.width && position.y < y + rect.height)
                {
                    result = max(result, (i32)element->layer);
                }
            }
            return result;
        }

        static i32 popup_action_layer(GUICore::IContext* context, const Internal::PopupAction& action)
        {
            const GUICore::Element* element = context->get_element(action.root.index);
            return element ? (i32)element->layer : -1;
        }

        LUNA_GUI_API ResolveResult resolve_interactions(GUICore::IContext* context)
        {
            ResolveResult result;
            if(!context) return result;
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            f32 delta_time = max(context->get_frame_desc().delta_time, 0.0f);
            for(const GUICore::InputEvent& event : context->get_input_events())
            {
                bool navigation_back = (event.type == GUICore::InputEventType::key_down &&
                    event.key == KeyCode::esc) || event.type == GUICore::InputEventType::navigation_back;
                if(navigation_back)
                {
                    Internal::PopupAction* topmost = nullptr;
                    i32 topmost_layer = -1;
                    for(Internal::Action& record : frame->actions)
                    {
                        if(record.type != Internal::ActionType::popup) continue;
                        Internal::PopupAction* action = (Internal::PopupAction*)record.data;
                        if(!action || !test_flags(action->flags, PopupFlag::close_on_escape)) continue;
                        i32 layer = popup_action_layer(context, *action);
                        if(layer > topmost_layer)
                        {
                            topmost = action;
                            topmost_layer = layer;
                        }
                    }
                    if(topmost) close_popup(context, topmost->id);
                }
                if(event.type == GUICore::InputEventType::pointer_down &&
                    event.button == GUICore::PointerButton::left)
                {
                    i32 hit_layer = popup_layer_at_position(context, *frame, event.position);
                    for(Internal::Action& record : frame->actions)
                    {
                        if(record.type != Internal::ActionType::popup) continue;
                        Internal::PopupAction* action = (Internal::PopupAction*)record.data;
                        if(action && test_flags(action->flags, PopupFlag::close_on_outside_click) &&
                            popup_action_layer(context, *action) > hit_layer)
                        {
                            close_popup(context, action->id);
                        }
                    }
                }
            }
            for(Internal::Action& record : frame->actions)
            {
                switch(record.type)
                {
                case Internal::ActionType::button:
                {
                    Internal::ButtonAction* action = (Internal::ButtonAction*)record.data;
                    if(action && action->state)
                    {
                        GUICore::InteractionState interaction = context->get_interaction_state(action->id);
                        action->state->hovered = Internal::smooth_step(action->state->hovered,
                            action->enabled && interaction.hovered ? 1.0f : 0.0f, 12.0f, delta_time);
                        action->state->active = Internal::smooth_step(action->state->active,
                            action->enabled && interaction.active ? 1.0f : 0.0f, 18.0f, delta_time);
                    }
                    break;
                }
                case Internal::ActionType::button_group:
                {
                    Internal::ButtonGroupAction* action = (Internal::ButtonGroupAction*)record.data;
                    if(!action || !action->selected_index || !action->state || !action->item_count) break;
                    if(action->enabled)
                    {
                        for(usize i = 0; i < action->item_count; ++i)
                        {
                            if(context->get_interaction_state(action->item_ids[i]).clicked &&
                                *action->selected_index != (i32)i)
                            {
                                *action->selected_index = (i32)i;
                                result.value_changed = true;
                                break;
                            }
                        }
                    }
                    if(!action->state->initialized)
                    {
                        action->state->animated_index = (f32)*action->selected_index;
                        action->state->initialized = true;
                    }
                    else
                    {
                        action->state->animated_index = Internal::smooth_step(action->state->animated_index,
                            (f32)*action->selected_index, 10.0f, delta_time);
                    }
                    break;
                }
                case Internal::ActionType::button_group_multi:
                    result.value_changed |= Internal::resolve_button_group_multi_action(context,
                        *(Internal::ButtonGroupMultiAction*)record.data);
                    break;
                case Internal::ActionType::choice:
                    result.value_changed |= Internal::resolve_choice_action(context,
                        *(Internal::ChoiceAction*)record.data);
                    break;
                case Internal::ActionType::disclosure:
                    result.value_changed |= Internal::resolve_disclosure_action(context,
                        *(Internal::DisclosureAction*)record.data);
                    break;
                case Internal::ActionType::input_text:
                    result.value_changed |= Internal::resolve_input_text_action(context,
                        *(Internal::TextInputAction*)record.data);
                    break;
                case Internal::ActionType::slider_float:
                    result.value_changed |= Internal::resolve_slider_float_action(context,
                        *(Internal::SliderFloatAction*)record.data);
                    break;
                case Internal::ActionType::slider_int:
                    result.value_changed |= Internal::resolve_slider_int_action(context,
                        *(Internal::SliderIntAction*)record.data);
                    break;
                case Internal::ActionType::drag_float:
                    result.value_changed |= Internal::resolve_drag_float_action(context,
                        *(Internal::DragFloatAction*)record.data);
                    break;
                case Internal::ActionType::drag_int:
                    result.value_changed |= Internal::resolve_drag_int_action(context,
                        *(Internal::DragIntAction*)record.data);
                    break;
                case Internal::ActionType::scroll_view:
                    result.relayout_requested |= Internal::resolve_scroll_action(context,
                        *(Internal::ScrollAction*)record.data);
                    break;
                case Internal::ActionType::tab_bar:
                    result.value_changed |= Internal::resolve_tab_action(context,
                        *(Internal::TabAction*)record.data);
                    break;
                case Internal::ActionType::table:
                    result.relayout_requested |= Internal::resolve_table_action(context,
                        *(Internal::TableAction*)record.data);
                    break;
                case Internal::ActionType::popup:
                    break;
                case Internal::ActionType::dock_space:
                    result.relayout_requested |= Internal::resolve_dock_space_action(context,
                        *(Internal::DockSpaceAction*)record.data);
                    break;
                default:
                    break;
                }
            }
            return result;
        }
    }
}
