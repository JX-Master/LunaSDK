/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VerticalLayout.hpp
* @author JXMaster
* @date 2024/7/13
*/
#pragma once
#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class HorizontalAlignment : u8
        {
            begin,
            center,
            end
        };

        // struct VerticalLayout : Widget
        // {
        //     lustruct("GUI::VerticalLayout", "04aaf290-ad94-4f27-9218-f25c27dd9464");

        //     HorizontalAlignment horizontal_alignment = HorizontalAlignment::begin;

            
        // };

        // LUNA_GUI_API VerticalLayout* begin_vertical_layout(IContext* ctx);

        // LUNA_GUI_API void end_vertical_layout(IContext* ctx);
    }
}