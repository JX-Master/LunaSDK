/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetList.hpp
* @author JXMaster
* @date 2024/3/30
*/
#pragma once
#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif
#include "Widget.hpp"
#include <Luna/Runtime/Interface.hpp>

namespace Luna
{
    namespace GUI
    {
        struct IWidgetList : virtual Interface
        {
            luiid("{c54b4dbd-4e59-452b-939a-07c820f79c05}");

            virtual void reset() = 0;

            virtual Widget* get_current_widget() = 0;
            virtual void set_current_widget(Widget* widget) = 0;

            virtual const Vector<Ref<Widget>> get_widget_stack() = 0;
            virtual void push_widget(Widget* widget) = 0;
            virtual void pop_widget(u32 pop_count = 1) = 0;

            virtual void add_widget(Widget* widget) = 0;
        };

        LUNA_GUI_API Ref<IWidgetList> new_widget_list();
    }
}