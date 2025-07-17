/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetBuilder.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once
#include "../WidgetBuilder.hpp"
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct RootWidget : Widget
        {
            lustruct("RHI::RootWidget", "RHI::108d0749-1ad1-4698-9c7a-645e3712f12e");
            
            virtual RV begin_update(IContext* ctx) override;
            virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            virtual RV update(IContext* ctx) override;
            virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
        struct WidgetBuilder : IWidgetBuilder
        {
            lustruct("RHI::WidgetBuilder", "f440e804-d7da-450e-9ebe-ec61a3de1b79");
            luiimpl();

            Vector<widget_id_t> m_id_stack;

            // Widget build context.
            Ref<Widget> m_root_widget;
            Ref<Widget> m_current_widget;
            Vector<Ref<Widget>> m_widget_stack;

            virtual void reset() override
            {
                m_id_stack.clear();
                m_widget_stack.clear();
                m_root_widget = new_object<RootWidget>();
                add_widget(m_root_widget);
                push_widget(m_root_widget);
            }
            WidgetBuilder()
            {
                reset();
            }
            virtual void push_id(const Name& name_id) override
            {
                m_id_stack.push_back(get_id(name_id));
            }
            virtual void push_id(const c8* str_id, usize str_len) override
            {
                m_id_stack.push_back(get_id(str_id, str_len));
            }
            virtual void pop_id() override
            {
                m_id_stack.pop_back();
            }
            //! Generates widget ID based on the current widget ID stack.
            virtual widget_id_t get_id() override
            {
                widget_id_t id = m_id_stack.empty() ? 0 : m_id_stack.back();
                return id;
            }
            virtual widget_id_t get_id(const Name& name_id) override
            {
                name_id_t id = name_id.id();
                return memhash32(&id, sizeof(name_id_t), get_id());
            }
            virtual widget_id_t get_id(const c8* str_id, usize str_len) override
            {
                if(str_len == USIZE_MAX) str_len = strlen(str_id);
                return memhash32(str_id, str_len, get_id());
            }
            virtual Widget* get_root_widget() override
            {
                return m_root_widget;
            }
            virtual Widget* get_current_widget() override
            {
                return m_current_widget;
            }
            virtual void set_current_widget(Widget* widget) override
            {
                m_current_widget = widget;
            }
            virtual void add_widget(Widget* widget) override;
            virtual void push_widget(Widget* widget) override
            {
                m_widget_stack.push_back(widget);
            }
            virtual void pop_widget() override
            {
                m_widget_stack.pop_back();
            }
        };
    }
}