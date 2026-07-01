/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DrawSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_draw_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_DRAW, "Draw Commands", 470.0f, 350.0f);
        draw_text(context, RectF(20.0f, 58.0f, 420.0f, 26.0f), "Primitive commands are recorded on elements, then compiled to VG.",
            16.0f, Float4U(0.82f, 0.88f, 0.94f, 1.0f));
        draw_rect(context, RectF(28.0f, 108.0f, 110.0f, 70.0f), Float4U(0.08f, 0.46f, 0.78f, 1.0f), 8.0f);
        draw_gradient_rect(context, RectF(164.0f, 108.0f, 110.0f, 70.0f),
            Float4U(0.20f, 0.62f, 0.92f, 1.0f), Float4U(0.18f, 0.86f, 0.66f, 1.0f),
            Float4U(0.08f, 0.18f, 0.28f, 1.0f), Float4U(0.05f, 0.10f, 0.15f, 1.0f));
        draw_line(context, Float2U(310.0f, 166.0f), Float2U(430.0f, 112.0f), Float4U(0.88f, 0.74f, 0.22f, 1.0f), 5.0f);
        draw_text(context, RectF(28.0f, 210.0f, 390.0f, 26.0f), "rect / gradient_rect / rounded_rect / line / text / image / shape / clip",
            15.0f, Float4U(0.72f, 0.80f, 0.86f, 1.0f));
        draw_rect(context, RectF(28.0f, 260.0f, 410.0f, 44.0f), Float4U(0.02f, 0.07f, 0.09f, 1.0f), 4.0f);
        draw_text(context, RectF(40.0f, 272.0f, 386.0f, 22.0f), "Command rectangles can be layer-space or element-relative.",
            15.0f, Float4U(0.86f, 0.93f, 0.98f, 1.0f));
        end_panel(context);
    }
}
