/*!
* This file is a portion of LunaSDK.
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
#include <Luna/Runtime/Log.hpp>
#include <Luna/Window/Event.hpp>

#include <Luna/Window/AppMain.hpp>

namespace Luna
{
    const c8* SAMPLE_TEXT = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

    const f32 MIN_SIZE = 10.0f;
    const f32 MAX_SIZE = 300.0f;
    constexpr u32 HEADER_TEXT_HEIGHT = 150;

    struct App
    {
        Ref<Window::IWindow> window;

        u32 command_queue;
    
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> command_buffer;
    
        Ref<VG::IFontAtlas> font_atlas;
        Vector<VG::TextArrangeSection> text_sections;
        Ref<VG::IShapeDrawList> shape_draw_list;
        Ref<VG::IShapeRenderer> shape_renderer;
    
        VG::TextArrangeResult text_arrange_result;
    
        f32 font_size;
        f32 font_size_increment;
    
        u64 last_frame_ticks;
    
        f64 render_time;
        f64 frame_time;

        RV recreate_window_resources(u32 width, u32 height)
        {
            using namespace RHI;
            lutry
            {
                if(width && height)
                {
                    if (!swap_chain)
                    {
                        luset(swap_chain, get_main_device()->new_swap_chain(command_queue, window, SwapChainDesc({ width, height, 2, Format::bgra8_unorm, true})));
                    }
                    else
                    {
                        luexp(swap_chain->reset({width, height, 2, Format::bgra8_unorm, true}));
                    }
                }
            }
            lucatchret;
            return ok;
        }

        void rearrange_text(const RectF& rect)
        {
            String text;
            for (usize i = 0; i < 300; ++i)
            {
                text.append(SAMPLE_TEXT);
            }
            text.push_back('\n');
            text_sections.clear();
            VG::TextArrangeSection section;
            section.color = Color::white();
            section.font_size = font_size;
            section.num_chars = text.size();
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            text_sections.push_back(section);
            text_arrange_result = VG::arrange_text(
                text.c_str(), text.size(), text_sections.cspan(),
                rect, VG::TextAlignment::center, VG::TextAlignment::center);
        }
    };

    void on_window_resize(App* app, Window::IWindow* window, u32 width, u32 height)
    {
        
    }
}

using namespace Luna;

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    lutry
    {
        luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg()}))
        luexp(init_modules());
        App app;
        // register event.
        luset(app.window, Window::new_window("Luna Vector Graphics Test"));
        auto sz = app.window->get_framebuffer_size();

        Window::set_event_handler([](object_t event, void* userdata){
            if(auto e = cast_object<Window::WindowFramebufferResizeEvent>(event))
            {
                App* app = (App*)userdata;
                lupanic_if_failed(app->recreate_window_resources(e->width, e->height));
                app->rearrange_text(RectF(0.0f, 0.0f, (f32)e->width, max((f32)(e->height - HEADER_TEXT_HEIGHT), 0.0f)));
            }
        }, &app);

        auto font = Font::get_default_font();

        app.shape_draw_list = VG::new_shape_draw_list();

        auto dev = RHI::get_main_device();

        app.command_queue = U32_MAX;
        u32 num_queues = dev->get_num_command_queues();
        for (u32 i = 0; i < num_queues; ++i)
        {
            auto desc = dev->get_command_queue_desc(i);
            if (desc.type == RHI::CommandQueueType::graphics)
            {
                app.command_queue = i;
                break;
            }
        }
        luexp(app.recreate_window_resources(sz.x, sz.y));
        app.shape_renderer = VG::new_fill_shape_renderer();

        app.font_atlas = VG::new_font_atlas();
        
        luset(app.command_buffer, dev->new_command_buffer(app.command_queue));
        app.font_size = 30.0f;
        app.font_size_increment = 1;

        app.render_time = 0;
        app.frame_time = 0;

        auto size = app.window->get_framebuffer_size();

        app.rearrange_text(RectF(0.0f, 0.0f, (f32)size.x, max((f32)(size.y - HEADER_TEXT_HEIGHT), 0.0f)));

        while(true)
        {
            Window::poll_events();
            if (app.window->is_closed()) break;
            if (app.window->is_minimized())
            {
                sleep(100);
                continue;
            }
            auto sz = app.window->get_framebuffer_size();
            f64 time1 = ((f64)get_ticks() / get_ticks_per_second()) * 1000;
            c8 buf[64];

            VG::TextArrangeSection section;
            section.font_size = 50.0f;
            section.color = Float4U(0.8f, 1.0f, 0.8f, 1.0f);
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            String fps_text;
            fps_text.append("FPS: ");
            snprintf(buf, 64, "%f", 1000.0f / app.frame_time);
            fps_text.append(buf);
            fps_text.push_back('\n');
            section.num_chars = fps_text.size();

            auto res = VG::arrange_text(
                fps_text.c_str(), fps_text.size(), {&section, 1},
                RectF(0, sz.y - HEADER_TEXT_HEIGHT, sz.x, HEADER_TEXT_HEIGHT), VG::TextAlignment::center, VG::TextAlignment::center);
            if (!res.lines.empty())
            {
                VG::commit_text_arrange_result(res, {&section, 1}, app.font_atlas, app.shape_draw_list);
            }

            if (!app.text_arrange_result.lines.empty())
            {
                VG::commit_text_arrange_result(app.text_arrange_result, app.text_sections.cspan(), app.font_atlas, app.shape_draw_list);
            }

            luexp(app.shape_draw_list->compile());

            RHI::RenderPassDesc desc;
            lulet(back_buffer, app.swap_chain->get_current_back_buffer());
            desc.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
            app.command_buffer->begin_render_pass(desc);
            app.command_buffer->end_render_pass();

            luexp(app.shape_renderer->begin(back_buffer));
            app.shape_renderer->draw(app.shape_draw_list->get_vertex_buffer(), app.shape_draw_list->get_index_buffer(),  app.shape_draw_list->get_draw_calls());
            luexp(app.shape_renderer->end());
            app.shape_renderer->submit(app.command_buffer);

            app.command_buffer->resource_barrier({},
                {
                    {back_buffer, RHI::SubresourceIndex(0, 0), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                });

            luexp(app.command_buffer->submit({}, {}, true));
            app.command_buffer->wait();

            luexp(app.swap_chain->present());
            luexp(app.command_buffer->reset());
            app.shape_draw_list->reset();

            u64 frame_ticks = get_ticks();

            app.frame_time = f64(frame_ticks - app.last_frame_ticks) / get_ticks_per_second() * 1000.0;
            app.last_frame_ticks = frame_ticks;
        }
    }
    lucatch
    {
        log_error("FontArrangeTest", "%s", explain(luerr));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}