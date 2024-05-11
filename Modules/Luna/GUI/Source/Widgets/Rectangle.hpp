/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Rectangle.hpp
* @author JXMaster
* @date 2024/5/8
*/
#pragma once
#include "../../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct RectangleBuildData : WidgetBuildData
        {
            lustruct("GUI::RectangleBuildData", "{7faae92b-28c6-4f00-ad71-5e12ef82d3f3}");

            virtual RV render(IContext* ctx, VG::IShapeDrawList* draw_list) override;
        };
    }
}