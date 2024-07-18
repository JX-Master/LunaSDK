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
#include "../Widgets/Container.hpp"
#include "../Widgets/Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct RootWidget : Widget, virtual IContainer
        {
            lustruct("RHI::RootWidget", "RHI::108d0749-1ad1-4698-9c7a-645e3712f12e");
            luiimpl();

            Vector<Ref<IWidget>> children;

            virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) override;
            virtual RV draw(IContext* ctx, IDrawList* draw_list) override;
            virtual void add_child(IWidget* child) override
            {
                children.push_back(child);
            }
            virtual Array<IWidget*> get_children() override
            {
                Array<IWidget*> ret(children.size());
                for(usize i = 0; i < children.size(); ++i)
                {
                    ret[i] = children[i];
                }
                return ret;
            }
        };
        struct WidgetBuilder : IWidgetBuilder
        {
            lustruct("RHI::WidgetBuilder", "f440e804-d7da-450e-9ebe-ec61a3de1b79");
            luiimpl();

            Vector<widget_id_t> m_id_stack;

            // Widget build context.
            Ref<IWidget> m_root_widget;
            Ref<IWidget> m_current_widget;
            Vector<Ref<IContainer>> m_widget_stack;

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
            virtual void push_id(const void* ptr_id) override
            {
                m_id_stack.push_back(get_id(ptr_id));
            }
            virtual void push_id(i32 int_id) override
            {
                m_id_stack.push_back(get_id(int_id));
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
            virtual widget_id_t get_id(const void* ptr_id) override
            {
                return memhash32(&ptr_id, sizeof(void*), get_id());
            }
            virtual widget_id_t get_id(i32 int_id) override
            {
                return memhash32(&int_id, sizeof(i32), get_id());
            }
            virtual IWidget* get_root_widget() override
            {
                return m_root_widget;
            }
            virtual IWidget* get_current_widget() override
            {
                return m_current_widget;
            }
            virtual void add_widget(IWidget* widget) override;
            virtual void push_widget(IWidget* widget) override
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