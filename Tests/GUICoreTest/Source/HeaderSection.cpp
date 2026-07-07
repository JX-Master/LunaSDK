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
#include <cstdio>

namespace Luna::GUICoreTest
{
    void build_slide_header(GUICore::IContext* context, const CoreSheetState& state)
    {
        const c8* title = "GUICore Layout";
        const c8* subtitle = "Layout is data on elements, evaluated by context-level algorithms.";
        switch(state.slice_index)
        {
        case 0:
            title = "Pointer Input";
            subtitle = "Hit testing walks layers and elements until a routing target is found.";
            break;
        case 1:
            title = "Keyboard Input";
            subtitle = "Keyboard state and text input are host events translated into core input.";
            break;
        case 2:
            title = "Navigation Input";
            subtitle = "Navigation events describe intent instead of binding behavior to one device.";
            break;
        default:
            title = layout_slice_title(state.slice_index - NUM_INPUT_SLICES);
            subtitle = layout_slice_subtitle(state.slice_index - NUM_INPUT_SLICES);
            break;
        }

        GUICore::ElementHandle header = context->begin_element(ID_HEADER, Name("Slide Header"));
        context->set_layout_config(header, fixed_layout(SHEET_WIDTH - 96.0f, 108.0f));
        draw_text(context, RectF(0.0f, 0.0f, 760.0f, 56.0f), title, 46.0f, Float4U(0.0f, 0.0f, 0.0f, 1.0f));
        draw_text(context, RectF(2.0f, 60.0f, 980.0f, 34.0f), subtitle, 22.0f, Float4U(0.20f, 0.20f, 0.20f, 1.0f));
        draw_line(context, Float2U(0.0f, 106.0f), Float2U(SHEET_WIDTH - 96.0f, 106.0f),
            Float4U(0.0f, 0.0f, 0.0f, 1.0f), 1.5f);

        char page[64];
        snprintf(page, sizeof(page), "%u / %u", state.slice_index + 1, NUM_SLICES);
        draw_text(context, RectF(SHEET_WIDTH - 300.0f, 8.0f, 200.0f, 34.0f), page, 22.0f,
            Float4U(0.0f, 0.0f, 0.0f, 1.0f), VG::TextAlignment::end);
        draw_text(context, RectF(SHEET_WIDTH - 420.0f, 62.0f, 320.0f, 28.0f), "Z previous   X next", 18.0f,
            Float4U(0.30f, 0.30f, 0.30f, 1.0f), VG::TextAlignment::end);
        context->end_element();
    }
}
