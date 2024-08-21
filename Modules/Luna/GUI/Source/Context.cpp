/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.cpp
* @author JXMaster
* @date 2024/3/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Context.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/Shapes.hpp>
#include "../Widgets/Widget.hpp"
#include "../Event.hpp"
#include "../Widgets/Container.hpp"

namespace Luna
{
    namespace GUI
    {
        void Context::set_widget(IWidget* root_widget)
        {
            m_root_widget = root_widget;
        }
        object_t Context::get_widget_state(widget_id_t id)
        {
            auto iter = m_widget_state_reg.find(id);
            return iter == m_widget_state_reg.end() ? nullptr : iter->second.state.get();
        }
        void Context::set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime)
        {
            m_widget_state_reg.insert_or_assign(id, WidgetStateEntry(ObjRef(state), lifetime));
        }
        RV Context::dispatch_event(object_t e)
        {
            lutry
            {
                bool handled = false;
                // Dispatch the event to capture firstly.
                for(auto iter = m_event_capture_stack.rbegin(); iter != m_event_capture_stack.rend(); ++iter)
                {
                    if(object_is_type(e, iter->second))
                    {
                        luexp(iter->first->handle_event(this, e, handled));
                        if(handled) return ok;
                    }
                }
                // If this is a mouse event, dispatch the event based on location.
                MouseEvent* me = cast_object<MouseEvent>(e);
                if(me)
                {
                    luexp(dispatch_event_by_pos(this, m_root_widget, e, me->x, me->y, handled));
                    if(handled) return ok;
                }
                // No widget can handle this event, do nothing.
            }
            lucatchret;
            return ok;
        }
        RV Context::update()
        {
            // Update state lifetime.
            auto iter = m_widget_state_reg.begin();
            while(iter != m_widget_state_reg.end())
            {
                if (iter->second.lifetime == WidgetStateLifetime::frame)
                {
                    iter = m_widget_state_reg.erase(iter);
                    continue;
                }
                else if (iter->second.lifetime == WidgetStateLifetime::next_frame)
                {
                    iter->second.lifetime = WidgetStateLifetime::frame;
                }
                ++iter;
            }
            // Clear all existing captures.
            m_event_capture_stack.clear();
            // Update widgets.
            lutry
            {
                luexp(m_root_widget->begin_update(this));
                luexp(m_root_widget->layout(this, OffsetRectF(0, 0, m_io.width, m_io.height)));
                while(!m_event_queue.empty())
                {
                    ObjRef event = m_event_queue.front();
                    m_event_queue.pop_front();
                    luexp(dispatch_event(event.get()));
                }
                luexp(m_root_widget->update(this));
            }
            lucatchret;
            return ok;
        }
        RV Context::render(IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            return m_root_widget->draw(this, draw_list, overlay_draw_list);
        }
        LUNA_GUI_API Ref<IContext> new_context()
        {
            return new_object<Context>();
        }
    }
}