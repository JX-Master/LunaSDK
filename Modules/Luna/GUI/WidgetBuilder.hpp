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
#include <Luna/Runtime/Interface.hpp>
#include "Widget.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct IWidgetBuilder : virtual Interface
        {
            luiid("cee0afe1-c196-445f-840f-10de72f8af18");

            virtual void reset() = 0;

            // Widget ID APIs.

            virtual void push_id(const Name& name_id) = 0;
            virtual void push_id(const c8* str_id, usize str_len) = 0;
            virtual void push_id(const void* ptr_id) = 0;
            virtual void push_id(i32 int_id) = 0;
            virtual void pop_id() = 0;

            //! Generates widget ID based on the current widget ID stack.
            virtual widget_id_t get_id() = 0;
            virtual widget_id_t get_id(const Name& name_id) = 0;
            virtual widget_id_t get_id(const c8* str_id, usize str_len) = 0;
            virtual widget_id_t get_id(const void* ptr_id) = 0;
            virtual widget_id_t get_id(i32 int_id) = 0;

            virtual IWidget* get_root_widget() = 0;

            virtual IWidget* get_current_widget() = 0;

            virtual void add_widget(IWidget* widget) = 0;

            //! Pushes one widget to the widget stack so that new widgets will be created as child widgets of this widget.
            virtual void push_widget(IWidget* widget) = 0;

            //! Pops one widget from the widget stack.
            virtual void pop_widget() = 0;
        };

        LUNA_GUI_API Ref<IWidgetBuilder> new_widget_builder();
    }
}