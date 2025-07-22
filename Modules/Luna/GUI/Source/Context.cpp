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
#include "../Widget.hpp"
#include "../Event.hpp"

namespace Luna
{
    namespace GUI
    {
        static void compute_widget_hash(Widget* widget, widget_hash_t base, u32 child_index)
        {
            if(widget->get_hash() == 0)
            {
                if(widget->is_global_id())
                {
                    base = 0;
                }
                Name id = widget->get_id();
                widget_hash_t h;
                if(!id)
                {
                    String hash_str;
                    strprintf(hash_str, "%s%u", get_type_name(get_object_type(widget)).c_str(), child_index);
                    h = memhash<widget_hash_t>(hash_str.c_str(), hash_str.size(), base);
                }
                else
                {
                    h = memhash<widget_hash_t>(id.c_str(), id.size(), base);
                }
                widget->set_hash(h);
            }
            auto h = widget->get_hash();
            auto children = widget->get_children();
            for(usize i = 0; i < children.size(); ++i)
            {
                compute_widget_hash(children[i].get(), h, (u32)i);
            }
        }
        void Context::set_widget(Widget* root_widget)
        {
            m_root_widget = root_widget;
            // Update widget hash value.
            compute_widget_hash(m_root_widget, 0, 0);
        }
        object_t Context::get_widget_state(widget_hash_t hash)
        {
            auto iter = m_widget_state_reg.find(hash);
            return iter == m_widget_state_reg.end() ? nullptr : iter->second.state.get();
        }
        void Context::set_widget_state(widget_hash_t hash, object_t state, WidgetStateLifetime lifetime)
        {
            m_widget_state_reg.insert_or_assign(hash, WidgetStateEntry(ObjRef(state), lifetime));
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