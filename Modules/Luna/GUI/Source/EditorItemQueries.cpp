/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorItemQueries.cpp
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
        static const GUICore::Element* valid_element(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            if(!context || !handle.id || handle.index == GUICore::INVALID_ELEMENT ||
                handle.context != context->get_object() || handle.generation != context->generation())
            {
                return nullptr;
            }
            const GUICore::Element* element = context->get_element(handle.index);
            return element && element->id == handle.id ? element : nullptr;
        }

        LUNA_GUI_API bool is_item_valid(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) != nullptr;
        }

        LUNA_GUI_API bool is_item_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) ? context->get_interaction_state(handle.id).clicked : false;
        }

        LUNA_GUI_API bool is_item_right_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            if(!valid_element(context, handle))
            {
                return false;
            }
            Span<const GUICore::InputEvent> events = context->get_delivered_input_events(handle.id);
            for(const GUICore::InputEvent& event : events)
            {
                if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::right)
                {
                    return true;
                }
            }
            return false;
        }

        LUNA_GUI_API bool is_item_double_clicked(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) ? context->get_interaction_state(handle.id).double_clicked : false;
        }

        LUNA_GUI_API bool is_item_hovered(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) ? context->get_interaction_state(handle.id).hovered : false;
        }

        LUNA_GUI_API bool is_item_active(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) ? context->get_interaction_state(handle.id).active : false;
        }

        LUNA_GUI_API bool is_item_focused(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            return valid_element(context, handle) ? context->get_interaction_state(handle.id).focused : false;
        }

        LUNA_GUI_API RectF get_item_rect(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            const GUICore::Element* element = valid_element(context, handle);
            return element ? element->layout_result.rect : RectF(0.0f, 0.0f, 0.0f, 0.0f);
        }

        LUNA_GUI_API RectF get_item_clip_rect(GUICore::IContext* context, const GUICore::ElementHandle& handle)
        {
            const GUICore::Element* element = valid_element(context, handle);
            return element ? element->layout_result.clip_rect : RectF(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}
