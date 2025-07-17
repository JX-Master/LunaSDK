/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Canvas.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once

#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Canvas : Widget
        {
            lustruct("GUI::Canvas", "15c6b337-07b0-4760-91cd-cfaeeb398f37");

            LUNA_GUI_API virtual RV begin_update(IContext* ctx) override;
            LUNA_GUI_API virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override;
            LUNA_GUI_API virtual RV update(IContext* ctx) override;
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
    }
}