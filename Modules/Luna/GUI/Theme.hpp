/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Theme.hpp
* @author JXMaster
* @date 2025/6/19
*/
#pragma once
#include "Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct WidgetBuildRule
        {
            //! Called when one new widget is required.
            Ref<Widget> (*on_new_widget)();
        };

        //! Describes how to generate widget tree for a widget builder.
        struct ITheme : virtual Interface
        {
            luiid("8dcd66be-d249-41a0-9dac-0180b0ca6436");

            virtual ITheme* get_parent() = 0;

            virtual void set_parent(ITheme* parent) = 0;

            virtual void set_widget_build_rule(const Guid& widget_guid, const WidgetBuildRule& rule) = 0;

            virtual void reset_widget_build_rule(const Guid& widget_guid) = 0;

            virtual Ref<Widget> new_widget(const Guid& widget_guid) = 0;
        };

        //! Creates a new theme.
        LUNA_GUI_API Ref<ITheme> new_theme();

        //! Creates one default theme.
        LUNA_GUI_API Ref<ITheme> new_default_theme();
    }
}