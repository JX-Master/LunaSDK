/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetBuilder.cpp
* @author JXMaster
* @date 2024/7/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "WidgetBuilder.hpp"

namespace Luna
{
    namespace GUI
    {
        RV RootWidget::update(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->update(ctx, layout_rect));
                }
            }
            lucatchret;
            return ok;
        }
        RV RootWidget::draw(IContext* ctx, IDrawList* draw_list)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->draw(ctx, draw_list));
                }
            }
            lucatchret;
            return ok;
        }
        void WidgetBuilder::add_widget(IWidget *widget)
        {
            m_current_widget = widget;
            m_current_widget->set_id(get_id());
            if(!m_widget_stack.empty())
            {
                m_widget_stack.back()->add_child(m_current_widget);
                m_current_widget->set_parent(m_widget_stack.back().as<IWidget>());
            }
        }
        LUNA_GUI_API Ref<IWidgetBuilder> new_widget_builder()
        {
            return new_object<WidgetBuilder>();
        }
    }
}