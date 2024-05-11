/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Text.hpp
* @author JXMaster
* @date 2024/5/8
*/
#pragma once
#include "../../Widget.hpp"
#include <Luna/VG/TextArranger.hpp>

namespace Luna
{
    namespace GUI
    {
        struct TextBuildData : WidgetBuildData
        {
            lustruct("GUI::TextBuildData", "{bc4f2b36-a3e6-4eca-aef6-86bf114b2bd1}");

            VG::TextArrangeResult arrange_result;
            Vector<VG::TextArrangeSection> text_arrange_sections;

            virtual RV build(IContext* ctx) override;
            virtual RV render(IContext* ctx, VG::IShapeDrawList* draw_list) override;
        };
    }
}