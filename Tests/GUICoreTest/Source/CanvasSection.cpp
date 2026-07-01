/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CanvasSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_canvas_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_CANVAS, "Canvas Placement", 470.0f, 260.0f);
        draw_rect(context, RectF(28.0f, 68.0f, 390.0f, 140.0f), Float4U(0.03f, 0.08f, 0.10f, 1.0f), 4.0f);
        draw_outline(context, RectF(28.0f, 68.0f, 390.0f, 140.0f), Float4U(0.20f, 0.34f, 0.42f, 1.0f), 1.0f);
        draw_rect(context, RectF(52.0f, 92.0f, 96.0f, 42.0f), Float4U(0.08f, 0.28f, 0.42f, 1.0f), 6.0f);
        draw_rect(context, RectF(246.0f, 142.0f, 136.0f, 42.0f), Float4U(0.12f, 0.38f, 0.34f, 1.0f), 6.0f);
        draw_line(context, Float2U(28.0f, 68.0f), Float2U(52.0f, 92.0f), Float4U(0.76f, 0.58f, 0.18f, 1.0f), 2.0f);
        draw_line(context, Float2U(418.0f, 208.0f), Float2U(382.0f, 184.0f), Float4U(0.76f, 0.58f, 0.18f, 1.0f), 2.0f);
        draw_text(context, RectF(28.0f, 222.0f, 410.0f, 24.0f), "Anchors + offsets place children in parent content space.",
            15.0f, Float4U(0.78f, 0.85f, 0.90f, 1.0f));
        end_panel(context);
    }
}
