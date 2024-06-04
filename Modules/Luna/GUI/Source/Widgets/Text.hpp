/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Text.hpp
* @author JXMaster
* @date 2024/5/21
*/
#pragma once
#include "../../Widget.hpp"
#include <Luna/VG/TextArranger.hpp>

namespace Luna
{
    namespace GUI
    {
        struct Text : Widget
        {
            lustruct("GUI::Text", "{06fa0393-d4f9-4dce-b340-d9790b36c732}");

            VG::TextArrangeResult arrange_result;
            Vector<VG::TextArrangeSection> text_arrange_sections;

            virtual RV update(IContext* ctx) override;
            virtual RV render(IContext* ctx, VG::IShapeDrawList* draw_list) override;
        };
    }
}