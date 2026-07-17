/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file InteractiveGUIDemo.hpp
* @author JXMaster
* @date 2026/6/19
*/
#pragma once
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>

namespace Luna
{
    namespace Test
    {
        struct InteractiveGUIDemoApp
        {
            Ref<Window::IWindow> window;
            Ref<RHI::ISwapChain> swap_chain;
            Ref<RHI::ICommandBuffer> cmdbuf;
            Ref<GUICore::IContext> gui;
            Ref<GUICore::IRenderer> renderer;
            u32 queue = U32_MAX;
            u32 width = 0;
            u32 height = 0;
        };

        using DemoInitFunc = RV(*)(InteractiveGUIDemoApp& app, void* userdata);
        using DemoBuildFunc = void(*)(GUICore::IContext* context, const GUICore::ElementHandle& root,
            const Float2U& surface_size, void* userdata);

        struct InteractiveGUIDemoDesc
        {
            const c8* title = "Luna GUI Demo";
            DemoInitFunc init = nullptr;
            DemoBuildFunc build = nullptr;
            void* userdata = nullptr;
        };

        inline GUICore::id_t hash_bytes(const void* data, usize size, u64 h = 14695981039346656037ull)
        {
            const byte_t* bytes = (const byte_t*)data;
            for(usize i = 0; i < size; ++i)
            {
                h ^= (u64)bytes[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        inline GUICore::id_t hash_cstr(const c8* text, u64 h = 14695981039346656037ull)
        {
            if(!text)
            {
                return h;
            }
            while(*text)
            {
                h ^= (u64)(byte_t)*text;
                h *= 1099511628211ull;
                ++text;
            }
            return h;
        }

        inline GUICore::id_t demo_id(const c8* scope, u64 value = 0)
        {
            u64 h = hash_cstr(scope);
            return hash_bytes(&value, sizeof(value), h);
        }

        inline GUICore::LayoutConfig fixed_layout(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        inline GUICore::LayoutConfig fill_layout()
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::percent;
            layout.height.value = 1.0f;
            layout.flex_grow = 1.0f;
            return layout;
        }

        inline GUICore::LayoutConfig fill_width_layout(f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        inline void set_rect(GUICore::IContext* context, const GUICore::ElementHandle& element, const RectF& rect)
        {
            GUICore::LayoutResult layout;
            layout.rect = rect;
            layout.clip_rect = rect;
            layout.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(element, layout);
        }

        inline GUICore::ElementHandle label_value(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const c8* value)
        {
            GUICore::ElementHandle row = GUI::begin_h_layout(context, id, label, fill_width_layout(28.0f));
            GUI::text(context, id + 1, label, fixed_layout(180.0f, 24.0f));
            GUI::text(context, id + 2, value, fill_width_layout(24.0f));
            GUICore::FlexLayoutDesc desc;
            desc.axis = GUICore::LayoutAxis::x;
            desc.main_axis_gap = 8.0f;
            GUI::end_h_layout(context, row, desc);
            return row;
        }

        inline RV init_interactive_gui_demo(InteractiveGUIDemoApp& app, const c8* title)
        {
            lutry
            {
                luexp(add_modules({
                    module_window(),
                    module_rhi(),
                    module_font(),
                    module_vg(),
                    GUICore::module_gui_core(),
                    GUI::module_gui(),
                    GUIWindow::module_gui_window()
                }));
                luexp(init_modules());

                luset(app.window, Window::new_window(title));
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
                GUI::register_style_schemas(app.gui);
                luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            }
            lucatchret;
            return ok;
        }

        inline RV render_interactive_gui_demo(InteractiveGUIDemoApp& app, RHI::ITexture* back_buffer)
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

        inline RV run_interactive_gui_demo(const InteractiveGUIDemoDesc& desc)
        {
            lutry
            {
                lucheck_msg(desc.build, "Interactive GUI demo build callback is null.");
                InteractiveGUIDemoApp app;
                luexp(init_interactive_gui_demo(app, desc.title));
                if(desc.init)
                {
                    luexp(desc.init(app, desc.userdata));
                }

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
                    app.gui->push_layer(1, Float2U(0.0f));
                    GUICore::ElementHandle root = app.gui->begin_element(1);
                    desc.build(app.gui, root, frame.screen_size, desc.userdata);
                    app.gui->end_element();
                    app.gui->pop_layer();
                    luexp(GUI::layout_tree(app.gui, root, RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                    app.gui->route_input();
                    GUI::ResolveResult resolved = GUI::resolve_interactions(app.gui);
                    if(resolved.relayout_requested)
                    {
                        luexp(GUI::layout_tree(app.gui, root,
                            RectF(0.0f, 0.0f, frame.screen_size.x, frame.screen_size.y)));
                    }
                    luexp(GUIWindow::update_text_input(&input_adapter));

                    lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                    RHI::RenderPassDesc render_pass;
                    render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store,
                        Float4U(0.02f, 0.025f, 0.03f, 1.0f));
                    app.cmdbuf->begin_render_pass(render_pass);
                    app.cmdbuf->end_render_pass();
                    luexp(render_interactive_gui_demo(app, back_buffer));
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
}
