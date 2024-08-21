/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Spacer.cpp
* @author JXMaster
* @date 2024/7/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Layout.hpp"
#include "../../WidgetBuilder.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API Spacer* spacer(IWidgetBuilder* builder)
        {
            Ref<Spacer> widget = new_object<Spacer>();
            builder->add_widget(widget);
            return widget;
        }
    }
}