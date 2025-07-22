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
#include "../Theme.hpp"

namespace Luna
{
    namespace GUI
    {
        // This is a special widget that only used as the root widget of one widget tree.
        struct RootWidget : Widget
        {
            lustruct("RHI::RootWidget", "108d0749-1ad1-4698-9c7a-645e3712f12e");
            
            virtual RV begin_update(IContext* ctx) override;
            virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            virtual RV update(IContext* ctx) override;
            virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
        struct WidgetBuilder : IWidgetBuilder
        {
            lustruct("RHI::WidgetBuilder", "f440e804-d7da-450e-9ebe-ec61a3de1b79");
            luiimpl();

            Vector<Ref<ITheme>> m_themes;

            // Widget build context.
            Ref<Widget> m_root_widget;
            Ref<Widget> m_current_widget;
            Vector<Ref<Widget>> m_widget_stack;

            Widget* new_widget_internal(const Guid& guid, bool push);

            virtual void reset() override
            {
                m_widget_stack.clear();
                m_root_widget = new_object<RootWidget>();
                m_current_widget = m_root_widget;
                m_widget_stack.push_back(m_root_widget);
            }
            WidgetBuilder()
            {
                reset();
            }
            virtual void push_theme(ITheme* theme) override
            {
                m_themes.push_back(theme);
            }
            virtual void pop_theme() override
            {
                m_themes.pop_back();
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
            virtual Widget* new_widget(const Guid& widget_guid) override;
            virtual Widget* begin_widget(const Guid& widget_guid) override;
            virtual void end_widget() override;
        };
    }
}