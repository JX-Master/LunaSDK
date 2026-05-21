/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/GUI/GUI.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/HID/HID.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/HID/Mouse.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <cstdio>

using namespace Luna;

namespace Luna
{
    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUI::IGUIContext> gui;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        u32 apply_count = 0;
        bool checkbox = false;
        String input_text = "Type here";
    };

    static GUI::GUIPointerButton to_gui_button(HID::MouseButton button)
    {
        switch(button)
        {
        case HID::MouseButton::left: return GUI::GUIPointerButton::left;
        case HID::MouseButton::right: return GUI::GUIPointerButton::right;
        case HID::MouseButton::middle: return GUI::GUIPointerButton::middle;
        case HID::MouseButton::function1: return GUI::GUIPointerButton::extra1;
        case HID::MouseButton::function2: return GUI::GUIPointerButton::extra2;
        default: return GUI::GUIPointerButton::left;
        }
    }

    static GUI::GUIKey to_gui_key(HID::KeyCode key)
    {
        using namespace HID;
        switch(key)
        {
        case KeyCode::tab: return GUI::GUIKey::tab;
        case KeyCode::left: return GUI::GUIKey::left;
        case KeyCode::right: return GUI::GUIKey::right;
        case KeyCode::up: return GUI::GUIKey::up;
        case KeyCode::down: return GUI::GUIKey::down;
        case KeyCode::enter: return GUI::GUIKey::enter;
        case KeyCode::esc: return GUI::GUIKey::esc;
        case KeyCode::backspace: return GUI::GUIKey::backspace;
        case KeyCode::del: return GUI::GUIKey::del;
        case KeyCode::spacebar: return GUI::GUIKey::space;
        case KeyCode::a: return GUI::GUIKey::a;
        case KeyCode::b: return GUI::GUIKey::b;
        case KeyCode::c: return GUI::GUIKey::c;
        case KeyCode::d: return GUI::GUIKey::d;
        case KeyCode::e: return GUI::GUIKey::e;
        case KeyCode::f: return GUI::GUIKey::f;
        case KeyCode::g: return GUI::GUIKey::g;
        case KeyCode::h: return GUI::GUIKey::h;
        case KeyCode::i: return GUI::GUIKey::i;
        case KeyCode::j: return GUI::GUIKey::j;
        case KeyCode::k: return GUI::GUIKey::k;
        case KeyCode::l: return GUI::GUIKey::l;
        case KeyCode::m: return GUI::GUIKey::m;
        case KeyCode::n: return GUI::GUIKey::n;
        case KeyCode::o: return GUI::GUIKey::o;
        case KeyCode::p: return GUI::GUIKey::p;
        case KeyCode::q: return GUI::GUIKey::q;
        case KeyCode::r: return GUI::GUIKey::r;
        case KeyCode::s: return GUI::GUIKey::s;
        case KeyCode::t: return GUI::GUIKey::t;
        case KeyCode::u: return GUI::GUIKey::u;
        case KeyCode::v: return GUI::GUIKey::v;
        case KeyCode::w: return GUI::GUIKey::w;
        case KeyCode::x: return GUI::GUIKey::x;
        case KeyCode::y: return GUI::GUIKey::y;
        case KeyCode::z: return GUI::GUIKey::z;
        default: return GUI::GUIKey::unknown;
        }
    }

    static GUI::GUIKeyModifierFlag get_modifiers()
    {
        u8 flags = 0;
        if(HID::get_key_state(HID::KeyCode::ctrl)) flags |= (u8)GUI::GUIKeyModifierFlag::ctrl;
        if(HID::get_key_state(HID::KeyCode::shift)) flags |= (u8)GUI::GUIKeyModifierFlag::shift;
        if(HID::get_key_state(HID::KeyCode::menu)) flags |= (u8)GUI::GUIKeyModifierFlag::alt;
        if(HID::get_key_state(HID::KeyCode::system)) flags |= (u8)GUI::GUIKeyModifierFlag::system;
        return (GUI::GUIKeyModifierFlag)flags;
    }

    static Float2U get_client_mouse_pos(Window::IWindow* window)
    {
        Int2U screen_pos = HID::get_mouse_pos();
        Int2U client_pos = window->screen_to_client(screen_pos);
        return Float2U((f32)client_pos.x, (f32)client_pos.y);
    }

    static void handle_window_event(object_t event, void* userdata)
    {
        App* app = (App*)userdata;
        if(!app || !app->gui) return;
        if(auto window_event = cast_object<Window::WindowEvent>(event))
        {
            if(window_event->window != app->window) return;
            GUI::GUIInputEvent ge;
            if(auto e = cast_object<Window::WindowMouseEnterEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_enter;
                ge.position = get_client_mouse_pos(app->window);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowMouseLeaveEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_leave;
                ge.position = get_client_mouse_pos(app->window);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowMouseMoveEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_move;
                ge.position = Float2U((f32)e->x, (f32)e->y);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowMouseDownEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_down;
                ge.position = get_client_mouse_pos(app->window);
                ge.button = to_gui_button(e->button);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowMouseUpEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_up;
                ge.position = get_client_mouse_pos(app->window);
                ge.button = to_gui_button(e->button);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowScrollEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::pointer_wheel;
                ge.position = get_client_mouse_pos(app->window);
                ge.wheel_delta = Float2U(e->scroll_x, e->scroll_y);
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowKeyDownEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::key_down;
                ge.key = to_gui_key(e->key);
                ge.modifiers = get_modifiers();
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowKeyUpEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::key_up;
                ge.key = to_gui_key(e->key);
                ge.modifiers = get_modifiers();
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowInputTextEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::text_utf8;
                ge.text = e->text;
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowInputFocusEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::focus;
                app->gui->add_input_event(ge);
            }
            else if(auto e = cast_object<Window::WindowLoseInputFocusEvent>(event))
            {
                ge.type = GUI::GUIInputEventType::blur;
                app->gui->add_input_event(ge);
            }
        }
    }

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg(), GUI::module_gui(), module_hid()}));
            luexp(init_modules());
            set_log_to_platform_enabled(true);
            using namespace RHI;

            App app;
            luset(app.window, Window::new_window("Luna GUI Test"));

            Ref<IDevice> dev = get_main_device();
            u32 num_queues = dev->get_num_command_queues();
            for(u32 i = 0; i < num_queues; ++i)
            {
                auto desc = dev->get_command_queue_desc(i);
                if(desc.type == RHI::CommandQueueType::graphics)
                {
                    app.queue = i;
                    break;
                }
            }
            luset(app.swap_chain, dev->new_swap_chain(app.queue, app.window, SwapChainDesc({0, 0, 2, Format::bgra8_unorm, true})));
            luset(app.cmdbuf, dev->new_command_buffer(app.queue));
            app.gui = GUI::new_context(dev);
            Window::set_event_handler(handle_window_event, &app);

            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed()) break;
                if(app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }

                auto fb_sz = app.window->get_framebuffer_size();
                if(fb_sz.x && fb_sz.y && (fb_sz.x != app.width || fb_sz.y != app.height))
                {
                    luexp(app.swap_chain->reset({fb_sz.x, fb_sz.y, 2, Format::unknown, true}));
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }

                auto logical_sz = app.window->get_size();
                GUI::GUIFrameDesc frame;
                frame.surface_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);

                GUI::BeginWindow("Main", GUI::GUISize::fixed(420.0f, 360.0f));
                GUI::Text("Luna GUI");
                GUI::GUIItemHandle apply = GUI::Button("Apply");
                GUI::GUIItemHandle header = GUI::CollapsingHeader("Details");
                if(GUI::GetItemState(header, GUI::GUIState::open()))
                {
                    GUI::Checkbox("Enable option", &app.checkbox);
                    GUI::InputText("Name", app.input_text);
                }
                GUI::BeginScrollView("Scroll", GUI::GUISize::fixed(380.0f, 110.0f));
                GUI::Text("ScrollView");
                GUI::Text("Wheel events are translated outside the GUI core.");
                GUI::Text("The same GUIContext can be driven by a window adapter");
                GUI::Text("or by game raycast coordinates.");
                GUI::EndScrollView();
                GUI::BeginRow("Images");
                GUI::Image(nullptr, GUI::GUISize::fixed(64.0f, 64.0f));
                GUI::Text("Image nodes use VG textured rects.");
                GUI::EndRow();
                GUI::EndWindow();

                lulet(desc, app.gui->end_build());
                luexp(app.gui->submit(desc));
                if(GUI::IsItemClicked(apply))
                {
                    ++app.apply_count;
                }

                c8 buf[128];
                snprintf(buf, 128, "Apply count: %u", app.apply_count);
                // Submit-after-query is tested by changing the window title immediately.
                luexp(app.window->set_title(buf));

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                RenderPassDesc render_pass;
                render_pass.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, Float4U(0.02f, 0.025f, 0.03f, 1.0f));
                app.cmdbuf->begin_render_pass(render_pass);
                app.cmdbuf->end_render_pass();
                luexp(app.gui->render(app.cmdbuf, back_buffer));
                app.cmdbuf->resource_barrier({}, {
                    {back_buffer, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::present, ResourceBarrierFlag::none}
                    });
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
            }
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    auto r = Luna::run_app();
    if(failed(r))
    {
        Luna::log_error("GUITest", "%s", Luna::explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
