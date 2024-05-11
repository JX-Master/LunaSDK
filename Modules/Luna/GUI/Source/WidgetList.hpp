/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetList.hpp
* @author JXMaster
* @date 2024/3/29
*/
#pragma once
#include "../WidgetList.hpp"
namespace Luna
{
    namespace GUI
    {
        struct WidgetList : IWidgetList
        {
            lustruct("GUI::WidgetList", "{978cad33-41b8-4d26-b450-3829fd30c55b}");
            luiimpl();

            Ref<Widget> m_current_widget;
            Vector<Ref<Widget>> m_widget_stack;

            virtual void reset() override
            {
                m_current_widget.reset();
                m_widget_stack.clear();
            }
            virtual Widget* get_current_widget() override
            {
                return m_current_widget;
            }
            virtual void set_current_widget(Widget* widget) override
            {
                m_current_widget = widget;
            }
            virtual const Vector<Ref<Widget>> get_widget_stack() override
            {
                return m_widget_stack;
            }
            virtual void push_widget(Widget* widget) override
            {
                m_widget_stack.push_back(widget);
            }
            virtual void pop_widget(u32 pop_count) override
            {
                while(pop_count)
                {
                    m_widget_stack.pop_back();
                    --pop_count;
                }
            }
            virtual void add_widget(Widget* widget) override
            {
                m_current_widget = widget;
                if(!m_widget_stack.empty())
                {
                    m_widget_stack.back()->children.push_back(m_current_widget);
                    m_current_widget->parent = m_widget_stack.back();
                }
            }
        };
    }
}