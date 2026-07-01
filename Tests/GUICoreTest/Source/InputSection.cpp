/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InputSection.cpp
* @author JXMaster
* @date 2026/7/1
*/
#include "GUICoreTest.hpp"

namespace Luna::GUICoreTest
{
    void build_input_panel(GUICore::IContext* context)
    {
        begin_panel(context, ID_INPUT, "Input Routing", 520.0f, 350.0f);
        bullet(context, 20.0f, 62.0f, "Upper layers and newer elements are tested first.");
        bullet(context, 20.0f, 90.0f, "target stops routing and receives events.");
        bullet(context, 20.0f, 118.0f, "pass_through reports a hit but lets lower elements continue.");
        draw_text(context, RectF(22.0f, 166.0f, 420.0f, 24.0f), "Move pointer over samples:", 16.0f,
            Float4U(0.86f, 0.92f, 0.96f, 1.0f));
        panel_label_value(context, 288.0f, "focus", context->focused_element() ? "set" : "none");
        end_panel(context);
    }

    void build_hit_sample(GUICore::IContext* context, GUICore::id_t id, const c8* label,
        const Float4U& base, bool circle, GUICore::PointerHitBehavior behavior)
    {
        GUICore::ElementHandle element = context->begin_element(id, Name(label));
        context->set_layout_config(element, fixed_layout(circle ? 76.0f : 132.0f, circle ? 76.0f : 46.0f));
        GUICore::InteractionState state = context->get_interaction_state(id);
        Float4U color = state.hovered ? Float4U(0.10f, 0.44f, 0.62f, 1.0f) : base;
        set_interactable(context, element, behavior,
            GUICore::InteractableFlag::hoverable | GUICore::InteractableFlag::activatable |
            GUICore::InteractableFlag::focusable);
        if(circle)
        {
            GUICore::ElementHitTestConfig config;
            config.mode = GUICore::ElementHitTestMode::callback;
            config.callback = circle_hit_test;
            context->set_hit_test_config(element, config);
            draw_rect(context, RectF(0.0f, 0.0f, 76.0f, 76.0f), color, 38.0f);
            draw_text(context, RectF(8.0f, 25.0f, 60.0f, 24.0f), label, 14.0f, Float4U(0.95f, 0.98f, 1.0f, 1.0f),
                VG::TextAlignment::center);
        }
        else
        {
            draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), color, 6.0f);
            draw_text(context, RectF(10.0f, 13.0f, 112.0f, 20.0f), label, 14.0f, Float4U(0.95f, 0.98f, 1.0f, 1.0f),
                VG::TextAlignment::center);
        }
        context->end_element();
    }
}
