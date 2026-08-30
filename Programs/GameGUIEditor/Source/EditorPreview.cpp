/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorPreview.cpp
* @author JXMaster
* @date 2026/8/28
*/
#include "EditorApp.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            usize preview_zoom_level_index(f32 zoom)
            {
                usize result = 0;
                f32 minimum_difference = abs(zoom - PREVIEW_ZOOM_LEVELS[0]);
                for(usize i = 1; i < PREVIEW_ZOOM_LEVEL_COUNT; ++i)
                {
                    f32 difference = abs(zoom - PREVIEW_ZOOM_LEVELS[i]);
                    if(difference < minimum_difference)
                    {
                        result = i;
                        minimum_difference = difference;
                    }
                }
                return result;
            }

            Float2U preview_logical_position(const PreviewState& preview,
                const Float2U& surface_position)
            {
                Float2U center((f32)PREVIEW_LAYOUT_WIDTH * 0.5f,
                    (f32)PREVIEW_LAYOUT_HEIGHT * 0.5f);
                return Float2U(
                    center.x + (surface_position.x - center.x - preview.pan.x) / preview.zoom,
                    center.y + (surface_position.y - center.y - preview.pan.y) / preview.zoom);
            }

            void build_preview_background(GUI::IContext* context, const PreviewState& preview)
            {
                Float2U logical_top_left = preview_logical_position(preview, Float2U(0.0f));
                Float2U logical_bottom_right = preview_logical_position(preview,
                    preview.viewport_size);
                GUI::DrawCommand background;
                background.type = GUI::DrawCommandType::rect;
                background.rect_reference = GUI::DrawCommandRectReference::layer;
                background.rect = RectF(logical_top_left.x, logical_top_left.y,
                    logical_bottom_right.x - logical_top_left.x,
                    logical_bottom_right.y - logical_top_left.y);
                background.color = Float4U(0.26f, 0.26f, 0.26f, 1.0f);
                context->draw(background);

                f32 grid_size = PREVIEW_GRID_SIZE;
                while(grid_size * preview.zoom < 12.0f) grid_size *= 2.0f;
                i32 first_x = (i32)floor(logical_top_left.x / grid_size);
                i32 last_x = (i32)ceil(logical_bottom_right.x / grid_size);
                i32 first_y = (i32)floor(logical_top_left.y / grid_size);
                i32 last_y = (i32)ceil(logical_bottom_right.y / grid_size);
                for(i32 i = first_x; i <= last_x; ++i)
                {
                    f32 x = (f32)i * grid_size;
                    GUI::DrawCommand line;
                    line.type = GUI::DrawCommandType::line;
                    line.rect_reference = GUI::DrawCommandRectReference::layer;
                    line.rect = RectF(x, logical_top_left.y, 0.0f, 0.0f);
                    line.point1 = Float2U(x, logical_bottom_right.y);
                    line.color = Float4U(1.0f, 1.0f, 1.0f,
                        (i % 4) ? 0.12f : 0.22f);
                    line.line_width = ((i % 4) ? 1.0f : 1.25f) / preview.zoom;
                    context->draw(line);
                }
                for(i32 i = first_y; i <= last_y; ++i)
                {
                    f32 y = (f32)i * grid_size;
                    GUI::DrawCommand line;
                    line.type = GUI::DrawCommandType::line;
                    line.rect_reference = GUI::DrawCommandRectReference::layer;
                    line.rect = RectF(logical_top_left.x, y, 0.0f, 0.0f);
                    line.point1 = Float2U(logical_bottom_right.x, y);
                    line.color = Float4U(1.0f, 1.0f, 1.0f,
                        (i % 4) ? 0.12f : 0.22f);
                    line.line_width = ((i % 4) ? 1.0f : 1.25f) / preview.zoom;
                    context->draw(line);
                }
            }

            void build_preview_overlay(GUI::IContext* context, const PreviewState& preview)
            {
                constexpr f32 BADGE_WIDTH = 68.0f;
                constexpr f32 BADGE_HEIGHT = 26.0f;
                constexpr f32 BADGE_MARGIN = 8.0f;
                Float2U badge_top_left = preview_logical_position(preview,
                    Float2U(preview.viewport_size.x - BADGE_WIDTH - BADGE_MARGIN,
                        preview.viewport_size.y - BADGE_HEIGHT - BADGE_MARGIN));
                RectF badge(badge_top_left.x, badge_top_left.y,
                    BADGE_WIDTH / preview.zoom, BADGE_HEIGHT / preview.zoom);
                GUI::DrawCommand background;
                background.type = GUI::DrawCommandType::rounded_rect;
                background.rect_reference = GUI::DrawCommandRectReference::layer;
                background.rect = badge;
                background.color = Float4U(0.08f, 0.08f, 0.08f, 0.82f);
                background.radius = 5.0f / preview.zoom;
                context->draw(background);

                String label;
                f32 percentage = preview.zoom * 100.0f;
                if(abs(percentage - round(percentage)) < 0.01f)
                    strprintf(label, "%d%%", (i32)round(percentage));
                else strprintf(label, "%.1f%%", percentage);
                GUI::DrawCommand text;
                text.type = GUI::DrawCommandType::text;
                text.rect_reference = GUI::DrawCommandRectReference::layer;
                text.rect = badge;
                text.color = Float4U(1.0f);
                text.font = Name("default");
                text.font_size = 13.0f / preview.zoom;
                text.horizontal_alignment = VG::TextAlignment::center;
                text.vertical_alignment = VG::TextAlignment::center;
                text.text = move(label);
                context->draw(text);
            }

            void draw_preview_node_background(GUI::IContext* context)
            {
                GUI::DrawCommand background;
                background.type = GUI::DrawCommandType::rect;
                background.rect_reference = GUI::DrawCommandRectReference::element;
                background.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
                background.color = Float4U(0.08f, 0.09f, 0.11f, 0.35f);
                context->draw(background);
            }

            void draw_preview_node_border(GUI::IContext* context, f32 zoom)
            {
                f32 thickness = 1.5f / zoom;
                const Float4U color(0.86f, 0.88f, 0.92f, 0.9f);
                GUI::DrawCommand border;
                border.type = GUI::DrawCommandType::rect;
                border.rect_reference = GUI::DrawCommandRectReference::element;
                border.color = color;

                border.rect = RectF(0.0f, 0.0f, 0.0f, thickness);
                border.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 0.0f);
                context->draw(border);
                border.rect = RectF(0.0f, -thickness, 0.0f, thickness);
                border.rect_layout_scale = Float4U(0.0f, 1.0f, 1.0f, 0.0f);
                context->draw(border);
                border.rect = RectF(0.0f, 0.0f, thickness, 0.0f);
                border.rect_layout_scale = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
                context->draw(border);
                border.rect = RectF(-thickness, 0.0f, thickness, 0.0f);
                border.rect_layout_scale = Float4U(1.0f, 0.0f, 0.0f, 1.0f);
                context->draw(border);
            }

            GUI::ElementHandle build_preview_resize_handle(GUI::IContext* context,
                PreviewState& preview)
            {
                GUI::ElementHandle handle = context->begin_element(
                    context->make_id("preview.node.resize_handle"));
                f32 handle_size = min(PREVIEW_RESIZE_HANDLE_SIZE / preview.zoom,
                    min(preview.node_size.x, preview.node_size.y) * 0.5f);
                GUI::LayoutConfig layout = fixed_layout(handle_size, handle_size);
                context->set_layout_config(handle, layout);
                GUI::Interactable interactable;
                interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
                set_flags(interactable.flags, GUI::InteractableFlag::activatable);
                context->set_interactable(handle, interactable);

                GUI::InteractionState interaction = context->get_interaction_state(handle.id);
                GUI::DrawCommand background;
                background.type = GUI::DrawCommandType::rounded_rect;
                background.rect_reference = GUI::DrawCommandRectReference::element;
                background.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
                background.color = preview.resizing ? Float4U(1.0f, 0.42f, 0.50f, 1.0f) :
                    interaction.hovered ? Float4U(1.0f, 0.55f, 0.61f, 1.0f) :
                    Float4U(0.96f, 0.34f, 0.44f, 1.0f);
                background.radius = 2.0f / preview.zoom;
                context->draw(background);
                context->end_element();
                return handle;
            }

            bool process_preview_resize(PreviewState& preview,
                const GUI::ElementHandle& handle)
            {
                bool changed = false;
                for(const GUI::RoutedInputEvent& routed :
                    preview.context->get_routed_input_events(handle.id))
                {
                    const GUI::InputEvent& event = routed.event;
                    if(event.type == GUI::InputEventType::pointer_down &&
                        event.button == GUI::PointerButton::left)
                    {
                        preview.resizing = true;
                        preview.resize_pointer = event.position;
                        preview.resize_start_size = preview.node_size;
                        continue;
                    }
                    if(!preview.resizing) continue;
                    if(event.type == GUI::InputEventType::pointer_move ||
                        (event.type == GUI::InputEventType::pointer_up &&
                            event.button == GUI::PointerButton::left))
                    {
                        Float2U size(
                            preview.resize_start_size.x + event.position.x -
                                preview.resize_pointer.x,
                            preview.resize_start_size.y + event.position.y -
                                preview.resize_pointer.y);
                        size.x = round(clamp(size.x, PREVIEW_MIN_NODE_SIZE,
                            PREVIEW_MAX_NODE_SIZE));
                        size.y = round(clamp(size.y, PREVIEW_MIN_NODE_SIZE,
                            PREVIEW_MAX_NODE_SIZE));
                        if(size != preview.node_size)
                        {
                            preview.node_size = size;
                            changed = true;
                        }
                    }
                    if(event.type == GUI::InputEventType::pointer_up &&
                        event.button == GUI::PointerButton::left)
                    {
                        preview.resizing = false;
                    }
                }
                if(preview.resizing &&
                    !preview.context->is_pointer_button_down(GUI::PointerButton::left))
                {
                    preview.resizing = false;
                }
                return changed;
            }

            void EditorApp::collect_preview_input(DocumentView& document,
                const GUI::ElementHandle& preview_host, const RectF& preview_rect,
                const GUI::FrameDesc& editor_frame, PreviewInput& preview_input)
            {
                preview_input.document_id = document.id;
                if(!preview_host.id || preview_rect.width <= 0.0f || preview_rect.height <= 0.0f)
                    return;
                PreviewState& preview = document.preview;
                preview.pending_viewport_size = Float2U(preview_rect.width,
                    preview_rect.height);
                f32 render_scale_x = editor_frame.logical_size.x > 0.0f ?
                    (f32)editor_frame.render_size.x / editor_frame.logical_size.x : 1.0f;
                f32 render_scale_y = editor_frame.logical_size.y > 0.0f ?
                    (f32)editor_frame.render_size.y / editor_frame.logical_size.y : 1.0f;
                preview.pending_render_size = UInt2U(
                    (u32)max(1.0f, round(preview_rect.width * render_scale_x)),
                    (u32)max(1.0f, round(preview_rect.height * render_scale_y)));
                for(const GUI::RoutedInputEvent& routed :
                    gui->get_routed_input_events(preview_host.id))
                {
                    GUI::InputEvent event = routed.event;
                    Float2U surface_position;
                    if(routed.has_element_position)
                    {
                        surface_position.x = routed.element_position.x / preview_rect.width *
                            preview.viewport_size.x;
                        surface_position.y = routed.element_position.y / preview_rect.height *
                            preview.viewport_size.y;
                    }

                    if(event.type == GUI::InputEventType::pointer_down &&
                        event.button == GUI::PointerButton::middle && routed.has_element_position)
                    {
                        preview.panning = true;
                        preview.pan_pointer = surface_position;
                        gui->capture_pointer(preview_host.id);
                        continue;
                    }
                    if(event.type == GUI::InputEventType::pointer_up &&
                        event.button == GUI::PointerButton::middle)
                    {
                        preview.panning = false;
                        gui->release_pointer_capture(preview_host.id);
                        continue;
                    }
                    if(event.type == GUI::InputEventType::pointer_move && preview.panning &&
                        routed.has_element_position)
                    {
                        preview.pan.x += surface_position.x - preview.pan_pointer.x;
                        preview.pan.y += surface_position.y - preview.pan_pointer.y;
                        preview.pan_pointer = surface_position;
                        continue;
                    }
                    if(event.type == GUI::InputEventType::pointer_wheel &&
                        routed.has_element_position)
                    {
                        preview.zoom_wheel_accumulator += event.wheel_delta.y;
                        i32 zoom_direction = 0;
                        if(preview.zoom_wheel_accumulator >= 1.0f) zoom_direction = 1;
                        else if(preview.zoom_wheel_accumulator <= -1.0f) zoom_direction = -1;
                        if(!zoom_direction) continue;
                        preview.zoom_wheel_accumulator = 0.0f;
                        i32 current_level = (i32)preview_zoom_level_index(preview.zoom);
                        i32 new_level = clamp(current_level + zoom_direction, 0,
                            (i32)PREVIEW_ZOOM_LEVEL_COUNT - 1);
                        f32 new_zoom = PREVIEW_ZOOM_LEVELS[new_level];
                        if(new_zoom == preview.zoom) continue;
                        Float2U logical_position = preview_logical_position(preview,
                            surface_position);
                        Float2U center((f32)PREVIEW_LAYOUT_WIDTH * 0.5f,
                            (f32)PREVIEW_LAYOUT_HEIGHT * 0.5f);
                        preview.pan.x = surface_position.x - center.x -
                            new_zoom * (logical_position.x - center.x);
                        preview.pan.y = surface_position.y - center.y -
                            new_zoom * (logical_position.y - center.y);
                        preview.zoom = new_zoom;
                        continue;
                    }

                    if(routed.has_element_position)
                        event.position = preview_logical_position(preview, surface_position);
                    preview_input.events.push_back(move(event));
                }
                if(preview.panning && !gui->is_pointer_button_down(GUI::PointerButton::middle))
                {
                    preview.panning = false;
                    gui->release_pointer_capture(preview_host.id);
                }
            }


            RV EditorApp::apply_preview_surface_size(DocumentView& document)
            {
                lutry
                {
                    PreviewState& preview = document.preview;
                    UInt2U render_size = preview.pending_render_size;
                    if(!render_size.x || !render_size.y)
                        render_size = UInt2U(PREVIEW_LAYOUT_WIDTH, PREVIEW_LAYOUT_HEIGHT);
                    bool recreate_target = !preview.target ||
                        preview.target->get_desc().width != render_size.x ||
                        preview.target->get_desc().height != render_size.y;
                    if(recreate_target)
                    {
                        Ref<RHI::ITexture> target;
                        luset(target, RHI::get_main_device()->new_texture(
                            RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                                RHI::Format::rgba8_unorm,
                                RHI::TextureUsageFlag::color_attachment |
                                    RHI::TextureUsageFlag::read_texture,
                                render_size.x, render_size.y, 1, 1)));
                        preview.target = move(target);
                    }
                    preview.viewport_size = preview.pending_viewport_size;
                    preview.render_size = render_size;
                }
                lucatchret;
                return ok;
            }

            RV EditorApp::ensure_preview(DocumentView& document)
            {
                lutry
                {
                    if(!document.preview.context)
                    {
                        document.preview.context = GUI::new_context();
                        luexp(document.preview.context->register_font("default",
                            Font::get_default_font()));
                    }
                    if(!document.preview.target)
                    {
                        luset(document.preview.target, RHI::get_main_device()->new_texture(
                            RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                                RHI::Format::rgba8_unorm,
                                RHI::TextureUsageFlag::color_attachment |
                                    RHI::TextureUsageFlag::read_texture,
                                document.preview.render_size.x,
                                document.preview.render_size.y, 1, 1)));
                    }
                    if(!document.preview.instance || document.preview.revision != document.revision)
                    {
                        if(!document.cooked_snapshot)
                        {
                            document.preview.instance.reset();
                            document.preview.revision = document.revision;
                            return ok;
                        }
                        GameGUI::InstanceDesc desc;
                        desc.document = document.cooked_snapshot;
                        desc.instance_scope = GUI::make_scoped_id(GUI::DEFAULT_DATA_SCOPE,
                            document.id);
                        if(document.asset_guid != Guid())
                            desc.source_asset = Asset::get_asset(document.asset_guid);
                        document.preview.instance = GameGUI::new_instance(desc);
                        RV prepared = document.preview.instance->prepare();
                        if(failed(prepared)) error_message = explain(prepared.errcode());
                        document.preview.revision = document.revision;
                    }
                }
                lucatchret;
                return ok;
            }

            RV EditorApp::build_preview(DocumentView& document,
                Span<const GUI::InputEvent> input)
            {
                lutry
                {
                    luexp(ensure_preview(document));
                    GUI::FrameDesc frame;
                    frame.logical_size = document.preview.viewport_size;
                    frame.render_size = document.preview.render_size;
                    frame.delta_time = 1.0f / 60.0f;
                    document.preview.context->begin_frame(frame);
                    document.preview.context->add_input_events(input);
                    document.preview.context->push_layer(
                        document.preview.context->make_id("preview.background.layer"));
                    document.preview.context->begin_element(
                        document.preview.context->make_id("preview.background"));
                    build_preview_background(document.preview.context, document.preview);
                    document.preview.context->end_element();
                    document.preview.context->pop_layer();
                    document.preview.context->push_layer(
                        document.preview.context->make_id("preview.root.layer"));
                    GUI::ElementHandle preview_node = document.preview.context->begin_element(
                        document.preview.context->make_id("preview.node"));
                    document.preview.context->set_element_debug_name(preview_node,
                        Name("GameGUI Preview Parent"));
                    document.preview.context->set_layout_config(preview_node,
                        fixed_layout(document.preview.node_size.x,
                            document.preview.node_size.y));
                    draw_preview_node_background(document.preview.context);
                    GUI::ElementHandle root;
                    if(document.preview.instance)
                        luset(root, document.preview.instance->build(document.preview.context));
                    if(root.id)
                    {
                        const GUI::Element* root_element =
                            document.preview.context->get_element(root.index);
                        if(root_element && root_element->id == root.id)
                        {
                            GUI::LayoutConfig root_layout = root_element->layout;
                            root_layout.margin = Float4U(0.0f);
                            document.preview.context->set_layout_config(root, root_layout);
                        }
                    }
                    GUI::ElementHandle resize_handle = build_preview_resize_handle(
                        document.preview.context, document.preview);
                    draw_preview_node_border(document.preview.context,
                        document.preview.zoom);
                    document.preview.context->end_element();
                    document.preview.context->pop_layer();

                    GUI::CanvasLayoutItem preview_items[2];
                    usize preview_item_count = 0;
                    if(root.id)
                    {
                        GUI::CanvasLayoutItem& root_item =
                            preview_items[preview_item_count++];
                        root_item.element_id = root.id;
                        root_item.anchor_min = Float2U(0.0f);
                        root_item.anchor_max = Float2U(1.0f);
                        root_item.offset = Float4U(0.0f);
                    }
                    GUI::CanvasLayoutItem& resize_item =
                        preview_items[preview_item_count++];
                    resize_item.element_id = resize_handle.id;
                    resize_item.anchor_min = Float2U(1.0f);
                    resize_item.anchor_max = Float2U(1.0f);
                    resize_item.offset = Float4U(0.0f);
                    resize_item.pivot = Float2U(1.0f);
                    GUI::CanvasLayoutDesc preview_layout;
                    preview_layout.items = Span<const GUI::CanvasLayoutItem>(preview_items,
                        preview_item_count);
                    preview_layout.clip_children = true;
                    GUI::LayoutCallbackConfig preview_layout_config;
                    preview_layout_config.algorithm = Name("game_gui_editor.preview_parent");
                    preview_layout_config.callback = GUI::layout_canvas;
                    preview_layout_config.userdata = &preview_layout;
                    document.preview.context->set_layout_callback_config(preview_node,
                        preview_layout_config);

                    document.preview.context->push_layer(
                        document.preview.context->make_id("preview.overlay.layer"));
                    document.preview.context->begin_element(
                        document.preview.context->make_id("preview.overlay"));
                    build_preview_overlay(document.preview.context, document.preview);
                    document.preview.context->end_element();
                    document.preview.context->pop_layer();
                    luexp(document.preview.context->apply_layout(preview_node,
                        RectF(0.0f, 0.0f, document.preview.node_size.x,
                            document.preview.node_size.y)));
                    document.preview.context->route_input();
                    bool preview_resized = process_preview_resize(document.preview,
                        resize_handle);
                    if(document.preview.instance)
                        luexp(document.preview.instance->resolve_interactions(
                            document.preview.context));
                    if(preview_resized || (document.preview.instance &&
                        document.preview.instance->relayout_requested()))
                    {
                        document.preview.context->set_layout_config(preview_node,
                            fixed_layout(document.preview.node_size.x,
                                document.preview.node_size.y));
                        luexp(document.preview.context->apply_layout(preview_node,
                            RectF(0.0f, 0.0f, document.preview.node_size.x,
                                document.preview.node_size.y)));
                    }
                    luexp(document.preview.context->generate_draw_commands());
                }
                lucatchret;
                return ok;
            }

            RV EditorApp::render_frame(DocumentView* preview_document)
            {
                lutry
                {
                    if(preview_document && preview_document->preview.target &&
                        preview_document->preview.context)
                    {
                        GUI::RenderTargetDesc preview_target(preview_document->preview.target);
                        preview_target.color_load_op = RHI::LoadOp::clear;
                        preview_target.color_clear_value = Float4U(0.26f, 0.26f, 0.26f, 1.0f);
                        preview_target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
                        const PreviewState& preview = preview_document->preview;
                        Float2U center((f32)PREVIEW_LAYOUT_WIDTH * 0.5f,
                            (f32)PREVIEW_LAYOUT_HEIGHT * 0.5f);
                        f32 translation_x = center.x + preview.pan.x - preview.zoom * center.x;
                        f32 translation_y = center.y + preview.pan.y - preview.zoom * center.y;
                        Float4x4 canvas_transform(
                            preview.zoom, 0.0f, 0.0f, 0.0f,
                            0.0f, preview.zoom, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            translation_x, translation_y, 0.0f, 1.0f);
                        Float4x4 projection = ProjectionMatrix::make_orthographic_off_center(
                            0.0f, preview.viewport_size.x, preview.viewport_size.y,
                            0.0f, 0.0f, 1.0f);
                        GUI::RenderSurfaceDesc preview_surface;
                        preview_surface.use_custom_transform = true;
                        preview_surface.surface_to_clip = mul(canvas_transform, projection);
                        if(preview.zoom == 1.0f && preview.pan == Float2U(0.0f))
                        {
                            luexp(preview_renderer->render(preview_document->preview.context,
                                cmdbuf, preview_target));
                        }
                        else
                        {
                            luexp(preview_renderer->render(preview_document->preview.context,
                                cmdbuf, preview_target, preview_surface));
                        }
                    }
                    GUI::RenderTargetDesc editor_target(gui_target);
                    editor_target.color_load_op = RHI::LoadOp::clear;
                    editor_target.color_clear_value = Float4U(0.06f, 0.07f, 0.09f, 1.0f);
                    editor_target.color_final_state = RHI::TextureStateFlag::shader_read_ps;
                    luexp(renderer->render(gui, cmdbuf, editor_target));
                    lulet(back_buffer, swap_chain->get_current_back_buffer());
                    blit->reset();
                    RHI::SamplerDesc sampler(RHI::Filter::linear, RHI::Filter::linear,
                        RHI::Filter::nearest, RHI::TextureAddressMode::clamp,
                        RHI::TextureAddressMode::clamp, RHI::TextureAddressMode::clamp);
                    blit->blit(back_buffer, RHI::SubresourceIndex(0, 0),
                        RHI::TextureViewDesc::tex2d(gui_target), sampler,
                        Float2U(0.0f), Float2U((f32)width, 0.0f),
                        Float2U(0.0f, (f32)height), Float2U((f32)width, (f32)height));
                    luexp(blit->commit(cmdbuf, false));
                    cmdbuf->resource_barrier({}, {
                        {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES,
                            RHI::TextureStateFlag::automatic,
                            RHI::TextureStateFlag::present,
                            RHI::ResourceBarrierFlag::none}
                    });
                    luexp(cmdbuf->submit({}, {}, true));
                    cmdbuf->wait();
                    luexp(cmdbuf->reset());
                    luexp(swap_chain->present());
                }
                lucatchret;
                return ok;
            }
        }
    }
}
