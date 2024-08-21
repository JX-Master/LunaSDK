/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HorizontalLayout.cpp
* @author JXMaster
* @date 2024/7/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Widgets/HorizontalLayout.hpp"
#include "../../Layout.hpp"
#include "../../WidgetBuilder.hpp"
#include "../../LayoutUtils.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 HorizontalLayout::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            bool found = false;
            f32 ret = get_desired_size_x_attr(this, type, &found);
            if(found) return ret;
            if(type == DesiredSizeType::required || type == DesiredSizeType::preferred)
            {
                for(auto& c : children)
                {
                    ret += c->get_desired_size_x(type, suggested_size_y);
                }
            }
            else
            {
                ret = get_sattr(this, SATTR_FILLING_SIZE_X, false, 0.0f);
            }
            return ret;
        }
        LUNA_GUI_API f32 HorizontalLayout::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            bool found = false;
            f32 ret = get_desired_size_y_attr(this, type, &found);
            if(found) return ret;
            if(type == DesiredSizeType::required || type == DesiredSizeType::preferred)
            {
                if(suggested_size_x)
                {
                    // try to layout using the suggested size.
                    Array<f32> children_size(children.size() * 4);
                    Array<IWidget*> children_widgets(children.size());
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        children_widgets[i] = children[i];
                    }
                    calc_hlayout(children_widgets.data(), children_widgets.size(), *suggested_size_x, nullptr, 
                        children_size.data(), children_size.data() + children.size(),
                        children_size.data() + children.size() * 2, children_size.data() + children.size() * 3);
                    f32* allocated_size = children_size.data();
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        ret = max(children[i]->get_desired_size_y(type, allocated_size + i), ret);
                    }
                }
                else
                {
                    for(auto& c : children)
                    {
                        ret = max(c->get_desired_size_y(type, nullptr), ret);
                    }
                }
            }
            else
            {
                ret = get_sattr(this, SATTR_FILLING_SIZE_Y, false, 0.0f);
            }
            return ret;
        }
        LUNA_GUI_API RV HorizontalLayout::begin_update(IContext* ctx)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->begin_update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV HorizontalLayout::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                f32 total_size = layout_rect.right - layout_rect.left;
                f32 total_size_other = layout_rect.bottom - layout_rect.top;
                Array<f32> children_size(children.size() * 4);
                Array<IWidget*> children_widgets(children.size());
                for(usize i = 0; i < children.size(); ++i)
                {
                    children_widgets[i] = children[i];
                }
                calc_hlayout(children_widgets.data(), children_widgets.size(), total_size, &total_size_other, 
                        children_size.data(), children_size.data() + children.size(),
                        children_size.data() + children.size() * 2, children_size.data() + children.size() * 3);
                f32* allocated_size = children_size.data();
                // Update children.
                f32 current_offset = layout_rect.left;
                for(usize i = 0; i < children.size(); ++i)
                {
                    OffsetRectF rect(current_offset, layout_rect.top, current_offset + allocated_size[i], layout_rect.bottom);
                    luexp(children[i]->layout(ctx, rect));
                    current_offset += allocated_size[i];
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV HorizontalLayout::update(IContext* ctx)
        {
            lutry
            {
                for(usize i = 0; i < children.size(); ++i)
                {
                    luexp(children[i]->update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV HorizontalLayout::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->draw(ctx, draw_list, overlay_draw_list));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API void HorizontalLayout::add_child(IWidget* child)
        {
            children.push_back(child);
        }
        LUNA_GUI_API void HorizontalLayout::get_children(Vector<IWidget*>& out_children)
        {
            out_children.insert(out_children.end(), children.begin(), children.end());
        }
        LUNA_GUI_API usize HorizontalLayout::get_num_children()
        {
            return children.size();
        }
        LUNA_GUI_API HorizontalLayout* begin_hlayout(IWidgetBuilder* builder)
        {
            Ref<HorizontalLayout> widget = new_object<HorizontalLayout>();
            builder->add_widget(widget);
            builder->push_widget(widget);
            return widget;
        }
        LUNA_GUI_API void end_hlayout(IWidgetBuilder* builder)
        {
            builder->pop_widget();
        }
    }
}