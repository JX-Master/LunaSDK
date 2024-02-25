/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    Ref<Window::IWindow> g_window;

    u32 g_command_queue;

    Ref<RHI::ISwapChain> g_swap_chain;
    Ref<RHI::ICommandBuffer> g_command_buffer;

    Ref<VG::IFontAtlas> g_font_atlas;
    Ref<VG::ITextArranger> g_time_text_arranger;
    Ref<VG::ITextArranger> g_text_arranger;
    Ref<VG::IShapeDrawList> g_shape_draw_list;
    Ref<VG::IShapeRenderer> g_shape_renderer;

    Unconstructed<VG::TextArrangeResult> g_text_arrange_result;

    f32 g_font_size;
    f32 g_font_size_increment;

    u64 g_last_frame_ticks;

    f64 g_render_time;
    f64 g_frame_time;
}

using namespace Luna;

RV recreate_window_resources(u32 width, u32 height)
{
    using namespace RHI;
    lutry
    {
        if(width && height)
        {
            if (!g_swap_chain)
            {
                luset(g_swap_chain, get_main_device()->new_swap_chain(g_command_queue, g_window, SwapChainDesc({ width, height, 2, Format::bgra8_unorm, true})));
            }
            else
            {
                luexp(g_swap_chain->reset({width, height, 2, Format::bgra8_unorm, true}));
            }
        }
    }
    lucatchret;
    return ok;
}

const c8* sample_text = u8"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

const f32 min_size = 10.0f;
const f32 max_size = 300.0f;

void rearrange_text(const RectF& rect)
{
    g_text_arranger->clear_text_buffer();
    g_text_arranger->set_font_color(Color::to_rgba8(Color::white()));
    g_text_arranger->set_font_size(g_font_size);
    for (usize i = 0; i < 300; ++i)
    {
        g_text_arranger->add_text(sample_text);
    }
    g_text_arranger->add_text("\n");
    g_text_arrange_result.get() = g_text_arranger->arrange(rect, VG::TextAlignment::center, VG::TextAlignment::center);
}

constexpr u32 HEADER_TEXT_HEIGHT = 150;

void on_window_resize(Window::IWindow* window, u32 width, u32 height)
{
    lupanic_if_failed(recreate_window_resources(width, height));
    rearrange_text(RectF(0.0f, 0.0f, (f32)width, max((f32)(height - HEADER_TEXT_HEIGHT), 0.0f)));
}

void on_window_close(Window::IWindow* window)
{
    window->close();
}

void init()
{
    lupanic_if_failed(add_modules({module_window(), module_rhi(), module_font(), module_vg()}))
    lupanic_if_failed(init_modules());

    g_text_arrange_result.construct();

    // register event.
    g_window = Window::new_window("Luna Vector Graphics Test", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::resizable).get();
    auto sz = g_window->get_framebuffer_size();

    g_window->get_close_event().add_handler(on_window_close);
    g_window->get_framebuffer_resize_event().add_handler(on_window_resize);

    auto font = Font::get_default_font();

    g_shape_draw_list = VG::new_shape_draw_list();

    auto dev = RHI::get_main_device();

    g_command_queue = U32_MAX;
    u32 num_queues = dev->get_num_command_queues();
    for (u32 i = 0; i < num_queues; ++i)
    {
        auto desc = dev->get_command_queue_desc(i);
        if (desc.type == RHI::CommandQueueType::graphics)
        {
            g_command_queue = i;
            break;
        }
    }
    lupanic_if_failed(recreate_window_resources(sz.x, sz.y));
    g_shape_renderer = VG::new_fill_shape_renderer(g_swap_chain->get_current_back_buffer().get()).get();

    g_font_atlas = VG::new_font_atlas(font, 0);
    g_time_text_arranger = VG::new_text_arranger(g_font_atlas);
    g_text_arranger = VG::new_text_arranger(g_font_atlas);
    g_command_buffer = dev->new_command_buffer(g_command_queue).get();
    g_font_size = 30.0f;
    g_font_size_increment = 1;

    g_render_time = 0;
    g_frame_time = 0;

    auto size = g_window->get_framebuffer_size();

    rearrange_text(RectF(0.0f, 0.0f, (f32)size.x, max((f32)(size.y - HEADER_TEXT_HEIGHT), 0.0f)));
}

void run()
{
    while (true)
    {
        //new_frame();
        Window::poll_events();
        if (g_window->is_closed()) break;
        if (g_window->is_minimized())
        {
            sleep(100);
            continue;
        }
        auto sz = g_window->get_framebuffer_size();
        f64 time1 = ((f64)get_ticks() / get_ticks_per_second()) * 1000;
        c8 buf[64];

        g_time_text_arranger->set_font_size(50.0f);
        g_time_text_arranger->set_font_color(0xCCFFCCFF);
        g_time_text_arranger->add_text("FPS: ");
        sprintf(buf, "%f", 1000.0f / g_frame_time);
        g_time_text_arranger->add_text(buf);
        g_time_text_arranger->add_text("\n");

        auto res = g_time_text_arranger->arrange(RectF(0, sz.y - HEADER_TEXT_HEIGHT, sz.x, HEADER_TEXT_HEIGHT), VG::TextAlignment::center, VG::TextAlignment::center);
        if (!res.lines.empty())
        {
            lupanic_if_failed(g_time_text_arranger->commit(res, g_shape_draw_list));
            g_time_text_arranger->clear_text_buffer();
        }

        if (!g_text_arrange_result.get().lines.empty())
        {
            lupanic_if_failed(g_text_arranger->commit(g_text_arrange_result.get(), g_shape_draw_list));
        }

        lupanic_if_failed(g_shape_draw_list->close());

        RHI::RenderPassDesc desc;
        desc.color_attachments[0] = RHI::ColorAttachment(g_swap_chain->get_current_back_buffer().get(), RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
        g_command_buffer->begin_render_pass(desc);
        g_command_buffer->end_render_pass();

        auto dcs = g_shape_draw_list->get_draw_calls();

        lupanic_if_failed(g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get()));
        lupanic_if_failed(g_shape_renderer->render(g_command_buffer, g_shape_draw_list->get_vertex_buffer(), g_shape_draw_list->get_index_buffer(),  { dcs.data(), (u32)dcs.size() }));

        g_command_buffer->resource_barrier({},
            {
                {g_swap_chain->get_current_back_buffer().get(), RHI::SubresourceIndex(0, 0), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
            });

        lupanic_if_failed(g_command_buffer->submit({}, {}, true));
        g_command_buffer->wait();

        lupanic_if_failed(g_swap_chain->present());
        lupanic_if_failed(g_command_buffer->reset());
        g_shape_renderer->reset();
        g_shape_draw_list->reset();

        u64 frame_ticks = get_ticks();

        g_frame_time = f64(frame_ticks - g_last_frame_ticks) / get_ticks_per_second() * 1000.0;
        g_last_frame_ticks = frame_ticks;
    }
}
void shutdown()
{
    g_window = nullptr;

    g_swap_chain = nullptr;
    g_command_buffer = nullptr;

    g_font_atlas = nullptr;
    g_text_arranger = nullptr;
    g_time_text_arranger = nullptr;
    g_shape_draw_list = nullptr;
    g_shape_renderer = nullptr;

    g_text_arrange_result.destruct();
}
int main()
{
    Luna::init();
    ::init();
    run();
    ::shutdown();
    Luna::close();
}
