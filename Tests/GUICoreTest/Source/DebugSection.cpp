/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DebugSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"
#include <cstdio>

namespace Luna::GUICoreTest
{
    void build_debug_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_DEBUG, "Debug and Performance", 470.0f, 260.0f);
        GUICore::PerformanceCounters counters = context->get_performance_counters();
        char text[128];
        snprintf(text, sizeof(text), "%u", counters.frame_generation);
        panel_label_value(context, 62.0f, "frame", text);
        snprintf(text, sizeof(text), "%u", counters.element_count);
        panel_label_value(context, 90.0f, "elements", text);
        snprintf(text, sizeof(text), "%u", counters.draw_command_count);
        panel_label_value(context, 118.0f, "draw cmds", text);
        snprintf(text, sizeof(text), "%.3f ms", counters.draw_compile_ms);
        panel_label_value(context, 146.0f, "compile", text);
        bullet(context, 20.0f, 196.0f, "dump_debug_info can be consumed by in-app or external tools.");
        end_panel(context);
    }
}
