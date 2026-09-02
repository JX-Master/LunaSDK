/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorGUITest.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include "Showcase.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/GUI/GUI.hpp>
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
        Ref<GUI::IContext> gui;
        Ref<GUI::IRenderer> renderer;
        u32 queue = U32_MAX;
        u32 width = 0;
        u32 height = 0;
        i32 applied_theme = -1;
        i32 applied_density = -1;
        EditorGUITest::ShowcaseState state;
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

    void create_circle_shape(EditorGUITest::ShowcaseState& state)
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
        EditorGUI::DefaultStyleDesc desc;
        desc.color_theme = app.state.theme == 0 ? EditorGUI::ColorTheme::light : EditorGUI::ColorTheme::dark;
        desc.input_mode = app.state.density == 0 ? EditorGUI::InputMode::pointer : EditorGUI::InputMode::touch;
        desc.accent = Float4U(0.890f, 0.310f, 0.349f, 1.0f);
        EditorGUI::set_default_style(app.gui, desc);
        // The reference showcase renders to a sampleable GUI target. Popups and tooltips
        // intentionally exaggerate blur and transmission so the neutral Overlay sample makes
        // capture and filtering visually obvious.
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.popup.backdrop_softness"), GUI::style_f32(30.0f));
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.popup.backdrop_downsample_level"), GUI::style_f32(1.0f));
        Float4U popup_background = app.gui->get_style_value(
            Name(EditorGUI::DEFAULT_STYLE_NAME), Name("gui.popup.background"),
            GUI::style_f32x4(Float4U(0.1f))).number;
        popup_background.w = 0.35f;
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.popup.background"), GUI::style_f32x4(popup_background));
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.tooltip.backdrop_softness"), GUI::style_f32(30.0f));
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.tooltip.backdrop_downsample_level"), GUI::style_f32(1.0f));
        Float4U tooltip_background = app.gui->get_style_value(
            Name(EditorGUI::DEFAULT_STYLE_NAME), Name("gui.tooltip.background"),
            GUI::style_f32x4(Float4U(0.1f))).number;
        tooltip_background.w = 0.35f;
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.tooltip.background"), GUI::style_f32x4(tooltip_background));
        app.gui->set_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME),
            Name("gui.dock_panel.floating.backdrop_softness"), GUI::style_f32(16.0f));
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
                GUI::module_gui(),
                EditorGUI::module_editor_gui(),
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
            luset(app.renderer, GUI::new_renderer(device));
            app.gui = GUI::new_context();
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            EditorGUI::register_style_schemas(app.gui);

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
            GUI::RenderTargetDesc target(app.gui_target);
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

    void verify_input_text_direction_navigation(GUI::IContext* context)
    {
        String value("\xE7\x94\xB2\xE4\xB9\x99");
        auto build_frame = [&]()
        {
            GUI::FrameDesc frame;
            frame.logical_size = Float2U(320.0f, 120.0f);
            frame.render_size = UInt2U(320, 120);
            frame.delta_time = 1.0f / 60.0f;
            context->begin_frame(frame);
            context->push_layer(1, Float2U(0.0f));
            GUI::LayoutConfig root_layout;
            root_layout.width.kind = GUI::SizeKind::percent;
            root_layout.width.value = 1.0f;
            root_layout.height.kind = GUI::SizeKind::percent;
            root_layout.height.value = 1.0f;
            GUI::ElementHandle root = EditorGUI::begin_v_layout(
                context, context->make_id("input.navigation.test.root"),
                "Input navigation test", root_layout);
            GUI::LayoutConfig control_layout;
            control_layout.width.kind = GUI::SizeKind::percent;
            control_layout.width.value = 1.0f;
            control_layout.height.kind = GUI::SizeKind::fixed;
            control_layout.height.value = 40.0f;
            GUI::ElementHandle input = EditorGUI::input_text(
                context, context->make_id("input.navigation.test.value"), value,
                control_layout);
            EditorGUI::text_button(context, context->make_id("input.navigation.test.sibling"),
                "Sibling", control_layout);
            GUI::FlexLayoutDesc flex;
            flex.axis = GUI::LayoutAxis::y;
            flex.cross_alignment = GUI::FlexAlignment::stretch;
            EditorGUI::end_v_layout(context, root, flex);
            context->pop_layer();
            lupanic_if_failed(EditorGUI::layout_tree(
                context, root, RectF(0.0f, 0.0f, 320.0f, 120.0f)));
            return input;
        };

        GUI::ElementHandle input = build_frame();
        GUI::NavigationConfig navigation = context->get_navigation_config(input);
        luassert(navigation.left == GUI::NavigationMode::callback);
        luassert(navigation.right == GUI::NavigationMode::callback);
        luassert(navigation.up == GUI::NavigationMode::callback);
        luassert(navigation.down == GUI::NavigationMode::callback);
        luassert(navigation.forward == GUI::NavigationMode::automatic);
        context->focus_element(input.id);
        context->route_input();
        EditorGUI::resolve_interactions(context);

        auto apply_direction = [&](GUI::NavigationDirection direction, const c8* text)
        {
            input = build_frame();
            GUI::InputEvent navigation_event;
            navigation_event.type = GUI::InputEventType::navigation_dpad;
            navigation_event.navigation_direction = direction;
            context->add_input_event(navigation_event);
            GUI::InputEvent event;
            event.type = GUI::InputEventType::text_utf8;
            event.text = text;
            context->add_input_event(event);
            context->route_input();
            EditorGUI::resolve_interactions(context);
            luassert(context->focused_element() == input.id);
        };

        apply_direction(GUI::NavigationDirection::right, "X");
        apply_direction(GUI::NavigationDirection::up, "Y");
        apply_direction(GUI::NavigationDirection::down, "Z");
        apply_direction(GUI::NavigationDirection::left, "W");
        luassert(!strcmp(value.c_str(), "Y\xE7\x94\xB2X\xE4\xB9\x99WZ"));

        input = build_frame();
        GUI::InputEvent home;
        home.type = GUI::InputEventType::key_down;
        home.key = KeyCode::home;
        context->add_input_event(home);
        GUI::InputEvent home_text;
        home_text.type = GUI::InputEventType::text_utf8;
        home_text.text = "H";
        context->add_input_event(home_text);
        context->route_input();
        EditorGUI::resolve_interactions(context);
        luassert(context->focused_element() == input.id);
        luassert(!strcmp(value.c_str(), "Y\xE7\x94\xB2X\xE4\xB9\x99WHZ"));

        input = build_frame();
        GUI::InputEvent end;
        end.type = GUI::InputEventType::key_down;
        end.key = KeyCode::end;
        context->add_input_event(end);
        GUI::InputEvent end_text;
        end_text.type = GUI::InputEventType::text_utf8;
        end_text.text = "E";
        context->add_input_event(end_text);
        context->route_input();
        EditorGUI::resolve_interactions(context);
        luassert(context->focused_element() == input.id);
        luassert(!strcmp(value.c_str(), "Y\xE7\x94\xB2X\xE4\xB9\x99WHEZ"));
        context->focus_element(0);
    }

    R<GUI::paint_order_id_t> draw_invalid_mixed_paint_order(GUI::IContext* context,
        const GUI::ElementHandle&, GUI::DrawPhase, GUI::paint_order_id_t paint_order_id, void*)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::rect;
        context->draw(command, paint_order_id + 1);
        context->draw(command);
        return paint_order_id;
    }

    void verify_paint_order_generation(GUI::IContext* context)
    {
        GUI::FrameDesc frame;
        frame.logical_size = Float2U(640.0f, 240.0f);
        frame.render_size = UInt2U(640, 240);
        frame.delta_time = 1.0f / 60.0f;
        context->begin_frame(frame);

        GUI::LayoutConfig root_layout;
        root_layout.width.kind = GUI::SizeKind::fixed;
        root_layout.width.value = 240.0f;
        root_layout.height.kind = GUI::SizeKind::fixed;
        root_layout.height.value = 48.0f;
        GUI::LayoutConfig button_layout;
        button_layout.width.kind = GUI::SizeKind::fixed;
        button_layout.width.value = 96.0f;
        button_layout.height.kind = GUI::SizeKind::fixed;
        button_layout.height.value = 40.0f;

        const GUI::id_t empty_layer_id = context->make_id("paint_order.empty.layer");
        context->push_layer(empty_layer_id, Float2U(0.0f));
        context->pop_layer();

        const GUI::id_t shared_layer_id = context->make_id("paint_order.shared.layer");
        const GUI::id_t shared_button_a_id = context->make_id("paint_order.shared.button.a");
        const GUI::id_t shared_button_b_id = context->make_id("paint_order.shared.button.b");
        const GUI::id_t shared_capture_id = context->make_id("paint_order.shared.capture");
        context->push_layer(shared_layer_id, Float2U(0.0f));
        GUI::ElementHandle shared_root = EditorGUI::begin_h_layout(context,
            context->make_id("paint_order.shared.root"), "Shared button row", root_layout);
        context->set_child_paint_order_mode(shared_root, GUI::ChildPaintOrderMode::shared);
        GUI::ElementHandle shared_button_a = EditorGUI::text_button(context,
            shared_button_a_id, "First", button_layout);
        GUI::ElementHandle shared_button_b = EditorGUI::text_button(context,
            shared_button_b_id, "Second", button_layout);
        GUI::LayoutConfig empty_element_layout;
        empty_element_layout.width.kind = GUI::SizeKind::fixed;
        empty_element_layout.height.kind = GUI::SizeKind::fixed;
        GUI::ElementHandle legacy_scope = context->begin_element(
            context->make_id("paint_order.shared.legacy_scope"));
        context->set_layout_config(legacy_scope, empty_element_layout);
        GUI::ElementHandle legacy_child = context->begin_element(
            context->make_id("paint_order.shared.legacy_child"));
        context->set_layout_config(legacy_child, empty_element_layout);
        GUI::DrawCommand legacy_command;
        legacy_command.type = GUI::DrawCommandType::line;
        legacy_command.rect_reference = GUI::DrawCommandRectReference::element;
        legacy_command.point1 = Float2U(1.0f, 1.0f);
        context->draw_for_element(shared_button_a, legacy_command);
        context->end_element();
        context->end_element();
        GUI::ElementHandle shared_capture = context->begin_element(shared_capture_id);
        context->set_layout_config(shared_capture, empty_element_layout);
        GUI::BackdropBlurCaptureDesc shared_capture_desc;
        shared_capture_desc.softness = 1.0f;
        context->set_backdrop_blur_capture(shared_capture, shared_capture_desc);
        context->end_element();
        GUI::FlexLayoutDesc shared_flex;
        shared_flex.main_axis_gap = 8.0f;
        shared_flex.cross_alignment = GUI::FlexAlignment::center;
        EditorGUI::end_h_layout(context, shared_root, shared_flex);
        context->pop_layer();

        const GUI::id_t sequential_layer_id = context->make_id("paint_order.sequential.layer");
        const GUI::id_t overlap_button_a_id = context->make_id("paint_order.overlap.button.a");
        const GUI::id_t overlap_button_b_id = context->make_id("paint_order.overlap.button.b");
        context->push_layer(sequential_layer_id, Float2U(0.0f, 64.0f));
        GUI::ElementHandle overlap_root = EditorGUI::begin_canvas_layout(context,
            context->make_id("paint_order.overlap.root"), "Overlapping buttons", root_layout);
        GUI::ElementHandle overlap_button_a = EditorGUI::text_button(context,
            overlap_button_a_id, "Below", button_layout);
        GUI::ElementHandle overlap_button_b = EditorGUI::text_button(context,
            overlap_button_b_id, "Above", button_layout);
        GUI::CanvasLayoutItem overlap_items[2];
        overlap_items[0].element_id = overlap_button_a_id;
        overlap_items[1].element_id = overlap_button_b_id;
        GUI::CanvasLayoutDesc overlap_canvas;
        overlap_canvas.items = Span<const GUI::CanvasLayoutItem>(overlap_items, 2);
        EditorGUI::end_canvas_layout(context, overlap_root, overlap_canvas);
        context->pop_layer();

        const GUI::id_t capture_layer_id = context->make_id("paint_order.capture.layer");
        context->push_layer(capture_layer_id, Float2U(0.0f, 128.0f));
        GUI::ElementHandle captured_button = EditorGUI::text_button(context,
            context->make_id("paint_order.capture.button"), "Captured", button_layout);
        GUI::BackdropBlurCaptureDesc capture;
        capture.softness = 4.0f;
        context->set_backdrop_blur_capture(captured_button, capture);
        context->pop_layer();

        lupanic_if_failed(EditorGUI::layout_tree(context, shared_root,
            RectF(0.0f, 0.0f, 240.0f, 48.0f)));
        lupanic_if_failed(EditorGUI::layout_tree(context, overlap_root,
            RectF(0.0f, 0.0f, 240.0f, 48.0f)));
        lupanic_if_failed(EditorGUI::layout_tree(context, captured_button,
            RectF(0.0f, 0.0f, 96.0f, 40.0f)));
        lupanic_if_failed(context->generate_draw_commands());

        GUI::ElementHandle shared_text_a = context->find_element_handle(
            GUI::make_scoped_id(shared_button_a_id, "text"));
        GUI::ElementHandle shared_text_b = context->find_element_handle(
            GUI::make_scoped_id(shared_button_b_id, "text"));
        GUI::ElementHandle overlap_text_a = context->find_element_handle(
            GUI::make_scoped_id(overlap_button_a_id, "text"));
        GUI::paint_order_id_t shared_surface_a[2] = {
            GUI::INVALID_PAINT_ORDER_ID, GUI::INVALID_PAINT_ORDER_ID };
        GUI::paint_order_id_t shared_surface_b[2] = {
            GUI::INVALID_PAINT_ORDER_ID, GUI::INVALID_PAINT_ORDER_ID };
        GUI::paint_order_id_t shared_text_a_order = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t shared_text_b_order = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t overlap_a_max = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t overlap_b_first = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t shared_capture_order = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t legacy_order = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t capture_order = GUI::INVALID_PAINT_ORDER_ID;
        GUI::paint_order_id_t captured_surface_order = GUI::INVALID_PAINT_ORDER_ID;
        u32 shared_surface_a_count = 0;
        u32 shared_surface_b_count = 0;
        for(const GUI::DrawCommand& command : context->get_draw_commands())
        {
            if(command.element == shared_button_a.index && command.type == GUI::DrawCommandType::sdf &&
                shared_surface_a_count < 2)
                shared_surface_a[shared_surface_a_count++] = command.paint_order_id;
            if(command.element == shared_button_b.index && command.type == GUI::DrawCommandType::sdf &&
                shared_surface_b_count < 2)
                shared_surface_b[shared_surface_b_count++] = command.paint_order_id;
            if(command.element == shared_text_a.index && command.type == GUI::DrawCommandType::text)
                shared_text_a_order = command.paint_order_id;
            if(command.element == shared_text_b.index && command.type == GUI::DrawCommandType::text)
                shared_text_b_order = command.paint_order_id;
            if((command.element == overlap_button_a.index || command.element == overlap_text_a.index) &&
                (overlap_a_max == GUI::INVALID_PAINT_ORDER_ID || command.paint_order_id > overlap_a_max))
                overlap_a_max = command.paint_order_id;
            if(command.element == overlap_button_b.index && command.type == GUI::DrawCommandType::sdf &&
                overlap_b_first == GUI::INVALID_PAINT_ORDER_ID)
                overlap_b_first = command.paint_order_id;
            if(command.element == shared_capture.index &&
                command.type == GUI::DrawCommandType::backdrop_blur_capture)
                shared_capture_order = command.paint_order_id;
            if(command.element == shared_button_a.index && command.type == GUI::DrawCommandType::line)
                legacy_order = command.paint_order_id;
            if(command.element == captured_button.index &&
                command.type == GUI::DrawCommandType::backdrop_blur_capture)
                capture_order = command.paint_order_id;
            if(command.element == captured_button.index && command.type == GUI::DrawCommandType::sdf &&
                captured_surface_order == GUI::INVALID_PAINT_ORDER_ID)
                captured_surface_order = command.paint_order_id;
        }
        luassert(shared_surface_a_count == 2 && shared_surface_b_count == 2);
        luassert(shared_surface_a[0] == shared_surface_b[0]);
        luassert(shared_surface_a[1] == shared_surface_b[1]);
        luassert(shared_text_a_order == shared_text_b_order);
        luassert(shared_surface_a[0] < shared_surface_a[1] &&
            shared_surface_a[1] < shared_text_a_order);
        luassert(legacy_order == shared_text_a_order + 1);
        luassert(shared_capture_order == legacy_order + 1);
        u32 shared_capture_event_count = 0;
        for(const GUI::DrawCommand& command : context->get_draw_commands())
        {
            if(command.paint_order_id == shared_capture_order) ++shared_capture_event_count;
        }
        luassert(shared_capture_event_count == 1);
        luassert(overlap_a_max != GUI::INVALID_PAINT_ORDER_ID &&
            overlap_b_first == overlap_a_max + 1);

        const GUI::Layer* empty_layer = nullptr;
        const GUI::Layer* shared_layer = nullptr;
        const GUI::Layer* sequential_layer = nullptr;
        const GUI::Layer* capture_layer = nullptr;
        for(const GUI::Layer& layer : context->get_layers())
        {
            if(layer.id == empty_layer_id) empty_layer = &layer;
            else if(layer.id == shared_layer_id) shared_layer = &layer;
            else if(layer.id == sequential_layer_id) sequential_layer = &layer;
            else if(layer.id == capture_layer_id) capture_layer = &layer;
        }
        luassert(empty_layer && shared_layer && sequential_layer && capture_layer);
        luassert(empty_layer->first_paint_order_id == GUI::INVALID_PAINT_ORDER_ID &&
            empty_layer->max_paint_order_id == GUI::INVALID_PAINT_ORDER_ID);
        luassert(shared_layer->first_paint_order_id == 0);
        luassert(sequential_layer->first_paint_order_id == shared_layer->max_paint_order_id + 1);
        luassert(capture_layer->first_paint_order_id == sequential_layer->max_paint_order_id + 1);
        luassert(capture_order == capture_layer->first_paint_order_id);
        luassert(captured_surface_order == capture_order + 1);

        luassert(context->bring_layer_to_front(shared_layer_id));
        lupanic_if_failed(context->generate_draw_commands());
        empty_layer = nullptr;
        shared_layer = nullptr;
        sequential_layer = nullptr;
        capture_layer = nullptr;
        for(const GUI::Layer& layer : context->get_layers())
        {
            if(layer.id == empty_layer_id) empty_layer = &layer;
            else if(layer.id == shared_layer_id) shared_layer = &layer;
            else if(layer.id == sequential_layer_id) sequential_layer = &layer;
            else if(layer.id == capture_layer_id) capture_layer = &layer;
        }
        luassert(empty_layer && shared_layer && sequential_layer && capture_layer);
        luassert(empty_layer->first_paint_order_id == GUI::INVALID_PAINT_ORDER_ID &&
            empty_layer->max_paint_order_id == GUI::INVALID_PAINT_ORDER_ID);
        luassert(sequential_layer->first_paint_order_id == 0);
        luassert(capture_layer->first_paint_order_id == sequential_layer->max_paint_order_id + 1);
        luassert(shared_layer->first_paint_order_id == capture_layer->max_paint_order_id + 1);

        context->begin_frame(frame);
        context->push_layer(context->make_id("paint_order.invalid.layer"), Float2U(0.0f));
        GUI::ElementHandle invalid_element = context->begin_element(
            context->make_id("paint_order.invalid.element"));
        context->set_layout_config(invalid_element, button_layout);
        GUI::DrawConfig invalid_draw;
        invalid_draw.callback = draw_invalid_mixed_paint_order;
        context->set_draw_config(invalid_element, invalid_draw);
        context->end_element();
        context->pop_layer();
        lupanic_if_failed(EditorGUI::layout_tree(context, invalid_element,
            RectF(0.0f, 0.0f, 96.0f, 40.0f)));
        luassert(failed(context->generate_draw_commands()));
    }

    RV run_demo(const DemoOptions& options)
    {
        lutry
        {
            DemoApp app;
            luexp(init_demo(app, options));
            verify_input_text_direction_navigation(app.gui);
            verify_paint_order_generation(app.gui);
            GUIWindow::GUIWindowInputAdapter input_adapter;
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
                GUI::FrameDesc frame;
                frame.logical_size = Float2U((f32)logical_size.x, (f32)logical_size.y);
                frame.render_size = framebuffer_size;
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);
                bool verify_popup_blur = options.max_frames >= 0 &&
                    frame_index == 0 && app.state.section == 7;
                if(verify_popup_blur)
                {
                    EditorGUI::open_popup(app.gui,
                        app.gui->make_id("overlay.popup.layer"));
                }

                EditorGUITest::ShowcaseHandles handles;
                GUI::ElementHandle root = EditorGUITest::build_showcase(app.gui, app.state, handles);
                luexp(EditorGUI::layout_tree(app.gui, root,
                    RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                app.gui->route_input();
                EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(app.gui);
                EditorGUITest::resolve_showcase(app.gui, app.state, handles);
                bool style_changed = app.state.theme != app.applied_theme ||
                    app.state.density != app.applied_density;
                if(style_changed) apply_style(app);
                if(resolved.relayout_requested || style_changed)
                {
                    luexp(EditorGUI::layout_tree(app.gui, root,
                        RectF(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y)));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));
                luexp(app.gui->generate_draw_commands());
                if(verify_popup_blur)
                {
                    u32 capture_commands = 0;
                    u32 blur_commands = 0;
                    for(const GUI::DrawCommand& command : app.gui->get_draw_commands())
                    {
                        if(command.type == GUI::DrawCommandType::backdrop_blur_capture)
                        {
                            ++capture_commands;
                        }
                        else if(command.type == GUI::DrawCommandType::backdrop_blur)
                        {
                            ++blur_commands;
                        }
                    }
                    luassert(capture_commands == 1);
                    luassert(blur_commands == 1);
                }

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                Float4U clear_color = app.gui->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name("gui.canvas"),
                    GUI::style_f32x4(Float4U(0.92f, 0.93f, 0.92f, 1.0f))).number;
                luexp(render_demo(app, back_buffer, clear_color));
                if(verify_popup_blur)
                {
                    GUI::RendererPerformanceCounters counters =
                        app.renderer->get_performance_counters();
                    luassert(counters.backdrop_capture_count == 1);
                    luassert(counters.backdrop_blur_dispatch_count >= 3);
                    luassert(counters.backdrop_filtered_pixel_count > 0);
                }
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
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(run_demo(options));
    Luna::close();
    return 0;
}
