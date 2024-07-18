#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/GUI/Widgets.hpp>
#include <Luna/GUI/Context.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/GUI/WidgetBuilder.hpp>
#include <Luna/GUI/Widgets.hpp>

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
    Ref<IWindow> window = new_window("GUITest", WindowDisplaySettings::as_windowed(), WindowCreationFlag::resizable).get();
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

    Ref<GUI::IContext> ctx = GUI::new_context();

    Ref<GUI::IWidgetBuilder> builder = GUI::new_widget_builder();

    Ref<VG::IShapeDrawList> draw_list = VG::new_shape_draw_list(dev);

    Ref<GUI::IDrawList> gui_draw_list = GUI::new_draw_list();

    Ref<VG::IShapeRenderer> renderer = VG::new_fill_shape_renderer();

    // Create back buffer.
    u32 w = 0, h = 0;

    while (true)
    {
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
        auto& io = ctx->get_io();
        auto gui_size = window->get_size();
        io.width = gui_size.x;
        io.height = gui_size.y;
        builder->reset();
        begin_canvas(builder);
        {
            using namespace GUI;
            for(u32 y = 0; y < 4; ++y)
            {
                for (u32 x = 0; x < 4; ++x)
                {
                    begin_canvas(builder);
                    set_anthor(builder, ((f32)x) / 4, ((f32)y) / 4, ((f32)x + 1) / 4, ((f32)y + 1) / 4);
                    {
                        // background.
                        rectangle(builder);
                        set_anthor(builder, 0, 0, 1, 1);
                        Float4 color = Float4(((f32)x) / 3, ((f32)y) / 3, 0.0f, 1.0f);
                        set_vattr(builder, VATTR_BACKGROUND_COLOR, color);
                        // Text canvas.
                        begin_canvas(builder);
                        f32 anchor_x_min, anchor_x_max, anchor_y_min, anchor_y_max;
                        f32 rect_x_min, rect_x_max, rect_y_min, rect_y_max;
                        switch(x)
                        {
                            case 0: anchor_x_min = 0.0f; anchor_x_max = 0.0f; rect_x_min = 10.0f; rect_x_max = 50.0f; break;
                            case 1: anchor_x_min = 0.5f; anchor_x_max = 0.5f; rect_x_min = -20.0f; rect_x_max = 20.0f; break;
                            case 2: anchor_x_min = 1.0f; anchor_x_max = 1.0f; rect_x_min = -50.0f; rect_x_max = -10.0f; break;
                            case 3: anchor_x_min = 0.0f; anchor_x_max = 1.0f; rect_x_min = 10.0f; rect_x_max = -10.0f; break;
                            default: break;
                        }
                        switch(y)
                        {
                            case 0: anchor_y_min = 0.0f; anchor_y_max = 0.0f; rect_y_min = 10.0f; rect_y_max = 50.0f; break;
                            case 1: anchor_y_min = 0.5f; anchor_y_max = 0.5f; rect_y_min = -20.0f; rect_y_max = 20.0f; break;
                            case 2: anchor_y_min = 1.0f; anchor_y_max = 1.0f; rect_y_min = -50.0f; rect_y_max = -10.0f; break;
                            case 3: anchor_y_min = 0.0f; anchor_y_max = 1.0f; rect_y_min = 10.0f; rect_y_max = -10.0f; break;
                            default: break;
                        }
                        set_anthor(builder, anchor_x_min, anchor_y_min, anchor_x_max, anchor_y_max);
                        set_offset(builder, rect_x_min, rect_y_min, rect_x_max, rect_y_max);
                        {
                            // Text background.
                            rectangle(builder);
                            set_anthor(builder, 0, 0, 1, 1);
                            color.z = 1.0f;
                            set_vattr(builder, VATTR_BACKGROUND_COLOR, color);

                            text(builder, "Text");
                            set_anthor(builder, 0, 0, 1, 1);
                            set_sattr(builder, SATTR_TEXT_SIZE, 32);
                        }
                        end_canvas(builder);
                    }
                    end_canvas(builder);
                }
            }
        }
        end_canvas(builder);
        ctx->set_widget(builder->get_root_widget());
        lupanic_if_failed(ctx->update());
        gui_draw_list->begin(draw_list);
        lupanic_if_failed(ctx->render(gui_draw_list));
        gui_draw_list->end();
        lupanic_if_failed(draw_list->compile());
        
        auto back_buffer = swap_chain->get_current_back_buffer().get();
        lupanic_if_failed(renderer->set_render_target(back_buffer));
        RenderPassDesc desc;
        desc.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, { 0.0f, 0.0f, 0.0f, 1.0f });
        cmdbuf->begin_render_pass(desc);
        cmdbuf->end_render_pass();

        Float4x4U projection = ProjectionMatrix::make_orthographic_off_center(0 , (f32)gui_size.x, 0, (f32)gui_size.y, 0, 1);
        lupanic_if_failed(renderer->render(cmdbuf, draw_list->get_vertex_buffer(), draw_list->get_index_buffer(), draw_list->get_draw_calls(), &projection));
        cmdbuf->resource_barrier({}, {TextureBarrier(back_buffer, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::present)});
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
    lupanic_if_failed(add_modules({module_window(), module_rhi(), module_gui()}));
    auto res = Luna::init_modules();
    if (failed(res))
    {
        log_error("GUITest", "Module init error: %s\n", explain(res.errcode()));
        lupanic();
        Luna::close();
        return 0;
    }
    run();
    Luna::close();
    return 0;
}