/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetList.cpp
* @author JXMaster
* @date 2024/3/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "WidgetList.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API Ref<IWidgetList> new_widget_list()
        {
            return new_object<WidgetList>();
        }
    }
}