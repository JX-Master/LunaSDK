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
#include "../Theme.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Theme : virtual ITheme
        {
            lustruct("GUI::Theme", "6c5601ed-4422-4f16-9405-b71cd0fc9c7a");
            luiimpl();

            Ref<ITheme> m_parent;
            HashMap<Guid, WidgetBuildRule> m_widget_build_rules;

            virtual ITheme* get_parent() override
            {
                return m_parent;
            }

            virtual void set_parent(ITheme* parent) override
            {
                m_parent = parent;
            }

            virtual void set_widget_build_rule(const Guid& widget_guid, const WidgetBuildRule& rule) override;

            virtual void reset_widget_build_rule(const Guid& widget_guid) override;

            virtual Ref<Widget> new_widget(const Guid& widget_guid) override;
        };
    }
}