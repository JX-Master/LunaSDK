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
        LUNA_GUI_API ResolveResult resolve_interactions(GUICore::IContext* context)
        {
            ResolveResult result;
            if(!context) return result;
            Ref<Internal::FrameState> frame = Internal::frame_state(context);
            f32 delta_time = max(context->get_frame_desc().delta_time, 0.0f);
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
                case Internal::ActionType::scroll_view:
                    result.relayout_requested |= Internal::resolve_scroll_action(context,
                        *(Internal::ScrollAction*)record.data);
                    break;
                case Internal::ActionType::tab_bar:
                    result.value_changed |= Internal::resolve_tab_action(context,
                        *(Internal::TabAction*)record.data);
                    break;
                default:
                    break;
                }
            }
            return result;
        }
    }
}
