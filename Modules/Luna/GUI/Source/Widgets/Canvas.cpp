/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Canvas.cpp
* @author JXMaster
* @date 2024/7/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../WidgetBuilder.hpp"
#include "../../Widgets/Canvas.hpp"
#include "../../Attributes.hpp"
#include "../../Context.hpp"
#include "../../LayoutUtils.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV Canvas::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                for(auto& c : children)
                {
                    // Calculate bounding rect.
                    Float4U anthor = get_vattr(c, VATTR_ANTHOR, false, {0, 0, 1, 1});
                    Float4U offset = get_vattr(c, VATTR_OFFSET, false, {0, 0, 0, 0});
                    OffsetRectF bounding_rect = calc_widget_bounding_rect(layout_rect, OffsetRectF{anthor.x, anthor.y, anthor.z, anthor.w}, 
                            OffsetRectF{offset.x, offset.y, offset.z, offset.w});
                    luexp(c->layout(ctx, bounding_rect));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Canvas::begin_update(IContext* ctx)
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
        LUNA_GUI_API RV Canvas::update(IContext* ctx)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Canvas::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
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
        LUNA_GUI_API void Canvas::add_child(IWidget* child)
        {
            children.push_back(Ref<IWidget>(child));
        }
        LUNA_GUI_API void Canvas::get_children(Vector<IWidget*>& out_children)
        {
            out_children.reserve(out_children.size() + children.size());
            for(usize i = 0; i < children.size(); ++i)
            {
                out_children.push_back(children[i]);
            }
        }
        LUNA_GUI_API usize Canvas::get_num_children()
        {
            return children.size();
        }
        LUNA_GUI_API Canvas* begin_canvas(IWidgetBuilder* builder)
        {
            Ref<Canvas> widget = new_object<Canvas>();
            builder->add_widget(widget);
            builder->push_widget(widget);
            return widget;
        }
        LUNA_GUI_API void end_canvas(IWidgetBuilder* builder)
        {
            builder->pop_widget();
        }
    }
}