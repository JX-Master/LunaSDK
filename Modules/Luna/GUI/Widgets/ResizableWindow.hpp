/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResizableWindow.hpp
* @author JXMaster
* @date 2024/5/21
*/
#pragma once
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct ResizableWindow : Widget
        {
            lustruct("GUI::ResizableWindow", "{22637fc6-330d-46e9-ad80-16d2ed7ec328}");

            LUNA_GUI_API virtual Ref<WidgetBuildData> new_build_data() override;
        };
    }
}