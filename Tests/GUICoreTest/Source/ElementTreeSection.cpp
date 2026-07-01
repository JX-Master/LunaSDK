/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ElementTreeSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_element_tree_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_ELEMENT_TREE, "Element Tree", 360.0f, 300.0f);
        draw_text(context, RectF(24.0f, 62.0f, 320.0f, 24.0f), "Layer[default]", 16.0f, Float4U(0.90f, 0.95f, 1.0f, 1.0f));
        draw_line(context, Float2U(36.0f, 92.0f), Float2U(36.0f, 238.0f), Float4U(0.25f, 0.36f, 0.44f, 1.0f), 1.0f);
        draw_text(context, RectF(52.0f, 94.0f, 260.0f, 24.0f), "ScreenRoot: canvas", 15.0f, Float4U(0.75f, 0.84f, 0.90f, 1.0f));
        draw_line(context, Float2U(70.0f, 122.0f), Float2U(70.0f, 230.0f), Float4U(0.21f, 0.31f, 0.39f, 1.0f), 1.0f);
        draw_text(context, RectF(88.0f, 128.0f, 260.0f, 24.0f), "Sheet: fixed + canvas", 15.0f, Float4U(0.75f, 0.84f, 0.90f, 1.0f));
        draw_text(context, RectF(112.0f, 164.0f, 220.0f, 24.0f), "Section elements", 15.0f, Float4U(0.66f, 0.76f, 0.84f, 1.0f));
        draw_text(context, RectF(112.0f, 194.0f, 220.0f, 24.0f), "Hit-test samples", 15.0f, Float4U(0.66f, 0.76f, 0.84f, 1.0f));
        bullet(context, 20.0f, 248.0f, "Elements are typeless data; algorithms operate on attached data.");
        end_panel(context);
    }
}
