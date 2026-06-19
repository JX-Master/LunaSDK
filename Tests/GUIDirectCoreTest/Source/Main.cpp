/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/GUI/Editor.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/ShapeBuffer.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/VG.hpp>

using namespace Luna;

#define lutest luassert_always

static String g_test_clipboard_text;

static GUICore::LayoutInput fixed_layout(f32 width, f32 height)
{
    GUICore::LayoutInput layout;
    if(width > 0.0f)
    {
        layout.width.kind = GUICore::SizeKind::pixels;
        layout.width.value = width;
    }
    if(height > 0.0f)
    {
        layout.height.kind = GUICore::SizeKind::pixels;
        layout.height.value = height;
    }
    return layout;
}

static RV test_get_clipboard_text(String& out_text, void*)
{
    out_text = g_test_clipboard_text;
    return ok;
}

static RV test_set_clipboard_text(const c8* text, usize size, void*)
{
    g_test_clipboard_text.assign(text, size);
    return ok;
}

int main()
{
    Luna::init();
    lupanic_if_failed(add_modules({module_rhi(), module_font(), module_vg(), GUICore::module_gui_core(), GUI::module_gui()}));
    lupanic_if_failed(init_modules());

    {
    Ref<GUICore::IContext> context = GUICore::new_context();
    GUICore::FrameDesc frame;
    frame.screen_size = Float2U(640.0f, 360.0f);
    frame.framebuffer_size = UInt2U(640, 360);

    GUICore::InputEvent move;
    move.type = GUICore::InputEventType::pointer_move;
    move.position = Float2U(32.0f, 24.0f);
    GUICore::InputEvent down = move;
    down.type = GUICore::InputEventType::pointer_down;
    down.button = GUICore::PointerButton::left;
    GUICore::InputEvent up = down;
    up.type = GUICore::InputEventType::pointer_up;
    GUICore::InputEvent right_down = move;
    right_down.type = GUICore::InputEventType::pointer_down;
    right_down.button = GUICore::PointerButton::right;
    GUICore::InputEvent right_up = right_down;
    right_up.type = GUICore::InputEventType::pointer_up;
    GUICore::InputEvent key_down;
    key_down.type = GUICore::InputEventType::key_down;
    key_down.key = KeyCode::n;
    key_down.modifiers = GUICore::KeyModifierFlag::ctrl;

    context->begin_frame(frame);
    context->add_input_event(move);
    context->add_input_event(down);
    context->add_input_event(up);
    context->add_input_event(right_down);
    context->add_input_event(right_up);
    context->add_input_event(key_down);
    GUI::register_editor_style_schemas(context.get());
    context->define_style(Name("test"));
    context->set_style_value(Name("test"), Name("gui.editor.button.background"),
        GUICore::style_f32x4(Float4U(0.18f, 0.34f, 0.58f, 1.0f)));
    context->push_style(Name("test"));
    context->push_layer(1, Float2U(0.0f), Name("default"));

    GUICore::ElementHandle root = context->begin_element(1, Name("root"));
    GUICore::LayoutResult root_layout;
    root_layout.rect = RectF(0.0f, 0.0f, 640.0f, 360.0f);
    root_layout.clip_rect = root_layout.rect;
    context->set_layout_result(root, root_layout);

    GUICore::ElementHandle button = GUI::text_button(context.get(), 2, "Apply");
    GUICore::LayoutResult button_layout;
    button_layout.rect = RectF(16.0f, 12.0f, 120.0f, 32.0f);
    button_layout.clip_rect = button_layout.rect;
    context->set_layout_result(button, button_layout);

    GUICore::ElementHandle label = GUI::text(context.get(), 3, "Direct GUI package");
    GUICore::LayoutResult label_layout;
    label_layout.rect = RectF(16.0f, 52.0f, 220.0f, 24.0f);
    label_layout.clip_rect = label_layout.rect;
    context->set_layout_result(label, label_layout);

    GUICore::ElementHandle selectable = GUI::selectable(context.get(), 4, "Selected", true);
    GUICore::LayoutResult selectable_layout;
    selectable_layout.rect = RectF(16.0f, 84.0f, 160.0f, 28.0f);
    selectable_layout.clip_rect = selectable_layout.rect;
    context->set_layout_result(selectable, selectable_layout);

    GUICore::ElementHandle checkbox = GUI::checkbox(context.get(), 5, "Checked", true);
    GUICore::LayoutResult checkbox_layout;
    checkbox_layout.rect = RectF(16.0f, 120.0f, 160.0f, 28.0f);
    checkbox_layout.clip_rect = checkbox_layout.rect;
    context->set_layout_result(checkbox, checkbox_layout);

    GUICore::ElementHandle radio = GUI::radio_button(context.get(), 6, "Radio", true);
    GUICore::LayoutResult radio_layout;
    radio_layout.rect = RectF(16.0f, 156.0f, 160.0f, 28.0f);
    radio_layout.clip_rect = radio_layout.rect;
    context->set_layout_result(radio, radio_layout);

    GUICore::ElementHandle toggle = GUI::toggle_switch(context.get(), 7, "Switch", true);
    GUICore::LayoutResult toggle_layout;
    toggle_layout.rect = RectF(16.0f, 196.0f, 180.0f, 28.0f);
    toggle_layout.clip_rect = toggle_layout.rect;
    context->set_layout_result(toggle, toggle_layout);

    GUICore::ElementHandle progress = GUI::progress_bar(context.get(), 8, 0.62f);
    GUICore::LayoutResult progress_layout;
    progress_layout.rect = RectF(16.0f, 236.0f, 180.0f, 24.0f);
    progress_layout.clip_rect = progress_layout.rect;
    context->set_layout_result(progress, progress_layout);

    f32 slider_float_value = 0.25f;
    GUICore::ElementHandle slider_float = GUI::slider_float(context.get(), 70, &slider_float_value, 0.0f, 1.0f);
    GUICore::LayoutResult slider_float_layout;
    slider_float_layout.rect = RectF(16.0f, 272.0f, 180.0f, 24.0f);
    slider_float_layout.clip_rect = slider_float_layout.rect;
    context->set_layout_result(slider_float, slider_float_layout);

    i32 slider_int_value = 4;
    GUICore::ElementHandle slider_int = GUI::slider_int(context.get(), 71, &slider_int_value, 0, 10);
    GUICore::LayoutResult slider_int_layout;
    slider_int_layout.rect = RectF(16.0f, 304.0f, 180.0f, 24.0f);
    slider_int_layout.clip_rect = slider_int_layout.rect;
    context->set_layout_result(slider_int, slider_int_layout);

    f32 slider_float3_value[3] = {0.0f, 0.5f, 1.0f};
    GUICore::ElementHandle slider_float3 = GUI::slider_float3(context.get(), 72, slider_float3_value, 0.0f, 1.0f);
    GUICore::LayoutResult slider_float3_layout;
    slider_float3_layout.rect = RectF(440.0f, 12.0f, 180.0f, 60.0f);
    slider_float3_layout.clip_rect = slider_float3_layout.rect;
    context->set_layout_result(slider_float3, slider_float3_layout);

    i32 slider_int3_value[3] = {1, 5, 9};
    GUICore::ElementHandle slider_int3 = GUI::slider_int3(context.get(), 73, slider_int3_value, 0, 10);
    GUICore::LayoutResult slider_int3_layout;
    slider_int3_layout.rect = RectF(440.0f, 84.0f, 180.0f, 60.0f);
    slider_int3_layout.clip_rect = slider_int3_layout.rect;
    context->set_layout_result(slider_int3, slider_int3_layout);

    GUICore::ElementHandle header;
    bool header_open = GUI::collapsing_header(context.get(), 9, "Section", true, GUICore::LayoutInput(), &header);
    lutest(header_open);
    GUICore::LayoutResult header_layout;
    header_layout.rect = RectF(240.0f, 12.0f, 180.0f, 30.0f);
    header_layout.clip_rect = header_layout.rect;
    context->set_layout_result(header, header_layout);

    GUICore::ElementHandle tree;
    bool tree_open = GUI::tree_node(context.get(), 10, "Tree", GUI::TreeNodeFlag::default_open, 0, GUICore::LayoutInput(), &tree);
    lutest(tree_open);
    GUICore::LayoutResult tree_layout;
    tree_layout.rect = RectF(240.0f, 52.0f, 180.0f, 26.0f);
    tree_layout.clip_rect = tree_layout.rect;
    context->set_layout_result(tree, tree_layout);

    GUICore::ElementHandle leaf;
    bool leaf_open = GUI::tree_node(context.get(), 11, "Leaf", GUI::TreeNodeFlag::leaf | GUI::TreeNodeFlag::selected,
        1, GUICore::LayoutInput(), &leaf);
    lutest(!leaf_open);
    GUICore::LayoutResult leaf_layout;
    leaf_layout.rect = RectF(258.0f, 80.0f, 180.0f, 26.0f);
    leaf_layout.clip_rect = leaf_layout.rect;
    context->set_layout_result(leaf, leaf_layout);

    context->end_element();
    context->pop_layer();
    context->pop_style();
    context->route_input();

    GUICore::InteractionState button_state = context->get_interaction_state(2);
    lutest(context->get_pointer_position().x == 32.0f);
    lutest(context->get_pointer_position().y == 24.0f);
    lutest(context->is_pointer_inside());
    lutest(context->is_key_down(KeyCode::n));
    lutest(((u8)context->get_key_modifiers() & (u8)GUICore::KeyModifierFlag::ctrl) != 0);
    lutest(button_state.hovered);
    lutest(button_state.clicked);
    lutest(button_state.pointer_element_rect.width == 120.0f);
    lutest(button_state.clicked_element_rect.width == 120.0f);
    lutest(GUI::is_item_valid(context.get(), button));
    lutest(GUI::is_item_hovered(context.get(), button));
    lutest(GUI::is_item_clicked(context.get(), button));
    lutest(GUI::is_item_right_clicked(context.get(), button));
    lutest(!GUI::is_item_double_clicked(context.get(), button));
    lutest(GUI::is_item_focused(context.get(), button));
    lutest(GUI::get_item_rect(context.get(), button).width == 120.0f);
    lutest(GUI::get_item_clip_rect(context.get(), button).height == 32.0f);
    GUICore::ElementHandle invalid_handle;
    lutest(!GUI::is_item_valid(context.get(), invalid_handle));
    lutest(!GUI::is_item_clicked(context.get(), invalid_handle));
    lutest(GUI::get_item_rect(context.get(), invalid_handle).width == 0.0f);
    lutest(context->get_interaction_state(3).hovered == false);
    lutest(context->get_interaction_state(4).hovered == false);
    lutest(context->get_interaction_state(5).hovered == false);
    lutest(context->get_interaction_state(6).hovered == false);
    lutest(context->get_interaction_state(7).hovered == false);
    lutest(context->get_interaction_state(8).hovered == false);
    lutest(context->get_interaction_state(9).hovered == false);
    lutest(context->get_interaction_state(10).hovered == false);
    lutest(context->get_interaction_state(11).hovered == false);
    object_t switch_state = context->get_state(GUICore::make_state_id<GUI::SwitchAnimationState>(7));
    lutest(switch_state != nullptr);
    GUI::SwitchAnimationState* typed_switch_state = cast_object<GUI::SwitchAnimationState>(switch_state);
    lutest(typed_switch_state && typed_switch_state->initialized);
    object_t disclosure_state = context->get_state(GUICore::make_state_id<GUI::DisclosureState>(9));
    lutest(disclosure_state != nullptr);
    GUI::DisclosureState* typed_disclosure_state = cast_object<GUI::DisclosureState>(disclosure_state);
    lutest(typed_disclosure_state && typed_disclosure_state->open_initialized && typed_disclosure_state->open);
    object_t tree_disclosure_state = context->get_state(GUICore::make_state_id<GUI::DisclosureState>(10));
    lutest(tree_disclosure_state != nullptr);
    GUI::DisclosureState* typed_tree_disclosure_state = cast_object<GUI::DisclosureState>(tree_disclosure_state);
    lutest(typed_tree_disclosure_state && typed_tree_disclosure_state->open_initialized && typed_tree_disclosure_state->open);
    object_t leaf_disclosure_state = context->get_state(GUICore::make_state_id<GUI::DisclosureState>(11));
    lutest(leaf_disclosure_state != nullptr);
    GUI::DisclosureState* typed_leaf_disclosure_state = cast_object<GUI::DisclosureState>(leaf_disclosure_state);
    lutest(typed_leaf_disclosure_state && typed_leaf_disclosure_state->open_initialized && !typed_leaf_disclosure_state->open);

    GUICore::DebugInfo debug = context->dump_debug_info();
    lutest(debug.elements.size() == 15);
    lutest(debug.counters.interactable_count == 12);
    lutest(debug.counters.style_schema_count == 136);
    lutest(debug.draw_commands.size() == 55);
    for(const GUICore::DrawCommand& command : debug.draw_commands)
    {
        lutest(command.rect_reference == GUICore::DrawCommandRectReference::element);
    }

    Ref<VG::IShapeDrawList> draw_list = VG::new_shape_draw_list();
    lupanic_if_failed(context->compile_draw_commands(draw_list));

    context->begin_frame(frame);
    context->add_input_event(move);
    context->add_input_event(down);
    context->add_input_event(up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    button = GUI::text_button(context.get(), 2, "Apply");
    context->set_layout_result(button, button_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(GUI::is_item_clicked(context.get(), button));
    lutest(GUI::is_item_double_clicked(context.get(), button));
    lutest(context->get_interaction_state(1).subtree_clicked);
    lutest(context->get_interaction_state(1).subtree_double_clicked);
    GUICore::DebugInfo double_click_debug = context->dump_debug_info();
    lutest(double_click_debug.elements.size() == 2);
    lutest(double_click_debug.elements[1].clicked);
    lutest(double_click_debug.elements[1].double_clicked);
    lutest(double_click_debug.elements[0].subtree_double_clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    GUICore::ElementHandle debug_panel = GUI::show_debug_info(context.get(), 8000, debug);
    context->pop_layer();
    lupanic_if_failed(GUI::layout_editor_tree(context.get(), debug_panel, RectF(0.0f, 0.0f, 640.0f, 360.0f)));
    context->route_input();
    GUICore::DebugInfo panel_debug = context->dump_debug_info();
    lutest(context->find_element(8000) != nullptr);
    lutest(panel_debug.elements.size() > debug.elements.size());
    object_t panel_state_obj = context->get_state(GUICore::make_state_id<GUI::CoreDebugPanelState>(8000));
    lutest(panel_state_obj != nullptr);
    GUI::CoreDebugPanelState* panel_state = cast_object<GUI::CoreDebugPanelState>(panel_state_obj);
    lutest(panel_state && panel_state->selected_element == debug.elements.front().id);
    RectF debug_tree_viewport_rect;
    bool found_initial_tree_viewport = false;
    for(const GUICore::DebugElementInfo& element : panel_debug.elements)
    {
        if(element.debug_name == Name("Element Tree Viewport"))
        {
            debug_tree_viewport_rect = element.rect;
            found_initial_tree_viewport = true;
            break;
        }
    }
    lutest(found_initial_tree_viewport);

    GUICore::InputEvent debug_panel_wheel = move;
    debug_panel_wheel.type = GUICore::InputEventType::pointer_wheel;
    debug_panel_wheel.position = Float2U(
        debug_tree_viewport_rect.offset_x + debug_tree_viewport_rect.width * 0.5f,
        debug_tree_viewport_rect.offset_y + min(debug_tree_viewport_rect.height * 0.5f, 48.0f));
    debug_panel_wheel.wheel_delta = Float2U(0.0f, -1.0f);
    context->begin_frame(frame);
    context->add_input_event(debug_panel_wheel);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    debug_panel = GUI::show_debug_info(context.get(), 8000, debug);
    context->pop_layer();
    lupanic_if_failed(GUI::layout_editor_tree(context.get(), debug_panel, RectF(0.0f, 0.0f, 640.0f, 360.0f)));
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    debug_panel = GUI::show_debug_info(context.get(), 8000, debug);
    context->pop_layer();
    lupanic_if_failed(GUI::layout_editor_tree(context.get(), debug_panel, RectF(0.0f, 0.0f, 640.0f, 360.0f)));
    panel_debug = context->dump_debug_info();
    GUICore::id_t tree_viewport_id = 0;
    for(const GUICore::DebugElementInfo& element : panel_debug.elements)
    {
        if(element.debug_name == Name("Element Tree Viewport"))
        {
            tree_viewport_id = element.id;
            break;
        }
    }
    lutest(tree_viewport_id != 0);
    object_t tree_scroll_state_obj = context->get_state(GUICore::make_state_id<GUI::CoreScrollViewState>(tree_viewport_id));
    lutest(tree_scroll_state_obj != nullptr);
    GUI::CoreScrollViewState* tree_scroll_state = cast_object<GUI::CoreScrollViewState>(tree_scroll_state_obj);
    lutest(tree_scroll_state && tree_scroll_state->scroll.y > 0.0f);
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUI::draw_rect(context.get(), 20, RectF(12.0f, 16.0f, 48.0f, 24.0f), Float4U(1.0f, 0.0f, 0.0f, 1.0f), 3.0f);
    GUI::draw_circle(context.get(), 21, Float2U(92.0f, 28.0f), 12.0f, Float4U(0.0f, 1.0f, 0.0f, 1.0f));
    GUI::draw_line(context.get(), 22, Float2U(120.0f, 16.0f), Float2U(168.0f, 40.0f), Float4U(0.0f, 0.0f, 1.0f, 1.0f), 2.5f);
    GUI::draw_text(context.get(), 23, RectF(12.0f, 48.0f, 180.0f, 28.0f), "Draw Text",
        Float4U(1.0f), 18.0f, GUI::TextAlignment::center, GUI::TextAlignment::end);
    GUI::draw_image(context.get(), 24, nullptr, RectF(12.0f, 84.0f, 48.0f, 32.0f),
        Float4U(1.0f), (GUI::ImageFlag)((u32)GUI::ImageFlag::flip_y | (u32)GUI::ImageFlag::nearest));
    Ref<VG::IShapeBuffer> icon_buffer = VG::new_shape_buffer();
    Vector<f32>& icon_points = icon_buffer->get_shape_points(true);
    VG::ShapeBuilder::add_triangle_filled(icon_points, 0.0f, 16.0f, 8.0f, 0.0f, 16.0f, 16.0f);
    GUICore::ShapeDesc icon_shape;
    icon_shape.buffer = icon_buffer.get();
    icon_shape.first_command = 0;
    icon_shape.num_commands = (u32)icon_points.size();
    icon_shape.bounds = RectF(0.0f, 0.0f, 16.0f, 16.0f);
    GUICore::ElementHandle shape_button = GUI::shape_button(context.get(), 30, "ShapeButton", icon_shape);
    GUICore::LayoutResult shape_button_layout;
    shape_button_layout.rect = RectF(72.0f, 84.0f, 48.0f, 32.0f);
    shape_button_layout.clip_rect = shape_button_layout.rect;
    context->set_layout_result(shape_button, shape_button_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();

    GUICore::DebugInfo drawing_debug = context->dump_debug_info();
    lutest(drawing_debug.elements.size() == 7);
    lutest(drawing_debug.draw_commands.size() == 7);
    lutest(drawing_debug.draw_commands[0].type == GUICore::DrawCommandType::rounded_rect);
    lutest(drawing_debug.draw_commands[0].rect_reference == GUICore::DrawCommandRectReference::layer);
    lutest(drawing_debug.draw_commands[0].rect.offset_x == 12.0f);
    lutest(drawing_debug.draw_commands[0].radius == 3.0f);
    lutest(drawing_debug.draw_commands[1].type == GUICore::DrawCommandType::rounded_rect);
    lutest(drawing_debug.draw_commands[1].rect.offset_x == 80.0f);
    lutest(drawing_debug.draw_commands[1].radius == 12.0f);
    lutest(drawing_debug.draw_commands[2].type == GUICore::DrawCommandType::line);
    lutest(drawing_debug.draw_commands[2].line_width == 2.5f);
    lutest(drawing_debug.draw_commands[2].point1.x == 168.0f);
    lutest(drawing_debug.draw_commands[3].type == GUICore::DrawCommandType::text);
    lutest(!strcmp(drawing_debug.draw_commands[3].text.c_str(), "Draw Text"));
    lutest(drawing_debug.draw_commands[3].horizontal_alignment == VG::TextAlignment::center);
    lutest(drawing_debug.draw_commands[3].vertical_alignment == VG::TextAlignment::end);
    lutest(drawing_debug.draw_commands[4].type == GUICore::DrawCommandType::image);
    lutest(drawing_debug.draw_commands[4].nearest_sampler);
    lutest(drawing_debug.draw_commands[4].min_texcoord.y == 1.0f);
    lutest(drawing_debug.draw_commands[4].max_texcoord.y == 0.0f);
    lutest(drawing_debug.draw_commands[5].type == GUICore::DrawCommandType::rounded_rect);
    lutest(drawing_debug.draw_commands[5].rect_reference == GUICore::DrawCommandRectReference::element);
    lutest(drawing_debug.draw_commands[6].type == GUICore::DrawCommandType::shape);
    lutest(drawing_debug.draw_commands[6].shape.buffer == icon_buffer.get());
    lutest(drawing_debug.draw_commands[6].rect.offset_x == 6.0f);
    lutest(context->find_element(20)->layout_result.rect.width == 48.0f);
    lutest(context->find_element(30)->layout_result.rect.width == 48.0f);
    lupanic_if_failed(context->compile_draw_commands(draw_list));

    GUICore::InputEvent shape_button_move = move;
    shape_button_move.position = Float2U(88.0f, 100.0f);
    GUICore::InputEvent shape_button_down = shape_button_move;
    shape_button_down.type = GUICore::InputEventType::pointer_down;
    shape_button_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent shape_button_up = shape_button_down;
    shape_button_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(shape_button_move);
    context->add_input_event(shape_button_down);
    context->add_input_event(shape_button_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    shape_button = GUI::shape_button(context.get(), 30, "ShapeButton", icon_shape);
    context->set_layout_result(shape_button, shape_button_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(30).clicked);

    GUICore::LayoutResult hit_box_layout;
    hit_box_layout.rect = RectF(124.0f, 84.0f, 48.0f, 32.0f);
    hit_box_layout.clip_rect = hit_box_layout.rect;
    GUICore::InputEvent hit_box_move = move;
    hit_box_move.position = Float2U(140.0f, 100.0f);
    GUICore::InputEvent hit_box_down = hit_box_move;
    hit_box_down.type = GUICore::InputEventType::pointer_down;
    hit_box_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent hit_box_up = hit_box_down;
    hit_box_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(hit_box_move);
    context->add_input_event(hit_box_down);
    context->add_input_event(hit_box_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle hit_box = GUI::hit_box(context.get(), 31);
    context->set_layout_result(hit_box, hit_box_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(31).hovered);
    lutest(context->get_interaction_state(31).clicked);
    drawing_debug = context->dump_debug_info();
    lutest(drawing_debug.elements.size() == 2);
    lutest(drawing_debug.draw_commands.empty());

    struct DragDropTestPayload
    {
        u32 value = 0;
    };
    Name drag_payload_type("direct_core_test.payload");
    GUICore::LayoutResult scope_drag_source_layout;
    scope_drag_source_layout.rect = RectF(16.0f, 16.0f, 80.0f, 28.0f);
    scope_drag_source_layout.clip_rect = scope_drag_source_layout.rect;
    GUICore::LayoutResult scope_drag_target_layout;
    scope_drag_target_layout.rect = RectF(160.0f, 16.0f, 96.0f, 28.0f);
    scope_drag_target_layout.clip_rect = scope_drag_target_layout.rect;

    GUICore::InputEvent drag_source_move = move;
    drag_source_move.position = Float2U(32.0f, 28.0f);
    GUICore::InputEvent drag_source_down = drag_source_move;
    drag_source_down.type = GUICore::InputEventType::pointer_down;
    drag_source_down.button = GUICore::PointerButton::left;
    context->begin_frame(frame);
    context->add_input_event(drag_source_move);
    context->add_input_event(drag_source_down);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle scope_drag_source = GUI::hit_box(context.get(), 1000);
    context->set_layout_result(scope_drag_source, scope_drag_source_layout);
    GUICore::ElementHandle scope_drag_target = GUI::hit_box(context.get(), 1001);
    context->set_layout_result(scope_drag_target, scope_drag_target_layout);
    lutest(!GUI::begin_drag_drop_source(context.get(), scope_drag_source, drag_payload_type));
    lutest(!GUI::begin_drag_drop_target(context.get(), scope_drag_target, drag_payload_type));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(1000).active);

    GUICore::InputEvent drag_target_move = move;
    drag_target_move.position = Float2U(180.0f, 28.0f);
    GUICore::InputEvent drag_target_up = drag_target_move;
    drag_target_up.type = GUICore::InputEventType::pointer_up;
    drag_target_up.button = GUICore::PointerButton::left;
    context->begin_frame(frame);
    context->add_input_event(drag_target_move);
    context->add_input_event(drag_target_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    scope_drag_source = GUI::hit_box(context.get(), 1000);
    context->set_layout_result(scope_drag_source, scope_drag_source_layout);
    scope_drag_target = GUI::hit_box(context.get(), 1001);
    context->set_layout_result(scope_drag_target, scope_drag_target_layout);
    lutest(GUI::begin_drag_drop_source(context.get(), scope_drag_source, drag_payload_type));
    DragDropTestPayload drag_payload;
    drag_payload.value = 42;
    GUI::set_drag_drop_payload(context.get(), &drag_payload, sizeof(drag_payload));
    GUI::end_drag_drop_source(context.get());
    lutest(GUI::begin_drag_drop_target(context.get(), scope_drag_target, drag_payload_type));
    context->end_element();
    context->pop_layer();
    context->route_input();
    const GUICore::DragDropPayload* delivered_payload = GUI::accept_drag_drop_payload(context.get(), drag_payload_type);
    lutest(delivered_payload && delivered_payload->delivery);
    const DragDropTestPayload* delivered_data = delivered_payload->data_as<DragDropTestPayload>();
    lutest(delivered_data && delivered_data->value == 42);
    GUI::end_drag_drop_target(context.get());

    bool disabled_checkbox_value = false;
    bool disabled_radio_value = false;
    bool disabled_switch_value = false;
    i32 disabled_button_group_current = 0;
    String disabled_input_value = "Disabled";
    GUICore::LayoutResult disabled_button_layout;
    disabled_button_layout.rect = RectF(200.0f, 16.0f, 120.0f, 28.0f);
    disabled_button_layout.clip_rect = disabled_button_layout.rect;
    GUICore::LayoutResult disabled_checkbox_layout;
    disabled_checkbox_layout.rect = RectF(200.0f, 52.0f, 180.0f, 28.0f);
    disabled_checkbox_layout.clip_rect = disabled_checkbox_layout.rect;
    GUICore::LayoutResult disabled_radio_layout;
    disabled_radio_layout.rect = RectF(200.0f, 88.0f, 180.0f, 28.0f);
    disabled_radio_layout.clip_rect = disabled_radio_layout.rect;
    GUICore::LayoutResult disabled_switch_layout;
    disabled_switch_layout.rect = RectF(200.0f, 124.0f, 180.0f, 28.0f);
    disabled_switch_layout.clip_rect = disabled_switch_layout.rect;
    GUICore::LayoutResult disabled_button_group_layout;
    disabled_button_group_layout.rect = RectF(200.0f, 160.0f, 180.0f, 28.0f);
    disabled_button_group_layout.clip_rect = disabled_button_group_layout.rect;
    GUICore::LayoutResult disabled_input_layout;
    disabled_input_layout.rect = RectF(200.0f, 196.0f, 180.0f, 28.0f);
    disabled_input_layout.clip_rect = disabled_input_layout.rect;

    auto add_click = [&](const Float2U& position) {
        GUICore::InputEvent click_move = move;
        click_move.position = position;
        GUICore::InputEvent click_down = click_move;
        click_down.type = GUICore::InputEventType::pointer_down;
        click_down.button = GUICore::PointerButton::left;
        GUICore::InputEvent click_up = click_down;
        click_up.type = GUICore::InputEventType::pointer_up;
        context->add_input_event(click_move);
        context->add_input_event(click_down);
        context->add_input_event(click_up);
    };

    const c8* disabled_group_items[] = {"One", "Two", "Three"};
    context->begin_frame(frame);
    add_click(Float2U(220.0f, 30.0f));
    add_click(Float2U(214.0f, 66.0f));
    add_click(Float2U(214.0f, 102.0f));
    add_click(Float2U(214.0f, 138.0f));
    add_click(Float2U(270.0f, 174.0f));
    add_click(Float2U(214.0f, 210.0f));
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle disabled_button = GUI::text_button(context.get(), 900, "Disabled", GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_button, disabled_button_layout);
    GUICore::ElementHandle disabled_checkbox = GUI::checkbox(context.get(), 901, "Disabled checkbox",
        &disabled_checkbox_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_checkbox, disabled_checkbox_layout);
    GUICore::ElementHandle disabled_radio = GUI::radio_button(context.get(), 902, "Disabled radio",
        &disabled_radio_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_radio, disabled_radio_layout);
    GUICore::ElementHandle disabled_switch = GUI::toggle_switch(context.get(), 903, "Disabled switch",
        &disabled_switch_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_switch, disabled_switch_layout);
    GUICore::ElementHandle disabled_button_group = GUI::button_group(context.get(), 904, &disabled_button_group_current,
        Span<const c8*>(disabled_group_items, 3), GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_button_group, disabled_button_group_layout);
    GUICore::ElementHandle disabled_input = GUI::input_text(context.get(), 905, disabled_input_value,
        GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_input, disabled_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!context->get_interaction_state(900).hovered && !context->get_interaction_state(900).clicked);
    lutest(!context->get_interaction_state(901).hovered && !context->get_interaction_state(901).clicked);
    lutest(!context->get_interaction_state(902).hovered && !context->get_interaction_state(902).clicked);
    lutest(!context->get_interaction_state(903).hovered && !context->get_interaction_state(903).clicked);
    lutest(!context->get_interaction_state(904).hovered && !context->get_interaction_state(904).clicked);
    lutest(!context->get_interaction_state(905).hovered && !context->get_interaction_state(905).clicked);
    lutest(context->focused_element() != 905);
    GUICore::DebugInfo disabled_debug = context->dump_debug_info();
    const GUICore::DebugElementInfo* disabled_info = nullptr;
    for(const GUICore::DebugElementInfo& element : disabled_debug.elements)
    {
        if(element.id == 900)
        {
            disabled_info = &element;
            break;
        }
    }
    lutest(disabled_info && disabled_info->disabled);

    GUICore::InputEvent disabled_text_event;
    disabled_text_event.type = GUICore::InputEventType::text_utf8;
    disabled_text_event.text = "!";
    context->begin_frame(frame);
    context->add_input_event(disabled_text_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    disabled_checkbox = GUI::checkbox(context.get(), 901, "Disabled checkbox",
        &disabled_checkbox_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_checkbox, disabled_checkbox_layout);
    disabled_radio = GUI::radio_button(context.get(), 902, "Disabled radio",
        &disabled_radio_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_radio, disabled_radio_layout);
    disabled_switch = GUI::toggle_switch(context.get(), 903, "Disabled switch",
        &disabled_switch_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_switch, disabled_switch_layout);
    disabled_button_group = GUI::button_group(context.get(), 904, &disabled_button_group_current,
        Span<const c8*>(disabled_group_items, 3), GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_button_group, disabled_button_group_layout);
    disabled_input = GUI::input_text(context.get(), 905, disabled_input_value, GUICore::LayoutInput(), false);
    context->set_layout_result(disabled_input, disabled_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!disabled_checkbox_value);
    lutest(!disabled_radio_value);
    lutest(!disabled_switch_value);
    lutest(disabled_button_group_current == 0);
    lutest(!strcmp(disabled_input_value.c_str(), "Disabled"));

    GUICore::LayoutResult focus_scope_layout;
    focus_scope_layout.rect = RectF(16.0f, 200.0f, 360.0f, 72.0f);
    focus_scope_layout.clip_rect = focus_scope_layout.rect;
    GUICore::LayoutResult focus_a_layout;
    focus_a_layout.rect = RectF(24.0f, 208.0f, 80.0f, 28.0f);
    focus_a_layout.clip_rect = focus_a_layout.rect;
    GUICore::LayoutResult focus_disabled_layout;
    focus_disabled_layout.rect = RectF(112.0f, 208.0f, 80.0f, 28.0f);
    focus_disabled_layout.clip_rect = focus_disabled_layout.rect;
    GUICore::LayoutResult focus_b_layout;
    focus_b_layout.rect = RectF(200.0f, 208.0f, 80.0f, 28.0f);
    focus_b_layout.clip_rect = focus_b_layout.rect;
    GUICore::LayoutResult other_scope_layout;
    other_scope_layout.rect = RectF(400.0f, 200.0f, 160.0f, 72.0f);
    other_scope_layout.clip_rect = other_scope_layout.rect;
    GUICore::LayoutResult other_focus_layout;
    other_focus_layout.rect = RectF(408.0f, 208.0f, 80.0f, 28.0f);
    other_focus_layout.clip_rect = other_focus_layout.rect;
    auto build_direct_focus_scopes = [&]() {
        context->push_layer(1, Float2U(0.0f), Name("default"));
        root = context->begin_element(1, Name("root"));
        context->set_layout_result(root, root_layout);
        GUICore::ElementHandle focus_scope = GUI::begin_focus_scope(context.get(), 910, "focus scope");
        context->set_layout_result(focus_scope, focus_scope_layout);
        GUICore::ElementHandle focus_a = GUI::text_button(context.get(), 911, "A");
        context->set_layout_result(focus_a, focus_a_layout);
        GUICore::ElementHandle focus_disabled = GUI::text_button(context.get(), 913, "Disabled", GUICore::LayoutInput(), false);
        context->set_layout_result(focus_disabled, focus_disabled_layout);
        GUICore::ElementHandle focus_b = GUI::text_button(context.get(), 912, "B");
        context->set_layout_result(focus_b, focus_b_layout);
        GUI::end_focus_scope(context.get());
        GUICore::ElementHandle other_scope = GUI::begin_focus_scope(context.get(), 920, "other focus scope");
        context->set_layout_result(other_scope, other_scope_layout);
        GUICore::ElementHandle other_focus = GUI::text_button(context.get(), 921, "Other");
        context->set_layout_result(other_focus, other_focus_layout);
        GUI::end_focus_scope(context.get());
        context->end_element();
        context->pop_layer();
    };
    GUICore::InputEvent direct_tab_event;
    direct_tab_event.type = GUICore::InputEventType::key_down;
    direct_tab_event.key = KeyCode::tab;
    context->begin_frame(frame);
    context->add_input_event(direct_tab_event);
    build_direct_focus_scopes();
    context->focus_element(911);
    context->route_input();
    lutest(context->focused_element() == 912);
    lutest(!context->get_interaction_state(913).focused);
    GUICore::DebugInfo focus_scope_debug = context->dump_debug_info();
    lutest(focus_scope_debug.focused_scope == 910);

    GUICore::InputEvent direct_shift_tab_event = direct_tab_event;
    direct_shift_tab_event.modifiers = GUICore::KeyModifierFlag::shift;
    context->begin_frame(frame);
    context->add_input_event(direct_shift_tab_event);
    build_direct_focus_scopes();
    context->focus_element(912);
    context->route_input();
    lutest(context->focused_element() == 911);
    lutest(!context->get_interaction_state(921).focused);

    const c8* group_items[] = {"One", "Two", "Three"};
    i32 button_group_current = 0;
    GUICore::LayoutResult button_group_layout;
    button_group_layout.rect = RectF(200.0f, 16.0f, 180.0f, 28.0f);
    button_group_layout.clip_rect = button_group_layout.rect;
    GUICore::InputEvent button_group_move = move;
    button_group_move.position = Float2U(270.0f, 30.0f);
    GUICore::InputEvent button_group_down = button_group_move;
    button_group_down.type = GUICore::InputEventType::pointer_down;
    button_group_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent button_group_up = button_group_down;
    button_group_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(button_group_move);
    context->add_input_event(button_group_down);
    context->add_input_event(button_group_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle button_group = GUI::button_group(context.get(), 25, &button_group_current,
        Span<const c8*>(group_items, 3));
    context->set_layout_result(button_group, button_group_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(25).clicked);
    lutest(button_group_current == 0);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    button_group = GUI::button_group(context.get(), 25, &button_group_current, Span<const c8*>(group_items, 3));
    context->set_layout_result(button_group, button_group_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(button_group_current == 1);
    object_t button_group_state = context->get_state(GUICore::make_state_id<GUI::ButtonGroupAnimationState>(25));
    lutest(button_group_state != nullptr);

    bool button_group_selected[3] = {false, false, true};
    GUICore::LayoutResult multi_button_group_layout;
    multi_button_group_layout.rect = RectF(200.0f, 56.0f, 180.0f, 28.0f);
    multi_button_group_layout.clip_rect = multi_button_group_layout.rect;
    GUICore::InputEvent multi_button_group_move = move;
    multi_button_group_move.position = Float2U(270.0f, 70.0f);
    GUICore::InputEvent multi_button_group_down = multi_button_group_move;
    multi_button_group_down.type = GUICore::InputEventType::pointer_down;
    multi_button_group_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent multi_button_group_up = multi_button_group_down;
    multi_button_group_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(multi_button_group_move);
    context->add_input_event(multi_button_group_down);
    context->add_input_event(multi_button_group_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle multi_button_group = GUI::button_group(context.get(), 26,
        Span<bool>(button_group_selected, 3), Span<const c8*>(group_items, 3));
    context->set_layout_result(multi_button_group, multi_button_group_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(26).clicked);
    lutest(!button_group_selected[1]);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    multi_button_group = GUI::button_group(context.get(), 26, Span<bool>(button_group_selected, 3),
        Span<const c8*>(group_items, 3));
    context->set_layout_result(multi_button_group, multi_button_group_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(button_group_selected[1]);

    bool direct_checkbox_value = false;
    GUICore::LayoutResult direct_checkbox_layout;
    direct_checkbox_layout.rect = RectF(200.0f, 96.0f, 180.0f, 28.0f);
    direct_checkbox_layout.clip_rect = direct_checkbox_layout.rect;
    GUICore::InputEvent direct_checkbox_move = move;
    direct_checkbox_move.position = Float2U(214.0f, 110.0f);
    GUICore::InputEvent direct_checkbox_down = direct_checkbox_move;
    direct_checkbox_down.type = GUICore::InputEventType::pointer_down;
    direct_checkbox_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent direct_checkbox_up = direct_checkbox_down;
    direct_checkbox_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(direct_checkbox_move);
    context->add_input_event(direct_checkbox_down);
    context->add_input_event(direct_checkbox_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle direct_checkbox = GUI::checkbox(context.get(), 27, "Bound checkbox", &direct_checkbox_value);
    context->set_layout_result(direct_checkbox, direct_checkbox_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(27).clicked);
    lutest(!direct_checkbox_value);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    direct_checkbox = GUI::checkbox(context.get(), 27, "Bound checkbox", &direct_checkbox_value);
    context->set_layout_result(direct_checkbox, direct_checkbox_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(direct_checkbox_value);

    i32 direct_radio_value = 0;
    GUICore::LayoutResult direct_radio_layout;
    direct_radio_layout.rect = RectF(200.0f, 132.0f, 180.0f, 28.0f);
    direct_radio_layout.clip_rect = direct_radio_layout.rect;
    GUICore::InputEvent direct_radio_move = move;
    direct_radio_move.position = Float2U(214.0f, 146.0f);
    GUICore::InputEvent direct_radio_down = direct_radio_move;
    direct_radio_down.type = GUICore::InputEventType::pointer_down;
    direct_radio_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent direct_radio_up = direct_radio_down;
    direct_radio_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(direct_radio_move);
    context->add_input_event(direct_radio_down);
    context->add_input_event(direct_radio_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle direct_radio = GUI::radio_button(context.get(), 28, "Bound radio", &direct_radio_value, 7);
    context->set_layout_result(direct_radio, direct_radio_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(28).clicked);
    lutest(direct_radio_value == 0);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    direct_radio = GUI::radio_button(context.get(), 28, "Bound radio", &direct_radio_value, 7);
    context->set_layout_result(direct_radio, direct_radio_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(direct_radio_value == 7);

    bool direct_switch_value = false;
    GUICore::LayoutResult direct_switch_layout;
    direct_switch_layout.rect = RectF(200.0f, 168.0f, 180.0f, 28.0f);
    direct_switch_layout.clip_rect = direct_switch_layout.rect;
    GUICore::InputEvent direct_switch_move = move;
    direct_switch_move.position = Float2U(214.0f, 182.0f);
    GUICore::InputEvent direct_switch_down = direct_switch_move;
    direct_switch_down.type = GUICore::InputEventType::pointer_down;
    direct_switch_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent direct_switch_up = direct_switch_down;
    direct_switch_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(direct_switch_move);
    context->add_input_event(direct_switch_down);
    context->add_input_event(direct_switch_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle direct_switch = GUI::toggle_switch(context.get(), 29, "Bound switch", &direct_switch_value);
    context->set_layout_result(direct_switch, direct_switch_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(29).clicked);
    lutest(!direct_switch_value);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    direct_switch = GUI::toggle_switch(context.get(), 29, "Bound switch", &direct_switch_value);
    context->set_layout_result(direct_switch, direct_switch_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(direct_switch_value);

    GUICore::InputEvent slider_move = move;
    slider_move.position = Float2U(151.0f, 284.0f);
    GUICore::InputEvent slider_down = slider_move;
    slider_down.type = GUICore::InputEventType::pointer_down;
    slider_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent slider_up = slider_down;
    slider_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(slider_move);
    context->add_input_event(slider_down);
    context->add_input_event(slider_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_float = GUI::slider_float(context.get(), 70, &slider_float_value, 0.0f, 1.0f);
    context->set_layout_result(slider_float, slider_float_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(70).clicked);
    lutest(context->get_interaction_state(70).clicked_element_rect.width == 180.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_float = GUI::slider_float(context.get(), 70, &slider_float_value, 0.0f, 1.0f);
    context->set_layout_result(slider_float, slider_float_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(slider_float_value > 0.749f && slider_float_value < 0.751f);

    slider_move.position = Float2U(160.0f, 316.0f);
    slider_down = slider_move;
    slider_down.type = GUICore::InputEventType::pointer_down;
    slider_down.button = GUICore::PointerButton::left;
    slider_up = slider_down;
    slider_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(slider_move);
    context->add_input_event(slider_down);
    context->add_input_event(slider_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_int = GUI::slider_int(context.get(), 71, &slider_int_value, 0, 10);
    context->set_layout_result(slider_int, slider_int_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(71).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_int = GUI::slider_int(context.get(), 71, &slider_int_value, 0, 10);
    context->set_layout_result(slider_int, slider_int_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(slider_int_value == 8);

    GUICore::InputEvent slider_float3_move = move;
    slider_float3_move.position = Float2U(530.0f, 42.0f);
    GUICore::InputEvent slider_float3_down = slider_float3_move;
    slider_float3_down.type = GUICore::InputEventType::pointer_down;
    slider_float3_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent slider_float3_up = slider_float3_down;
    slider_float3_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(slider_float3_move);
    context->add_input_event(slider_float3_down);
    context->add_input_event(slider_float3_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_float3 = GUI::slider_float3(context.get(), 72, slider_float3_value, 0.0f, 1.0f);
    context->set_layout_result(slider_float3, slider_float3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(72).clicked);
    lutest(context->get_interaction_state(72).clicked_element_rect.width == 180.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_float3 = GUI::slider_float3(context.get(), 72, slider_float3_value, 0.0f, 1.0f);
    context->set_layout_result(slider_float3, slider_float3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(slider_float3_value[0] == 0.0f);
    lutest(slider_float3_value[1] > 0.499f && slider_float3_value[1] < 0.501f);
    lutest(slider_float3_value[2] == 1.0f);

    GUICore::InputEvent slider_int3_move = move;
    slider_int3_move.position = Float2U(566.0f, 134.0f);
    GUICore::InputEvent slider_int3_down = slider_int3_move;
    slider_int3_down.type = GUICore::InputEventType::pointer_down;
    slider_int3_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent slider_int3_up = slider_int3_down;
    slider_int3_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(slider_int3_move);
    context->add_input_event(slider_int3_down);
    context->add_input_event(slider_int3_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_int3 = GUI::slider_int3(context.get(), 73, slider_int3_value, 0, 10);
    context->set_layout_result(slider_int3, slider_int3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(73).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    slider_int3 = GUI::slider_int3(context.get(), 73, slider_int3_value, 0, 10);
    context->set_layout_result(slider_int3, slider_int3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(slider_int3_value[0] == 1);
    lutest(slider_int3_value[1] == 5);
    lutest(slider_int3_value[2] == 7);

    f32 slider_with_input_value = 0.25f;
    RectF slider_with_input_rect(240.0f, 116.0f, 220.0f, 28.0f);
    GUICore::InputEvent slider_with_input_move = move;
    slider_with_input_move.position = Float2U(345.0f, 130.0f);
    GUICore::InputEvent slider_with_input_down = slider_with_input_move;
    slider_with_input_down.type = GUICore::InputEventType::pointer_down;
    slider_with_input_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent slider_with_input_up = slider_with_input_down;
    slider_with_input_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle slider_with_input = GUI::slider_float_with_input(context.get(), 90, "SliderWithInput",
        &slider_with_input_value, 0.0f, 1.0f, slider_with_input_rect);
    context->end_element();
    context->pop_layer();
    context->route_input();
    const GUICore::Element* slider_with_input_element = context->find_element(90);
    lutest(slider_with_input_element && slider_with_input_element->layout_result.rect.width == 220.0f);
    GUICore::DebugInfo slider_with_input_debug = context->dump_debug_info();
    u32 slider_with_input_text_count = 0;
    u32 slider_with_input_slider_count = 0;
    for(const GUICore::DebugElementInfo& element : slider_with_input_debug.elements)
    {
        if(element.debug_name == Name("input_text"))
        {
            ++slider_with_input_text_count;
            lutest(element.rect.width == 72.0f);
        }
        if(element.debug_name == Name("slider_float"))
        {
            ++slider_with_input_slider_count;
        }
    }
    lutest(slider_with_input_text_count == 1);
    lutest(slider_with_input_slider_count == 1);
    lutest(slider_with_input.index != GUICore::INVALID_ELEMENT);

    context->begin_frame(frame);
    context->add_input_event(slider_with_input_move);
    context->add_input_event(slider_with_input_down);
    context->add_input_event(slider_with_input_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUI::slider_float_with_input(context.get(), 90, "SliderWithInput", &slider_with_input_value, 0.0f, 1.0f,
        slider_with_input_rect);
    context->end_element();
    context->pop_layer();
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUI::slider_float_with_input(context.get(), 90, "SliderWithInput", &slider_with_input_value, 0.0f, 1.0f,
        slider_with_input_rect);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(slider_with_input_value > 0.749f && slider_with_input_value < 0.751f);

    f32 drag_float_value = 1.0f;
    GUICore::LayoutResult drag_float_layout;
    drag_float_layout.rect = RectF(240.0f, 116.0f, 180.0f, 24.0f);
    drag_float_layout.clip_rect = drag_float_layout.rect;
    GUICore::InputEvent drag_float_down = move;
    drag_float_down.type = GUICore::InputEventType::pointer_down;
    drag_float_down.button = GUICore::PointerButton::left;
    drag_float_down.position = Float2U(250.0f, 128.0f);
    GUICore::InputEvent drag_float_move = drag_float_down;
    drag_float_move.type = GUICore::InputEventType::pointer_move;
    drag_float_move.position = Float2U(270.0f, 128.0f);
    GUICore::InputEvent drag_float_up = drag_float_down;
    drag_float_up.type = GUICore::InputEventType::pointer_up;
    drag_float_up.position = drag_float_move.position;

    context->begin_frame(frame);
    context->add_input_event(drag_float_down);
    context->add_input_event(drag_float_move);
    context->add_input_event(drag_float_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle drag_float = GUI::drag_float(context.get(), 75, &drag_float_value, 0.1f, 0.0f, 10.0f);
    context->set_layout_result(drag_float, drag_float_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(75).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    drag_float = GUI::drag_float(context.get(), 75, &drag_float_value, 0.1f, 0.0f, 10.0f);
    context->set_layout_result(drag_float, drag_float_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(drag_float_value > 2.999f && drag_float_value < 3.001f);

    i32 drag_int3_value[3] = {0, 10, 20};
    GUICore::LayoutResult drag_int3_layout;
    drag_int3_layout.rect = RectF(240.0f, 156.0f, 180.0f, 60.0f);
    drag_int3_layout.clip_rect = drag_int3_layout.rect;
    GUICore::InputEvent drag_int3_down = move;
    drag_int3_down.type = GUICore::InputEventType::pointer_down;
    drag_int3_down.button = GUICore::PointerButton::left;
    drag_int3_down.position = Float2U(250.0f, 186.0f);
    GUICore::InputEvent drag_int3_move = drag_int3_down;
    drag_int3_move.type = GUICore::InputEventType::pointer_move;
    drag_int3_move.position = Float2U(262.0f, 186.0f);
    GUICore::InputEvent drag_int3_up = drag_int3_down;
    drag_int3_up.type = GUICore::InputEventType::pointer_up;
    drag_int3_up.position = drag_int3_move.position;

    context->begin_frame(frame);
    context->add_input_event(drag_int3_down);
    context->add_input_event(drag_int3_move);
    context->add_input_event(drag_int3_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle drag_int3 = GUI::drag_int3(context.get(), 76, drag_int3_value, 0.5f, -100, 100);
    context->set_layout_result(drag_int3, drag_int3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(76).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    drag_int3 = GUI::drag_int3(context.get(), 76, drag_int3_value, 0.5f, -100, 100);
    context->set_layout_result(drag_int3, drag_int3_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(drag_int3_value[0] == 0);
    lutest(drag_int3_value[1] == 16);
    lutest(drag_int3_value[2] == 20);

    GUICore::LayoutInput tab_content_layout;
    tab_content_layout.height.kind = GUICore::SizeKind::pixels;
    tab_content_layout.height.value = 24.0f;
    RectF tab_bar_rect(240.0f, 116.0f, 180.0f, 100.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle tab_bar = GUI::begin_tab_bar(context.get(), 80, "tabs");
    if(GUI::begin_tab_item(context.get(), 81, "First"))
    {
        GUI::text(context.get(), 82, "First content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    if(GUI::begin_tab_item(context.get(), 83, "Second"))
    {
        GUI::text(context.get(), 84, "Second content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    lupanic_if_failed(GUI::end_tab_bar(context.get(), tab_bar, tab_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->find_element(82) != nullptr);
    lutest(context->find_element(84) == nullptr);

    GUICore::InputEvent tab_move = move;
    tab_move.position = Float2U(330.0f, 128.0f);
    GUICore::InputEvent tab_down = tab_move;
    tab_down.type = GUICore::InputEventType::pointer_down;
    tab_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent tab_up = tab_down;
    tab_up.type = GUICore::InputEventType::pointer_up;
    context->begin_frame(frame);
    context->add_input_event(tab_move);
    context->add_input_event(tab_down);
    context->add_input_event(tab_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tab_bar = GUI::begin_tab_bar(context.get(), 80, "tabs");
    if(GUI::begin_tab_item(context.get(), 81, "First"))
    {
        GUI::text(context.get(), 82, "First content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    if(GUI::begin_tab_item(context.get(), 83, "Second"))
    {
        GUI::text(context.get(), 84, "Second content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    lupanic_if_failed(GUI::end_tab_bar(context.get(), tab_bar, tab_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(83).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tab_bar = GUI::begin_tab_bar(context.get(), 80, "tabs");
    if(GUI::begin_tab_item(context.get(), 81, "First"))
    {
        GUI::text(context.get(), 82, "First content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    if(GUI::begin_tab_item(context.get(), 83, "Second"))
    {
        GUI::text(context.get(), 84, "Second content", tab_content_layout);
        GUI::end_tab_item(context.get());
    }
    lupanic_if_failed(GUI::end_tab_bar(context.get(), tab_bar, tab_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->find_element(82) == nullptr);
    lutest(context->find_element(84) != nullptr);

    String input_value = "Hi";
    GUICore::LayoutResult input_layout;
    input_layout.rect = RectF(240.0f, 236.0f, 180.0f, 28.0f);
    input_layout.clip_rect = input_layout.rect;
    GUICore::InputEvent input_move = move;
    input_move.position = Float2U(266.0f, 250.0f);
    GUICore::InputEvent input_down = input_move;
    input_down.type = GUICore::InputEventType::pointer_down;
    input_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent input_up = input_down;
    input_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(input_move);
    context->add_input_event(input_down);
    context->add_input_event(input_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(74).clicked);
    lutest(context->focused_element() == 74);

    GUICore::InputEvent text_event;
    text_event.type = GUICore::InputEventType::text_utf8;
    text_event.text = "!";
    context->begin_frame(frame);
    context->add_input_event(text_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    GUICore::TextInputState text_input_state = context->get_text_input_state();
    lutest(text_input_state.active);
    lutest(text_input_state.rect.offset_x == input_layout.rect.offset_x);
    lutest(text_input_state.rect.offset_y == input_layout.rect.offset_y);
    lutest(!strcmp(input_value.c_str(), "Hi"));

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(input_value.c_str(), "Hi!"));

    GUICore::ClipboardIO clipboard_io;
    clipboard_io.get_text = test_get_clipboard_text;
    clipboard_io.set_text = test_set_clipboard_text;
    context->set_clipboard_io(clipboard_io);
    g_test_clipboard_text = " Paste";
    GUICore::InputEvent paste_event;
    paste_event.type = GUICore::InputEventType::key_down;
    paste_event.key = KeyCode::v;
    paste_event.modifiers = GUICore::KeyModifierFlag::ctrl;
    context->begin_frame(frame);
    context->add_input_event(paste_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(input_value.c_str(), "Hi! Paste"));

    GUICore::InputEvent backspace_event;
    backspace_event.type = GUICore::InputEventType::key_down;
    backspace_event.key = KeyCode::backspace;
    context->begin_frame(frame);
    context->add_input_event(backspace_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(input_value.c_str(), "Hi! Paste"));

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    input = GUI::input_text(context.get(), 74, input_value);
    context->set_layout_result(input, input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(input_value.c_str(), "Hi! Past"));

    String readonly_input_value = "Locked";
    GUICore::LayoutResult readonly_input_layout;
    readonly_input_layout.rect = RectF(240.0f, 272.0f, 180.0f, 28.0f);
    readonly_input_layout.clip_rect = readonly_input_layout.rect;
    GUICore::InputEvent readonly_input_move = move;
    readonly_input_move.position = Float2U(266.0f, 286.0f);
    GUICore::InputEvent readonly_input_down = readonly_input_move;
    readonly_input_down.type = GUICore::InputEventType::pointer_down;
    readonly_input_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent readonly_input_up = readonly_input_down;
    readonly_input_up.type = GUICore::InputEventType::pointer_up;
    context->begin_frame(frame);
    context->add_input_event(readonly_input_move);
    context->add_input_event(readonly_input_down);
    context->add_input_event(readonly_input_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle readonly_input = GUI::input_text(context.get(), 75, readonly_input_value,
        GUICore::LayoutInput(), true, true);
    context->set_layout_result(readonly_input, readonly_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!context->get_interaction_state(75).clicked);
    lutest(context->focused_element() == 75);

    GUICore::InputEvent readonly_text_event;
    readonly_text_event.type = GUICore::InputEventType::text_utf8;
    readonly_text_event.text = "!";
    context->begin_frame(frame);
    context->add_input_event(readonly_text_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    readonly_input = GUI::input_text(context.get(), 75, readonly_input_value, GUICore::LayoutInput(), true, true);
    context->set_layout_result(readonly_input, readonly_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();

    g_test_clipboard_text = " Mutated";
    context->begin_frame(frame);
    context->add_input_event(paste_event);
    context->add_input_event(backspace_event);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    readonly_input = GUI::input_text(context.get(), 75, readonly_input_value, GUICore::LayoutInput(), true, true);
    context->set_layout_result(readonly_input, readonly_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(readonly_input_value.c_str(), "Locked"));

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    readonly_input = GUI::input_text(context.get(), 75, readonly_input_value, GUICore::LayoutInput(), true, true);
    context->set_layout_result(readonly_input, readonly_input_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!strcmp(readonly_input_value.c_str(), "Locked"));

    GUICore::InputEvent header_move = move;
    header_move.position = Float2U(260.0f, 24.0f);
    GUICore::InputEvent header_down = header_move;
    header_down.type = GUICore::InputEventType::pointer_down;
    header_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent header_up = header_down;
    header_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(header_move);
    context->add_input_event(header_down);
    context->add_input_event(header_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    header_open = GUI::collapsing_header(context.get(), 9, "Section", true, GUICore::LayoutInput(), &header);
    lutest(header_open);
    context->set_layout_result(header, header_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(9).hovered);
    lutest(context->get_interaction_state(9).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    header_open = GUI::collapsing_header(context.get(), 9, "Section", true, GUICore::LayoutInput(), &header);
    context->set_layout_result(header, header_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!header_open);
    disclosure_state = context->get_state(GUICore::make_state_id<GUI::DisclosureState>(9));
    lutest(disclosure_state != nullptr);
    typed_disclosure_state = cast_object<GUI::DisclosureState>(disclosure_state);
    lutest(typed_disclosure_state && typed_disclosure_state->open_initialized && !typed_disclosure_state->open);

    GUICore::InputEvent tree_move = move;
    tree_move.position = Float2U(260.0f, 64.0f);
    GUICore::InputEvent tree_down = tree_move;
    tree_down.type = GUICore::InputEventType::pointer_down;
    tree_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent tree_up = tree_down;
    tree_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(tree_move);
    context->add_input_event(tree_down);
    context->add_input_event(tree_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 10, "Tree", GUI::TreeNodeFlag::default_open, 0, GUICore::LayoutInput(), &tree);
    lutest(tree_open);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(10).hovered);
    lutest(context->get_interaction_state(10).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 10, "Tree", GUI::TreeNodeFlag::default_open, 0, GUICore::LayoutInput(), &tree);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!tree_open);
    tree_disclosure_state = context->get_state(GUICore::make_state_id<GUI::DisclosureState>(10));
    lutest(tree_disclosure_state != nullptr);
    typed_tree_disclosure_state = cast_object<GUI::DisclosureState>(tree_disclosure_state);
    lutest(typed_tree_disclosure_state && typed_tree_disclosure_state->open_initialized && !typed_tree_disclosure_state->open);

    GUICore::InputEvent tree_text_move = move;
    tree_text_move.position = Float2U(340.0f, 64.0f);
    GUICore::InputEvent tree_text_down = tree_text_move;
    tree_text_down.type = GUICore::InputEventType::pointer_down;
    tree_text_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent tree_text_up = tree_text_down;
    tree_text_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(tree_text_move);
    context->add_input_event(tree_text_down);
    context->add_input_event(tree_text_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 12, "Arrow Tree",
        GUI::TreeNodeFlag::default_open | GUI::TreeNodeFlag::open_on_arrow, 0, GUICore::LayoutInput(), &tree);
    lutest(tree_open);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(12).clicked);
    lutest(context->get_interaction_state(12).clicked_element_position.x == 100.0f);
    lutest(context->get_interaction_state(12).clicked_element_position.y == 12.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 12, "Arrow Tree",
        GUI::TreeNodeFlag::default_open | GUI::TreeNodeFlag::open_on_arrow, 0, GUICore::LayoutInput(), &tree);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(tree_open);

    GUICore::InputEvent tree_arrow_move = move;
    tree_arrow_move.position = Float2U(250.0f, 64.0f);
    GUICore::InputEvent tree_arrow_down = tree_arrow_move;
    tree_arrow_down.type = GUICore::InputEventType::pointer_down;
    tree_arrow_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent tree_arrow_up = tree_arrow_down;
    tree_arrow_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(tree_arrow_move);
    context->add_input_event(tree_arrow_down);
    context->add_input_event(tree_arrow_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 12, "Arrow Tree",
        GUI::TreeNodeFlag::default_open | GUI::TreeNodeFlag::open_on_arrow, 0, GUICore::LayoutInput(), &tree);
    lutest(tree_open);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(12).clicked);
    lutest(context->get_interaction_state(12).clicked_element_position.x == 10.0f);
    lutest(context->get_interaction_state(12).clicked_element_position.y == 12.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tree_open = GUI::tree_node(context.get(), 12, "Arrow Tree",
        GUI::TreeNodeFlag::default_open | GUI::TreeNodeFlag::open_on_arrow, 0, GUICore::LayoutInput(), &tree);
    context->set_layout_result(tree, tree_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(!tree_open);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle row = GUI::begin_h_layout(context.get(), 20, "row");
    GUICore::LayoutInput row_button_layout;
    row_button_layout.width.kind = GUICore::SizeKind::pixels;
    row_button_layout.width.value = 64.0f;
    row_button_layout.height.kind = GUICore::SizeKind::expand;
    GUI::text_button(context.get(), 21, "Run", row_button_layout);
    GUICore::LayoutInput row_check_layout;
    row_check_layout.width.kind = GUICore::SizeKind::pixels;
    row_check_layout.width.value = 120.0f;
    row_check_layout.height.kind = GUICore::SizeKind::expand;
    GUI::checkbox(context.get(), 22, "Enabled", true, row_check_layout);
    GUICore::LayoutInput row_progress_layout;
    row_progress_layout.width.kind = GUICore::SizeKind::ratio;
    row_progress_layout.width.value = 1.0f;
    row_progress_layout.height.kind = GUICore::SizeKind::expand;
    GUI::progress_bar(context.get(), 23, 0.5f, nullptr, row_progress_layout);
    GUICore::LinearLayoutDesc row_desc;
    row_desc.gap = 4.0f;
    lupanic_if_failed(GUI::end_h_layout(context.get(), row, RectF(16.0f, 16.0f, 300.0f, 32.0f), row_desc));
    context->end_element();
    context->pop_layer();
    context->route_input();
    const GUICore::Element* row_button = context->find_element(21);
    const GUICore::Element* row_check = context->find_element(22);
    const GUICore::Element* row_progress = context->find_element(23);
    GUICore::ElementHandle row_button_handle = context->find_element_handle(21);
    GUICore::ElementHandle missing_handle = context->find_element_handle(99999);
    lutest(row_button_handle.id == 21 && row_button_handle.index != GUICore::INVALID_ELEMENT);
    lutest(row_button_handle.generation == context->generation());
    lutest(!missing_handle.id && missing_handle.index == GUICore::INVALID_ELEMENT);
    lutest(row_button && row_button->layout_result.rect.offset_x == 16.0f);
    lutest(row_button->layout_result.rect.width == 64.0f);
    lutest(row_button->layout_result.rect.height == 32.0f);
    lutest(row_check && row_check->layout_result.rect.offset_x == 84.0f);
    lutest(row_check->layout_result.rect.width == 120.0f);
    lutest(row_progress && row_progress->layout_result.rect.offset_x == 208.0f);
    lutest(row_progress->layout_result.rect.width == 108.0f);
    lutest(context->dump_debug_info().elements.size() == 5);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle grid = GUI::begin_grid_layout(context.get(), 30, "grid");
    GUI::text(context.get(), 31, "A");
    GUI::text(context.get(), 32, "B");
    GUI::text(context.get(), 33, "C");
    GUI::text(context.get(), 34, "D");
    GUICore::GridLayoutDesc grid_desc;
    grid_desc.mode = GUICore::GridLayoutMode::fixed_column_count;
    grid_desc.column_count = 2;
    grid_desc.cell_size.y = 20.0f;
    grid_desc.gap = Float2U(5.0f, 5.0f);
    lupanic_if_failed(GUI::end_grid_layout(context.get(), grid, RectF(10.0f, 20.0f, 105.0f, 80.0f), grid_desc));

    GUICore::ElementHandle stack = GUI::begin_stack_layout(context.get(), 40, "stack");
    GUICore::LayoutInput stack_child_layout;
    stack_child_layout.width.kind = GUICore::SizeKind::pixels;
    stack_child_layout.width.value = 20.0f;
    stack_child_layout.height.kind = GUICore::SizeKind::pixels;
    stack_child_layout.height.value = 10.0f;
    GUI::text(context.get(), 41, "center", stack_child_layout);
    GUICore::StackLayoutDesc stack_desc;
    stack_desc.alignment = Float2U(0.5f, 0.5f);
    lupanic_if_failed(GUI::end_stack_layout(context.get(), stack, RectF(200.0f, 20.0f, 100.0f, 50.0f), stack_desc));

    GUICore::ElementHandle canvas = GUI::begin_canvas_layout(context.get(), 50, "canvas");
    GUICore::LayoutInput canvas_child_layout;
    canvas_child_layout.width.kind = GUICore::SizeKind::pixels;
    canvas_child_layout.width.value = 20.0f;
    canvas_child_layout.height.kind = GUICore::SizeKind::pixels;
    canvas_child_layout.height.value = 10.0f;
    GUI::text(context.get(), 51, "anchored", canvas_child_layout);
    GUICore::CanvasLayoutItem canvas_items[1];
    canvas_items[0].element_id = 51;
    canvas_items[0].anchor_min = Float2U(0.5f, 0.5f);
    canvas_items[0].anchor_max = Float2U(0.5f, 0.5f);
    canvas_items[0].pivot = Float2U(0.5f, 0.5f);
    GUICore::CanvasLayoutDesc canvas_desc;
    canvas_desc.items = Span<const GUICore::CanvasLayoutItem>(canvas_items, 1);
    lupanic_if_failed(GUI::end_canvas_layout(context.get(), canvas, RectF(300.0f, 20.0f, 100.0f, 80.0f), canvas_desc));

    GUICore::ElementHandle scroll = GUI::begin_scroll_viewport(context.get(), 60, "scroll");
    GUICore::LayoutInput scroll_child_layout;
    scroll_child_layout.width.kind = GUICore::SizeKind::pixels;
    scroll_child_layout.width.value = 200.0f;
    scroll_child_layout.height.kind = GUICore::SizeKind::pixels;
    scroll_child_layout.height.value = 100.0f;
    GUI::text(context.get(), 61, "scroll content", scroll_child_layout);
    GUICore::ScrollViewportLayoutDesc scroll_desc;
    scroll_desc.scroll_offset = Float2U(15.0f, 25.0f);
    lupanic_if_failed(GUI::end_scroll_viewport(context.get(), scroll, RectF(10.0f, 120.0f, 80.0f, 40.0f), scroll_desc));
    context->end_element();
    context->pop_layer();
    context->route_input();

    const GUICore::Element* grid_a = context->find_element(31);
    const GUICore::Element* grid_b = context->find_element(32);
    const GUICore::Element* grid_c = context->find_element(33);
    lutest(grid_a && grid_a->layout_result.rect.offset_x == 10.0f && grid_a->layout_result.rect.offset_y == 20.0f);
    lutest(grid_a->layout_result.rect.width == 50.0f && grid_a->layout_result.rect.height == 20.0f);
    lutest(grid_b && grid_b->layout_result.rect.offset_x == 65.0f && grid_b->layout_result.rect.offset_y == 20.0f);
    lutest(grid_c && grid_c->layout_result.rect.offset_x == 10.0f && grid_c->layout_result.rect.offset_y == 45.0f);
    const GUICore::Element* stack_child = context->find_element(41);
    lutest(stack_child && stack_child->layout_result.rect.offset_x == 240.0f && stack_child->layout_result.rect.offset_y == 40.0f);
    const GUICore::Element* canvas_child = context->find_element(51);
    lutest(canvas_child && canvas_child->layout_result.rect.offset_x == 340.0f && canvas_child->layout_result.rect.offset_y == 55.0f);
    const GUICore::Element* scroll_child = context->find_element(61);
    lutest(scroll_child && scroll_child->layout_result.rect.offset_x == -5.0f && scroll_child->layout_result.rect.offset_y == 95.0f);
    lutest(scroll_child->layout_result.clip_rect.offset_x == 10.0f && scroll_child->layout_result.clip_rect.offset_y == 120.0f);
    lutest(scroll_child->layout_result.clip_rect.width == 80.0f && scroll_child->layout_result.clip_rect.height == 40.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle scroll_view = GUI::begin_scroll_view(context.get(), 62, "scroll view");
    GUICore::LayoutInput scroll_view_child_layout;
    scroll_view_child_layout.width.kind = GUICore::SizeKind::pixels;
    scroll_view_child_layout.width.value = 80.0f;
    scroll_view_child_layout.height.kind = GUICore::SizeKind::pixels;
    scroll_view_child_layout.height.value = 160.0f;
    GUI::text(context.get(), 63, "scrollable content", scroll_view_child_layout);
    lupanic_if_failed(GUI::end_scroll_view(context.get(), scroll_view, RectF(10.0f, 180.0f, 80.0f, 40.0f)));
    context->end_element();
    context->pop_layer();
    context->route_input();

    context->begin_frame(frame);
    GUICore::InputEvent scroll_wheel;
    scroll_wheel.type = GUICore::InputEventType::pointer_wheel;
    scroll_wheel.position = Float2U(20.0f, 190.0f);
    scroll_wheel.wheel_delta = Float2U(0.0f, -4.0f);
    context->add_input_event(scroll_wheel);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    scroll_view = GUI::begin_scroll_view(context.get(), 62, "scroll view");
    GUI::text(context.get(), 63, "scrollable content", scroll_view_child_layout);
    lupanic_if_failed(GUI::end_scroll_view(context.get(), scroll_view, RectF(10.0f, 180.0f, 80.0f, 40.0f)));
    context->end_element();
    context->pop_layer();
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    scroll_view = GUI::begin_scroll_view(context.get(), 62, "scroll view");
    GUI::text(context.get(), 63, "scrollable content", scroll_view_child_layout);
    lupanic_if_failed(GUI::end_scroll_view(context.get(), scroll_view, RectF(10.0f, 180.0f, 80.0f, 40.0f)));
    context->end_element();
    context->pop_layer();
    context->route_input();
    scroll_child = context->find_element(63);
    lutest(scroll_child && scroll_child->layout_result.rect.offset_y == 60.0f);
    object_t scroll_state_object = context->get_state(GUICore::make_state_id<GUI::CoreScrollViewState>(62));
    lutest(scroll_state_object);
    GUI::CoreScrollViewState* scroll_state = cast_object<GUI::CoreScrollViewState>(scroll_state_object);
    lutest(scroll_state && scroll_state->scroll.y == 120.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::LayoutInput table_layout;
    table_layout.padding = Float4U(10.0f, 10.0f, 10.0f, 10.0f);
    GUICore::ElementHandle table = GUI::begin_table_layout(context.get(), 70, "table", table_layout);
    GUI::text(context.get(), 71, "A");
    GUICore::LayoutInput table_b_layout;
    table_b_layout.width.kind = GUICore::SizeKind::fit;
    table_b_layout.height.kind = GUICore::SizeKind::fit;
    table_b_layout.margin = Float4U(1.0f, 2.0f, 3.0f, 4.0f);
    GUICore::ElementHandle table_b = GUI::text(context.get(), 72, "B", table_b_layout);
    GUICore::LayoutResult table_b_measure;
    table_b_measure.content_size = Float2U(60.0f, 30.0f);
    context->set_layout_result(table_b, table_b_measure);
    GUI::text(context.get(), 73, "C");
    GUICore::TableTrackDesc columns[3];
    columns[0].kind = GUICore::TableTrackSizeKind::pixels;
    columns[0].value = 40.0f;
    columns[1].kind = GUICore::TableTrackSizeKind::fit;
    columns[2].kind = GUICore::TableTrackSizeKind::ratio;
    columns[2].value = 1.0f;
    GUICore::TableTrackDesc rows[2];
    rows[0].kind = GUICore::TableTrackSizeKind::pixels;
    rows[0].value = 20.0f;
    rows[1].kind = GUICore::TableTrackSizeKind::fit;
    GUICore::TableLayoutCell table_cells[3];
    table_cells[0].element_id = 71;
    table_cells[0].row = 0;
    table_cells[0].column = 0;
    table_cells[1].element_id = 72;
    table_cells[1].row = 1;
    table_cells[1].column = 1;
    table_cells[1].padding = Float4U(2.0f, 2.0f, 2.0f, 2.0f);
    table_cells[2].element_id = 73;
    table_cells[2].row = 0;
    table_cells[2].column = 2;
    table_cells[2].row_span = 2;
    table_cells[2].padding = Float4U(1.0f, 1.0f, 1.0f, 1.0f);
    GUICore::TableLayoutDesc table_desc;
    table_desc.columns = Span<const GUICore::TableTrackDesc>(columns, 3);
    table_desc.rows = Span<const GUICore::TableTrackDesc>(rows, 2);
    table_desc.cells = Span<const GUICore::TableLayoutCell>(table_cells, 3);
    table_desc.gap = Float2U(5.0f, 4.0f);
    lupanic_if_failed(GUI::end_table_layout(context.get(), table, RectF(0.0f, 0.0f, 300.0f, 120.0f), table_desc));
    context->end_element();
    context->pop_layer();
    context->route_input();
    const GUICore::Element* table_a_result = context->find_element(71);
    const GUICore::Element* table_b_result = context->find_element(72);
    const GUICore::Element* table_c_result = context->find_element(73);
    lutest(table_a_result && table_b_result && table_c_result);
    lutest(table_a_result->layout_result.rect.offset_x == 10.0f);
    lutest(table_a_result->layout_result.rect.offset_y == 10.0f);
    lutest(table_a_result->layout_result.rect.width == 40.0f);
    lutest(table_a_result->layout_result.rect.height == 20.0f);
    lutest(table_b_result->layout_result.rect.offset_x == 58.0f);
    lutest(table_b_result->layout_result.rect.offset_y == 38.0f);
    lutest(table_b_result->layout_result.rect.width == 60.0f);
    lutest(table_b_result->layout_result.rect.height == 30.0f);
    lutest(table_c_result->layout_result.rect.offset_x == 129.0f);
    lutest(table_c_result->layout_result.rect.offset_y == 11.0f);
    lutest(table_c_result->layout_result.rect.width == 160.0f);
    lutest(table_c_result->layout_result.rect.height == 62.0f);
    const GUICore::Element* table_result = context->find_element(70);
    lutest(table_result && table_result->layout_result.content_size.x == 280.0f);
    lutest(table_result->layout_result.content_size.y == 64.0f);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::LayoutInput row_table_layout;
    row_table_layout.padding = Float4U(5.0f, 5.0f, 5.0f, 5.0f);
    GUICore::ElementHandle row_table = GUI::begin_table_layout(context.get(), 170, "row table", row_table_layout);
    GUICore::TableTrackDesc row_table_columns[2];
    row_table_columns[0].kind = GUICore::TableTrackSizeKind::pixels;
    row_table_columns[0].value = 50.0f;
    row_table_columns[1].kind = GUICore::TableTrackSizeKind::pixels;
    row_table_columns[1].value = 70.0f;
    GUI::set_table_columns(context.get(), Span<const GUICore::TableTrackDesc>(row_table_columns, 2));
    GUI::set_table_gap(context.get(), Float2U(3.0f, 2.0f));
    GUICore::TableTrackDesc row0;
    row0.kind = GUICore::TableTrackSizeKind::pixels;
    row0.value = 20.0f;
    if(GUI::begin_table_row(context.get(), row0))
    {
        GUI::text(context.get(), 171, "row A");
        GUI::text(context.get(), 172, "row B");
        GUI::end_table_row(context.get());
    }
    GUICore::TableTrackDesc row1;
    row1.kind = GUICore::TableTrackSizeKind::pixels;
    row1.value = 24.0f;
    if(GUI::begin_table_row(context.get(), row1))
    {
        GUI::text(context.get(), 173, "row C");
        GUI::text(context.get(), 174, "row D");
        GUI::end_table_row(context.get());
    }
    lupanic_if_failed(GUI::end_table_layout(context.get(), row_table, RectF(0.0f, 0.0f, 200.0f, 80.0f)));
    context->end_element();
    context->pop_layer();
    context->route_input();
    const GUICore::Element* row_a_result = context->find_element(171);
    const GUICore::Element* row_b_result = context->find_element(172);
    const GUICore::Element* row_c_result = context->find_element(173);
    const GUICore::Element* row_d_result = context->find_element(174);
    lutest(row_a_result && row_b_result && row_c_result && row_d_result);
    lutest(row_a_result->layout_result.rect.offset_x == 5.0f);
    lutest(row_a_result->layout_result.rect.offset_y == 5.0f);
    lutest(row_a_result->layout_result.rect.width == 50.0f);
    lutest(row_a_result->layout_result.rect.height == 20.0f);
    lutest(row_b_result->layout_result.rect.offset_x == 58.0f);
    lutest(row_b_result->layout_result.rect.offset_y == 5.0f);
    lutest(row_b_result->layout_result.rect.width == 70.0f);
    lutest(row_b_result->layout_result.rect.height == 20.0f);
    lutest(row_c_result->layout_result.rect.offset_x == 5.0f);
    lutest(row_c_result->layout_result.rect.offset_y == 27.0f);
    lutest(row_d_result->layout_result.rect.offset_x == 58.0f);
    lutest(row_d_result->layout_result.rect.offset_y == 27.0f);

    GUICore::InputEvent popup_move = move;
    popup_move.position = Float2U(316.0f, 56.0f);
    GUICore::InputEvent popup_down = popup_move;
    popup_down.type = GUICore::InputEventType::pointer_down;
    popup_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent popup_up = popup_down;
    popup_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(popup_move);
    context->add_input_event(popup_down);
    context->add_input_event(popup_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle covered_button = GUI::text_button(context.get(), 90, "Covered");
    GUICore::LayoutResult covered_button_layout;
    covered_button_layout.rect = RectF(300.0f, 40.0f, 120.0f, 32.0f);
    covered_button_layout.clip_rect = covered_button_layout.rect;
    context->set_layout_result(covered_button, covered_button_layout);
    context->end_element();
    context->pop_layer();
    GUI::open_popup(context.get(), 100);
    GUICore::ElementHandle popup_root;
    GUI::PopupDesc popup_desc;
    popup_desc.position = Float2U(300.0f, 40.0f);
    popup_desc.layout = fixed_layout(160.0f, 52.0f);
    lutest(GUI::begin_popup(context.get(), 100, popup_desc, &popup_root));
    GUICore::LayoutInput popup_text_layout;
    popup_text_layout.width.kind = GUICore::SizeKind::pixels;
    popup_text_layout.width.value = 120.0f;
    popup_text_layout.height.kind = GUICore::SizeKind::pixels;
    popup_text_layout.height.value = 20.0f;
    GUI::text(context.get(), 101, "Popup item", popup_text_layout);
    lupanic_if_failed(GUI::end_popup(context.get(), popup_root, RectF(0.0f, 0.0f, 160.0f, 52.0f)));
    context->route_input();
    lutest(GUI::is_popup_open(context.get(), 100));
    lutest(context->get_interaction_state(popup_root.id).hovered);
    lutest(!context->get_interaction_state(90).hovered);
    const GUICore::Element* popup_text = context->find_element(101);
    lutest(popup_text && popup_text->layout_result.rect.offset_x == 6.0f);
    lutest(popup_text->layout_result.rect.offset_y == 6.0f);
    lutest(context->dump_debug_info().layers.size() == 2);
    GUI::close_popup(context.get(), 100);
    lutest(!GUI::is_popup_open(context.get(), 100));

    GUICore::InputEvent tooltip_move = move;
    tooltip_move.position = Float2U(44.0f, 28.0f);
    context->begin_frame(frame);
    context->add_input_event(tooltip_move);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle tooltip_owner = GUI::text_button(context.get(), 110, "Hover");
    GUICore::LayoutResult tooltip_owner_layout;
    tooltip_owner_layout.rect = RectF(16.0f, 16.0f, 120.0f, 32.0f);
    tooltip_owner_layout.clip_rect = tooltip_owner_layout.rect;
    context->set_layout_result(tooltip_owner, tooltip_owner_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(110).hovered);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tooltip_owner = GUI::text_button(context.get(), 110, "Hover");
    context->set_layout_result(tooltip_owner, tooltip_owner_layout);
    context->end_element();
    context->pop_layer();
    GUI::TooltipDesc tooltip_desc;
    tooltip_desc.delay = 0.0f;
    tooltip_desc.offset = Float2U(10.0f, 12.0f);
    tooltip_desc.layout = fixed_layout(140.0f, 36.0f);
    GUICore::ElementHandle tooltip_root;
    lutest(GUI::begin_tooltip(context.get(), 120, tooltip_owner, tooltip_desc, &tooltip_root));
    GUICore::LayoutInput tooltip_text_layout;
    tooltip_text_layout.width.kind = GUICore::SizeKind::pixels;
    tooltip_text_layout.width.value = 100.0f;
    tooltip_text_layout.height.kind = GUICore::SizeKind::pixels;
    tooltip_text_layout.height.value = 20.0f;
    GUI::text(context.get(), 121, "Tooltip", tooltip_text_layout);
    lupanic_if_failed(GUI::end_tooltip(context.get(), tooltip_root, RectF(0.0f, 0.0f, 140.0f, 36.0f)));
    context->route_input();
    debug = context->dump_debug_info();
    lutest(debug.layers.size() == 2);
    lutest(debug.layers[1].screen_position.x == 54.0f);
    lutest(debug.layers[1].screen_position.y == 40.0f);
    lutest(context->find_element(121) != nullptr);

    context->begin_frame(frame);
    context->add_input_event(tooltip_move);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tooltip_owner = GUI::text_button(context.get(), 130, "Hover simple");
    context->set_layout_result(tooltip_owner, tooltip_owner_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(130).hovered);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    tooltip_owner = GUI::text_button(context.get(), 130, "Hover simple");
    context->set_layout_result(tooltip_owner, tooltip_owner_layout);
    context->end_element();
    context->pop_layer();
    GUI::TooltipDesc simple_tooltip_desc;
    simple_tooltip_desc.delay = 0.0f;
    GUICore::ElementHandle simple_tooltip = GUI::set_item_tooltip(context.get(), 131, tooltip_owner,
        "Simple tooltip", simple_tooltip_desc);
    lutest(simple_tooltip.id != 0);
    context->route_input();
    debug = context->dump_debug_info();
    lutest(debug.layers.size() == 2);
    lutest(debug.elements.size() >= 4);

    const c8* combo_items[] = { "Alpha", "Beta", "Gamma" };
    i32 combo_index = 0;
    GUICore::LayoutResult combo_layout;
    combo_layout.rect = RectF(300.0f, 100.0f, 180.0f, 28.0f);
    combo_layout.clip_rect = combo_layout.rect;
    GUICore::InputEvent combo_preview_move = move;
    combo_preview_move.position = Float2U(320.0f, 114.0f);
    GUICore::InputEvent combo_preview_down = combo_preview_move;
    combo_preview_down.type = GUICore::InputEventType::pointer_down;
    combo_preview_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent combo_preview_up = combo_preview_down;
    combo_preview_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(combo_preview_move);
    context->add_input_event(combo_preview_down);
    context->add_input_event(combo_preview_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle combo = GUI::combo(context.get(), 130, "combo", &combo_index, Span<const c8*>(combo_items, 3));
    context->set_layout_result(combo, combo_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(130).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    combo = GUI::combo(context.get(), 130, "combo", &combo_index, Span<const c8*>(combo_items, 3));
    context->set_layout_result(combo, combo_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->dump_debug_info().layers.size() == 2);
    lutest(context->find_element(130) != nullptr);

    GUICore::InputEvent combo_item_move = move;
    combo_item_move.position = Float2U(310.0f, 177.0f);
    GUICore::InputEvent combo_item_down = combo_item_move;
    combo_item_down.type = GUICore::InputEventType::pointer_down;
    combo_item_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent combo_item_up = combo_item_down;
    combo_item_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(combo_item_move);
    context->add_input_event(combo_item_down);
    context->add_input_event(combo_item_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    combo = GUI::combo(context.get(), 130, "combo", &combo_index, Span<const c8*>(combo_items, 3));
    context->set_layout_result(combo, combo_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(combo_index == 0);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    combo = GUI::combo(context.get(), 130, "combo", &combo_index, Span<const c8*>(combo_items, 3));
    context->set_layout_result(combo, combo_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(combo_index == 1);
    lutest(context->dump_debug_info().layers.size() == 1);

    f32 color4_value[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    GUICore::LayoutResult color4_layout;
    color4_layout.rect = RectF(300.0f, 140.0f, 180.0f, 28.0f);
    color4_layout.clip_rect = color4_layout.rect;
    GUICore::InputEvent color_preview_move = move;
    color_preview_move.position = Float2U(310.0f, 154.0f);
    GUICore::InputEvent color_preview_down = color_preview_move;
    color_preview_down.type = GUICore::InputEventType::pointer_down;
    color_preview_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent color_preview_up = color_preview_down;
    color_preview_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(color_preview_move);
    context->add_input_event(color_preview_down);
    context->add_input_event(color_preview_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle color_edit = GUI::color_edit4(context.get(), 135, "Color", color4_value);
    context->set_layout_result(color_edit, color4_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(135).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    color_edit = GUI::color_edit4(context.get(), 135, "Color", color4_value);
    context->set_layout_result(color_edit, color4_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    debug = context->dump_debug_info();
    lutest(debug.layers.size() == 2);
    bool has_color_gradient = false;
    for(const GUICore::DrawCommand& command : debug.draw_commands)
    {
        if(command.type == GUICore::DrawCommandType::gradient_rect)
        {
            has_color_gradient = true;
            break;
        }
    }
    lutest(has_color_gradient);

    GUICore::InputEvent color_square_move = move;
    color_square_move.position = Float2U(324.0f, 160.0f);
    GUICore::InputEvent color_square_down = color_square_move;
    color_square_down.type = GUICore::InputEventType::pointer_down;
    color_square_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent color_square_up = color_square_down;
    color_square_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(color_square_move);
    context->add_input_event(color_square_down);
    context->add_input_event(color_square_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    color_edit = GUI::color_edit4(context.get(), 135, "Color", color4_value);
    context->set_layout_result(color_edit, color4_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    color_edit = GUI::color_edit4(context.get(), 135, "Color", color4_value);
    context->set_layout_result(color_edit, color4_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(color4_value[0] > 0.49f && color4_value[0] < 0.51f);
    lutest(color4_value[1] > 0.24f && color4_value[1] < 0.26f);
    lutest(color4_value[2] > 0.24f && color4_value[2] < 0.26f);

    u32 rgba8_color = 0x11223344;
    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle color3 = GUI::color_edit3(context.get(), 136, "Packed", &rgba8_color);
    context->set_layout_result(color3, color4_layout);
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(((rgba8_color >> 24) & 0xFFu) == 0xFFu);

    bool menu_checked = false;
    RectF menu_bar_rect(16.0f, 16.0f, 240.0f, 28.0f);
    GUICore::InputEvent menu_header_move = move;
    menu_header_move.position = Float2U(30.0f, 28.0f);
    GUICore::InputEvent menu_header_down = menu_header_move;
    menu_header_down.type = GUICore::InputEventType::pointer_down;
    menu_header_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent menu_header_up = menu_header_down;
    menu_header_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(menu_header_move);
    context->add_input_event(menu_header_down);
    context->add_input_event(menu_header_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context.get(), 140, "main_menu");
    lutest(!GUI::begin_menu(context.get(), 141, "File"));
    lupanic_if_failed(GUI::end_menu_bar(context.get(), menu_bar, menu_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(141).clicked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    menu_bar = GUI::begin_menu_bar(context.get(), 140, "main_menu");
    lutest(GUI::begin_menu(context.get(), 141, "File"));
    GUI::menu_item(context.get(), 142, "Show Grid", nullptr, &menu_checked);
    GUI::menu_separator(context.get(), 143);
    GUI::menu_item(context.get(), 144, "Disabled", nullptr, false, false);
    lupanic_if_failed(GUI::end_menu(context.get(), RectF(0.0f, 0.0f, 190.0f, 72.0f)));
    lupanic_if_failed(GUI::end_menu_bar(context.get(), menu_bar, menu_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->dump_debug_info().layers.size() == 2);
    lutest(context->find_element(142) != nullptr);
    lutest(!menu_checked);

    GUICore::InputEvent menu_item_move = move;
    menu_item_move.position = Float2U(32.0f, 52.0f);
    GUICore::InputEvent menu_item_down = menu_item_move;
    menu_item_down.type = GUICore::InputEventType::pointer_down;
    menu_item_down.button = GUICore::PointerButton::left;
    GUICore::InputEvent menu_item_up = menu_item_down;
    menu_item_up.type = GUICore::InputEventType::pointer_up;

    context->begin_frame(frame);
    context->add_input_event(menu_item_move);
    context->add_input_event(menu_item_down);
    context->add_input_event(menu_item_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    menu_bar = GUI::begin_menu_bar(context.get(), 140, "main_menu");
    lutest(GUI::begin_menu(context.get(), 141, "File"));
    GUI::menu_item(context.get(), 142, "Show Grid", nullptr, &menu_checked);
    lupanic_if_failed(GUI::end_menu(context.get(), RectF(0.0f, 0.0f, 190.0f, 72.0f)));
    lupanic_if_failed(GUI::end_menu_bar(context.get(), menu_bar, menu_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_interaction_state(142).clicked);
    lutest(!menu_checked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    menu_bar = GUI::begin_menu_bar(context.get(), 140, "main_menu");
    lutest(GUI::begin_menu(context.get(), 141, "File"));
    GUI::menu_item(context.get(), 142, "Show Grid", nullptr, &menu_checked);
    lupanic_if_failed(GUI::end_menu(context.get(), RectF(0.0f, 0.0f, 190.0f, 72.0f)));
    lupanic_if_failed(GUI::end_menu_bar(context.get(), menu_bar, menu_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(menu_checked);

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    menu_bar = GUI::begin_menu_bar(context.get(), 140, "main_menu");
    lutest(!GUI::begin_menu(context.get(), 141, "File"));
    lupanic_if_failed(GUI::end_menu_bar(context.get(), menu_bar, menu_bar_rect));
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->dump_debug_info().layers.size() == 1);

    Name drag_number_type("test.number");
    Name drag_text_type("test.text");
    Name drag_number_types[] = { drag_number_type };
    Name drag_text_types[] = { drag_text_type };
    i32 drag_value = 42;

    GUICore::LayoutResult drag_source_layout;
    drag_source_layout.rect = RectF(16.0f, 240.0f, 100.0f, 28.0f);
    drag_source_layout.clip_rect = drag_source_layout.rect;
    GUICore::LayoutResult drag_text_target_layout;
    drag_text_target_layout.rect = RectF(160.0f, 240.0f, 120.0f, 28.0f);
    drag_text_target_layout.clip_rect = drag_text_target_layout.rect;
    GUICore::LayoutResult drag_number_target_layout;
    drag_number_target_layout.rect = RectF(300.0f, 240.0f, 120.0f, 28.0f);
    drag_number_target_layout.clip_rect = drag_number_target_layout.rect;

    GUICore::InputEvent drag_wrong_up = move;
    drag_wrong_up.type = GUICore::InputEventType::pointer_up;
    drag_wrong_up.button = GUICore::PointerButton::left;
    drag_wrong_up.position = Float2U(190.0f, 254.0f);

    context->begin_frame(frame);
    context->add_input_event(drag_wrong_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    GUICore::ElementHandle drag_source = GUI::text_button(context.get(), 150, "Source");
    context->set_layout_result(drag_source, drag_source_layout);
    GUI::set_drag_drop_source_types(context.get(), drag_source, Span<const Name>(drag_number_types, 1));
    GUICore::ElementHandle drag_text_target = GUI::text_button(context.get(), 151, "Text target");
    context->set_layout_result(drag_text_target, drag_text_target_layout);
    GUI::set_drag_drop_target_types(context.get(), drag_text_target, Span<const Name>(drag_text_types, 1));
    GUICore::ElementHandle drag_number_target = GUI::text_button(context.get(), 152, "Number target");
    context->set_layout_result(drag_number_target, drag_number_target_layout);
    GUI::set_drag_drop_target_types(context.get(), drag_number_target, Span<const Name>(drag_number_types, 1));
    context->end_element();
    context->pop_layer();
    lupanic_if_failed(GUI::start_drag_drop(context.get(), drag_source, drag_number_type, &drag_value, sizeof(drag_value)));
    lutest(GUI::is_drag_drop_active(context.get()));
    lutest(GUI::get_drag_drop_payload(context.get())->source.id == 150);
    lutest(!GUI::hit_test_drag_drop_target(context.get(), drag_number_type, drag_wrong_up.position).id);
    context->route_input();
    lutest(!GUI::is_drag_drop_active(context.get()));
    lutest(!GUI::accept_drag_drop_payload(context.get(), drag_text_target, drag_number_type));
    lutest(!GUI::accept_drag_drop_payload(context.get(), drag_number_target, drag_number_type));

    GUICore::InputEvent drag_right_up = drag_wrong_up;
    drag_right_up.position = Float2U(330.0f, 254.0f);

    context->begin_frame(frame);
    context->add_input_event(drag_right_up);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);
    drag_source = GUI::text_button(context.get(), 150, "Source");
    context->set_layout_result(drag_source, drag_source_layout);
    GUI::set_drag_drop_source_types(context.get(), drag_source, Span<const Name>(drag_number_types, 1));
    drag_text_target = GUI::text_button(context.get(), 151, "Text target");
    context->set_layout_result(drag_text_target, drag_text_target_layout);
    GUI::set_drag_drop_target_types(context.get(), drag_text_target, Span<const Name>(drag_text_types, 1));
    drag_number_target = GUI::text_button(context.get(), 152, "Number target");
    context->set_layout_result(drag_number_target, drag_number_target_layout);
    GUI::set_drag_drop_target_types(context.get(), drag_number_target, Span<const Name>(drag_number_types, 1));
    context->end_element();
    context->pop_layer();
    lupanic_if_failed(GUI::start_drag_drop(context.get(), drag_source, drag_number_type, &drag_value, sizeof(drag_value)));
    lutest(GUI::hit_test_drag_drop_target(context.get(), drag_number_type, drag_right_up.position).id == 152);
    context->route_input();
    lutest(!GUI::is_drag_drop_active(context.get()));
    const GUICore::DragDropPayload* delivered = GUI::accept_drag_drop_payload(context.get(), drag_number_target, drag_number_type);
    lutest(delivered);
    lutest(delivered->delivery);
    lutest(delivered->source.id == 150);
    lutest(delivered->target.id == 152);
    const i32* delivered_value = delivered->data_as<i32>();
    lutest(delivered_value && *delivered_value == drag_value);
    lutest(!GUI::accept_drag_drop_payload(context.get(), drag_number_target, drag_text_type));

    context->begin_frame(frame);
    context->push_layer(1, Float2U(0.0f), Name("default"));
    root = context->begin_element(1, Name("root"));
    context->set_layout_result(root, root_layout);

    GUI::DockSpaceLayoutDesc dock_layout;
    GUI::DockSpaceLayoutNodeDesc dock_root;
    dock_root.split = true;
    dock_root.split_axis = GUI::DockSplitAxis::x;
    dock_root.split_ratio = 0.3f;
    dock_root.child0 = 1;
    dock_root.child1 = 2;
    GUI::DockSpaceLayoutNodeDesc dock_left;
    dock_left.tabs.push_back(201);
    dock_left.tabs.push_back(202);
    dock_left.selected_tab = 202;
    GUI::DockSpaceLayoutNodeDesc dock_right;
    dock_right.tabs.push_back(203);
    dock_layout.nodes.push_back(dock_root);
    dock_layout.nodes.push_back(dock_left);
    dock_layout.nodes.push_back(dock_right);
    dock_layout.root_node = 0;
    GUI::DockSpaceFloatingPanelDesc floating_panel_desc;
    floating_panel_desc.panel = 204;
    floating_panel_desc.rect = RectF(320.0f, 40.0f, 160.0f, 100.0f);
    dock_layout.floating_panels.push_back(floating_panel_desc);

    GUICore::ElementHandle dock_space = GUI::begin_dock_space(context.get(), 200, "Direct DockSpace");
    GUI::set_dockspace_layout(context.get(), 200, dock_layout);
    GUICore::ElementHandle hidden_panel;
    lutest(GUI::begin_dock_panel(context.get(), 201, "Hidden Tab", nullptr, GUI::DockPanelStyle(), GUICore::LayoutInput(), &hidden_panel));
    GUI::text(context.get(), 211, "hidden");
    GUI::end_dock_panel(context.get());
    GUICore::ElementHandle selected_panel;
    lutest(GUI::begin_dock_panel(context.get(), 202, "Selected Tab", nullptr, GUI::DockPanelStyle(), GUICore::LayoutInput(), &selected_panel));
    GUI::text(context.get(), 212, "selected");
    GUI::end_dock_panel(context.get());
    GUICore::ElementHandle right_panel;
    lutest(GUI::begin_dock_panel(context.get(), 203, "Right", nullptr, GUI::DockPanelStyle(), GUICore::LayoutInput(), &right_panel));
    GUI::text(context.get(), 213, "right");
    GUI::end_dock_panel(context.get());
    GUICore::ElementHandle floating_panel;
    lutest(GUI::begin_dock_panel(context.get(), 204, "Floating", nullptr, GUI::DockPanelStyle(), GUICore::LayoutInput(), &floating_panel));
    GUI::text(context.get(), 214, "floating");
    GUI::end_dock_panel(context.get());
    lupanic_if_failed(GUI::end_dock_space(context.get(), dock_space, RectF(20.0f, 20.0f, 500.0f, 300.0f)));
    context->end_element();
    context->pop_layer();
    context->route_input();

    const GUICore::Element* hidden_panel_element = context->find_element(201);
    const GUICore::Element* selected_panel_element = context->find_element(202);
    const GUICore::Element* right_panel_element = context->find_element(203);
    const GUICore::Element* floating_panel_element = context->find_element(204);
    const GUICore::Element* selected_text_element = context->find_element(212);
    lutest(hidden_panel_element && hidden_panel_element->layout_result.rect.width == 0.0f);
    lutest(selected_panel_element && selected_panel_element->layout_result.rect.offset_x == 20.0f);
    lutest(selected_panel_element->layout_result.rect.width == 150.0f);
    lutest(right_panel_element && right_panel_element->layout_result.rect.offset_x == 170.0f);
    lutest(right_panel_element->layout_result.rect.width == 350.0f);
    lutest(floating_panel_element && floating_panel_element->layout_result.rect.offset_x == 340.0f);
    lutest(floating_panel_element->layout_result.rect.offset_y == 60.0f);
    lutest(floating_panel_element->layout_result.rect.width == 160.0f);
    lutest(selected_text_element && selected_text_element->layout_result.rect.offset_y > selected_panel_element->layout_result.rect.offset_y);
    lutest(context->dump_debug_info().elements.size() == 10);

    {
        context->begin_frame(frame);
        context->push_layer(1, Float2U(0.0f), Name("default"));
        GUICore::ElementHandle deferred_root = GUI::begin_v_layout(context.get(), 300, "Deferred Root");
        GUICore::LayoutInput row_layout;
        row_layout.width.kind = GUICore::SizeKind::expand;
        row_layout.height.kind = GUICore::SizeKind::pixels;
        row_layout.height.value = 40.0f;
        GUICore::ElementHandle deferred_row = GUI::begin_h_layout(context.get(), 301, "Deferred Row", row_layout);
        GUICore::LayoutInput first_button_layout;
        first_button_layout.width.kind = GUICore::SizeKind::pixels;
        first_button_layout.width.value = 100.0f;
        first_button_layout.height.kind = GUICore::SizeKind::pixels;
        first_button_layout.height.value = 30.0f;
        GUI::text_button(context.get(), 302, "First", first_button_layout);
        GUICore::LayoutInput second_button_layout = first_button_layout;
        second_button_layout.width.value = 80.0f;
        GUI::text_button(context.get(), 303, "Second", second_button_layout);
        GUICore::LinearLayoutDesc row_desc;
        row_desc.gap = 4.0f;
        lupanic_if_failed(GUI::end_h_layout(context.get(), deferred_row, row_desc));
        GUICore::LayoutInput bottom_button_layout;
        bottom_button_layout.width.kind = GUICore::SizeKind::pixels;
        bottom_button_layout.width.value = 120.0f;
        bottom_button_layout.height.kind = GUICore::SizeKind::pixels;
        bottom_button_layout.height.value = 24.0f;
        GUI::text_button(context.get(), 304, "Bottom", bottom_button_layout);
        GUICore::LinearLayoutDesc root_desc;
        root_desc.gap = 6.0f;
        lupanic_if_failed(GUI::end_v_layout(context.get(), deferred_root, root_desc));
        context->pop_layer();
        lupanic_if_failed(GUI::layout_editor_tree(context.get(), deferred_root, RectF(10.0f, 20.0f, 300.0f, 200.0f)));
        const GUICore::Element* deferred_row_element = context->find_element(301);
        const GUICore::Element* first_button_element = context->find_element(302);
        const GUICore::Element* second_button_element = context->find_element(303);
        const GUICore::Element* bottom_button_element = context->find_element(304);
        lutest(deferred_row_element && deferred_row_element->layout_result.rect.offset_x == 10.0f);
        lutest(deferred_row_element->layout_result.rect.offset_y == 20.0f);
        lutest(deferred_row_element->layout_result.rect.width == 300.0f);
        lutest(deferred_row_element->layout_result.rect.height == 40.0f);
        lutest(first_button_element && first_button_element->layout_result.rect.offset_x == 10.0f);
        lutest(first_button_element->layout_result.rect.width == 100.0f);
        lutest(second_button_element && second_button_element->layout_result.rect.offset_x == 114.0f);
        lutest(second_button_element->layout_result.rect.width == 80.0f);
        lutest(bottom_button_element && bottom_button_element->layout_result.rect.offset_y == 66.0f);
        lutest(bottom_button_element->layout_result.rect.width == 120.0f);
    }

    draw_list = nullptr;
    debug = GUICore::DebugInfo();
    drawing_debug = GUICore::DebugInfo();
    slider_with_input_debug = GUICore::DebugInfo();
    context = nullptr;
    }
    Luna::close();
    return 0;
}
