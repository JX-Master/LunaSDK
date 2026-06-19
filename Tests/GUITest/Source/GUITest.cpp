/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/Editor.hpp>
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/VG/VG.hpp>
#include <cstdio>

using namespace Luna;

namespace
{
    void set_layout(GUICore::IContext* context, const GUICore::ElementHandle& element, const RectF& rect)
    {
        GUICore::LayoutResult layout;
        layout.rect = rect;
        layout.clip_rect = rect;
        context->set_layout_result(element, layout);
    }
}

int main()
{
    Luna::init();
    lupanic_if_failed(add_modules({module_rhi(), module_font(), module_vg(), GUICore::module_gui_core(), GUI::module_gui()}));
    lupanic_if_failed(init_modules());

    {
    Ref<GUICore::IContext> context = GUICore::new_context();
    GUI::register_editor_style_schemas(context.get());

    GUICore::FrameDesc frame;
    frame.screen_size = Float2U(800.0f, 480.0f);
    frame.framebuffer_size = UInt2U(800, 480);

    GUICore::InputEvent move;
    move.type = GUICore::InputEventType::pointer_move;
    move.position = Float2U(32.0f, 52.0f);
    GUICore::InputEvent down = move;
    down.type = GUICore::InputEventType::pointer_down;
    down.button = GUICore::PointerButton::left;
    GUICore::InputEvent up = down;
    up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(move);
    context->add_input_event(down);
    context->add_input_event(up);
    context->push_layer(1, Float2U(0.0f), Name("default"));

    GUICore::ElementHandle root = context->begin_element(1, Name("root"));
    set_layout(context.get(), root, RectF(0.0f, 0.0f, 800.0f, 480.0f));

    GUICore::ElementHandle label = GUI::text(context.get(), 2, "Luna GUI direct Core smoke test");
    set_layout(context.get(), label, RectF(16.0f, 12.0f, 280.0f, 28.0f));

    GUICore::ElementHandle button = GUI::text_button(context.get(), 3, "Apply");
    set_layout(context.get(), button, RectF(16.0f, 44.0f, 96.0f, 32.0f));

    context->end_element();
    context->pop_layer();
    context->route_input();

    luassert_always(GUI::is_item_valid(context.get(), button));
    luassert_always(GUI::is_item_hovered(context.get(), button));
    luassert_always(GUI::is_item_clicked(context.get(), button));
    }

    Luna::close();
    return 0;
}
