/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include "GUICoreTest.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>

using namespace Luna;

namespace
{
    struct CoreTestApp
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUICore::IContext> gui;
        Ref<GUICore::IRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        GUICoreTest::CoreSheetState sheet;
    };

    void build_sheet(GUICore::IContext* context, GUICoreTest::CoreSheetState& state)
    {
        using namespace GUICoreTest;

        state.sheet_items.clear();
        add_canvas_item(state.sheet_items, ID_HEADER, 48.0f, 42.0f);
        if(state.slice_index == 0)
        {
            add_canvas_item(state.sheet_items, ID_INPUT, 64.0f, 174.0f);
        }
        else if(state.slice_index == 1)
        {
            add_canvas_item(state.sheet_items, ID_KEYBOARD, 64.0f, 174.0f);
        }
        else if(state.slice_index == 2)
        {
            add_canvas_item(state.sheet_items, ID_NAVIGATION, 64.0f, 174.0f);
        }
        else if(state.slice_index < NUM_INPUT_SLICES + NUM_LAYOUT_SLICES)
        {
            add_layout_slice_items(state, state.slice_index - NUM_INPUT_SLICES);
        }
        else
        {
            add_canvas_item(state.sheet_items, ID_SDF, 64.0f, 174.0f);
            add_sdf_slice_items(state);
        }

        GUICore::ElementHandle sheet = context->begin_element(ID_SHEET);
        context->set_layout_config(sheet, fixed_layout(SHEET_WIDTH, SHEET_HEIGHT));
        GUICore::DrawConfig sheet_draw;
        sheet_draw.name = Name("guicore.test.sheet");
        sheet_draw.callback = draw_sheet_callback;
        sheet_draw.phases = GUICore::DrawPhaseFlag::before_children | GUICore::DrawPhaseFlag::after_children;
        context->set_draw_config(sheet, sheet_draw);

        build_slide_header(context, state);
        if(state.slice_index == 0)
        {
            build_pointer_input_slice(context, state);
        }
        else if(state.slice_index == 1)
        {
            build_keyboard_input_slice(context, state);
        }
        else if(state.slice_index == 2)
        {
            build_navigation_input_slice(context, state);
        }
        else if(state.slice_index < NUM_INPUT_SLICES + NUM_LAYOUT_SLICES)
        {
            build_layout_slice(context, state, state.slice_index - NUM_INPUT_SLICES);
        }
        else
        {
            build_sdf_slice(context, state);
        }

        state.sheet_canvas.items = Span<const GUICore::CanvasLayoutItem>(state.sheet_items.data(), state.sheet_items.size());
        state.sheet_canvas.default_item = GUICore::CanvasLayoutItem();
        state.sheet_canvas.clip_children = false;
        set_canvas_layout(context, sheet, &state.sheet_canvas);
        context->end_element();
    }

    GUICore::ElementHandle build_frame(GUICore::IContext* context, GUICoreTest::CoreSheetState& state)
    {
        using namespace GUICoreTest;

        state.screen_item = GUICore::CanvasLayoutItem();
        state.screen_item.element_id = ID_SHEET;
        state.screen_item.offset = Float4U(state.sheet_position.x, state.sheet_position.y, 0.0f, 0.0f);
        state.screen_canvas.items = Span<const GUICore::CanvasLayoutItem>(&state.screen_item, 1);
        state.screen_canvas.clip_children = false;

        context->push_layer(1, Float2U(0.0f));
        GUICore::ElementHandle root = context->begin_element(ID_SCREEN_ROOT);
        context->set_layout_config(root, fixed_layout(SHEET_WIDTH, SHEET_HEIGHT));
        build_sheet(context, state);
        set_canvas_layout(context, root, &state.screen_canvas);
        context->end_element();
        context->pop_layer();
        return root;
    }

    RV init_core_test(CoreTestApp& app)
    {
        lutry
        {
            luexp(add_modules({
                module_window(),
                module_rhi(),
                module_font(),
                module_vg(),
                GUICore::module_gui_core(),
                GUIWindow::module_gui_window()
            }));
            luexp(init_modules());

            luset(app.window, Window::new_window("Luna GUICore Slides"));
            auto dev = RHI::get_main_device();
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
            lucheck_msg(app.queue != U32_MAX, "No graphics queue available.");
            auto sz = app.window->get_framebuffer_size();
            luset(app.swap_chain, dev->new_swap_chain(app.queue, app.window,
                RHI::SwapChainDesc({ sz.x, sz.y, 2, RHI::Format::bgra8_unorm, true })));
            luset(app.cmdbuf, dev->new_command_buffer(app.queue));
            luset(app.renderer, GUICore::new_renderer(dev));
            app.gui = GUICore::new_context();
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
        }
        lucatchret;
        return ok;
    }

    RV render_core_test(CoreTestApp& app, RHI::ITexture* back_buffer)
    {
        lutry
        {
            luexp(app.renderer->prepare(app.gui, app.cmdbuf, back_buffer));
            RHI::RenderPassDesc render_pass;
            render_pass.color_attachments[0] = RHI::ColorAttachment(
                back_buffer, RHI::LoadOp::load, RHI::StoreOp::store);
            app.cmdbuf->begin_render_pass(render_pass);
            app.renderer->render(app.cmdbuf);
            app.cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }

    RV run_core_test()
    {
        lutry
        {
            CoreTestApp app;
            luexp(init_core_test(app));

            GUIWindow::GUICoreWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed())
                {
                    break;
                }
                if(app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }

                auto fb_sz = app.window->get_framebuffer_size();
                if(fb_sz.x && fb_sz.y && (fb_sz.x != app.width || fb_sz.y != app.height))
                {
                    luexp(app.swap_chain->reset({ fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true }));
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }

                auto logical_sz = app.window->get_size();
                GUICore::FrameDesc frame;
                frame.screen_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);
                bool z_down = app.gui->is_key_down(KeyCode::z);
                bool x_down = app.gui->is_key_down(KeyCode::x);
                bool test_text_input_focused = app.gui->focused_element() == GUICoreTest::ID_IME_INPUT;
                if(z_down && !app.sheet.z_down && !test_text_input_focused)
                {
                    app.sheet.slice_index = app.sheet.slice_index ? app.sheet.slice_index - 1 : GUICoreTest::NUM_SLICES - 1;
                }
                if(x_down && !app.sheet.x_down && !test_text_input_focused)
                {
                    app.sheet.slice_index = (app.sheet.slice_index + 1) % GUICoreTest::NUM_SLICES;
                }
                app.sheet.z_down = z_down;
                app.sheet.x_down = x_down;
                GUICore::ElementHandle root = build_frame(app.gui, app.sheet);
                const GUICore::Element* root_element = app.gui->get_element(root.index);
                GUICore::ElementHandle sheet = app.gui->find_element_handle(GUICoreTest::ID_SHEET);
                const GUICore::Element* sheet_element = app.gui->get_element(sheet.index);
                luassert(root_element && root_element->layout_callback_config != U32_MAX &&
                    root_element->navigation_config == U32_MAX && root_element->hit_test_config == U32_MAX &&
                    root_element->draw_config == U32_MAX);
                luassert(sheet_element && sheet_element->layout_callback_config != U32_MAX &&
                    sheet_element->draw_config != U32_MAX);
                luassert(app.gui->get_layout_callback_config(root).algorithm == Name("guicore.test.canvas"));
                luexp(app.gui->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                app.gui->route_input();
                bool layout_dirty = false;
                if(app.sheet.slice_index >= GUICoreTest::NUM_INPUT_SLICES &&
                    app.sheet.slice_index < GUICoreTest::NUM_INPUT_SLICES + GUICoreTest::NUM_LAYOUT_SLICES)
                {
                    layout_dirty = GUICoreTest::process_layout_slice_input(app.gui, app.sheet,
                        app.sheet.slice_index - GUICoreTest::NUM_INPUT_SLICES);
                }
                if(app.gui->is_pointer_button_down(GUICore::PointerButton::middle))
                {
                    Float2U delta = app.gui->get_pointer_delta();
                    app.sheet.sheet_position.x += delta.x;
                    app.sheet.sheet_position.y += delta.y;
                    app.sheet.screen_item.offset = Float4U(app.sheet.sheet_position.x, app.sheet.sheet_position.y, 0.0f, 0.0f);
                    layout_dirty = true;
                }
                if(layout_dirty)
                {
                    luexp(app.gui->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(app.gui->generate_draw_commands());
                Span<const GUICore::DrawCommand> generated_commands = app.gui->get_draw_commands();
                luassert(generated_commands.size() >= 6);
                luassert(generated_commands.front().element == sheet.index &&
                    generated_commands.front().type == GUICore::DrawCommandType::shadow);
                luassert(generated_commands[1].element == sheet.index &&
                    generated_commands[1].type == GUICore::DrawCommandType::rect);
                for(usize i = generated_commands.size() - 4; i < generated_commands.size(); ++i)
                {
                    luassert(generated_commands[i].element == sheet.index &&
                        generated_commands[i].type == GUICore::DrawCommandType::line);
                }
                luassert(app.gui->get_performance_counters().draw_callback_count >= 2);

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                RHI::RenderPassDesc render_pass;
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store,
                    Float4U(0.90f, 0.90f, 0.90f, 1.0f));
                app.cmdbuf->begin_render_pass(render_pass);
                app.cmdbuf->end_render_pass();
                luexp(render_core_test(app, back_buffer));
                app.cmdbuf->resource_barrier({}, {
                    { back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                        RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none }
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
    (void)argc;
    (void)argv;
    Luna::init();
    lupanic_if_failed(run_core_test());
    Luna::close();
    return 0;
}
