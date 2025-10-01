/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Thread.hpp>

#include <Luna/Window/AppMain.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

RV start()
{

    return ok;
}

void draw()
{
    auto cb = get_command_buffer();
    cb->resource_barrier({},
        {
            {get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
        });
    RenderPassDesc render_pass;
    render_pass.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, Color::blue_violet());
    cb->begin_render_pass(render_pass);
    cb->end_render_pass();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
}

namespace Luna
{
    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppStatus::failing;
        lutry
        {
            luexp(add_modules({module_rhi_test_bed()}));
            luexp(init_modules());
            register_init_func(start);
            register_close_func(cleanup);
            register_resize_func(resize);
            register_draw_func(draw);
            luexp(RHITestBed::init());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    Window::AppStatus app_update(opaque_t app_state)
    {
        auto window = RHITestBed::get_window();
        if(window->is_closed()) return Window::AppStatus::exiting;
        if(window->is_minimized())
        {
            sleep(100);
            return Window::AppStatus::running;
        }
        lutry
        {
            luexp(RHITestBed::update());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    void app_close(opaque_t app_state, Window::AppStatus status)
    {
        RHITestBed::close();
        Luna::close();
    }
}