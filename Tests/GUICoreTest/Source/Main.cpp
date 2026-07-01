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
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
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
        Ref<VG::IShapeDrawList> draw_list;
        Ref<VG::IShapeRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        GUICoreTest::CoreSheetState sheet;
    };

    void build_sheet(GUICore::IContext* context, GUICoreTest::CoreSheetState& state)
    {
        using namespace GUICoreTest;

        state.sheet_items.clear();
        add_canvas_item(state.sheet_items, ID_HEADER, 40.0f, 36.0f);
        add_canvas_item(state.sheet_items, ID_FRAME, 40.0f, 190.0f);
        add_canvas_item(state.sheet_items, ID_ELEMENT_TREE, 430.0f, 190.0f);
        add_canvas_item(state.sheet_items, ID_LAYOUT, 820.0f, 190.0f);
        add_canvas_item(state.sheet_items, ID_INPUT, 40.0f, 530.0f);
        add_canvas_item(state.sheet_items, ID_HIT_RECT, 64.0f, 720.0f);
        add_canvas_item(state.sheet_items, ID_HIT_CIRCLE, 228.0f, 704.0f);
        add_canvas_item(state.sheet_items, ID_HIT_PASS, 340.0f, 720.0f);
        add_canvas_item(state.sheet_items, ID_DRAW, 590.0f, 530.0f);
        add_canvas_item(state.sheet_items, ID_STATE, 1090.0f, 530.0f);
        add_canvas_item(state.sheet_items, ID_DEBUG, 590.0f, 820.0f);
        add_canvas_item(state.sheet_items, ID_CANVAS, 1090.0f, 820.0f);

        GUICore::ElementHandle sheet = context->begin_element(ID_SHEET, Name("Fixed Cheat Sheet"));
        context->set_layout_config(sheet, fixed_layout(SHEET_WIDTH, SHEET_HEIGHT));
        draw_rect(context, RectF(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.045f, 0.058f, 0.070f, 1.0f), 12.0f);
        draw_outline(context, RectF(0.0f, 0.0f, SHEET_WIDTH, SHEET_HEIGHT), Float4U(0.16f, 0.24f, 0.30f, 1.0f), 1.0f);

        build_header(context);
        build_frame_panel(context, state);
        build_element_tree_panel(context);
        build_layout_panel(context);
        build_input_panel(context);
        build_hit_sample(context, ID_HIT_RECT, "target", Float4U(0.07f, 0.28f, 0.42f, 1.0f));
        build_hit_sample(context, ID_HIT_CIRCLE, "circle", Float4U(0.28f, 0.18f, 0.44f, 1.0f), true);
        build_hit_sample(context, ID_HIT_PASS, "pass", Float4U(0.36f, 0.25f, 0.08f, 0.82f), false,
            GUICore::PointerHitBehavior::pass_through);
        build_draw_panel(context);
        build_state_panel(context);
        build_debug_panel(context);
        build_canvas_panel(context);

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

        context->push_layer(1, Float2U(0.0f), Name("default"));
        GUICore::ElementHandle root = context->begin_element(ID_SCREEN_ROOT, Name("Screen Root"));
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

            luset(app.window, Window::new_window("Luna GUICore Cheat Sheet"));
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
            app.draw_list = VG::new_shape_draw_list(dev);
            app.renderer = VG::new_fill_shape_renderer();
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
            luexp(app.gui->compile_draw_commands(app.draw_list));
            luexp(app.draw_list->compile());
            Span<const VG::ShapeDrawCall> draw_calls = app.draw_list->get_draw_calls();
            if(!draw_calls.empty())
            {
                GUICore::FrameDesc frame = app.gui->get_frame_desc();
                Float4x4U transform = ProjectionMatrix::make_orthographic_off_center(
                    0.0f, max(frame.screen_size.x, 1.0f),
                    0.0f, max(frame.screen_size.y, 1.0f),
                    0.0f, 1.0f);
                luexp(app.renderer->begin(back_buffer));
                app.renderer->draw(app.draw_list->get_vertex_buffer(), app.draw_list->get_index_buffer(), draw_calls, &transform);
                luexp(app.renderer->end());
                app.renderer->submit(app.cmdbuf);
            }
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
                GUICore::ElementHandle root = build_frame(app.gui, app.sheet);
                luexp(app.gui->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                app.gui->route_input();
                if(app.gui->is_pointer_button_down(GUICore::PointerButton::middle))
                {
                    Float2U delta = app.gui->get_pointer_delta();
                    app.sheet.sheet_position.x += delta.x;
                    app.sheet.sheet_position.y += delta.y;
                    app.sheet.screen_item.offset = Float4U(app.sheet.sheet_position.x, app.sheet.sheet_position.y, 0.0f, 0.0f);
                    luexp(app.gui->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                RHI::RenderPassDesc render_pass;
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store,
                    Float4U(0.020f, 0.024f, 0.028f, 1.0f));
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
