/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include "GUITest.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHIUtility/BlitContext.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>
#include <cstdlib>
#include <cstring>

using namespace Luna;

namespace
{
    struct GUITestApp
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<RHI::ITexture> gui_target;
        Ref<RHI::ITexture> depth_texture;
        Ref<RHIUtility::IBlitContext> blit_context;
        Ref<GUI::IContext> gui;
        Ref<GUI::IRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        GUITest::SheetState sheet;
    };

    bool is_world_surface_slice(const GUITestApp& app)
    {
        return app.sheet.slice_index == GUITest::WORLD_SURFACE_SLICE;
    }

    void validate_surface_ray_mapping()
    {
        GUI::SurfaceRayHit hit;
        bool intersects = GUI::ray_to_surface(Float3U(24.0f, 48.0f, -10.0f),
            Float3U(0.0f, 0.0f, 2.0f), Float4x4U(Float4x4::identity()), hit);
        luassert(intersects);
        luassert(abs(hit.position.x - 24.0f) < 0.0001f);
        luassert(abs(hit.position.y - 48.0f) < 0.0001f);
        luassert(abs(hit.ray_distance - 5.0f) < 0.0001f);

        Float4x4 surface_to_world = AffineMatrix::make_translation(Float3(10.0f, 20.0f, 5.0f));
        intersects = GUI::ray_to_surface(Float3U(13.0f, 27.0f, 0.0f),
            Float3U(0.0f, 0.0f, 1.0f), Float4x4U(inverse(surface_to_world)), hit);
        luassert(intersects);
        luassert(abs(hit.position.x - 3.0f) < 0.0001f);
        luassert(abs(hit.position.y - 7.0f) < 0.0001f);
        luassert(!GUI::ray_to_surface(Float3U(0.0f), Float3U(1.0f, 0.0f, 0.0f),
            Float4x4U(Float4x4::identity()), hit));
    }

    RV draw_blur_planning_sample(GUI::IContext* context,
        const GUI::ElementHandle&, GUI::DrawPhase, void*)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::backdrop_blur;
        command.rect_reference =
            GUI::DrawCommandRectReference::element;
        context->draw(command);
        return ok;
    }

    RV run_blur_planning_case(GUITestApp& app, u32 size,
        f32 softness, u8 downsample_level,
        const GUI::RendererPerformanceCounters& expected)
    {
        lutry
        {
            Ref<GUI::IContext> context = GUI::new_context();
            GUI::FrameDesc frame;
            frame.screen_size = Float2U((f32)size);
            frame.framebuffer_size = UInt2U(size);
            frame.dpi_scale = 1.0f;
            frame.delta_time = 1.0f / 60.0f;
            context->begin_frame(frame);
            context->push_layer(1, Float2U(0.0f));
            GUI::ElementHandle root = context->begin_element(1);
            GUI::BackdropBlurCaptureDesc capture;
            capture.softness = softness;
            capture.downsample_level = downsample_level;
            context->set_backdrop_blur_capture(root, capture);
            GUI::DrawConfig draw;
            draw.name = Name("gui.test.blur.planning");
            draw.callback = draw_blur_planning_sample;
            context->set_draw_config(root, draw);
            context->end_element();
            context->pop_layer();
            luexp(context->apply_layout(root,
                RectF(0.0f, 0.0f, (f32)size, (f32)size)));

            lulet(texture, RHI::get_main_device()->new_texture(
                RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    RHI::Format::bgra8_unorm,
                    RHI::TextureUsageFlag::color_attachment |
                        RHI::TextureUsageFlag::read_texture,
                    size, size, 1, 1)));
            GUI::RenderTargetDesc target(texture);
            target.color_load_op = RHI::LoadOp::clear;
            target.color_clear_value = Float4U(0.25f, 0.5f, 0.75f, 1.0f);
            target.color_final_state =
                RHI::TextureStateFlag::shader_read_ps;
            luexp(app.renderer->render(context, app.cmdbuf, target));
            GUI::RendererPerformanceCounters counters =
                app.renderer->get_performance_counters();
            luexp(app.cmdbuf->submit({}, {}, true));
            app.cmdbuf->wait();
            luexp(app.cmdbuf->reset());

            luassert(counters.backdrop_capture_count ==
                expected.backdrop_capture_count);
            luassert(counters.render_pass_count ==
                expected.render_pass_count);
            luassert(counters.backdrop_blur_dispatch_count ==
                expected.backdrop_blur_dispatch_count);
            luassert(counters.backdrop_filtered_pixel_count ==
                expected.backdrop_filtered_pixel_count);
            luassert(counters.backdrop_temporary_texture_bytes ==
                expected.backdrop_temporary_texture_bytes);
        }
        lucatchret;
        return ok;
    }

    RV validate_blur_filter_planning(GUITestApp& app)
    {
        GUI::RendererPerformanceCounters snapshot;
        snapshot.backdrop_capture_count = 1;
        snapshot.render_pass_count = 2;
        snapshot.backdrop_blur_dispatch_count = 1;
        snapshot.backdrop_filtered_pixel_count = 64u * 64u;
        snapshot.backdrop_temporary_texture_bytes =
            64u * 64u * 2u * 4u;
        RV result = run_blur_planning_case(app, 64, 0.0f, 0, snapshot);
        if(failed(result)) return result;

        GUI::RendererPerformanceCounters quantized_subpixel;
        quantized_subpixel.backdrop_capture_count = 1;
        quantized_subpixel.render_pass_count = 2;
        quantized_subpixel.backdrop_blur_dispatch_count = 1;
        quantized_subpixel.backdrop_filtered_pixel_count = 32u * 32u;
        quantized_subpixel.backdrop_temporary_texture_bytes =
            32u * 32u * 2u * 4u;
        result = run_blur_planning_case(app, 32, 0.74f, 0,
            quantized_subpixel);
        if(failed(result)) return result;

        GUI::RendererPerformanceCounters downsample_only;
        downsample_only.backdrop_capture_count = 1;
        downsample_only.render_pass_count = 2;
        downsample_only.backdrop_blur_dispatch_count = 2;
        downsample_only.backdrop_filtered_pixel_count =
            32u * 32u + 16u * 16u;
        downsample_only.backdrop_temporary_texture_bytes =
            (32u * 32u + 16u * 16u + 2u * 16u * 16u) * 4u;
        result = run_blur_planning_case(app, 64, 0.0f, 2,
            downsample_only);
        if(failed(result)) return result;

        GUI::RendererPerformanceCounters automatic_downsample;
        automatic_downsample.backdrop_capture_count = 1;
        automatic_downsample.render_pass_count = 2;
        automatic_downsample.backdrop_blur_dispatch_count = 5;
        automatic_downsample.backdrop_filtered_pixel_count =
            128u * 128u + 64u * 64u + 32u * 32u +
            2u * 32u * 32u;
        automatic_downsample.backdrop_temporary_texture_bytes =
            (usize)automatic_downsample.backdrop_filtered_pixel_count * 4u;
        return run_blur_planning_case(app, 256, 80.0f, 0,
            automatic_downsample);
    }

    RV resize_depth_texture(GUITestApp& app, const UInt2U& size)
    {
        if(!size.x || !size.y)
        {
            app.depth_texture = nullptr;
            return ok;
        }
        auto dev = RHI::get_main_device();
        auto result = dev->new_texture(RHI::MemoryType::local,
            RHI::TextureDesc::tex2d(RHI::Format::d32_float,
                RHI::TextureUsageFlag::depth_stencil_attachment, size.x, size.y, 1, 1));
        if(failed(result)) return result.errcode();
        app.depth_texture = result.get();
        return ok;
    }

    RV resize_gui_target(GUITestApp& app, const UInt2U& size)
    {
        lutry
        {
            luset(app.gui_target, RHI::get_main_device()->new_texture(
                RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                    app.swap_chain->get_desc().format,
                    RHI::TextureUsageFlag::color_attachment |
                        RHI::TextureUsageFlag::read_texture,
                    size.x, size.y, 1, 1)));
        }
        lucatchret;
        return ok;
    }

    void build_sheet(GUI::IContext* context, GUITest::SheetState& state)
    {
        using namespace GUITest;

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
        else if(state.slice_index == BLUR_MATERIALS_SLICE)
        {
            add_canvas_item(state.sheet_items, ID_BLUR, 64.0f, 174.0f);
            add_blur_slice_items(state);
        }
        else
        {
            add_canvas_item(state.sheet_items, ID_SDF, 64.0f, 174.0f);
            add_sdf_slice_items(state);
        }

        GUI::ElementHandle sheet = context->begin_element(ID_SHEET);
        context->set_layout_config(sheet, fixed_layout(SHEET_WIDTH, SHEET_HEIGHT));
        GUI::DrawConfig sheet_draw;
        sheet_draw.name = Name("gui.test.sheet");
        sheet_draw.callback = draw_sheet_callback;
        sheet_draw.phases = GUI::DrawPhaseFlag::before_children | GUI::DrawPhaseFlag::after_children;
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
        else if(state.slice_index == BLUR_MATERIALS_SLICE)
        {
            build_blur_slice(context, state);
        }
        else
        {
            build_sdf_slice(context, state);
        }

        state.sheet_canvas.items = Span<const GUI::CanvasLayoutItem>(state.sheet_items.data(), state.sheet_items.size());
        state.sheet_canvas.default_item = GUI::CanvasLayoutItem();
        state.sheet_canvas.clip_children = false;
        set_canvas_layout(context, sheet, &state.sheet_canvas);
        context->end_element();
    }

    GUI::ElementHandle build_frame(GUI::IContext* context, GUITest::SheetState& state)
    {
        using namespace GUITest;

        state.screen_item = GUI::CanvasLayoutItem();
        state.screen_item.element_id = ID_SHEET;
        state.screen_item.offset = Float4U(state.sheet_position.x, state.sheet_position.y, 0.0f, 0.0f);
        state.screen_canvas.items = Span<const GUI::CanvasLayoutItem>(&state.screen_item, 1);
        state.screen_canvas.clip_children = false;

        context->push_layer(1, Float2U(0.0f));
        GUI::ElementHandle root = context->begin_element(ID_SCREEN_ROOT);
        context->set_layout_config(root, fixed_layout(SHEET_WIDTH, SHEET_HEIGHT));
        build_sheet(context, state);
        set_canvas_layout(context, root, &state.screen_canvas);
        context->end_element();
        context->pop_layer();
        return root;
    }

    RV init_gui_test(GUITestApp& app)
    {
        lutry
        {
            luexp(add_modules({
                module_window(),
                module_rhi(),
                module_rhi_utility(),
                module_font(),
                module_vg(),
                GUI::module_gui(),
                GUIWindow::module_gui_window()
            }));
            luexp(init_modules());

            luset(app.window, Window::new_window("Luna GUI Slides"));
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
            luset(app.blit_context, RHIUtility::new_blit_context(
                dev, app.swap_chain->get_desc().format));
            luset(app.renderer, GUI::new_renderer(dev));
            app.gui = GUI::new_context();
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            validate_surface_ray_mapping();
        }
        lucatchret;
        return ok;
    }

    RV render_gui_test(GUITestApp& app, RHI::ITexture* back_buffer)
    {
        lutry
        {
            GUI::RenderSurfaceDesc surface;
            bool world_surface = is_world_surface_slice(app);
            if(world_surface)
            {
                GUI::FrameDesc frame = app.gui->get_frame_desc();
                f32 width = max(frame.screen_size.x, 1.0f);
                f32 height = max(frame.screen_size.y, 1.0f);
                Float3 eye(width * 0.74f, height * 0.28f, height * 1.85f);
                Float3 target(width * 0.5f, height * 0.5f, 0.0f);
                Float4x4 view = AffineMatrix::make_look_at(eye, target, Float3(0.0f, -1.0f, 0.0f));
                Float4x4 projection = ProjectionMatrix::make_perspective_fov(PI / 2.6f,
                    (f32)app.width / (f32)app.height, 1.0f, height * 8.0f);
                surface.use_custom_transform = true;
                surface.surface_to_clip = mul(view, projection);
                surface.depth_test_enable = true;
                surface.depth_write_enable = false;
                surface.depth_compare_function = RHI::CompareFunction::less_equal;
            }
            GUI::RenderTargetDesc target(app.gui_target);
            target.color_load_op = RHI::LoadOp::clear;
            target.color_clear_value = Float4U(0.90f, 0.90f, 0.90f, 1.0f);
            target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
            if(world_surface)
            {
                target.depth_stencil_texture = app.depth_texture;
                target.depth_load_op = RHI::LoadOp::clear;
                target.depth_clear_value = 1.0f;
            }
            luexp(app.renderer->render(app.gui, app.cmdbuf, target, surface));
            app.blit_context->reset();
            RHI::SamplerDesc sampler(RHI::Filter::linear, RHI::Filter::linear,
                RHI::Filter::nearest, RHI::TextureAddressMode::clamp,
                RHI::TextureAddressMode::clamp, RHI::TextureAddressMode::clamp);
            app.blit_context->blit(back_buffer, RHI::SubresourceIndex(0, 0),
                RHI::TextureViewDesc::tex2d(app.gui_target), sampler,
                Float2U(0.0f), Float2U((f32)app.width, 0.0f),
                Float2U(0.0f, (f32)app.height),
                Float2U((f32)app.width, (f32)app.height));
            luexp(app.blit_context->commit(app.cmdbuf, false));
            app.cmdbuf->resource_barrier({}, {
                {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES,
                    RHI::TextureStateFlag::automatic,
                    RHI::TextureStateFlag::present,
                    RHI::ResourceBarrierFlag::none}
            });
        }
        lucatchret;
        return ok;
    }

    RV run_gui_test(u32 initial_slice, u32 max_frames)
    {
        lutry
        {
            GUITestApp app;
            luexp(init_gui_test(app));
            app.sheet.slice_index = min(initial_slice, GUITest::NUM_SLICES - 1);
            if(app.sheet.slice_index == GUITest::BLUR_MATERIALS_SLICE)
            {
                luexp(validate_blur_filter_planning(app));
            }
            u32 rendered_frames = 0;

            GUIWindow::GUIWindowInputAdapter input_adapter;
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
                    luexp(resize_gui_target(app, fb_sz));
                    luexp(resize_depth_texture(app, fb_sz));
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }

                auto logical_sz = app.window->get_size();
                GUI::FrameDesc frame;
                frame.screen_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.sheet.animation_time += frame.delta_time;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);
                bool z_down = app.gui->is_key_down(KeyCode::z);
                bool x_down = app.gui->is_key_down(KeyCode::x);
                bool test_text_input_focused = app.gui->focused_element() == GUITest::ID_IME_INPUT;
                if(z_down && !app.sheet.z_down && !test_text_input_focused)
                {
                    app.sheet.slice_index = app.sheet.slice_index ? app.sheet.slice_index - 1 : GUITest::NUM_SLICES - 1;
                }
                if(x_down && !app.sheet.x_down && !test_text_input_focused)
                {
                    app.sheet.slice_index = (app.sheet.slice_index + 1) % GUITest::NUM_SLICES;
                }
                app.sheet.z_down = z_down;
                app.sheet.x_down = x_down;
                GUI::ElementHandle root = build_frame(app.gui, app.sheet);
                const GUI::Element* root_element = app.gui->get_element(root.index);
                GUI::ElementHandle sheet = app.gui->find_element_handle(GUITest::ID_SHEET);
                const GUI::Element* sheet_element = app.gui->get_element(sheet.index);
                luassert(root_element && root_element->layout_callback_config != U32_MAX &&
                    root_element->navigation_config == U32_MAX && root_element->hit_test_config == U32_MAX &&
                    root_element->draw_config == U32_MAX);
                luassert(sheet_element && sheet_element->layout_callback_config != U32_MAX &&
                    sheet_element->draw_config != U32_MAX);
                luassert(app.gui->get_layout_callback_config(root).algorithm == Name("gui.test.canvas"));
                luexp(app.gui->apply_layout(root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                app.gui->route_input();
                bool layout_dirty = false;
                if(app.sheet.slice_index >= GUITest::NUM_INPUT_SLICES &&
                    app.sheet.slice_index < GUITest::NUM_INPUT_SLICES + GUITest::NUM_LAYOUT_SLICES)
                {
                    layout_dirty = GUITest::process_layout_slice_input(app.gui, app.sheet,
                        app.sheet.slice_index - GUITest::NUM_INPUT_SLICES);
                }
                if(app.gui->is_pointer_button_down(GUI::PointerButton::middle))
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
                Span<const GUI::DrawCommand> generated_commands = app.gui->get_draw_commands();
                luassert(generated_commands.size() >= 6);
                luassert(generated_commands.front().element == sheet.index &&
                    generated_commands.front().type == GUI::DrawCommandType::shadow);
                luassert(generated_commands[1].element == sheet.index &&
                    generated_commands[1].type == GUI::DrawCommandType::rect);
                for(usize i = generated_commands.size() - 4; i < generated_commands.size(); ++i)
                {
                    luassert(generated_commands[i].element == sheet.index &&
                        generated_commands[i].type == GUI::DrawCommandType::line);
                }
                luassert(app.gui->get_performance_counters().draw_callback_count >= 2);
                if(app.sheet.slice_index == GUITest::BLUR_MATERIALS_SLICE)
                {
                    u32 capture_commands = 0;
                    u32 blur_commands = 0;
                    for(const GUI::DrawCommand& command : generated_commands)
                    {
                        if(command.type == GUI::DrawCommandType::backdrop_blur_capture)
                        {
                            ++capture_commands;
                        }
                        else if(command.type == GUI::DrawCommandType::backdrop_blur)
                        {
                            ++blur_commands;
                        }
                    }
                    luassert(capture_commands == 8);
                    luassert(blur_commands == 8);
                }

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                luexp(render_gui_test(app, back_buffer));
                if(app.sheet.slice_index == GUITest::BLUR_MATERIALS_SLICE)
                {
                    const GUI::RendererPerformanceCounters& counters =
                        app.renderer->get_performance_counters();
                    luassert(counters.backdrop_capture_count == 8);
                    luassert(counters.render_pass_count ==
                        counters.backdrop_capture_count + 1);
                    luassert(counters.backdrop_blur_dispatch_count >= 16);
                    luassert(counters.backdrop_filtered_pixel_count > 0);
                    luassert(app.gui_target->get_desc().format ==
                        RHI::Format::bgra8_unorm);
                    luassert(counters.backdrop_temporary_texture_bytes ==
                        counters.backdrop_filtered_pixel_count * 4u);
                }
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
                ++rendered_frames;
                if(rendered_frames >= max_frames) break;
            }

            GUIWindow::uninstall_window_event_handler(&input_adapter);
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    u32 initial_slice = 0;
    u32 max_frames = U32_MAX;
    for(int i = 1; i < argc; ++i)
    {
        if(!strcmp(argv[i], "--world-space"))
        {
            initial_slice = GUITest::WORLD_SURFACE_SLICE;
        }
        else if(!strcmp(argv[i], "--blur"))
        {
            initial_slice = GUITest::BLUR_MATERIALS_SLICE;
        }
        else if(!strncmp(argv[i], "--frames=", 9))
        {
            max_frames = max<u32>((u32)strtoul(argv[i] + 9, nullptr, 10), 1);
        }
    }
    Luna::init();
    lupanic_if_failed(run_gui_test(initial_slice, max_frames));
    Luna::close();
    return 0;
}
