/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.cpp
* @author JXMaster
* @date 2024/8/14
*/
#include "../Event.hpp"
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV dispatch_event_by_pos(IContext* ctx, Widget* widget, object_t e, f32 x, f32 y, bool& handled)
        {
            lutry
            {
                if (!widget->contains_point(x, y)) return ok;
                // Walk the widget tree to find one widget that is suitable for this event.
                Vector<Widget*> widgets;
                Vector<Widget*> container_stack;
                container_stack.push_back(widget);
                while(!container_stack.empty())
                {
                    Widget* container = container_stack.back();
                    container_stack.pop_back();
                    widgets.push_back(container);
                    auto children = container->get_children();
                    for(auto& c : children)
                    {
                        if(c->contains_point(x, y))
                        {
                            container_stack.push_back(c);
                        }
                    }
                }
                // Dispatch event to all widgets.
                for(auto iter = widgets.begin(); iter != widgets.end(); ++iter)
                {
                    Widget* w = *iter;
                    luexp(w->handle_event(ctx, e, handled));
                    if(handled) return ok;
                }
            }
            lucatchret;
            return ok;
        }
    }
}