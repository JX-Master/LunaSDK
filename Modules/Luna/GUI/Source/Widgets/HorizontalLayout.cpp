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

namespace Luna
{
    namespace GUI
    {
        static Array<f32> compute_hlayout(const Vector<Ref<IWidget>>& children, f32 total_size, const f32* total_size_other)
        {
            // Arrange in X.
            Array<f32> children_size(children.size() * 4, 0);
            f32* allocated_size = children_size.data();
            f32* required_size = children_size.data() + children.size();
            f32* preferred_size = children_size.data() + children.size() * 2;
            f32* filling_size = children_size.data() + children.size() * 3;
            f32 allocated = 0;
            // Allocate required size.
            for(usize i = 0; i < children.size(); ++i)
            {
                required_size[i] = children[i]->get_desired_size_x(DesiredSizeType::required, total_size_other);
                allocated_size[i] = required_size[i];
                allocated += required_size[i];
            }
            // Allocate preferred size.
            if(total_size > allocated)
            {
                f32 preferred_size_sum = 0;
                for(usize i = 0; i < children.size(); ++i)
                {
                    preferred_size[i] = max(children[i]->get_desired_size_x(DesiredSizeType::preferred, total_size_other), required_size[i]);
                    preferred_size_sum += preferred_size[i];
                }
                if(preferred_size_sum <= total_size)
                {
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        allocated_size[i] = preferred_size[i];
                    }
                    allocated = preferred_size_sum;
                }
                else
                {
                    f32 ratio = total_size / preferred_size_sum;
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        allocated_size[i] = preferred_size[i] * ratio;
                    }
                    allocated = total_size;
                }
            }
            // Allocate filling size.
            if(total_size > allocated)
            {
                f32 total_filling_size = total_size - allocated;
                f32 filling_size_weight = 0;
                for(usize i = 0; i < children.size(); ++i)
                {
                    filling_size[i] = children[i]->get_desired_size_x(DesiredSizeType::filling, total_size_other);
                    filling_size_weight += filling_size[i];
                }
                if(filling_size_weight > 0)
                {
                    f32 filling_size_per_unit = total_filling_size / filling_size_weight;
                    for(usize i = 0; i < children.size(); ++i)
                    {
                        allocated_size[i] += filling_size_per_unit * filling_size[i];
                    }
                    allocated = total_size;
                }
            }
            return children_size;
        }
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
                    Array<f32> children_size = compute_hlayout(children, *suggested_size_x, nullptr);
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
                Array<f32> children_size = compute_hlayout(children, total_size, &total_size_other);
                f32* allocated_size = children_size.data();
                // Update children.
                f32 current_offset = 0;
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
        LUNA_GUI_API RV HorizontalLayout::draw(IContext* ctx, IDrawList* draw_list)
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