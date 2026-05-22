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
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Font/Font.hpp>
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

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg(), GUI::module_gui(), GUIWindow::module_gui_window()}));
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
            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

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
                GUI::GUILayoutDesc toolbar_layout;
                toolbar_layout.gap = 8.0f;
                toolbar_layout.cross_axis_alignment = GUI::GUILayoutCrossAxisAlignment::center;
                GUI::BeginHLayout("Toolbar", toolbar_layout);
                GUI::Text("HLayout");
                GUI::Button("One");
                GUI::Button("Two");
                GUI::SetNextItemLayout(GUI::GUILayoutStyle::fill_width());
                GUI::InputText("Inline Name", app.input_text);
                GUI::EndHLayout();
                GUI::GUIItemHandle apply = GUI::Button("Apply");
                GUI::GUIItemHandle header = GUI::CollapsingHeader("Details");
                if(GUI::GetItemState(header, GUI::GUIState::open()))
                {
                    GUI::GUILayoutDesc detail_layout;
                    detail_layout.padding = GUI::GUIEdgeInsets::xy(4.0f, 2.0f);
                    detail_layout.gap = 4.0f;
                    GUI::BeginVLayout("Detail Layout", detail_layout);
                    GUI::Checkbox("Enable option", &app.checkbox);
                    GUI::InputText("Name", app.input_text);
                    GUI::EndVLayout();
                }
                GUI::BeginScrollView("Scroll", GUI::GUISize::fixed(380.0f, 110.0f));
                GUI::Text("ScrollView");
                GUI::Text("Wheel events are translated outside the GUI core.");
                GUI::Text("The same GUIContext can be driven by a window adapter");
                GUI::Text("or by game raycast coordinates.");
                GUI::EndScrollView();
                GUI::BeginHLayout("Images");
                GUI::Image(nullptr, GUI::GUISize::fixed(64.0f, 64.0f));
                GUI::Text("Image nodes use VG textured rects.");
                GUI::EndHLayout();
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
            GUIWindow::uninstall_window_event_handler(&input_adapter);
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
