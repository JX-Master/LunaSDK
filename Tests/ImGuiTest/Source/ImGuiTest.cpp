/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGuiTest.cpp
* @author JXMaster
* @date 2020/4/12
*/
#include <Luna/ImGui/ImGui.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>

#include <Luna/Window/AppMain.hpp>

using namespace Luna;

void on_window_close(Window::IWindow* window)
{
    window->close();
}

namespace Luna
{
    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        u32 width = 0;
        u32 height = 0;
    };

    Window::AppResult app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppResult::failed;
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_imgui()}));
            luexp(init_modules());
            set_log_to_platform_enabled(true);
            using namespace RHI;
            using namespace Window;

            App* app = memnew<App>();
            *app_state = app;

            luset(app->window, new_window("ImGui Demo", WindowDisplaySettings::as_windowed(), WindowCreationFlag::resizable));
            app->window->get_close_event().add_handler(on_window_close);

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

            luset(app->swap_chain, dev->new_swap_chain(queue, app->window, SwapChainDesc({0, 0, 2, Format::bgra8_unorm, true})));
            luset(app->cmdbuf, dev->new_command_buffer(queue));

            // Create back buffer.
            u32 w = 0, h = 0;

            ImGuiUtils::set_active_window(app->window);
        }
        lucatch
        {
            log_error("ImGuiTest", "%s", explain(luerr));
            return Window::AppResult::failed;
        }
        return Window::AppResult::ok;
    }

    Window::AppResult app_update(opaque_t app_state)
    {
        lutry
        {
            App* app = (App*)app_state;
            using namespace RHI;
            if (app->window->is_closed()) return Window::AppResult::exiting;
            if (app->window->is_minimized())
            {
                sleep(100);
                return Window::AppResult::ok;
            }
            // Recreate the back buffer if needed.
            auto sz = app->window->get_framebuffer_size();
            auto ww = sz.x;
            auto wh = sz.y;
            if (ww != app->width || wh != app->height)
            {
                luexp(app->swap_chain->reset({ww, wh, 2, Format::unknown, true}));
                f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                app->width = ww;
                app->height = wh;
            }

            ImGuiUtils::update_io();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            
            auto window_size = app->window->get_size();
            ImGui::Text("Window Size: %ux%u", window_size.x, window_size.y);
            ImGui::Text("Framebuffer Size: %ux%u", sz.x, sz.y);
            ImGui::Text("DPI Scale: %f", app->window->get_dpi_scale_factor());

            ImGui::Render();
            
            auto back_buffer = app->swap_chain->get_current_back_buffer().get();
            RenderPassDesc desc;
            desc.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, { 0.0f, 0.0f, 0.0f, 1.0f });
            app->cmdbuf->begin_render_pass(desc);
            app->cmdbuf->end_render_pass();
            luexp(ImGuiUtils::render_draw_data(ImGui::GetDrawData(), app->cmdbuf, back_buffer));
            app->cmdbuf->resource_barrier({},
            {
                {app->swap_chain->get_current_back_buffer().get(), SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::present, ResourceBarrierFlag::none}
            });
            luexp(app->cmdbuf->submit({}, {}, true));
            app->cmdbuf->wait();
            luexp(app->swap_chain->present());
            luexp(app->cmdbuf->reset());
        }
        lucatch
        {
            log_error("ImGuiTest", "%s", explain(luerr));
            return Window::AppResult::failed;
        }
        return Window::AppResult::ok;
    }

    void app_close(opaque_t app_state)
    {
        App* app = (App*)app_state;
        memdelete(app);
        Luna::close();
    }
}