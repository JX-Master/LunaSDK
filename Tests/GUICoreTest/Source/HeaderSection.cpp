/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file HeaderSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_header(GUICore::IContext* context)
    {
        GUICore::ElementHandle header = context->begin_element(ID_HEADER, Name("Header"));
        context->set_layout_config(header, fixed_layout(1500.0f, 120.0f));
        draw_gradient_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f),
            Float4U(0.04f, 0.12f, 0.16f, 1.0f), Float4U(0.09f, 0.18f, 0.24f, 1.0f),
            Float4U(0.07f, 0.10f, 0.14f, 1.0f), Float4U(0.05f, 0.08f, 0.10f, 1.0f));
        draw_outline(context, RectF(0.0f, 0.0f, 1500.0f, 120.0f), Float4U(0.20f, 0.34f, 0.42f, 1.0f), 1.0f);
        draw_text(context, RectF(28.0f, 24.0f, 900.0f, 42.0f), "GUICore Cheat Sheet", 32.0f,
            Float4U(0.95f, 0.98f, 1.0f, 1.0f));
        draw_text(context, RectF(30.0f, 72.0f, 900.0f, 28.0f),
            "A single fixed-size page built only from GUICore elements, canvas layout, input routing and draw commands.",
            17.0f, Float4U(0.67f, 0.76f, 0.82f, 1.0f));
        draw_text(context, RectF(1120.0f, 28.0f, 340.0f, 30.0f), "Hold middle mouse button and drag to pan.",
            17.0f, Float4U(0.83f, 0.92f, 0.98f, 1.0f), VG::TextAlignment::end);
        draw_text(context, RectF(1120.0f, 62.0f, 340.0f, 30.0f), "The page size is 1580 x 1080 logical units.",
            15.0f, Float4U(0.56f, 0.68f, 0.76f, 1.0f), VG::TextAlignment::end);
        context->end_element();
    }
}
