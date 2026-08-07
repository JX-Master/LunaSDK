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
#include <Luna/EditorGUI/EditorGUI.hpp>
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
            Ref<GUI::IContext> gui;
            Ref<GUI::IRenderer> renderer;
            u32 queue = U32_MAX;
            u32 width = 0;
            u32 height = 0;
        };

        using DemoInitFunc = RV(*)(InteractiveGUIDemoApp& app, void* userdata);
        using DemoBuildFunc = void(*)(GUI::IContext* context, const GUI::ElementHandle& root,
            const Float2U& surface_size, void* userdata);

        struct InteractiveGUIDemoDesc
        {
            const c8* title = "Luna GUI Demo";
            DemoInitFunc init = nullptr;
            DemoBuildFunc build = nullptr;
            void* userdata = nullptr;
        };

        inline GUI::id_t hash_bytes(const void* data, usize size, u64 h = 14695981039346656037ull)
        {
            const byte_t* bytes = (const byte_t*)data;
            for(usize i = 0; i < size; ++i)
            {
                h ^= (u64)bytes[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        inline GUI::id_t hash_cstr(const c8* text, u64 h = 14695981039346656037ull)
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

        inline GUI::id_t demo_id(const c8* scope, u64 value = 0)
        {
            u64 h = hash_cstr(scope);
            return hash_bytes(&value, sizeof(value), h);
        }

        inline GUI::LayoutConfig fixed_layout(f32 width, f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::fixed;
            layout.width.value = width;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        inline GUI::LayoutConfig fill_layout()
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::percent;
            layout.height.value = 1.0f;
            layout.flex_grow = 1.0f;
            return layout;
        }

        inline GUI::LayoutConfig fill_width_layout(f32 height)
        {
            GUI::LayoutConfig layout;
            layout.width.kind = GUI::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUI::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        inline void set_rect(GUI::IContext* context, const GUI::ElementHandle& element, const RectF& rect)
        {
            GUI::LayoutResult layout;
            layout.rect = rect;
            layout.clip_rect = rect;
            layout.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(element, layout);
        }

        inline GUI::ElementHandle label_value(GUI::IContext* context, GUI::id_t id,
            const c8* label, const c8* value)
        {
            GUI::ElementHandle row = EditorGUI::begin_h_layout(context, id, label, fill_width_layout(28.0f));
            EditorGUI::text(context, id + 1, label, fixed_layout(180.0f, 24.0f));
            EditorGUI::text(context, id + 2, value, fill_width_layout(24.0f));
            GUI::FlexLayoutDesc desc;
            desc.axis = GUI::LayoutAxis::x;
            desc.main_axis_gap = 8.0f;
            EditorGUI::end_h_layout(context, row, desc);
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
                    GUI::module_gui(),
                    EditorGUI::module_editor_gui(),
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
                luset(app.renderer, GUI::new_renderer(dev));
                app.gui = GUI::new_context();
                EditorGUI::register_style_schemas(app.gui);
                luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
                EditorGUI::DefaultStyleDesc style_desc;
                style_desc.input_mode = EditorGUI::InputMode::pointer;
                EditorGUI::set_default_style(app.gui, style_desc);
            }
            lucatchret;
            return ok;
        }

        inline RV render_interactive_gui_demo(InteractiveGUIDemoApp& app, RHI::ITexture* back_buffer,
            const Float4U& clear_color)
        {
            lutry
            {
                GUI::RenderTargetDesc target(back_buffer);
                target.color_load_op = RHI::LoadOp::clear;
                target.color_clear_value = clear_color;
                target.color_final_state = RHI::TextureStateFlag::present;
                luexp(app.renderer->render(app.gui, app.cmdbuf, target));
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
                        app.width = fb_sz.x;
                        app.height = fb_sz.y;
                    }

                    auto logical_sz = app.window->get_size();
                    GUI::FrameDesc frame;
                    frame.logical_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                    frame.render_size = fb_sz;
                    frame.delta_time = 1.0f / 60.0f;
                    app.gui->begin_frame(frame);
                    GUIWindow::update_input(&input_adapter);
                    app.gui->push_layer(1, Float2U(0.0f));
                    GUI::ElementHandle root = app.gui->begin_element(1);
                    desc.build(app.gui, root, frame.logical_size, desc.userdata);
                    app.gui->end_element();
                    app.gui->pop_layer();
                    luexp(EditorGUI::layout_tree(app.gui, root,
                        RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                    app.gui->route_input();
                    EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(app.gui);
                    if(resolved.relayout_requested)
                    {
                        luexp(EditorGUI::layout_tree(app.gui, root,
                            RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                    }
                    luexp(GUIWindow::update_text_input(&input_adapter));

                    lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                    Float4U clear_color = app.gui->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name("gui.canvas"),
                        GUI::style_f32x4(Float4U(0.92f, 0.93f, 0.92f, 1.0f))).number;
                    luexp(render_interactive_gui_demo(app, back_buffer, clear_color));
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
