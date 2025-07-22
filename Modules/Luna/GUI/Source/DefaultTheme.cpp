/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DefaultTheme.cpp
* @author JXMaster
* @date 2025/7/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Theme.hpp"

#include "../Widgets/Canvas.hpp"
#include "../Widgets/Rectangle.hpp"
#include "../Widgets/Text.hpp"
#include "../Widgets/Button.hpp"
#include "../Widgets/Slider.hpp"
#include "../Widgets/HorizontalLayout.hpp"
#include "../Widgets/VerticalLayout.hpp"
#include "../Widgets/Spacer.hpp"
#include "../Widgets/Dockspace.hpp"

namespace Luna
{
    namespace GUI
    {
        template <typename _Ty>
        Ref<Widget> on_new_widget()
        {
            return new_object<_Ty>();
        }

        template <typename _Ty>
        void register_theme_widget(ITheme* theme)
        {
            WidgetBuildRule rule;
            rule.on_new_widget = on_new_widget<_Ty>;
            theme->set_widget_build_rule(_Ty::__guid, rule);
        }

        LUNA_GUI_API Ref<ITheme> new_default_theme()
        {
            Ref<ITheme> theme = new_theme();
            register_theme_widget<Canvas>(theme);
            register_theme_widget<Rectangle>(theme);
            register_theme_widget<Text>(theme);
            register_theme_widget<Button>(theme);
            register_theme_widget<Slider>(theme);
            register_theme_widget<HorizontalLayout>(theme);
            register_theme_widget<VerticalLayout>(theme);
            register_theme_widget<Spacer>(theme);
            register_theme_widget<Dockspace>(theme);
            return theme;
        }
    }
}