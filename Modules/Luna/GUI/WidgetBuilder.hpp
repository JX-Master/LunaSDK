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
        struct ITheme;

        struct IWidgetBuilder : virtual Interface
        {
            luiid("cee0afe1-c196-445f-840f-10de72f8af18");

            virtual void reset() = 0;

            // Theme APIs.

            virtual void push_theme(ITheme* theme) = 0;
            virtual void pop_theme() = 0;

            virtual Widget* get_root_widget() = 0;

            virtual Widget* get_current_widget() = 0;

            virtual void set_current_widget(Widget* widget) = 0;

            virtual Widget* new_widget(const Guid& widget_guid) = 0;

            template <typename _Ty>
            _Ty* new_widget()
            {
                return cast_object<_Ty>(new_widget(_Ty::__guid));
            }

            virtual Widget* begin_widget(const Guid& widget_guid) = 0;

            template <typename _Ty>
            _Ty* begin_widget()
            {
                return cast_object<_Ty>(begin_widget(_Ty::__guid));
            }

            virtual void end_widget() = 0;
        };

        LUNA_GUI_API Ref<IWidgetBuilder> new_widget_builder();
    }
}