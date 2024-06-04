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
#include "../Widgets.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/Shapes.hpp>

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API OffsetRectF calc_widget_bounding_rect(const OffsetRectF& parent_bounding_rect, const OffsetRectF& anthor, const OffsetRectF& offset)
        {
            // Calculate anthor point.
            f32 ax1 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.left);
            f32 ax2 = lerp(parent_bounding_rect.left, parent_bounding_rect.right, anthor.right);
            f32 ay1 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.top);
            f32 ay2 = lerp(parent_bounding_rect.top, parent_bounding_rect.bottom, anthor.bottom);
            // Calculate size.
            OffsetRectF r;
            r.left = ax1 + offset.left;
            r.right = ax2 + offset.right;
            r.top = ay1 + offset.top;
            r.bottom = ay2 + offset.bottom;
            return r;
        }
        void Context::begin_frame()
        {
            m_id_stack.clear();
            m_widget_stack.clear();
            Ref<Widget> root_widget = new_object<Widget>();
            add_widget(root_widget);
            push_widget(root_widget);
            m_root_widget = root_widget;
        }
        void Context::end_frame()
        {
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
        }
        void Context::add_widget(Widget *widget)
        {
            m_current_widget = widget;
            if(!m_widget_stack.empty())
            {
                m_widget_stack.back()->children.push_back(m_current_widget);
                m_current_widget->parent = m_widget_stack.back();
            }
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
        RV Context::update()
        {
            return m_root_widget->update(this);
        }
        RV Context::render(VG::IShapeDrawList* draw_list)
        {
            return m_root_widget->render(this, draw_list);
        }
        LUNA_GUI_API Ref<IContext> new_context()
        {
            return new_object<Context>();
        }
    }
}