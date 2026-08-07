/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file main.cpp
* @author JXMaster
* @date 2026/6/4
*/
#include <Luna/Font/Font.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Window/Event.hpp>
#include <cstdio>

using namespace Luna;

namespace Luna
{
    constexpr u32 NUM_CURVE_POINTS = 10;
    constexpr u32 NUM_RESOLUTIONS = 5;
    constexpr f32 TARGET_DISPLAY_SIZE = 768.0f;
    constexpr f32 CONTROL_LINE_WIDTH = 2.0f;
    constexpr f32 CONTROL_POINT_RADIUS = 8.0f;
    const c8* RESOLUTION_LABELS[NUM_RESOLUTIONS] = { "64", "128", "256", "512", "1024" };
    constexpr u32 RESOLUTION_VALUES[NUM_RESOLUTIONS] = { 64, 128, 256, 512, 1024 };
    const c8* POINT_LABELS[NUM_CURVE_POINTS] =
    {
        "P0",
        "Q0",
        "P1",
        "C0",
        "C1",
        "P2",
        "Q1",
        "P3",
        "C2",
        "C3"
    };

    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUI::IContext> gui;
        Ref<RHI::ITexture> curve_texture;
        Ref<VG::IShapeDrawList> shape_draw_list;
        Ref<VG::IShapeRenderer> shape_renderer;
        Ref<GUI::IRenderer> gui_renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        i32 resolution_index = 1;
        i32 dragging_point = -1;
        bool was_left_down = false;
        f32 display_size = 512.0f;
        RectF image_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        Float2U control_points[NUM_CURVE_POINTS] =
        {
            Float2U(0.16f, 0.48f),
            Float2U(0.28f, 0.08f),
            Float2U(0.52f, 0.18f),
            Float2U(0.84f, 0.04f),
            Float2U(0.88f, 0.52f),
            Float2U(0.62f, 0.66f),
            Float2U(0.46f, 0.94f),
            Float2U(0.24f, 0.74f),
            Float2U(0.02f, 0.68f),
            Float2U(0.02f, 0.26f)
        };
    };

    constexpr GUI::id_t DEFAULT_LAYER_ID = 1;
    constexpr GUI::id_t ROOT_ID = 2;
    constexpr GUI::id_t IMAGE_HIT_ID = 100;
    constexpr GUI::id_t RESOLUTION_GROUP_ID = 101;
    constexpr GUI::id_t FIRST_TEXT_ID = 1000;

    inline GUI::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUI::LayoutConfig layout;
        layout.width.kind = GUI::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUI::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    inline void set_element_rect(GUI::IContext* context, const GUI::ElementHandle& element, const RectF& rect)
    {
        GUI::LayoutResult layout;
        layout.rect = rect;
        layout.clip_rect = rect;
        layout.content_size = Float2U(rect.width, rect.height);
        context->set_layout_result(element, layout);
    }

    inline Float2U rect_min(const RectF& rect)
    {
        return Float2U(rect.offset_x, rect.offset_y);
    }

    inline Float2U rect_max(const RectF& rect)
    {
        return Float2U(rect.offset_x + rect.width, rect.offset_y + rect.height);
    }

    inline bool point_in_rect(const Float2U& point, const RectF& rect)
    {
        return in_bounds(point, rect_min(rect), rect_max(rect));
    }

    void draw_line(GUI::IContext* context, const Float2U& begin, const Float2U& end,
        const Float4U& color, f32 width)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::line;
        command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
        command.point1 = end;
        command.color = color;
        command.line_width = width;
        context->draw(command);
    }

    void draw_circle(GUI::IContext* context, const Float2U& center, f32 radius,
        const Float4U& color)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::rounded_rect;
        command.rect = RectF(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
        command.color = color;
        command.radius = radius;
        context->draw(command);
    }

    inline Float2U curve_point_to_texture(const App& app, const Float2U& point)
    {
        f32 size = (f32)RESOLUTION_VALUES[app.resolution_index];
        return Float2U(point.x * size, (1.0f - point.y) * size);
    }

    inline Float2U curve_point_to_screen(const App& app, const Float2U& point)
    {
        return Float2U(
            app.image_rect.offset_x + point.x * app.image_rect.width,
            app.image_rect.offset_y + point.y * app.image_rect.height);
    }

    inline Float2U curve_point_to_rect(const RectF& rect, const Float2U& point)
    {
        return Float2U(
            rect.offset_x + point.x * rect.width,
            rect.offset_y + point.y * rect.height);
    }

    void append_shape(App& app, u32 begin, u32 end, const Float4U& color)
    {
        f32 size = (f32)RESOLUTION_VALUES[app.resolution_index];
        app.shape_draw_list->draw_shape(begin, end - begin,
            Float2U(0.0f, 0.0f), Float2U(size, size),
            Float2U(0.0f, 0.0f), Float2U(size, size),
            color);
    }

    RV recreate_curve_texture(App& app)
    {
        using namespace RHI;
        lutry
        {
            u32 size = RESOLUTION_VALUES[app.resolution_index];
            TextureDesc desc = TextureDesc::tex2d(Format::rgba8_unorm,
                TextureUsageFlag::color_attachment | TextureUsageFlag::read_texture,
                size, size, 1, 1);
            luset(app.curve_texture, RHI::get_main_device()->new_texture(MemoryType::local, desc));
        }
        lucatchret;
        return ok;
    }

    RV draw_curve_texture(App& app)
    {
        using namespace RHI;
        lutry
        {
            RenderPassDesc clear_pass;
            clear_pass.color_attachments[0] = ColorAttachment(app.curve_texture, LoadOp::clear, StoreOp::store, Float4U(0.025f, 0.03f, 0.038f, 1.0f));
            app.cmdbuf->begin_render_pass(clear_pass);
            app.cmdbuf->end_render_pass();

            app.shape_draw_list->reset();
            app.shape_draw_list->set_shape_buffer(nullptr);
            auto& points = app.shape_draw_list->get_shape_buffer()->get_shape_points(true);

            Float2U p0 = curve_point_to_texture(app, app.control_points[0]);
            Float2U q0 = curve_point_to_texture(app, app.control_points[1]);
            Float2U p1 = curve_point_to_texture(app, app.control_points[2]);
            Float2U c0 = curve_point_to_texture(app, app.control_points[3]);
            Float2U c1 = curve_point_to_texture(app, app.control_points[4]);
            Float2U p2 = curve_point_to_texture(app, app.control_points[5]);
            Float2U q1 = curve_point_to_texture(app, app.control_points[6]);
            Float2U p3 = curve_point_to_texture(app, app.control_points[7]);
            Float2U c2 = curve_point_to_texture(app, app.control_points[8]);
            Float2U c3 = curve_point_to_texture(app, app.control_points[9]);

            u32 offset = (u32)points.size();
            VG::ShapeBuilder::move_to(points, p0.x, p0.y);
            VG::ShapeBuilder::curve_to(points, q0.x, q0.y, p1.x, p1.y);
            VG::ShapeBuilder::cubic_to(points, c0.x, c0.y, c1.x, c1.y, p2.x, p2.y);
            VG::ShapeBuilder::curve_to(points, q1.x, q1.y, p3.x, p3.y);
            VG::ShapeBuilder::cubic_to(points, c2.x, c2.y, c3.x, c3.y, p0.x, p0.y);
            append_shape(app, offset, (u32)points.size(), Float4U(0.24f, 0.56f, 0.95f, 1.0f));

            luexp(app.shape_draw_list->compile());
            luexp(app.shape_renderer->begin(app.curve_texture));
            app.shape_renderer->draw(app.shape_draw_list->get_instance_buffer(),
                app.shape_draw_list->get_state_buffer(),
                app.shape_draw_list->get_draw_calls(),
                nullptr);
            luexp(app.shape_renderer->end());
            app.shape_renderer->prepare(app.cmdbuf);
            RenderPassDesc render_pass;
            render_pass.color_attachments[0] = ColorAttachment(app.curve_texture, LoadOp::load, StoreOp::store);
            app.cmdbuf->begin_render_pass(render_pass);
            app.shape_renderer->submit(app.cmdbuf);
            app.cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }

    void draw_curve_overlay(App& app, const RectF& rect)
    {
        Float2U p0 = curve_point_to_rect(rect, app.control_points[0]);
        Float2U q0 = curve_point_to_rect(rect, app.control_points[1]);
        Float2U p1 = curve_point_to_rect(rect, app.control_points[2]);
        Float2U c0 = curve_point_to_rect(rect, app.control_points[3]);
        Float2U c1 = curve_point_to_rect(rect, app.control_points[4]);
        Float2U p2 = curve_point_to_rect(rect, app.control_points[5]);
        Float2U q1 = curve_point_to_rect(rect, app.control_points[6]);
        Float2U p3 = curve_point_to_rect(rect, app.control_points[7]);
        Float2U c2 = curve_point_to_rect(rect, app.control_points[8]);
        Float2U c3 = curve_point_to_rect(rect, app.control_points[9]);
        Float4U quad_line_color(0.95f, 0.95f, 1.0f, 0.48f);
        Float4U cubic_line_color(0.95f, 0.80f, 0.30f, 0.48f);
        draw_line(app.gui, p0, q0, quad_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, q0, p1, quad_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, p1, c0, cubic_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, c0, c1, cubic_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, c1, p2, cubic_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, p2, q1, quad_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, q1, p3, quad_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, p3, c2, cubic_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, c2, c3, cubic_line_color, CONTROL_LINE_WIDTH);
        draw_line(app.gui, c3, p0, cubic_line_color, CONTROL_LINE_WIDTH);
        for(u32 i = 0; i < NUM_CURVE_POINTS; ++i)
        {
            Float4U color = (i == 0 || i == 2 || i == 5 || i == 7) ?
                Float4U(1.0f, 1.0f, 1.0f, 1.0f) :
                Float4U(1.0f, 0.78f, 0.18f, 1.0f);
            draw_circle(app.gui, curve_point_to_rect(rect, app.control_points[i]), CONTROL_POINT_RADIUS, color);
        }
    }

    void draw_label(GUI::IContext* context, GUI::id_t id, f32 x, f32 y, f32 width, const c8* text)
    {
        EditorGUI::TextDesc desc;
        desc.font_size = 16.0f;
        GUI::ElementHandle element = EditorGUI::text(context, id, text, fixed_layout(width, 24.0f), desc);
        set_element_rect(context, element, RectF(x, y, width, 24.0f));
    }

    void build_gui(App& app, const Float2U& surface_size)
    {
        GUI::ElementHandle root = app.gui->begin_element(ROOT_ID);
        set_element_rect(app.gui, root, RectF(0.0f, 0.0f, surface_size.x, surface_size.y));

        draw_label(app.gui, FIRST_TEXT_ID, 14.0f, 14.0f, 280.0f, "VG Cubic Curve Test");
        draw_label(app.gui, FIRST_TEXT_ID + 1, 14.0f, 62.0f, 220.0f, "Texture resolution");
        GUI::ElementHandle resolution = EditorGUI::button_group(app.gui, RESOLUTION_GROUP_ID,
            Span<const c8*>(RESOLUTION_LABELS, NUM_RESOLUTIONS), &app.resolution_index, fixed_layout(420.0f, 34.0f));
        set_element_rect(app.gui, resolution, RectF(220.0f, 54.0f, 420.0f, 34.0f));
        c8 status[128];
        snprintf(status, 128, "Actual render target: %ux%u", RESOLUTION_VALUES[app.resolution_index], RESOLUTION_VALUES[app.resolution_index]);
        draw_label(app.gui, FIRST_TEXT_ID + 2, 654.0f, 60.0f, 320.0f, status);

        f32 display_limit_x = max(surface_size.x - 360.0f, 220.0f);
        f32 display_limit_y = max(surface_size.y - 132.0f, 220.0f);
        app.display_size = min(TARGET_DISPLAY_SIZE, min(display_limit_x, display_limit_y));
        app.image_rect = RectF(14.0f, 112.0f, app.display_size, app.display_size);

        EditorGUI::ImageDesc image_desc;
        image_desc.flags = EditorGUI::ImageFlag::flip_y | EditorGUI::ImageFlag::nearest;
        GUI::ElementHandle image = EditorGUI::image(app.gui, FIRST_TEXT_ID + 3, app.curve_texture,
            fixed_layout(app.display_size, app.display_size), image_desc);
        set_element_rect(app.gui, image, app.image_rect);
        GUI::ElementHandle image_hit = EditorGUI::hit_box(app.gui, IMAGE_HIT_ID, fixed_layout(app.display_size, app.display_size));
        set_element_rect(app.gui, image_hit, app.image_rect);
        if(app.image_rect.width > 1.0f && app.image_rect.height > 1.0f)
        {
            draw_curve_overlay(app, app.image_rect);
        }

        f32 side_x = app.image_rect.offset_x + app.image_rect.width + 24.0f;
        f32 side_y = app.image_rect.offset_y + 8.0f;
        draw_label(app.gui, FIRST_TEXT_ID + 4, side_x, side_y, 520.0f, "Drag control points on the texture.");
        draw_label(app.gui, FIRST_TEXT_ID + 5, side_x, side_y + 40.0f, 560.0f, "White points are anchors. Yellow points are controls.");
        draw_label(app.gui, FIRST_TEXT_ID + 6, side_x, side_y + 80.0f, 560.0f, "Quadratic: P0-Q0-P1 and P2-Q1-P3.");
        draw_label(app.gui, FIRST_TEXT_ID + 7, side_x, side_y + 120.0f, 560.0f, "Cubic: P1-C0-C1-P2 and P3-C2-C3-P0.");
        c8 dragging[64];
        if(app.dragging_point >= 0)
        {
            snprintf(dragging, 64, "Dragging %s", POINT_LABELS[app.dragging_point]);
        }
        else
        {
            snprintf(dragging, 64, "Dragging none");
        }
        draw_label(app.gui, FIRST_TEXT_ID + 8, side_x, side_y + 160.0f, 240.0f, dragging);
        for(u32 i = 0; i < NUM_CURVE_POINTS; ++i)
        {
            c8 line[128];
            snprintf(line, 128, "%s  x=%.3f  y=%.3f", POINT_LABELS[i], app.control_points[i].x, app.control_points[i].y);
            draw_label(app.gui, FIRST_TEXT_ID + 9 + i, side_x, side_y + 200.0f + (f32)i * 32.0f, 260.0f, line);
        }
        app.gui->end_element();
    }

    void update_curve_interaction(App& app)
    {
        GUI::InteractionState image_state = app.gui->get_interaction_state(IMAGE_HIT_ID);
        bool left_down = image_state.active;
        Float2U pointer = image_state.pointer_screen_position;
        if(left_down && !app.was_left_down && point_in_rect(pointer, app.image_rect))
        {
            f32 best_distance_sq = 18.0f * 18.0f;
            app.dragging_point = -1;
            for(u32 i = 0; i < NUM_CURVE_POINTS; ++i)
            {
                Float2U screen_point = curve_point_to_screen(app, app.control_points[i]);
                f32 dx = screen_point.x - pointer.x;
                f32 dy = screen_point.y - pointer.y;
                f32 distance_sq = dx * dx + dy * dy;
                if(distance_sq < best_distance_sq)
                {
                    best_distance_sq = distance_sq;
                    app.dragging_point = (i32)i;
                }
            }
        }
        if(left_down && app.dragging_point >= 0 && app.image_rect.width > 1.0f && app.image_rect.height > 1.0f)
        {
            Float2U p;
            p.x = clamp((pointer.x - app.image_rect.offset_x) / app.image_rect.width, 0.0f, 1.0f);
            p.y = clamp((pointer.y - app.image_rect.offset_y) / app.image_rect.height, 0.0f, 1.0f);
            app.control_points[app.dragging_point] = p;
        }
        if(!left_down)
        {
            app.dragging_point = -1;
        }
        app.was_left_down = left_down;
    }

    RV init_app(App& app)
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
                GUIWindow::module_gui_window() }));
            luexp(init_modules());

            luset(app.window, Window::new_window("Luna VG Cubic Curve Test"));
            auto dev = RHI::get_main_device();
            app.queue = U32_MAX;
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
                RHI::SwapChainDesc({ sz.x, sz.y, 2, RHI::Format::bgra8_unorm, true,
                    RHI::ColorSpace::srgb })));
            luset(app.cmdbuf, dev->new_command_buffer(app.queue));
            app.shape_draw_list = VG::new_shape_draw_list(dev);
            app.shape_renderer = VG::new_fill_shape_renderer();
            luset(app.gui_renderer, GUI::new_renderer(dev));
            app.gui = GUI::new_context();
            EditorGUI::register_style_schemas(app.gui);
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            EditorGUI::DefaultStyleDesc style_desc;
            style_desc.input_mode = EditorGUI::InputMode::pointer;
            EditorGUI::set_default_style(app.gui, style_desc);
            luexp(recreate_curve_texture(app));
        }
        lucatchret;
        return ok;
    }

    RV run_app()
    {
        lutry
        {
            App app;
            luexp(init_app(app));

            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);

            while(true)
            {
                Window::poll_events();
                if(app.window->is_closed()) break;
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

                u32 expected_texture_size = RESOLUTION_VALUES[app.resolution_index];
                if(!app.curve_texture || app.curve_texture->get_desc().width != expected_texture_size)
                {
                    luexp(recreate_curve_texture(app));
                }

                auto logical_sz = app.window->get_size();
                GUI::FrameDesc frame;
                frame.screen_size = Float2U((f32)logical_sz.x, (f32)logical_sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = app.window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);

                app.gui->push_layer(DEFAULT_LAYER_ID, Float2U(0.0f));
                build_gui(app, frame.screen_size);
                app.gui->pop_layer();
                app.gui->route_input();
                EditorGUI::resolve_interactions(app.gui);
                luexp(GUIWindow::update_text_input(&input_adapter));
                update_curve_interaction(app);

                expected_texture_size = RESOLUTION_VALUES[app.resolution_index];
                if(!app.curve_texture || app.curve_texture->get_desc().width != expected_texture_size)
                {
                    luexp(recreate_curve_texture(app));
                }
                luexp(draw_curve_texture(app));

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                Float4U clear_color = app.gui->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name("gui.canvas"),
                    GUI::style_f32x4(Float4U(0.92f, 0.93f, 0.92f, 1.0f))).number;
                GUI::RenderTargetDesc target(back_buffer);
                target.color_load_op = RHI::LoadOp::clear;
                target.color_clear_value = clear_color;
                target.color_final_state = RHI::TextureStateFlag::present;
                luexp(app.gui_renderer->render(app.gui, app.cmdbuf, target));
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

int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    auto r = Luna::run_app();
    if(failed(r))
    {
        Luna::log_error("VGCurveTest", "%s", Luna::explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
