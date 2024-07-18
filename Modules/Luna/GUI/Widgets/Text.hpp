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

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        struct Text : Widget
        {
            lustruct("GUI::Text", "{06fa0393-d4f9-4dce-b340-d9790b36c732}");

            Name text;
            OffsetRectF bounding_rect;

            LUNA_GUI_API virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list) override;
        };
    }
}