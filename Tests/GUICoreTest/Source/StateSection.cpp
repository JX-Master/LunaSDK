/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StateSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_state_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_STATE, "State and Style", 470.0f, 260.0f);
        bullet(context, 20.0f, 64.0f, "State objects are keyed by stable IDs and boxed Runtime objects.");
        bullet(context, 20.0f, 96.0f, "Lifetimes: current_frame, next_frame, process, persistent reserved.");
        bullet(context, 20.0f, 128.0f, "Style entries are independent typed values resolved by name.");
        bullet(context, 20.0f, 160.0f, "Core stores data; packages decide which values mean what.");
        draw_text(context, RectF(22.0f, 210.0f, 420.0f, 26.0f), "No widget-specific state is embedded in Element.",
            16.0f, Float4U(0.86f, 0.93f, 0.98f, 1.0f));
        end_panel(context);
    }
}
