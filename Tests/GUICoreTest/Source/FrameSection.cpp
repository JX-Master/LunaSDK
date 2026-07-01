/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file FrameSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"
#include <cstdio>

namespace Luna::GUICoreTest
{
    void build_frame_panel(GUICore::IContext* context, const CoreSheetState& state)
    {
        GUICore::ElementHandle panel = begin_panel(context, ID_FRAME, "Frame and Layers", 360.0f, 230.0f);
        char text[128];
        auto frame = context->get_frame_desc();
        snprintf(text, sizeof(text), "%.0f x %.0f", frame.screen_size.x, frame.screen_size.y);
        panel_label_value(context, 60.0f, "screen", text);
        snprintf(text, sizeof(text), "%u x %u", frame.framebuffer_size.x, frame.framebuffer_size.y);
        panel_label_value(context, 88.0f, "framebuffer", text);
        snprintf(text, sizeof(text), "%.1f, %.1f", state.sheet_position.x, state.sheet_position.y);
        panel_label_value(context, 116.0f, "sheet pos", text);
        bullet(context, 20.0f, 154.0f, "push_layer creates an ordered drawing and input plane.");
        bullet(context, 20.0f, 182.0f, "The first element in each layer becomes the layer root.");
        end_panel(context);
        (void)panel;
    }
}
