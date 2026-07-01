/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file LayoutSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_layout_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_LAYOUT, "Layout Algorithms", 720.0f, 300.0f);
        draw_text(context, RectF(22.0f, 60.0f, 660.0f, 26.0f),
            "This page uses CanvasLayout twice: screen -> fixed sheet, sheet -> fixed sections.",
            16.0f, Float4U(0.82f, 0.88f, 0.94f, 1.0f));
        const c8* names[] = { "Canvas", "Flex", "Grid", "Stack", "Table", "ScrollViewport" };
        for(u32 i = 0; i < 6; ++i)
        {
            f32 x = 24.0f + (f32)(i % 3) * 220.0f;
            f32 y = 104.0f + (f32)(i / 3) * 66.0f;
            draw_rect(context, RectF(x, y, 188.0f, 44.0f), Float4U(0.07f, 0.20f, 0.28f, 1.0f), 6.0f);
            draw_outline(context, RectF(x, y, 188.0f, 44.0f), Float4U(0.18f, 0.40f, 0.50f, 1.0f), 1.0f);
            draw_text(context, RectF(x + 12.0f, y + 12.0f, 164.0f, 22.0f), names[i], 16.0f,
                Float4U(0.86f, 0.93f, 0.98f, 1.0f), VG::TextAlignment::center);
        }
        bullet(context, 24.0f, 242.0f, "Layout is a context pass: apply_layout(root, rect), then route_input().");
        end_panel(context);
    }
}
