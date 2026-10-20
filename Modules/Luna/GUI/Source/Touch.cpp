/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Touch.cpp
* @author JXMaster
* @date 2026/10/20
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

namespace Luna::GUI
{
    namespace
    {
        constexpr f32 TOUCH_MOVEMENT_THRESHOLD = 8.0f;

        bool touch_target_valid(const Element* element)
        {
            return element && !has_flags(element->interactable, InteractableFlag::disabled) &&
                element->interactable.pointer_hit_behavior != PointerHitBehavior::none &&
                element->interactable.pointer_hit_behavior != PointerHitBehavior::block;
        }

        bool touch_target_activatable(const Element* element)
        {
            return touch_target_valid(element) && has_flags(element->interactable, InteractableFlag::activatable) &&
                !has_flags(element->interactable, InteractableFlag::read_only);
        }

        InputEvent contact_event(const TouchContact& contact, InputEventType type)
        {
            InputEvent event;
            event.type = type;
            event.device_id = contact.state.device_id;
            event.touch_id = contact.state.touch_id;
            event.position = contact.state.position;
            return event;
        }

        Vector<id_t> touch_targets(Context* context, const Float2U& position)
        {
            Vector<id_t> result;
            context->hit_test(position, [](const HitTestVisit& visit, void* userdata)
            {
                if(visit.event_target) ((Vector<id_t>*)userdata)->push_back(visit.element.id);
            }, &result);
            return result;
        }
    }

    bool Context::get_touch_state(u64 device_id, u64 touch_id, TouchState& out_state) const
    {
        lutsassert();
        for(const TouchContact& contact : m_touch_contacts)
        {
            if(contact.state.device_id == device_id && contact.state.touch_id == touch_id)
            {
                out_state = contact.state;
                return true;
            }
        }
        out_state = TouchState();
        return false;
    }

    void Context::capture_touch(u64 device_id, u64 touch_id, id_t element_id)
    {
        lutsassert();
        const Element* element = find_element(element_id);
        if(!touch_target_valid(element) || has_flags(element->interactable, InteractableFlag::read_only)) return;
        for(TouchContact& contact : m_touch_contacts)
        {
            if(contact.state.device_id != device_id || contact.state.touch_id != touch_id) continue;
            InputEvent cancel = contact_event(contact, InputEventType::touch_cancel);
            for(id_t target : contact.targets) if(target != element_id) deliver_input_event(target, cancel);
            contact.targets.clear();
            contact.targets.push_back(element_id);
            contact.state.captured_element = element_id;
            contact.gesture_claimed = true;
            contact.tap_canceled = true;
            return;
        }
    }

    void Context::release_touch_capture(u64 device_id, u64 touch_id)
    {
        lutsassert();
        for(TouchContact& contact : m_touch_contacts)
        {
            if(contact.state.device_id != device_id || contact.state.touch_id != touch_id) continue;
            InputEvent cancel = contact_event(contact, InputEventType::touch_cancel);
            for(id_t target : contact.targets) deliver_input_event(target, cancel);
            contact.targets.clear();
            contact.state.captured_element = 0;
            contact.tap_canceled = true;
            contact.gesture_claimed = true;
            return;
        }
    }

    void Context::prepare_touch_input()
    {
        for(auto iter = m_touch_contacts.begin(); iter != m_touch_contacts.end();)
        {
            bool valid = true;
            for(id_t target : iter->targets)
            {
                const Element* element = find_element(target);
                if(!touch_target_valid(element))
                    valid = false;
            }
            if(!valid)
            {
                InputEvent cancel = contact_event(*iter, InputEventType::touch_cancel);
                for(id_t target : iter->targets) deliver_input_event(target, cancel);
                iter = m_touch_contacts.erase(iter);
            }
            else
            {
                iter->state.delta = Float2U(0.0f);
                ++iter;
            }
        }
    }

    void Context::cancel_touch_contacts()
    {
        for(const TouchContact& contact : m_touch_contacts)
        {
            InputEvent cancel = contact_event(contact, InputEventType::touch_cancel);
            for(id_t target : contact.targets) deliver_input_event(target, cancel);
        }
        m_touch_contacts.clear();
    }

    void Context::route_touch_event(const InputEvent& input)
    {
        auto iter = m_touch_contacts.begin();
        while(iter != m_touch_contacts.end() &&
            (iter->state.device_id != input.device_id || iter->state.touch_id != input.touch_id)) ++iter;
        if(input.type == InputEventType::touch_down)
        {
            if(iter != m_touch_contacts.end())
            {
                InputEvent cancel = contact_event(*iter, InputEventType::touch_cancel);
                for(id_t target : iter->targets) deliver_input_event(target, cancel);
                m_touch_contacts.erase(iter);
            }
            TouchContact contact;
            contact.state.device_id = input.device_id;
            contact.state.touch_id = input.touch_id;
            contact.state.position = input.position;
            contact.state.start_position = input.position;
            contact.targets = touch_targets(this, input.position);
            bool focused = false;
            for(id_t target : contact.targets)
            {
                const Element* element = find_element(target);
                deliver_input_event(target, input);
                if(!contact.state.captured_element && touch_target_activatable(element))
                    contact.state.captured_element = target;
                if(!focused && element && element_can_focus(*element))
                {
                    m_focused_element = target;
                    focused = true;
                }
            }
            if(!focused) m_focused_element = 0;
            m_touch_contacts.push_back(move(contact));
            return;
        }
        if(iter == m_touch_contacts.end()) return;
        TouchContact& contact = *iter;
        InputEvent event = input;
        event.touch_delta = input.position - contact.state.position;
        contact.state.delta = contact.state.delta + event.touch_delta;
        contact.state.position = input.position;
        Float2U displacement = input.position - contact.state.start_position;
        bool moved = displacement.x * displacement.x + displacement.y * displacement.y >=
            TOUCH_MOVEMENT_THRESHOLD * TOUCH_MOVEMENT_THRESHOLD;
        if(moved) contact.tap_canceled = true;
        if(event.type == InputEventType::touch_move && moved && !contact.gesture_claimed)
        {
            TouchGestureFlag axis = abs(displacement.x) > abs(displacement.y) ?
                TouchGestureFlag::drag_x : TouchGestureFlag::drag_y;
            id_t winner = 0;
            for(id_t target : contact.targets)
            {
                const Element* element = find_element(target);
                while(element)
                {
                    if(touch_target_valid(element) && !has_flags(element->interactable, InteractableFlag::read_only) &&
                        test_flags(element->interactable.touch_gestures, axis))
                    {
                        winner = element->id;
                        break;
                    }
                    element = element->parent != INVALID_ELEMENT ? get_element(element->parent) : nullptr;
                }
                if(winner) break;
            }
            if(winner)
            {
                InputEvent cancel = contact_event(contact, InputEventType::touch_cancel);
                for(id_t target : contact.targets)
                {
                    if(target != winner) deliver_input_event(target, cancel);
                }
                contact.targets.clear();
                contact.targets.push_back(winner);
                contact.state.captured_element = winner;
                contact.gesture_claimed = true;
                event.touch_delta = displacement;
            }
        }
        event.touch_dragging = contact.gesture_claimed;
        for(id_t target : contact.targets) deliver_input_event(target, event);
        if(event.type == InputEventType::touch_up && !contact.tap_canceled)
        {
            Vector<id_t> hits = touch_targets(this, event.position);
            TouchTapHistory* previous = nullptr;
            for(TouchTapHistory& tap : m_touch_taps) if(tap.device_id == event.device_id) previous = &tap;
            if(!previous)
            {
                TouchTapHistory tap;
                tap.device_id = event.device_id;
                m_touch_taps.push_back(tap);
                previous = &m_touch_taps.back();
            }
            id_t representative = 0;
            for(id_t target : contact.targets)
            {
                const Element* element = find_element(target);
                if(!touch_target_activatable(element)) continue;
                bool hit = false;
                for(id_t id : hits) if(id == target) hit = true;
                if(!hit) continue;
                InteractionState& state = get_or_create_interaction(target);
                Float2U delta = event.position - previous->position;
                state.clicked = state.touch_clicked = true;
                state.touch_double_clicked = previous->element == target && m_elapsed_time - previous->time <= 0.35f &&
                    delta.x * delta.x + delta.y * delta.y <= 36.0f;
                state.double_clicked |= state.touch_double_clicked;
                state.clicked_screen_position = event.position;
                if(element->layer < m_layers.size())
                {
                    state.clicked_element_position = event.position - m_layers[element->layer].screen_position -
                        Float2U(element->layout_result.rect.offset_x, element->layout_result.rect.offset_y);
                    state.clicked_element_rect = element->layout_result.rect;
                }
                if(!representative) representative = target;
            }
            if(representative)
            {
                previous->element = representative;
                previous->position = event.position;
                previous->time = m_elapsed_time;
            }
        }
        if(event.type == InputEventType::touch_up || event.type == InputEventType::touch_cancel)
            m_touch_contacts.erase(iter);
    }

    void Context::update_touch_interactions()
    {
        for(const TouchContact& contact : m_touch_contacts)
        {
            for(id_t target : contact.targets)
            {
                if(!touch_target_activatable(find_element(target)) && target != contact.state.captured_element) continue;
                InteractionState& state = get_or_create_interaction(target);
                state.active = state.touch_active = true;
            }
        }
    }
}
