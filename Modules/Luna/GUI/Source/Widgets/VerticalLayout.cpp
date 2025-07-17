/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VerticalLayout.cpp
* @author JXMaster
* @date 2024/7/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Widgets/VerticalLayout.hpp"
#include "../../Layout.hpp"
#include "../../WidgetBuilder.hpp"
#include "../../LayoutUtils.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 VerticalLayout::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            bool found = false;
            f32 ret = get_desired_size_x_attr(this, type, &found);
            if(found) return ret;
            if(type == DesiredSizeType::required || type == DesiredSizeType::preferred)
            {
                if(suggested_size_y)
                {
                    auto children = get_children();
                    // try to layout using the suggested size.
                    Array<f32> children_size(children.size() * 4);
                    Array<Widget*> children_widgets(children.size());
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        children_widgets[i] = children[i];
                    }
                    calc_vlayout(children_widgets.data(), children_widgets.size(), *suggested_size_y, nullptr,
                        children_size.data(), children_size.data() + children.size(),
                        children_size.data() + children.size() * 2, children_size.data() + children.size() * 3);
                    f32* allocated_size = children_size.data();
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        ret = max(children[i]->get_desired_size_x(type, allocated_size + i), ret);
                    }
                }
                else
                {
                    for(auto& c : get_children())
                    {
                        ret = max(c->get_desired_size_x(type, nullptr), ret);
                    }
                }
            }
            return ret;
        }
        LUNA_GUI_API f32 VerticalLayout::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            bool found = false;
            f32 ret = get_desired_size_y_attr(this, type, &found);
            if(found) return ret;
            if(type == DesiredSizeType::required || type == DesiredSizeType::preferred)
            {
                for(auto& c : get_children())
                {
                    ret += c->get_desired_size_y(type, suggested_size_x);
                }
            }
            return ret;
        }
        LUNA_GUI_API RV VerticalLayout::begin_update(IContext* ctx)
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
        LUNA_GUI_API RV VerticalLayout::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                f32 total_size = layout_rect.bottom - layout_rect.top;
                f32 total_size_other = layout_rect.right - layout_rect.left;
                auto children = get_children();
                Array<f32> children_size(children.size() * 4);
                Array<Widget*> children_widgets(children.size());
                for(usize i = 0; i < children.size(); ++i)
                {
                    children_widgets[i] = children[i];
                }
                calc_vlayout(children_widgets.data(), children_widgets.size(), total_size, &total_size_other,
                    children_size.data(), children_size.data() + children.size(),
                    children_size.data() + children.size() * 2, children_size.data() + children.size() * 3);
                f32* allocated_size = children_size.data();
                // Update children.
                f32 current_offset = layout_rect.top;
                for(usize i = 0; i < children.size(); ++i)
                {
                    OffsetRectF rect(layout_rect.left, current_offset, layout_rect.right, current_offset + allocated_size[i]);
                    luexp(children[i]->layout(ctx, rect));
                    current_offset += allocated_size[i];
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV VerticalLayout::update(IContext* ctx)
        {
            lutry
            {
                auto children = get_children();
                for(usize i = 0; i < children.size(); ++i)
                {
                    luexp(children[i]->update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV VerticalLayout::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
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
        LUNA_GUI_API VerticalLayout* begin_vlayout(IWidgetBuilder* builder)
        {
            Ref<VerticalLayout> widget = new_object<VerticalLayout>();
            builder->add_widget(widget);
            builder->push_widget(widget);
            return widget;
        }
        LUNA_GUI_API void end_vlayout(IWidgetBuilder* builder)
        {
            builder->pop_widget();
        }
    }
}