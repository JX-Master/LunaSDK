/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Theme.cpp
* @author JXMaster
* @date 2025/6/19
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Theme.hpp"

namespace Luna
{
    namespace GUI
    {
        void Theme::set_widget_build_rule(const Guid& widget_guid, const WidgetBuildRule& rule)
        {
            m_widget_build_rules.insert_or_assign(widget_guid, rule);
        }

        void Theme::reset_widget_build_rule(const Guid& widget_guid)
        {
            m_widget_build_rules.erase(widget_guid);
        }

        Ref<Widget> Theme::new_widget(const Guid& widget_guid)
        {
            auto iter = m_widget_build_rules.find(widget_guid);
            Ref<Widget> ret;
            if(iter != m_widget_build_rules.end())
            {
                ret = iter->second.on_new_widget();
            }
            if(!ret && m_parent)
            {
                ret = m_parent->new_widget(widget_guid);
            }
            return ret;
        }
        LUNA_GUI_API Ref<ITheme> new_theme()
        {
            return new_object<Theme>();
        }
    }
}