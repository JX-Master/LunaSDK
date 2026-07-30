/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUITest.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include "Showcase.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHIUtility/BlitContext.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/MemoryUtils.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/ShapeBuffer.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>
#include <cstdio>
#include <cstring>

using namespace Luna;

namespace
{
    struct DemoOptions
    {
        i32 section = 0;
        i32 theme = 0;
        i32 density = 0;
        i32 max_frames = -1;
    };

    struct DemoApp
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<RHI::ITexture> gui_target;
        Ref<RHIUtility::IBlitContext> blit_context;
        Ref<GUICore::IContext> gui;
        Ref<GUICore::IRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        i32 applied_theme = -1;
        i32 applied_density = -1;
        GUITest::ShowcaseState state;
    };

    R<Ref<RHI::ITexture>> load_texture(RHI::IDevice* device, u32 queue, const c8* filename)
    {
        lutry
        {
            lulet(file, open_file(filename, FileOpenFlag::read, FileCreationMode::open_existing));
            lulet(file_data, load_file_data(file));
            Image::ImageDesc image_desc;
            lulet(image_data, Image::read_image_file(file_data.data(), file_data.size(),
                Image::ImageFormat::rgba8_unorm, image_desc));
            Ref<RHI::ITexture> texture;
            luset(texture, device->new_texture(RHI::MemoryType::local,
                RHI::TextureDesc::tex2d(RHI::Format::rgba8_unorm,
                    RHI::TextureUsageFlag::copy_dest | RHI::TextureUsageFlag::read_texture,
                    image_desc.width, image_desc.height, 1, 1)));
            lulet(upload_cmdbuf, device->new_command_buffer(queue));
            Ref<RHIUtility::IResourceWriteContext> writer = RHIUtility::new_resource_write_context(device);
            u32 row_pitch = 0;
            u32 slice_pitch = 0;
            lulet(mapped, writer->write_texture(texture, RHI::SubresourceIndex(0, 0), 0, 0, 0,
                image_desc.width, image_desc.height, 1, row_pitch, slice_pitch));
            memcpy_bitmap(mapped, image_data.data(), image_desc.width * 4, image_desc.height,
                row_pitch, image_desc.width * 4);
            luexp(writer->commit(upload_cmdbuf, true));
            return texture;
        }
        lucatchret;
    }

    void create_circle_shape(GUITest::ShowcaseState& state)
    {
        state.circle_buffer = VG::new_shape_buffer();
        Vector<f32>& points = state.circle_buffer->get_shape_points(true);
        state.circle.buffer = state.circle_buffer;
        state.circle.first_command = (u32)points.size();
        VG::ShapeBuilder::add_circle_filled(points, 8.5f, 8.5f, 8.0f);
        state.circle.num_commands = (u32)points.size() - state.circle.first_command;
        state.circle.bounds = RectF(0.0f, 0.0f, 17.0f, 17.0f);
    }

    void apply_style(DemoApp& app)
    {
        GUI::DefaultStyleDesc desc;
        desc.color_theme = app.state.theme == 0 ? GUI::ColorTheme::light : GUI::ColorTheme::dark;
        desc.input_mode = app.state.density == 0 ? GUI::InputMode::pointer : GUI::InputMode::touch;
        desc.accent = Float4U(0.890f, 0.310f, 0.349f, 1.0f);
        GUI::set_default_style(app.gui, desc);
        app.gui->set_style_value(Name(GUI::DEFAULT_STYLE_NAME),
            Name("gui.popup.backdrop_softness"), GUICore::style_f32(12.0f));
        app.gui->set_style_value(Name(GUI::DEFAULT_STYLE_NAME),
            Name("gui.tooltip.backdrop_softness"), GUICore::style_f32(10.0f));
        app.gui->set_style_value(Name(GUI::DEFAULT_STYLE_NAME),
            Name("gui.dock_panel.floating.backdrop_softness"),
            GUICore::style_f32(14.0f));
        Float4U popup_background = app.gui->get_style_value(
            Name(GUI::DEFAULT_STYLE_NAME), Name("gui.popup.background"),
            GUICore::style_f32x4(Float4U(0.1f))).number;
        popup_background.w = 0.72f;
        app.gui->set_style_value(Name(GUI::DEFAULT_STYLE_NAME),
            Name("gui.popup.background"),
            GUICore::style_f32x4(popup_background));
        Float4U tooltip_background = app.gui->get_style_value(
            Name(GUI::DEFAULT_STYLE_NAME), Name("gui.tooltip.background"),
            GUICore::style_f32x4(Float4U(0.1f))).number;
        tooltip_background.w = 0.76f;
        app.gui->set_style_value(Name(GUI::DEFAULT_STYLE_NAME),
            Name("gui.tooltip.background"),
            GUICore::style_f32x4(tooltip_background));
        app.applied_theme = app.state.theme;
        app.applied_density = app.state.density;
    }

    RV resize_gui_target(DemoApp& app, const UInt2U& size)
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

    RV init_demo(DemoApp& app, const DemoOptions& options)
    {
        lutry
        {
            luexp(add_modules({
                module_window(),
                module_rhi(),
                module_rhi_utility(),
                module_font(),
                module_vg(),
                module_image(),
                GUICore::module_gui_core(),
                GUI::module_gui(),
                GUIWindow::module_gui_window()
            }));
            luexp(init_modules());

            const c8* process_path = get_process_path();
            Path current_dir = process_path;
            release_process_path(process_path);
            current_dir.pop_back();
            luexp(set_current_dir(current_dir.encode().c_str()));

            luset(app.window, Window::new_window("Luna GUI Design Language Lab", Window::DEFAULT_POS,
                Window::DEFAULT_POS, 1440, 1024));
            luexp(app.window->set_foreground());
            RHI::IDevice* device = RHI::get_main_device();
            for(u32 i = 0; i < device->get_num_command_queues(); ++i)
            {
                if(device->get_command_queue_desc(i).type == RHI::CommandQueueType::graphics)
                {
                    app.queue = i;
                    break;
                }
            }
            lucheck_msg(app.queue != U32_MAX, "No graphics queue available.");
            UInt2U size = app.window->get_framebuffer_size();
            luset(app.swap_chain, device->new_swap_chain(app.queue, app.window,
                RHI::SwapChainDesc({ size.x, size.y, 2, RHI::Format::bgra8_unorm, true,
                    RHI::ColorSpace::srgb })));
            luset(app.cmdbuf, device->new_command_buffer(app.queue));
            luset(app.blit_context, RHIUtility::new_blit_context(
                device, app.swap_chain->get_desc().format));
            luexp(resize_gui_target(app, size));
            app.width = size.x;
            app.height = size.y;
            luset(app.renderer, GUICore::new_renderer(device));
            app.gui = GUICore::new_context();
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            GUI::register_style_schemas(app.gui);

            app.state.section = clamp(options.section, 0, 8);
            app.state.theme = clamp(options.theme, 0, 1);
            app.state.density = clamp(options.density, 0, 1);
            apply_style(app);
            luset(app.state.material_preview, load_texture(device, app.queue, "material-preview.png"));
            luset(app.state.material_sand, load_texture(device, app.queue, "material-sand.png"));
            luset(app.state.material_rusted, load_texture(device, app.queue, "material-rusted.png"));
            luset(app.state.material_concrete, load_texture(device, app.queue, "material-concrete.png"));
            create_circle_shape(app.state);
        }
        lucatchret;
        return ok;
    }

    RV render_demo(DemoApp& app, RHI::ITexture* back_buffer,
        const Float4U& clear_color)
    {
        lutry
        {
            GUICore::RenderTargetDesc target(app.gui_target);
            target.color_load_op = RHI::LoadOp::clear;
            target.color_clear_value = clear_color;
            target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
            luexp(app.renderer->render(app.gui, app.cmdbuf, target));
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

    RV run_demo(const DemoOptions& options)
    {
        lutry
        {
            DemoApp app;
            luexp(init_demo(app, options));
            GUIWindow::GUICoreWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            i32 frame_index = 0;
            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed()) break;
                if(app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }
                if(app.state.theme != app.applied_theme || app.state.density != app.applied_density)
                {
                    apply_style(app);
                }
                UInt2U framebuffer_size = app.window->get_framebuffer_size();
                if(framebuffer_size.x && framebuffer_size.y &&
                    (framebuffer_size.x != app.width || framebuffer_size.y != app.height))
                {
                    luexp(app.swap_chain->reset({ framebuffer_size.x, framebuffer_size.y, 2,
                        RHI::Format::unknown, true }));
                    luexp(resize_gui_target(app, framebuffer_size));
                    app.width = framebuffer_size.x;
                    app.height = framebuffer_size.y;
                }

                UInt2U logical_size = app.window->get_size();
                GUICore::FrameDesc frame;
                frame.screen_size = Float2U((f32)logical_size.x, (f32)logical_size.y);
                frame.framebuffer_size = framebuffer_size;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);
                if(options.max_frames >= 0 && frame_index == 0 &&
                    app.state.section == 7)
                {
                    GUI::open_popup(app.gui,
                        app.gui->make_id("overlay.popup.layer"));
                }

                GUITest::ShowcaseHandles handles;
                GUICore::ElementHandle root = GUITest::build_showcase(app.gui, app.state, handles);
                luexp(GUI::layout_tree(app.gui, root,
                    RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                app.gui->route_input();
                GUI::ResolveResult resolved = GUI::resolve_interactions(app.gui);
                GUITest::resolve_showcase(app.gui, app.state, handles);
                bool style_changed = app.state.theme != app.applied_theme ||
                    app.state.density != app.applied_density;
                if(style_changed) apply_style(app);
                if(resolved.relayout_requested || style_changed)
                {
                    luexp(GUI::layout_tree(app.gui, root,
                        RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(app.gui->generate_draw_commands());

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                Float4U clear_color = app.gui->get_style_value(Name(GUI::DEFAULT_STYLE_NAME), Name("gui.canvas"),
                    GUICore::style_f32x4(Float4U(0.92f, 0.93f, 0.92f, 1.0f))).number;
                luexp(render_demo(app, back_buffer, clear_color));
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
                ++frame_index;
                if(options.max_frames >= 0 && frame_index >= options.max_frames) break;
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
        }
        lucatchret;
        return ok;
    }
}

int luna_main(int argc, const char* argv[])
{
    DemoOptions options;
    for(int i = 1; i < argc; ++i)
    {
        i32 value = 0;
        if(sscanf(argv[i], "--section=%d", &value) == 1) options.section = value;
        else if(sscanf(argv[i], "--frames=%d", &value) == 1) options.max_frames = value;
        else if(!strcmp(argv[i], "--theme=dark")) options.theme = 1;
        else if(!strcmp(argv[i], "--theme=light")) options.theme = 0;
        else if(!strcmp(argv[i], "--density=compact")) options.density = 0;
        else if(!strcmp(argv[i], "--density=touch")) options.density = 1;
    }
    Luna::init();
    lupanic_if_failed(run_demo(options));
    Luna::close();
    return 0;
}
