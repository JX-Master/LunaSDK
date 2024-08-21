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
#include "../Widgets/Container.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV dispatch_event_by_pos(IContext* ctx, IWidget* widget, object_t e, f32 x, f32 y, bool& handled)
        {
            lutry
            {
                if (!widget->contains_point(x, y)) return ok;
                // Walk the widget tree to find one widget that is suitable for this event.
                Vector<IWidget*> widgets;
                Vector<IContainer*> container_stack;
                container_stack.push_back(query_interface<IContainer>(widget->get_object()));
                Vector<IWidget*> children;
                while(!container_stack.empty())
                {
                    children.clear();
                    IContainer* container = container_stack.back();
                    container_stack.pop_back();
                    widgets.push_back(container);
                    container->get_children(children);
                    for(auto iter = children.begin(); iter != children.end(); ++iter)
                    {
                        IWidget* c = *iter;
                        if(c->contains_point(x, y))
                        {
                            widgets.push_back(c);
                            IContainer* child_container = query_interface<IContainer>(c->get_object());
                            if(child_container)
                            {
                                container_stack.push_back(child_container);
                            }
                        }
                    }
                }
                // Dispatch event to all widgets.
                for(auto iter = widgets.begin(); iter != widgets.end(); ++iter)
                {
                    IWidget* w = *iter;
                    luexp(w->handle_event(ctx, e, handled));
                    if(handled) return ok;
                }
            }
            lucatchret;
            return ok;
        }
    }
}