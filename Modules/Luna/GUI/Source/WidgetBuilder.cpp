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
#include "../Theme.hpp"

namespace Luna
{
    namespace GUI
    {
        RV RootWidget::begin_update(IContext* ctx)
        {
            lutry
            {
                for(auto& c : get_children())
                {
                    luexp(c->begin_update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        RV RootWidget::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                for(auto& c : get_children())
                {
                    luexp(c->layout(ctx, layout_rect));
                }
            }
            lucatchret;
            return ok;
        }
        RV RootWidget::update(IContext* ctx)
        {
            lutry
            {
                for(auto& c : get_children())
                {
                    luexp(c->update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        RV RootWidget::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            lutry
            {
                for(auto& c : get_children())
                {
                    luexp(c->draw(ctx, draw_list, overlay_draw_list));
                }
            }
            lucatchret;
            return ok;
        }
        Widget* WidgetBuilder::new_widget_internal(const Guid& guid, bool push)
        {
            Ref<Widget> ret;
            for(auto iter = m_themes.rbegin(); iter != m_themes.rend(); ++iter)
            {
                ret = (*iter)->new_widget(guid);
                if(ret) break;
            }
            lucheck_msg(ret, "Widget GUID is not supported by any theme in the current theme stack.");
            m_current_widget = ret;
            if(!m_widget_stack.empty())
            {
                m_widget_stack.back()->add_child(m_current_widget);
            }
            if(push)
            {
                m_widget_stack.push_back(m_current_widget);
            }
            return ret.get();
        }
        Widget* WidgetBuilder::new_widget(const Guid& widget_guid)
        {
            return new_widget_internal(widget_guid, false);
        }
        Widget* WidgetBuilder::begin_widget(const Guid& widget_guid)
        {
            return new_widget_internal(widget_guid, true);
        }
        void WidgetBuilder::end_widget()
        {
            m_widget_stack.pop_back();
        }
        LUNA_GUI_API Ref<IWidgetBuilder> new_widget_builder()
        {
            Ref<WidgetBuilder> builder = new_object<WidgetBuilder>();
            Ref<ITheme> theme = new_default_theme();
            builder->push_theme(theme);
            return builder;
        }
    }
}