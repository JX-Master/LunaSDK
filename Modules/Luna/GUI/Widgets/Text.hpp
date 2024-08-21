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
#include "Widget.hpp"
#include <Luna/VG/TextArranger.hpp>

namespace Luna
{
    namespace GUI
    {
        struct Text : Widget
        {
            lustruct("GUI::Text", "{06fa0393-d4f9-4dce-b340-d9790b36c732}");

            Name text;
            VG::TextAlignment vertical_alignment = VG::TextAlignment::begin;
            VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin;

            LUNA_GUI_API virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) override;
            LUNA_GUI_API virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) override;

            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
    }
}