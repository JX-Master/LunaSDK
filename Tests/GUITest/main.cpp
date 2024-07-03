/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2024/7/3
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUI/Context.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/Widgets.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/Runtime/Math/Transform.hpp>

using namespace Luna;

void on_window_close(Window::IWindow* window)
{
    window->close();
}

void run()
{
    set_log_to_platform_enabled(true);
    using namespace RHI;
    using namespace Window;
    Ref<IWindow> window = new_window("GUI Demo", WindowDisplaySettings::as_windowed(), WindowCreationFlag::resizable).get();
    window->get_close_event().add_handler(on_window_close);

    Ref<IDevice> dev = get_main_device();

    u32 queue = U32_MAX;
    u32 num_queues = dev->get_num_command_queues();
    for (u32 i = 0; i < num_queues; ++i)
    {
        auto desc = dev->get_command_queue_desc(i);
        if (desc.type == RHI::CommandQueueType::graphics)
        {
            queue = i;
            break;
        }
    }

    Ref<ISwapChain> swap_chain = dev->new_swap_chain(queue, window, SwapChainDesc({0, 0, 2, Format::bgra8_unorm, true})).get();

    Ref<ICommandBuffer> cmdbuf = dev->new_command_buffer(queue).get();

    Ref<VG::IFontAtlas> font_atlas = VG::new_font_atlas();
    font_atlas->set_font(Font::get_default_font(), 0);

    Ref<GUI::IContext> ctx = GUI::new_context(font_atlas);

    Ref<VG::IShapeDrawList> draw_list = VG::new_shape_draw_list(dev);

    Ref<VG::IShapeRenderer> renderer = VG::new_fill_shape_renderer();

    // Create back buffer.
    u32 w = 0, h = 0;

    while (true)
    {
        ctx->begin_input();
        poll_events();

        if (window->is_closed())
        {
            break;
        }
        if (window->is_minimized())
        {
            sleep(100);
            continue;
        }
        // Recreate the back buffer if needed.
        auto sz = window->get_framebuffer_size();
        auto ww = sz.x;
        auto wh = sz.y;
        if (ww != w || wh != h)
        {
            lupanic_if_failed(swap_chain->reset({ww, wh, 2, Format::unknown, true}));
            f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            w = ww;
            h = wh;
        }
        ctx->end_input();
        f32 dpi_scale_factor = window->get_dpi_scale_factor();
        UInt2U gui_size = UInt2U((u32)((f32)w / dpi_scale_factor), (u32)((f32)h / dpi_scale_factor));
        ctx->set_viewport_size(gui_size);
        ctx->begin_frame();

        if(GUI::begin(ctx, "Debug Window", RectF(50, 50, 200, 200), 
            GUI::WindowFlag::border | GUI::WindowFlag::movable | GUI::WindowFlag::closable))
        {
            GUI::text(ctx, "Sample Text");
        }
        GUI::end(ctx);

        ctx->render(draw_list);

        lupanic_if_failed(draw_list->compile());
        
        auto back_buffer = swap_chain->get_current_back_buffer().get();
        RenderPassDesc desc;
        desc.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, { 0.0f, 0.0f, 0.0f, 1.0f });
        cmdbuf->begin_render_pass(desc);
        cmdbuf->end_render_pass();

        lupanic_if_failed(renderer->set_render_target(back_buffer));

        Float4x4U projection = ProjectionMatrix::make_orthographic_off_center(0 , (f32)gui_size.x, 0, (f32)gui_size.y, 0, 1);

        lupanic_if_failed(renderer->render(cmdbuf, draw_list->get_vertex_buffer(), draw_list->get_index_buffer(), draw_list->get_draw_calls(), &projection));
        cmdbuf->resource_barrier({},
        {
            {swap_chain->get_current_back_buffer().get(), SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::present, ResourceBarrierFlag::none}
        });
        lupanic_if_failed(cmdbuf->submit({}, {}, true));
        cmdbuf->wait();
        lupanic_if_failed(swap_chain->present());
        lupanic_if_failed(cmdbuf->reset());
        draw_list->reset();
    }
}

int main()
{
    // Start modules.
    Luna::init();
    lupanic_if_failed(add_modules({module_window(), module_rhi(), module_gui(), module_font()}));
    auto res = Luna::init_modules();
    if (failed(res))
    {
        log_error("ImGuiTest", "Module init error: %s\n", explain(res.errcode()));
        lupanic();
        Luna::close();
        return 0;
    }
    run();
    Luna::close();
    return 0;
}
