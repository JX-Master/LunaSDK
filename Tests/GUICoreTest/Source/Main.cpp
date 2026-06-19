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
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/Font/Font.hpp>

using namespace Luna;
using namespace Luna::GUICore;

#define lutest luassert_always

int main()
{
    Luna::init();
    lupanic_if_failed(add_modules({ module_gui_core() }));
    lupanic_if_failed(init_modules());

    Ref<IContext> context = new_context();
    FrameDesc frame;
    frame.screen_size = Float2U(1280.0f, 720.0f);
    frame.framebuffer_size = UInt2U(1280, 720);
    context->begin_frame(frame);
    auto has_debug_pass = [](const DebugInfo& info, DebugPassKind kind, const Name& name, GUICore::id_t element = 0)
    {
        for(const DebugPassInfo& pass : info.passes)
        {
            if(pass.kind == kind && pass.name == name && (!element || pass.element == element))
            {
                return true;
            }
        }
        return false;
    };
    lutest(context->generation() == 1);
    lutest(context->current_data_scope() == DEFAULT_DATA_SCOPE);
    GUICore::id_t root_item = context->make_id("item");
    GUICore::id_t root_numeric_item = context->make_id((GUICore::id_t)42);
    context->push_data_scope(100);
    GUICore::id_t scoped_item = context->make_id("item");
    GUICore::id_t scoped_numeric_item = context->make_id((GUICore::id_t)42);
    lutest(scoped_item != root_item);
    lutest(scoped_numeric_item != root_numeric_item);
    context->push_data_scope(200);
    GUICore::id_t nested_item = context->make_id("item");
    lutest(nested_item != scoped_item);
    context->pop_data_scope();
    lutest(context->make_id("item") == scoped_item);
    context->pop_data_scope();
    lutest(context->make_id("item") == root_item);

    InputEvent move_event;
    move_event.type = InputEventType::pointer_move;
    move_event.position = Float2U(32.0f, 32.0f);
    context->add_input_event(move_event);
    InputEvent down_event;
    down_event.type = InputEventType::pointer_down;
    down_event.position = Float2U(32.0f, 32.0f);
    down_event.button = PointerButton::left;
    context->add_input_event(down_event);
    InputEvent up_event;
    up_event.type = InputEventType::pointer_up;
    up_event.position = Float2U(32.0f, 32.0f);
    up_event.button = PointerButton::left;
    context->add_input_event(up_event);

    context->define_style(Name("base"));
    context->set_style_value(Name("base"), Name("accent"), style_f32x4(Float4U(0.1f, 0.2f, 0.3f, 1.0f)));
    context->define_style(Name("child"), Name("base"));
    StyleValue accent = context->get_style_value(Name("child"), Name("accent"), StyleValue());
    lutest(accent.type == StyleValueType::f32x4);
    lutest(accent.number.x == 0.1f);
    StyleEntrySchema accent_schema;
    accent_schema.owner = Name("gui.editor");
    accent_schema.entry = Name("accent");
    accent_schema.type = StyleValueType::f32x4;
    accent_schema.default_value = style_f32x4(Float4U(0.2f, 0.4f, 0.8f, 1.0f));
    accent_schema.category = "Colors";
    accent_schema.description = "Editor accent color.";
    context->register_style_entry_schema(accent_schema);
    StyleEntrySchema replacement_schema = accent_schema;
    replacement_schema.description = "Replacement description.";
    context->register_style_entry_schema(replacement_schema);
    Span<const StyleEntrySchema> schemas = context->get_style_entry_schemas();
    lutest(schemas.size() == 1);
    lutest(schemas[0].owner == Name("gui.editor"));
    lutest(schemas[0].entry == Name("accent"));
    lutest(schemas[0].type == StyleValueType::f32x4);
    lutest(strcmp(schemas[0].description.c_str(), "Replacement description.") == 0);

    context->push_style(Name("child"));
    lupanic_if_failed(context->set_state(9001, nullptr, StateLifetime::process));
    lutest(context->get_performance_counters().state_count == 1);

    context->push_layer(1, Float2U(0.0f), Name("default"));
    ElementHandle root = context->begin_element(10, Name("root"));
    LayoutResult root_layout;
    root_layout.rect = RectF(0.0f, 0.0f, 1280.0f, 720.0f);
    root_layout.clip_rect = root_layout.rect;
    context->set_layout_result(root, root_layout);

    ElementHandle button = context->begin_element(11, Name("button chrome"));
    LayoutInput button_input;
    button_input.width.kind = SizeKind::pixels;
    button_input.width.value = 120.0f;
    button_input.height.kind = SizeKind::pixels;
    button_input.height.value = 32.0f;
    button_input.margin = Float4U(1.0f, 2.0f, 3.0f, 4.0f);
    button_input.padding = Float4U(5.0f, 6.0f, 7.0f, 8.0f);
    context->set_layout(button, button_input);
    LayoutResult button_layout;
    button_layout.rect = RectF(16.0f, 16.0f, 120.0f, 32.0f);
    button_layout.clip_rect = button_layout.rect;
    button_layout.content_size = Float2U(96.0f, 24.0f);
    context->set_layout_result(button, button_layout);
    Interactable interactable;
    interactable.hit_test = true;
    interactable.hoverable = true;
    interactable.activatable = true;
    context->set_interactable(button, interactable);
    DrawCommand rect;
    rect.type = DrawCommandType::rounded_rect;
    rect.rect = button_layout.rect;
    rect.radius = 4.0f;
    rect.color = Float4U(0.2f, 0.4f, 0.8f, 1.0f);
    context->draw(rect);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->pop_style();

    context->push_layer(2, Float2U(0.0f), Name("overlay"));
    ElementHandle overlay_root = context->begin_element(20, Name("overlay root"));
    context->set_layout_result(overlay_root, root_layout);
    ElementHandle overlay_button = context->begin_element(21, Name("overlay button"));
    context->set_layout_result(overlay_button, button_layout);
    interactable.focusable = true;
    context->set_interactable(overlay_button, interactable);
    context->end_element();
    ElementHandle disabled_cover = context->begin_element(22, Name("disabled cover"));
    context->set_layout_result(disabled_cover, button_layout);
    Interactable disabled_interactable = interactable;
    disabled_interactable.disabled = true;
    context->set_interactable(disabled_cover, disabled_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();

    const Element* found = context->find_element(11);
    lutest(found);
    lutest(found->parent == root.index);
    lutest(found->draw_command_count == 1);
    lutest(found->style == Name("child"));
    context->bind_style(button, Name("base"));
    lutest(context->find_element(11)->style == Name("base"));
    lutest(context->current_style().empty());

    ElementHandle hit = context->hit_test(Float2U(32.0f, 32.0f));
    lutest(hit.id == 21);
    context->route_input();
    PerformanceCounters route_counters = context->get_performance_counters();
    lutest(route_counters.state_gc_ms >= 0.0);
    lutest(route_counters.input_route_ms >= 0.0);
    InteractionState base_button_state = context->get_interaction_state(11);
    InteractionState overlay_root_state = context->get_interaction_state(20);
    InteractionState overlay_button_state = context->get_interaction_state(21);
    InteractionState disabled_cover_state = context->get_interaction_state(22);
    lutest(!base_button_state.hovered);
    lutest(!base_button_state.clicked);
    lutest(!overlay_root_state.hovered);
    lutest(!overlay_root_state.clicked);
    lutest(overlay_root_state.subtree_hovered);
    lutest(overlay_root_state.subtree_clicked);
    lutest(overlay_root_state.subtree_focused);
    lutest(overlay_button_state.hovered);
    lutest(overlay_button_state.clicked);
    lutest(overlay_button_state.clicked_screen_position.x == 32.0f);
    lutest(overlay_button_state.clicked_screen_position.y == 32.0f);
    lutest(overlay_button_state.clicked_element_position.x == 16.0f);
    lutest(overlay_button_state.clicked_element_position.y == 16.0f);
    lutest(overlay_button_state.focused);
    lutest(overlay_button_state.subtree_hovered);
    lutest(overlay_button_state.subtree_clicked);
    lutest(overlay_button_state.subtree_focused);
    lutest(!overlay_button_state.active);
    lutest(!disabled_cover_state.hovered);
    lutest(!disabled_cover_state.clicked);

    context->log_debug_issue(DebugIssueSeverity::warning, Name("test"), "debug issue sample", 21);
    DebugInfo debug = context->dump_debug_info();
    lutest(debug.counters.debug_dump_ms >= 0.0);
    lutest(debug.counters.input_route_ms >= 0.0);
    lutest(debug.counters.debug_issue_count == 1);
    lutest(debug.counters.debug_pass_count == debug.passes.size());
    lutest(has_debug_pass(debug, DebugPassKind::frame, Name("begin_frame")));
    lutest(has_debug_pass(debug, DebugPassKind::state, Name("gc_states")));
    lutest(has_debug_pass(debug, DebugPassKind::input, Name("route_input")));
    lutest(debug.issues.size() == 1);
    lutest(debug.issues[0].severity == DebugIssueSeverity::warning);
    lutest(debug.issues[0].category == Name("test"));
    lutest(debug.issues[0].element == 21);
    lutest(strcmp(debug.issues[0].message.c_str(), "debug issue sample") == 0);
    lutest(debug.data_scope_stack.size() == 1);
    lutest(debug.data_scope_stack[0] == DEFAULT_DATA_SCOPE);
    lutest(debug.layers.size() == 2);
    lutest(debug.layers[0].first_draw_command == 0);
    lutest(debug.layers[0].draw_command_count == 1);
    lutest(debug.layers[1].first_draw_command == U32_MAX);
    lutest(debug.layers[1].draw_command_count == 0);
    lutest(debug.elements.size() == 5);
    lutest(debug.draw_commands.size() == 1);
    lutest(debug.draw_commands[0].layer == 0);
    lutest(debug.draw_commands[0].element == 1);
    lutest(debug.elements[1].first_draw_command == 0);
    lutest(debug.elements[1].draw_command_count == 1);
    lutest(debug.counters.input_event_count == 3);
    lutest(debug.counters.interactable_count == 3);
    lutest(debug.counters.style_schema_count == 1);
    lutest(debug.style_schemas.size() == 1);
    lutest(debug.style_schemas[0].entry == Name("accent"));
    const DebugElementInfo* button_debug = nullptr;
    const DebugElementInfo* overlay_button_debug = nullptr;
    const DebugElementInfo* disabled_cover_debug = nullptr;
    for(const DebugElementInfo& element_info : debug.elements)
    {
        if(element_info.id == 11)
        {
            button_debug = &element_info;
        }
        if(element_info.id == 21)
        {
            overlay_button_debug = &element_info;
        }
        if(element_info.id == 22)
        {
            disabled_cover_debug = &element_info;
        }
    }
    lutest(button_debug);
    lutest(button_debug->parent == root.index);
    lutest(button_debug->first_child == INVALID_ELEMENT);
    lutest(button_debug->last_child == INVALID_ELEMENT);
    lutest(button_debug->next_sibling == INVALID_ELEMENT);
    lutest(button_debug->prev_sibling == INVALID_ELEMENT);
    lutest(button_debug->layout.width.kind == SizeKind::pixels);
    lutest(button_debug->layout.width.value == 120.0f);
    lutest(button_debug->layout.height.kind == SizeKind::pixels);
    lutest(button_debug->layout.height.value == 32.0f);
    lutest(button_debug->layout.margin.x == 1.0f);
    lutest(button_debug->layout.margin.y == 2.0f);
    lutest(button_debug->layout.margin.z == 3.0f);
    lutest(button_debug->layout.margin.w == 4.0f);
    lutest(button_debug->layout.padding.x == 5.0f);
    lutest(button_debug->layout.padding.y == 6.0f);
    lutest(button_debug->layout.padding.z == 7.0f);
    lutest(button_debug->layout.padding.w == 8.0f);
    lutest(button_debug->content_size.x == 96.0f);
    lutest(button_debug->content_size.y == 24.0f);
    lutest(button_debug->hit_test);
    lutest(button_debug->hoverable);
    lutest(button_debug->activatable);
    lutest(!button_debug->focusable);
    lutest(button_debug->resolved_style.size() == 1);
    lutest(button_debug->resolved_style[0].owner == Name("gui.editor"));
    lutest(button_debug->resolved_style[0].entry == Name("accent"));
    lutest(!button_debug->resolved_style[0].defaulted);
    lutest(button_debug->resolved_style[0].value.type == StyleValueType::f32x4);
    lutest(button_debug->resolved_style[0].value.number.x == 0.1f);
    lutest(overlay_button_debug);
    lutest(overlay_button_debug->focusable);
    lutest(overlay_button_debug->resolved_style.size() == 1);
    lutest(overlay_button_debug->resolved_style[0].defaulted);
    lutest(disabled_cover_debug);
    lutest(disabled_cover_debug->disabled);
    const DebugStyleInfo* base_style_debug = nullptr;
    for(const DebugStyleInfo& style_info : debug.styles)
    {
        if(style_info.name == Name("base"))
        {
            base_style_debug = &style_info;
        }
    }
    lutest(base_style_debug);
    lutest(base_style_debug->entry_count == 1);
    lutest(base_style_debug->entries.size() == 1);
    lutest(base_style_debug->entries[0].entry == Name("accent"));
    lutest(base_style_debug->entries[0].mode == StyleEntryMode::set);
    lutest(base_style_debug->entries[0].value.type == StyleValueType::f32x4);
    lutest(base_style_debug->entries[0].value.number.x == 0.1f);
    lutest(debug.hovered_element == 21);
    lutest(debug.focused_element == 21);
    lutest(failed(context->compile_draw_commands(nullptr)));

    context->begin_frame(frame);
    DebugInfo clean_issue_debug = context->dump_debug_info();
    lutest(clean_issue_debug.issues.empty());
    lutest(clean_issue_debug.counters.debug_issue_count == 0);
    context->add_input_event(move_event);
    context->add_input_event(down_event);
    context->add_input_event(up_event);
    context->push_layer(2, Float2U(0.0f), Name("overlay"));
    overlay_root = context->begin_element(20, Name("overlay root"));
    context->set_layout_result(overlay_root, root_layout);
    overlay_button = context->begin_element(21, Name("overlay button"));
    context->set_layout_result(overlay_button, button_layout);
    context->set_interactable(overlay_button, interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    InteractionState double_click_button_state = context->get_interaction_state(21);
    lutest(double_click_button_state.clicked);
    lutest(double_click_button_state.double_clicked);
    lutest(context->get_interaction_state(20).subtree_clicked);
    lutest(context->get_interaction_state(20).subtree_double_clicked);
    DebugInfo double_click_debug = context->dump_debug_info();
    const DebugElementInfo* double_click_button_debug = nullptr;
    const DebugElementInfo* double_click_root_debug = nullptr;
    for(const DebugElementInfo& element_info : double_click_debug.elements)
    {
        if(element_info.id == 20)
        {
            double_click_root_debug = &element_info;
        }
        if(element_info.id == 21)
        {
            double_click_button_debug = &element_info;
        }
    }
    lutest(double_click_button_debug);
    lutest(double_click_button_debug->clicked);
    lutest(double_click_button_debug->double_clicked);
    lutest(double_click_root_debug);
    lutest(double_click_root_debug->subtree_double_clicked);

    context->begin_frame(frame);
    lutest(context->current_data_scope() == DEFAULT_DATA_SCOPE);
    lutest(context->make_id("item") == root_item);
    context->add_input_event(move_event);
    context->add_input_event(down_event);
    context->add_input_event(up_event);
    context->push_layer(3, Float2U(0.0f), Name("blocked base"));
    ElementHandle blocked_base_root = context->begin_element(30, Name("blocked base root"));
    context->set_layout_result(blocked_base_root, root_layout);
    ElementHandle blocked_base_button = context->begin_element(31, Name("blocked base button"));
    context->set_layout_result(blocked_base_button, button_layout);
    context->set_interactable(blocked_base_button, interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->push_layer(4, Float2U(0.0f), Name("blocker"));
    ElementHandle blocker_root = context->begin_element(40, Name("blocker root"));
    context->set_layout_result(blocker_root, root_layout);
    ElementHandle blocker = context->begin_element(41, Name("pointer blocker"));
    context->set_layout_result(blocker, button_layout);
    Interactable blocker_interactable;
    blocker_interactable.blocks_pointer_input = true;
    context->set_interactable(blocker, blocker_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    lutest(context->hit_test(Float2U(32.0f, 32.0f)).id == 41);
    context->route_input();
    lutest(!context->get_interaction_state(31).hovered);
    lutest(!context->get_interaction_state(31).clicked);
    lutest(!context->get_interaction_state(41).hovered);
    lutest(!context->get_interaction_state(41).clicked);
    lutest(context->get_delivered_input_events(31).empty());
    lutest(context->get_delivered_input_events(41).empty());
    DebugInfo blocker_debug = context->dump_debug_info();
    bool found_blocker_debug = false;
    for(const DebugElementInfo& element_info : blocker_debug.elements)
    {
        if(element_info.id == 41)
        {
            found_blocker_debug = true;
            lutest(!element_info.hit_test);
            lutest(element_info.blocks_pointer_input);
        }
    }
    lutest(found_blocker_debug);

    context->begin_frame(frame);
    context->add_input_event(move_event);
    context->add_input_event(down_event);
    context->add_input_event(up_event);
    context->push_layer(45, Float2U(0.0f), Name("pass through base"));
    ElementHandle pass_base_root = context->begin_element(450, Name("pass through base root"));
    context->set_layout_result(pass_base_root, root_layout);
    ElementHandle pass_base_button = context->begin_element(451, Name("pass through base button"));
    context->set_layout_result(pass_base_button, button_layout);
    context->set_interactable(pass_base_button, interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->push_layer(46, Float2U(0.0f), Name("pass through overlay"));
    ElementHandle pass_overlay_root = context->begin_element(460, Name("pass through overlay root"));
    context->set_layout_result(pass_overlay_root, root_layout);
    ElementHandle pass_overlay = context->begin_element(461, Name("pass through overlay"));
    context->set_layout_result(pass_overlay, button_layout);
    Interactable pass_overlay_interactable = interactable;
    pass_overlay_interactable.pointer_input_propagation = PointerInputPropagation::pass_through;
    context->set_interactable(pass_overlay, pass_overlay_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    lutest(context->hit_test(Float2U(32.0f, 32.0f)).id == 461);
    context->route_input();
    lutest(context->get_interaction_state(451).hovered);
    lutest(context->get_interaction_state(451).clicked);
    lutest(!context->get_interaction_state(461).hovered);
    lutest(!context->get_interaction_state(461).clicked);
    lutest(!context->get_delivered_input_events(451).empty());
    lutest(context->get_delivered_input_events(461).empty());
    DebugInfo pass_debug = context->dump_debug_info();
    bool found_pass_overlay_debug = false;
    for(const DebugElementInfo& element_info : pass_debug.elements)
    {
        if(element_info.id == 461)
        {
            found_pass_overlay_debug = true;
            lutest(element_info.hit_test);
            lutest(element_info.pointer_input_propagation == PointerInputPropagation::pass_through);
        }
    }
    lutest(found_pass_overlay_debug);

    context->begin_frame(frame);
    lutest(context->get_style_entry_schemas().size() == 1);
    lutest(context->get_state(9001) == nullptr);
    lutest(context->get_performance_counters().state_count == 1);
    lutest(context->current_style().empty());
    context->clear_state(9001);
    lutest(context->get_performance_counters().state_count == 0);

    context->begin_frame(frame);
    Name test_font("test.font");
    lupanic_if_failed(context->register_font(test_font, Font::get_default_font()));
    lutest(context->get_font(test_font).font == Font::get_default_font());
    lutest(failed(context->register_font(test_font, Font::get_default_font())));
    context->push_layer(5, Float2U(0.0f), Name("text drawing"));
    ElementHandle text_root = context->begin_element(50, Name("text root"));
    context->set_layout_result(text_root, root_layout);
    ElementHandle text_element = context->begin_element(51, Name("text element"));
    LayoutResult text_layout;
    text_layout.rect = RectF(8.0f, 8.0f, 160.0f, 32.0f);
    text_layout.clip_rect = text_layout.rect;
    context->set_layout_result(text_element, text_layout);
    DrawCommand text_command;
    text_command.type = DrawCommandType::text;
    text_command.rect = text_layout.rect;
    text_command.color = Float4U(1.0f);
    text_command.font = test_font;
    text_command.font_size = 16.0f;
    text_command.text = "GUI Core";
    context->draw(text_command);
    context->end_element();
    context->end_element();
    context->pop_layer();
    Ref<VG::IShapeDrawList> text_draw_list = VG::new_shape_draw_list();
    lupanic_if_failed(context->compile_draw_commands(text_draw_list));
    PerformanceCounters draw_counters = context->get_performance_counters();
    lutest(draw_counters.draw_compile_ms >= 0.0);
    DebugInfo text_debug = context->dump_debug_info();
    lutest(text_debug.draw_commands.size() == 1);
    lutest(text_debug.draw_commands[0].type == DrawCommandType::text);
    lutest(has_debug_pass(text_debug, DebugPassKind::render, Name("compile_draw_commands")));

    context->begin_frame(frame);
    context->push_layer(6, Float2U(0.0f), Name("image drawing"));
    ElementHandle image_root = context->begin_element(60, Name("image root"));
    context->set_layout_result(image_root, root_layout);
    ElementHandle image_element = context->begin_element(61, Name("image element"));
    context->set_layout_result(image_element, text_layout);
    DrawCommand image_command;
    image_command.type = DrawCommandType::image;
    image_command.rect = text_layout.rect;
    image_command.color = Float4U(1.0f);
    image_command.min_texcoord = Float2U(0.0f, 1.0f);
    image_command.max_texcoord = Float2U(1.0f, 0.0f);
    image_command.nearest_sampler = true;
    context->draw(image_command);
    context->end_element();
    context->end_element();
    context->pop_layer();
    DebugInfo image_debug = context->dump_debug_info();
    lutest(image_debug.draw_commands.size() == 1);
    lutest(image_debug.draw_commands[0].type == DrawCommandType::image);
    lutest(image_debug.draw_commands[0].min_texcoord.y == 1.0f);
    lutest(image_debug.draw_commands[0].max_texcoord.y == 0.0f);
    lutest(image_debug.draw_commands[0].nearest_sampler);

    context->begin_frame(frame);
    InputEvent capture_down;
    capture_down.type = InputEventType::pointer_down;
    capture_down.position = Float2U(32.0f, 32.0f);
    capture_down.button = PointerButton::left;
    context->add_input_event(capture_down);
    InputEvent capture_move;
    capture_move.type = InputEventType::pointer_move;
    capture_move.position = Float2U(320.0f, 320.0f);
    context->add_input_event(capture_move);
    context->push_layer(10, Float2U(0.0f), Name("capture"));
    ElementHandle capture_root = context->begin_element(100, Name("capture root"));
    context->set_layout_result(capture_root, root_layout);
    ElementHandle capture_button = context->begin_element(101, Name("capture button"));
    context->set_layout_result(capture_button, button_layout);
    Interactable capture_interactable;
    capture_interactable.hit_test = true;
    capture_interactable.hoverable = true;
    capture_interactable.activatable = true;
    capture_interactable.focusable = true;
    context->set_interactable(capture_button, capture_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    InteractionState capture_state = context->get_interaction_state(101);
    lutest(context->captured_element() == 101);
    lutest(capture_state.active);
    lutest(capture_state.focused);
    lutest(!capture_state.hovered);
    lutest(!capture_state.clicked);
    DebugInfo capture_debug = context->dump_debug_info();
    lutest(capture_debug.captured_element == 101);
    bool found_captured_debug = false;
    for(const DebugElementInfo& element_info : capture_debug.elements)
    {
        if(element_info.id == 101)
        {
            found_captured_debug = true;
            lutest(element_info.captured);
            lutest(element_info.active);
        }
    }
    lutest(found_captured_debug);
    context->release_pointer_capture(42);
    lutest(context->captured_element() == 101);
    context->release_pointer_capture(101);
    lutest(context->captured_element() == 0);
    context->capture_pointer(101);
    lutest(context->captured_element() == 101);
    context->release_pointer_capture();
    lutest(context->captured_element() == 0);

    context->begin_frame(frame);
    InputEvent capture_up;
    capture_up.type = InputEventType::pointer_up;
    capture_up.position = Float2U(320.0f, 320.0f);
    capture_up.button = PointerButton::left;
    context->add_input_event(capture_up);
    context->push_layer(10, Float2U(0.0f), Name("capture"));
    capture_root = context->begin_element(100, Name("capture root"));
    context->set_layout_result(capture_root, root_layout);
    capture_button = context->begin_element(101, Name("capture button"));
    context->set_layout_result(capture_button, button_layout);
    context->set_interactable(capture_button, capture_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    capture_state = context->get_interaction_state(101);
    lutest(!capture_state.active);
    lutest(capture_state.focused);
    lutest(!capture_state.clicked);

    context->begin_frame(frame);
    context->add_input_event(down_event);
    context->add_input_event(up_event);
    context->push_layer(20, Float2U(0.0f), Name("readonly"));
    ElementHandle readonly_root = context->begin_element(200, Name("readonly root"));
    context->set_layout_result(readonly_root, root_layout);
    ElementHandle readonly_item = context->begin_element(201, Name("readonly item"));
    context->set_layout_result(readonly_item, button_layout);
    Interactable readonly_interactable = capture_interactable;
    readonly_interactable.readonly_ = true;
    context->set_interactable(readonly_item, readonly_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    InteractionState readonly_state = context->get_interaction_state(201);
    lutest(readonly_state.hovered);
    lutest(readonly_state.focused);
    lutest(!readonly_state.active);
    lutest(!readonly_state.clicked);

    context->begin_frame(frame);
    InputEvent tab_event;
    tab_event.type = InputEventType::key_down;
    tab_event.key = KeyCode::tab;
    context->add_input_event(tab_event);
    context->push_layer(30, Float2U(0.0f), Name("focus scope"));
    ElementHandle focus_root = context->begin_element(300, Name("focus root"));
    context->set_layout_result(focus_root, root_layout);
    ElementHandle focus_a = context->begin_element(301, Name("focus A"));
    context->set_layout_result(focus_a, button_layout);
    Interactable focus_interactable = capture_interactable;
    focus_interactable.focus_scope = 7;
    context->set_interactable(focus_a, focus_interactable);
    context->end_element();
    ElementHandle focus_b = context->begin_element(302, Name("focus B"));
    context->set_layout_result(focus_b, button_layout);
    context->set_interactable(focus_b, focus_interactable);
    context->end_element();
    ElementHandle other_scope = context->begin_element(303, Name("other scope"));
    context->set_layout_result(other_scope, button_layout);
    focus_interactable.focus_scope = 8;
    context->set_interactable(other_scope, focus_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->focus_element(301);
    context->route_input();
    lutest(context->focused_element() == 302);
    DebugInfo focus_debug = context->dump_debug_info();
    lutest(focus_debug.focused_scope == 7);

    context->begin_frame(frame);
    context->add_input_event(tab_event);
    context->push_layer(31, Float2U(0.0f), Name("inherited focus scope"));
    ElementHandle inherited_scope_root = context->begin_element(310, Name("inherited focus root"));
    context->set_layout_result(inherited_scope_root, root_layout);
    Interactable scope_owner_interactable;
    scope_owner_interactable.focus_scope = 11;
    context->set_interactable(inherited_scope_root, scope_owner_interactable);
    ElementHandle inherited_focus_a = context->begin_element(311, Name("inherited focus A"));
    context->set_layout_result(inherited_focus_a, button_layout);
    Interactable inherited_focus_interactable = capture_interactable;
    inherited_focus_interactable.focus_scope = 0;
    context->set_interactable(inherited_focus_a, inherited_focus_interactable);
    context->end_element();
    ElementHandle inherited_focus_b = context->begin_element(312, Name("inherited focus B"));
    context->set_layout_result(inherited_focus_b, button_layout);
    context->set_interactable(inherited_focus_b, inherited_focus_interactable);
    context->end_element();
    ElementHandle inherited_other_scope = context->begin_element(313, Name("inherited other scope"));
    context->set_layout_result(inherited_other_scope, button_layout);
    Interactable inherited_other_interactable = capture_interactable;
    inherited_other_interactable.focus_scope = 12;
    context->set_interactable(inherited_other_scope, inherited_other_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->focus_element(311);
    context->route_input();
    lutest(context->focused_element() == 312);
    DebugInfo inherited_focus_debug = context->dump_debug_info();
    lutest(inherited_focus_debug.focused_scope == 11);

    auto make_test_layout = [](f32 x, f32 y, f32 w, f32 h) {
        LayoutResult result;
        result.rect = RectF(x, y, w, h);
        result.clip_rect = result.rect;
        return result;
    };
    auto build_spatial_focus_grid = [&]() {
        context->push_layer(32, Float2U(0.0f), Name("spatial focus"));
        ElementHandle spatial_root = context->begin_element(320, Name("spatial focus root"));
        context->set_layout_result(spatial_root, root_layout);
        Interactable spatial_scope;
        spatial_scope.focus_scope = 21;
        context->set_interactable(spatial_root, spatial_scope);

        Interactable spatial_interactable = capture_interactable;
        spatial_interactable.focus_scope = 0;
        ElementHandle spatial_a = context->begin_element(321, Name("spatial A"));
        context->set_layout_result(spatial_a, make_test_layout(10.0f, 10.0f, 20.0f, 20.0f));
        context->set_interactable(spatial_a, spatial_interactable);
        context->end_element();
        ElementHandle spatial_b = context->begin_element(322, Name("spatial B"));
        context->set_layout_result(spatial_b, make_test_layout(60.0f, 10.0f, 20.0f, 20.0f));
        context->set_interactable(spatial_b, spatial_interactable);
        context->end_element();
        ElementHandle spatial_c = context->begin_element(323, Name("spatial C"));
        context->set_layout_result(spatial_c, make_test_layout(10.0f, 60.0f, 20.0f, 20.0f));
        context->set_interactable(spatial_c, spatial_interactable);
        context->end_element();
        ElementHandle spatial_d = context->begin_element(324, Name("spatial D"));
        context->set_layout_result(spatial_d, make_test_layout(60.0f, 60.0f, 20.0f, 20.0f));
        context->set_interactable(spatial_d, spatial_interactable);
        context->end_element();

        ElementHandle other_spatial_scope = context->begin_element(325, Name("spatial other scope"));
        context->set_layout_result(other_spatial_scope, make_test_layout(110.0f, 10.0f, 20.0f, 20.0f));
        Interactable other_spatial_interactable = spatial_interactable;
        other_spatial_interactable.focus_scope = 22;
        context->set_interactable(other_spatial_scope, other_spatial_interactable);
        context->end_element();

        context->end_element();
        context->pop_layer();
    };

    InputEvent right_event;
    right_event.type = InputEventType::key_down;
    right_event.key = KeyCode::right;
    context->begin_frame(frame);
    context->add_input_event(right_event);
    build_spatial_focus_grid();
    context->focus_element(321);
    context->route_input();
    lutest(context->focused_element() == 322);

    InputEvent down_key_event;
    down_key_event.type = InputEventType::key_down;
    down_key_event.key = KeyCode::down;
    context->begin_frame(frame);
    context->add_input_event(down_key_event);
    build_spatial_focus_grid();
    context->focus_element(322);
    context->route_input();
    lutest(context->focused_element() == 324);

    InputEvent left_event;
    left_event.type = InputEventType::key_down;
    left_event.key = KeyCode::left;
    context->begin_frame(frame);
    context->add_input_event(left_event);
    build_spatial_focus_grid();
    context->focus_element(322);
    context->route_input();
    lutest(context->focused_element() == 321);

    InputEvent up_key_event;
    up_key_event.type = InputEventType::key_down;
    up_key_event.key = KeyCode::up;
    context->begin_frame(frame);
    context->add_input_event(up_key_event);
    build_spatial_focus_grid();
    context->focus_element(321);
    context->route_input();
    lutest(context->focused_element() == 321);
    Span<const InputEvent> spatial_delivered = context->get_delivered_input_events(321);
    lutest(spatial_delivered.size() == 1);
    lutest(spatial_delivered[0].key == KeyCode::up);

    context->begin_frame(frame);
    context->push_layer(40, Float2U(0.0f), Name("base interleave"));
    ElementHandle base_root = context->begin_element(400, Name("base root"));
    context->set_layout_result(base_root, root_layout);
    DrawCommand base_draw;
    base_draw.type = DrawCommandType::rect;
    base_draw.rect = RectF(0.0f, 0.0f, 20.0f, 20.0f);
    base_draw.color = Float4U(0.1f, 0.2f, 0.3f, 1.0f);
    context->draw(base_draw);
    context->push_layer(41, Float2U(10.0f, 10.0f), Name("overlay interleave"));
    ElementHandle interleaved_overlay_root = context->begin_element(401, Name("overlay root"));
    context->set_layout_result(interleaved_overlay_root, root_layout);
    DrawCommand overlay_draw = base_draw;
    overlay_draw.rect = RectF(1.0f, 1.0f, 10.0f, 10.0f);
    context->draw(overlay_draw);
    context->end_element();
    context->pop_layer();
    DrawCommand base_draw_after_overlay = base_draw;
    base_draw_after_overlay.rect = RectF(2.0f, 2.0f, 8.0f, 8.0f);
    context->draw(base_draw_after_overlay);
    context->end_element();
    context->pop_layer();
    DebugInfo interleaved_debug = context->dump_debug_info();
    lutest(interleaved_debug.layers.size() == 2);
    lutest(interleaved_debug.elements.size() == 2);
    lutest(interleaved_debug.elements[0].id == 400);
    lutest(interleaved_debug.elements[0].parent == INVALID_ELEMENT);
    lutest(interleaved_debug.elements[1].id == 401);
    lutest(interleaved_debug.elements[1].parent == INVALID_ELEMENT);
    lutest(interleaved_debug.draw_commands.size() == 3);
    lutest(interleaved_debug.draw_commands[0].layer == 0);
    lutest(interleaved_debug.draw_commands[1].layer == 1);
    lutest(interleaved_debug.draw_commands[2].layer == 0);
    lutest(interleaved_debug.layers[0].draw_command_count == 2);
    lutest(interleaved_debug.layers[0].draw_command_indices.size() == 2);
    lutest(interleaved_debug.layers[0].draw_command_indices[0] == 0);
    lutest(interleaved_debug.layers[0].draw_command_indices[1] == 2);
    lutest(interleaved_debug.layers[1].draw_command_count == 1);
    lutest(interleaved_debug.layers[1].draw_command_indices.size() == 1);
    lutest(interleaved_debug.layers[1].draw_command_indices[0] == 1);

    context->begin_frame(frame);
    context->push_layer(50, Float2U(0.0f), Name("linear layout"));
    ElementHandle row_root = context->begin_element(500, Name("row root"));
    LayoutInput row_layout;
    row_layout.padding = Float4U(10.0f, 5.0f, 10.0f, 5.0f);
    context->set_layout(row_root, row_layout);

    ElementHandle fixed_child = context->begin_element(501, Name("fixed child"));
    LayoutInput fixed_layout;
    fixed_layout.width.kind = SizeKind::pixels;
    fixed_layout.width.value = 50.0f;
    fixed_layout.height.kind = SizeKind::expand;
    context->set_layout(fixed_child, fixed_layout);
    context->end_element();

    ElementHandle percent_child = context->begin_element(502, Name("percent child"));
    LayoutInput percent_layout;
    percent_layout.width.kind = SizeKind::percent;
    percent_layout.width.value = 0.25f;
    percent_layout.height.kind = SizeKind::expand;
    context->set_layout(percent_child, percent_layout);
    context->end_element();

    ElementHandle ratio_child = context->begin_element(503, Name("ratio child"));
    LayoutInput ratio_layout;
    ratio_layout.width.kind = SizeKind::ratio;
    ratio_layout.width.value = 1.0f;
    ratio_layout.height.kind = SizeKind::expand;
    context->set_layout(ratio_child, ratio_layout);
    context->end_element();

    LinearLayoutDesc row_desc;
    row_desc.axis = LayoutAxis::x;
    row_desc.gap = 5.0f;
    lupanic_if_failed(layout_linear(context, row_root, RectF(0.0f, 0.0f, 300.0f, 50.0f), row_desc));
    context->end_element();
    context->pop_layer();
    DebugInfo row_layout_debug = context->dump_debug_info();
    lutest(has_debug_pass(row_layout_debug, DebugPassKind::layout, Name("layout_linear"), 500));
    const Element* fixed_result = context->find_element(501);
    const Element* percent_result = context->find_element(502);
    const Element* ratio_result = context->find_element(503);
    lutest(fixed_result && percent_result && ratio_result);
    lutest(fixed_result->layout_result.rect.offset_x == 10.0f);
    lutest(fixed_result->layout_result.rect.offset_y == 5.0f);
    lutest(fixed_result->layout_result.rect.width == 50.0f);
    lutest(fixed_result->layout_result.rect.height == 40.0f);
    lutest(percent_result->layout_result.rect.offset_x == 65.0f);
    lutest(percent_result->layout_result.rect.width == 70.0f);
    lutest(ratio_result->layout_result.rect.offset_x == 140.0f);
    lutest(ratio_result->layout_result.rect.width == 150.0f);
    lutest(context->find_element(500)->layout_result.content_size.x == 280.0f);
    lutest(context->find_element(500)->layout_result.content_size.y == 40.0f);

    context->begin_frame(frame);
    context->push_layer(51, Float2U(0.0f), Name("fit layout"));
    ElementHandle column_root = context->begin_element(510, Name("column root"));
    LayoutInput column_layout;
    column_layout.padding = Float4U(4.0f, 4.0f, 4.0f, 4.0f);
    context->set_layout(column_root, column_layout);
    ElementHandle fit_child = context->begin_element(511, Name("fit child"));
    LayoutInput fit_layout;
    fit_layout.width.kind = SizeKind::fit;
    fit_layout.height.kind = SizeKind::fit;
    fit_layout.margin = Float4U(2.0f, 3.0f, 2.0f, 3.0f);
    context->set_layout(fit_child, fit_layout);
    LayoutResult fit_measure;
    fit_measure.content_size = Float2U(33.0f, 17.0f);
    context->set_layout_result(fit_child, fit_measure);
    context->end_element();
    LinearLayoutDesc column_desc;
    column_desc.axis = LayoutAxis::y;
    lupanic_if_failed(layout_linear(context, column_root, RectF(0.0f, 0.0f, 100.0f, 80.0f), column_desc));
    context->end_element();
    context->pop_layer();
    const Element* fit_result = context->find_element(511);
    lutest(fit_result);
    lutest(fit_result->layout_result.rect.offset_x == 6.0f);
    lutest(fit_result->layout_result.rect.offset_y == 7.0f);
    lutest(fit_result->layout_result.rect.width == 33.0f);
    lutest(fit_result->layout_result.rect.height == 17.0f);
    lutest(context->find_element(510)->layout_result.content_size.x == 37.0f);
    lutest(context->find_element(510)->layout_result.content_size.y == 23.0f);

    context->begin_frame(frame);
    context->push_layer(515, Float2U(0.0f), Name("fit largest layout"));
    ElementHandle fit_largest_root = context->begin_element(5150, Name("fit largest root"));
    ElementHandle small_fit_largest_child = context->begin_element(5151, Name("small fit largest child"));
    LayoutInput fit_largest_layout;
    fit_largest_layout.width.kind = SizeKind::fit_largest;
    fit_largest_layout.height.kind = SizeKind::fit_largest;
    context->set_layout(small_fit_largest_child, fit_largest_layout);
    LayoutResult small_fit_largest_measure;
    small_fit_largest_measure.content_size = Float2U(24.0f, 9.0f);
    context->set_layout_result(small_fit_largest_child, small_fit_largest_measure);
    context->end_element();
    ElementHandle large_fit_largest_child = context->begin_element(5152, Name("large fit largest child"));
    context->set_layout(large_fit_largest_child, fit_largest_layout);
    LayoutResult large_fit_largest_measure;
    large_fit_largest_measure.content_size = Float2U(42.0f, 21.0f);
    context->set_layout_result(large_fit_largest_child, large_fit_largest_measure);
    context->end_element();
    LinearLayoutDesc fit_largest_desc;
    fit_largest_desc.axis = LayoutAxis::y;
    lupanic_if_failed(layout_linear(context, fit_largest_root, RectF(0.0f, 0.0f, 100.0f, 100.0f), fit_largest_desc));
    context->end_element();
    context->pop_layer();
    const Element* small_fit_largest_result = context->find_element(5151);
    const Element* large_fit_largest_result = context->find_element(5152);
    lutest(small_fit_largest_result && large_fit_largest_result);
    lutest(small_fit_largest_result->layout_result.rect.width == 42.0f);
    lutest(small_fit_largest_result->layout_result.rect.height == 21.0f);
    lutest(large_fit_largest_result->layout_result.rect.width == 42.0f);
    lutest(large_fit_largest_result->layout_result.rect.height == 21.0f);
    lutest(large_fit_largest_result->layout_result.rect.offset_y == 21.0f);
    lutest(context->find_element(5150)->layout_result.content_size.x == 42.0f);
    lutest(context->find_element(5150)->layout_result.content_size.y == 42.0f);

    context->begin_frame(frame);
    context->push_layer(52, Float2U(0.0f), Name("fixed cell grid"));
    ElementHandle grid_root = context->begin_element(520, Name("grid root"));
    LayoutInput grid_layout;
    grid_layout.padding = Float4U(10.0f, 10.0f, 10.0f, 10.0f);
    context->set_layout(grid_root, grid_layout);
    for(u32 i = 0; i < 4; ++i)
    {
        ElementHandle grid_child = context->begin_element(521 + i, Name("grid child"));
        context->set_layout(grid_child, LayoutInput());
        context->end_element();
    }
    GridLayoutDesc grid_desc;
    grid_desc.mode = GridLayoutMode::fixed_cell_size;
    grid_desc.cell_size = Float2U(50.0f, 30.0f);
    grid_desc.gap = Float2U(5.0f, 5.0f);
    lupanic_if_failed(layout_grid(context, grid_root, RectF(0.0f, 0.0f, 220.0f, 100.0f), grid_desc));
    context->end_element();
    context->pop_layer();
    const Element* grid_child_0 = context->find_element(521);
    const Element* grid_child_2 = context->find_element(523);
    const Element* grid_child_3 = context->find_element(524);
    lutest(grid_child_0 && grid_child_2 && grid_child_3);
    lutest(grid_child_0->layout_result.rect.offset_x == 10.0f);
    lutest(grid_child_0->layout_result.rect.offset_y == 10.0f);
    lutest(grid_child_2->layout_result.rect.offset_x == 120.0f);
    lutest(grid_child_2->layout_result.rect.offset_y == 10.0f);
    lutest(grid_child_3->layout_result.rect.offset_x == 10.0f);
    lutest(grid_child_3->layout_result.rect.offset_y == 45.0f);
    lutest(context->find_element(520)->layout_result.content_size.x == 160.0f);
    lutest(context->find_element(520)->layout_result.content_size.y == 65.0f);

    context->begin_frame(frame);
    context->push_layer(53, Float2U(0.0f), Name("fixed column grid"));
    ElementHandle column_grid_root = context->begin_element(530, Name("column grid root"));
    LayoutInput column_grid_layout;
    column_grid_layout.padding = Float4U(5.0f, 5.0f, 5.0f, 5.0f);
    context->set_layout(column_grid_root, column_grid_layout);
    for(u32 i = 0; i < 5; ++i)
    {
        ElementHandle grid_child = context->begin_element(531 + i, Name("column grid child"));
        LayoutInput child_layout;
        if(i == 4)
        {
            child_layout.margin = Float4U(1.0f, 2.0f, 3.0f, 4.0f);
        }
        context->set_layout(grid_child, child_layout);
        context->end_element();
    }
    GridLayoutDesc column_grid_desc;
    column_grid_desc.mode = GridLayoutMode::fixed_column_count;
    column_grid_desc.column_count = 3;
    column_grid_desc.cell_size = Float2U(0.0f, 20.0f);
    column_grid_desc.gap = Float2U(10.0f, 5.0f);
    lupanic_if_failed(layout_grid(context, column_grid_root, RectF(0.0f, 0.0f, 210.0f, 80.0f), column_grid_desc));
    context->end_element();
    context->pop_layer();
    const Element* column_grid_child_1 = context->find_element(532);
    const Element* column_grid_child_4 = context->find_element(535);
    lutest(column_grid_child_1 && column_grid_child_4);
    lutest(column_grid_child_1->layout_result.rect.offset_x == 75.0f);
    lutest(column_grid_child_1->layout_result.rect.offset_y == 5.0f);
    lutest(column_grid_child_1->layout_result.rect.width == 60.0f);
    lutest(column_grid_child_1->layout_result.rect.height == 20.0f);
    lutest(column_grid_child_4->layout_result.rect.offset_x == 76.0f);
    lutest(column_grid_child_4->layout_result.rect.offset_y == 32.0f);
    lutest(column_grid_child_4->layout_result.rect.width == 56.0f);
    lutest(column_grid_child_4->layout_result.rect.height == 14.0f);
    lutest(context->find_element(530)->layout_result.content_size.x == 200.0f);
    lutest(context->find_element(530)->layout_result.content_size.y == 45.0f);

    context->begin_frame(frame);
    context->push_layer(54, Float2U(0.0f), Name("stack layout"));
    ElementHandle stack_root = context->begin_element(540, Name("stack root"));
    LayoutInput stack_layout;
    stack_layout.padding = Float4U(10.0f, 5.0f, 10.0f, 5.0f);
    context->set_layout(stack_root, stack_layout);

    ElementHandle fixed_stack_child = context->begin_element(541, Name("fixed stack child"));
    LayoutInput fixed_stack_layout;
    fixed_stack_layout.width.kind = SizeKind::pixels;
    fixed_stack_layout.width.value = 20.0f;
    fixed_stack_layout.height.kind = SizeKind::pixels;
    fixed_stack_layout.height.value = 10.0f;
    fixed_stack_layout.margin = Float4U(5.0f, 5.0f, 5.0f, 5.0f);
    context->set_layout(fixed_stack_child, fixed_stack_layout);
    context->end_element();

    ElementHandle fit_stack_child = context->begin_element(542, Name("fit stack child"));
    LayoutInput fit_stack_layout;
    fit_stack_layout.width.kind = SizeKind::fit;
    fit_stack_layout.height.kind = SizeKind::fit;
    context->set_layout(fit_stack_child, fit_stack_layout);
    LayoutResult fit_stack_measure;
    fit_stack_measure.content_size = Float2U(18.0f, 12.0f);
    context->set_layout_result(fit_stack_child, fit_stack_measure);
    context->end_element();

    ElementHandle expand_stack_child = context->begin_element(543, Name("expand stack child"));
    LayoutInput expand_stack_layout;
    expand_stack_layout.width.kind = SizeKind::expand;
    expand_stack_layout.height.kind = SizeKind::expand;
    expand_stack_layout.margin = Float4U(2.0f, 3.0f, 4.0f, 5.0f);
    context->set_layout(expand_stack_child, expand_stack_layout);
    context->end_element();

    StackLayoutDesc stack_desc;
    stack_desc.alignment = Float2U(0.5f, 0.5f);
    lupanic_if_failed(layout_stack(context, stack_root, RectF(0.0f, 0.0f, 100.0f, 80.0f), stack_desc));
    context->end_element();
    context->pop_layer();
    const Element* fixed_stack_result = context->find_element(541);
    const Element* fit_stack_result = context->find_element(542);
    const Element* expand_stack_result = context->find_element(543);
    lutest(fixed_stack_result && fit_stack_result && expand_stack_result);
    lutest(fixed_stack_result->layout_result.rect.offset_x == 40.0f);
    lutest(fixed_stack_result->layout_result.rect.offset_y == 35.0f);
    lutest(fit_stack_result->layout_result.rect.offset_x == 41.0f);
    lutest(fit_stack_result->layout_result.rect.offset_y == 34.0f);
    lutest(expand_stack_result->layout_result.rect.offset_x == 12.0f);
    lutest(expand_stack_result->layout_result.rect.offset_y == 8.0f);
    lutest(expand_stack_result->layout_result.rect.width == 74.0f);
    lutest(expand_stack_result->layout_result.rect.height == 62.0f);
    lutest(context->find_element(540)->layout_result.content_size.x == 80.0f);
    lutest(context->find_element(540)->layout_result.content_size.y == 70.0f);

    context->begin_frame(frame);
    context->push_layer(55, Float2U(0.0f), Name("canvas layout"));
    ElementHandle canvas_root = context->begin_element(550, Name("canvas root"));
    LayoutInput canvas_layout;
    canvas_layout.padding = Float4U(10.0f, 20.0f, 10.0f, 20.0f);
    context->set_layout(canvas_root, canvas_layout);

    ElementHandle centered_canvas_child = context->begin_element(551, Name("centered canvas child"));
    LayoutInput centered_canvas_layout;
    centered_canvas_layout.width.kind = SizeKind::pixels;
    centered_canvas_layout.width.value = 40.0f;
    centered_canvas_layout.height.kind = SizeKind::pixels;
    centered_canvas_layout.height.value = 20.0f;
    context->set_layout(centered_canvas_child, centered_canvas_layout);
    context->end_element();

    ElementHandle stretched_canvas_child = context->begin_element(552, Name("stretched canvas child"));
    LayoutInput stretched_canvas_layout;
    stretched_canvas_layout.margin = Float4U(1.0f, 2.0f, 3.0f, 4.0f);
    context->set_layout(stretched_canvas_child, stretched_canvas_layout);
    context->end_element();

    ElementHandle default_canvas_child = context->begin_element(553, Name("default canvas child"));
    LayoutInput default_canvas_layout;
    default_canvas_layout.width.kind = SizeKind::pixels;
    default_canvas_layout.width.value = 20.0f;
    default_canvas_layout.height.kind = SizeKind::pixels;
    default_canvas_layout.height.value = 10.0f;
    context->set_layout(default_canvas_child, default_canvas_layout);
    context->end_element();

    CanvasLayoutItem canvas_items[2];
    canvas_items[0].element_id = 551;
    canvas_items[0].anchor_min = Float2U(0.5f, 0.5f);
    canvas_items[0].anchor_max = Float2U(0.5f, 0.5f);
    canvas_items[0].pivot = Float2U(0.5f, 0.5f);
    canvas_items[1].element_id = 552;
    canvas_items[1].anchor_min = Float2U(0.0f, 0.0f);
    canvas_items[1].anchor_max = Float2U(1.0f, 1.0f);
    canvas_items[1].offset = Float4U(5.0f, 6.0f, -7.0f, -8.0f);
    CanvasLayoutDesc canvas_desc;
    canvas_desc.items = Span<const CanvasLayoutItem>(canvas_items, 2);
    canvas_desc.default_item.anchor_min = Float2U(1.0f, 0.0f);
    canvas_desc.default_item.anchor_max = Float2U(1.0f, 0.0f);
    canvas_desc.default_item.offset = Float4U(-30.0f, 5.0f, 0.0f, 0.0f);
    canvas_desc.default_item.pivot = Float2U(1.0f, 0.0f);
    lupanic_if_failed(layout_canvas(context, canvas_root, RectF(0.0f, 0.0f, 200.0f, 120.0f), canvas_desc));
    context->end_element();
    context->pop_layer();
    const Element* centered_canvas_result = context->find_element(551);
    const Element* stretched_canvas_result = context->find_element(552);
    const Element* default_canvas_result = context->find_element(553);
    lutest(centered_canvas_result && stretched_canvas_result && default_canvas_result);
    lutest(centered_canvas_result->layout_result.rect.offset_x == 80.0f);
    lutest(centered_canvas_result->layout_result.rect.offset_y == 50.0f);
    lutest(centered_canvas_result->layout_result.rect.width == 40.0f);
    lutest(centered_canvas_result->layout_result.rect.height == 20.0f);
    lutest(stretched_canvas_result->layout_result.rect.offset_x == 16.0f);
    lutest(stretched_canvas_result->layout_result.rect.offset_y == 28.0f);
    lutest(stretched_canvas_result->layout_result.rect.width == 164.0f);
    lutest(stretched_canvas_result->layout_result.rect.height == 60.0f);
    lutest(default_canvas_result->layout_result.rect.offset_x == 140.0f);
    lutest(default_canvas_result->layout_result.rect.offset_y == 25.0f);
    lutest(context->find_element(550)->layout_result.content_size.x == 170.0f);
    lutest(context->find_element(550)->layout_result.content_size.y == 68.0f);

    context->begin_frame(frame);
    context->push_layer(56, Float2U(0.0f), Name("scroll viewport layout"));
    ElementHandle scroll_root = context->begin_element(560, Name("scroll viewport root"));
    LayoutInput scroll_layout;
    scroll_layout.padding = Float4U(5.0f, 10.0f, 5.0f, 10.0f);
    context->set_layout(scroll_root, scroll_layout);
    ElementHandle scroll_content = context->begin_element(561, Name("scroll content"));
    LayoutInput scroll_content_layout;
    scroll_content_layout.width.kind = SizeKind::pixels;
    scroll_content_layout.width.value = 300.0f;
    scroll_content_layout.height.kind = SizeKind::pixels;
    scroll_content_layout.height.value = 200.0f;
    scroll_content_layout.margin = Float4U(2.0f, 3.0f, 4.0f, 5.0f);
    context->set_layout(scroll_content, scroll_content_layout);
    context->end_element();
    ScrollViewportLayoutDesc scroll_desc;
    scroll_desc.scroll_offset = Float2U(20.0f, 30.0f);
    lupanic_if_failed(layout_scroll_viewport(context, scroll_root, RectF(0.0f, 0.0f, 100.0f, 80.0f), scroll_desc));
    context->end_element();
    context->pop_layer();
    const Element* scroll_content_result = context->find_element(561);
    lutest(scroll_content_result);
    lutest(scroll_content_result->layout_result.rect.offset_x == -13.0f);
    lutest(scroll_content_result->layout_result.rect.offset_y == -17.0f);
    lutest(scroll_content_result->layout_result.rect.width == 300.0f);
    lutest(scroll_content_result->layout_result.rect.height == 200.0f);
    lutest(scroll_content_result->layout_result.clip_rect.offset_x == 5.0f);
    lutest(scroll_content_result->layout_result.clip_rect.offset_y == 10.0f);
    lutest(scroll_content_result->layout_result.clip_rect.width == 90.0f);
    lutest(scroll_content_result->layout_result.clip_rect.height == 60.0f);
    lutest(context->find_element(560)->layout_result.content_size.x == 306.0f);
    lutest(context->find_element(560)->layout_result.content_size.y == 208.0f);

    context->begin_frame(frame);
    context->push_layer(57, Float2U(0.0f), Name("table layout"));
    ElementHandle table_root = context->begin_element(570, Name("table root"));
    LayoutInput table_layout;
    table_layout.padding = Float4U(10.0f, 10.0f, 10.0f, 10.0f);
    context->set_layout(table_root, table_layout);

    ElementHandle table_cell_a = context->begin_element(571, Name("table cell A"));
    context->set_layout(table_cell_a, LayoutInput());
    context->end_element();

    ElementHandle table_cell_b = context->begin_element(572, Name("table cell B"));
    LayoutInput table_cell_b_layout;
    table_cell_b_layout.width.kind = SizeKind::fit;
    table_cell_b_layout.height.kind = SizeKind::fit;
    table_cell_b_layout.margin = Float4U(1.0f, 2.0f, 3.0f, 4.0f);
    context->set_layout(table_cell_b, table_cell_b_layout);
    LayoutResult table_cell_b_measure;
    table_cell_b_measure.content_size = Float2U(60.0f, 30.0f);
    context->set_layout_result(table_cell_b, table_cell_b_measure);
    context->end_element();

    ElementHandle table_cell_c = context->begin_element(573, Name("table cell C"));
    context->set_layout(table_cell_c, LayoutInput());
    context->end_element();

    TableTrackDesc columns[3];
    columns[0].kind = TableTrackSizeKind::pixels;
    columns[0].value = 40.0f;
    columns[1].kind = TableTrackSizeKind::fit;
    columns[2].kind = TableTrackSizeKind::ratio;
    columns[2].value = 1.0f;
    TableTrackDesc rows[2];
    rows[0].kind = TableTrackSizeKind::pixels;
    rows[0].value = 20.0f;
    rows[1].kind = TableTrackSizeKind::fit;
    TableLayoutCell table_cells[3];
    table_cells[0].element_id = 571;
    table_cells[0].row = 0;
    table_cells[0].column = 0;
    table_cells[1].element_id = 572;
    table_cells[1].row = 1;
    table_cells[1].column = 1;
    table_cells[1].padding = Float4U(2.0f, 2.0f, 2.0f, 2.0f);
    table_cells[2].element_id = 573;
    table_cells[2].row = 0;
    table_cells[2].column = 2;
    table_cells[2].row_span = 2;
    table_cells[2].padding = Float4U(1.0f, 1.0f, 1.0f, 1.0f);
    TableLayoutDesc table_desc;
    table_desc.columns = Span<const TableTrackDesc>(columns, 3);
    table_desc.rows = Span<const TableTrackDesc>(rows, 2);
    table_desc.cells = Span<const TableLayoutCell>(table_cells, 3);
    table_desc.gap = Float2U(5.0f, 4.0f);
    lupanic_if_failed(layout_table(context, table_root, RectF(0.0f, 0.0f, 300.0f, 120.0f), table_desc));
    context->end_element();
    context->pop_layer();
    const Element* table_cell_a_result = context->find_element(571);
    const Element* table_cell_b_result = context->find_element(572);
    const Element* table_cell_c_result = context->find_element(573);
    lutest(table_cell_a_result && table_cell_b_result && table_cell_c_result);
    lutest(table_cell_a_result->layout_result.rect.offset_x == 10.0f);
    lutest(table_cell_a_result->layout_result.rect.offset_y == 10.0f);
    lutest(table_cell_a_result->layout_result.rect.width == 40.0f);
    lutest(table_cell_a_result->layout_result.rect.height == 20.0f);
    lutest(table_cell_b_result->layout_result.rect.offset_x == 58.0f);
    lutest(table_cell_b_result->layout_result.rect.offset_y == 38.0f);
    lutest(table_cell_b_result->layout_result.rect.width == 60.0f);
    lutest(table_cell_b_result->layout_result.rect.height == 30.0f);
    lutest(table_cell_c_result->layout_result.rect.offset_x == 129.0f);
    lutest(table_cell_c_result->layout_result.rect.offset_y == 11.0f);
    lutest(table_cell_c_result->layout_result.rect.width == 160.0f);
    lutest(table_cell_c_result->layout_result.rect.height == 62.0f);
    lutest(context->find_element(570)->layout_result.content_size.x == 280.0f);
    lutest(context->find_element(570)->layout_result.content_size.y == 64.0f);

    context->begin_frame(frame);
    context->push_layer(60, Float2U(0.0f), Name("drag drop"));
    ElementHandle drag_root = context->begin_element(600, Name("drag root"));
    context->set_layout_result(drag_root, root_layout);

    ElementHandle drag_source = context->begin_element(601, Name("drag source"));
    LayoutResult source_layout;
    source_layout.rect = RectF(0.0f, 0.0f, 40.0f, 40.0f);
    source_layout.clip_rect = source_layout.rect;
    context->set_layout_result(drag_source, source_layout);
    Interactable drag_interactable;
    drag_interactable.hit_test = true;
    drag_interactable.hoverable = true;
    context->set_interactable(drag_source, drag_interactable);
    Name number_payload("number");
    Name text_payload("text");
    Name source_types[] = { number_payload };
    context->set_drag_drop_source_types(drag_source, Span<const Name>(source_types, 1));
    context->end_element();

    ElementHandle number_target = context->begin_element(602, Name("number target"));
    LayoutResult number_target_layout;
    number_target_layout.rect = RectF(100.0f, 0.0f, 80.0f, 50.0f);
    number_target_layout.clip_rect = number_target_layout.rect;
    context->set_layout_result(number_target, number_target_layout);
    context->set_interactable(number_target, drag_interactable);
    Name number_target_types[] = { number_payload };
    context->set_drag_drop_target_types(number_target, Span<const Name>(number_target_types, 1));
    context->end_element();

    ElementHandle text_target = context->begin_element(603, Name("text target"));
    LayoutResult text_target_layout;
    text_target_layout.rect = RectF(220.0f, 0.0f, 80.0f, 50.0f);
    text_target_layout.clip_rect = text_target_layout.rect;
    context->set_layout_result(text_target, text_target_layout);
    context->set_interactable(text_target, drag_interactable);
    Name text_target_types[] = { text_payload };
    context->set_drag_drop_target_types(text_target, Span<const Name>(text_target_types, 1));
    context->end_element();

    ElementHandle blocked_number_target = context->begin_element(604, Name("blocked number target"));
    LayoutResult blocked_number_target_layout;
    blocked_number_target_layout.rect = RectF(320.0f, 0.0f, 80.0f, 50.0f);
    blocked_number_target_layout.clip_rect = blocked_number_target_layout.rect;
    context->set_layout_result(blocked_number_target, blocked_number_target_layout);
    context->set_interactable(blocked_number_target, drag_interactable);
    context->set_drag_drop_target_types(blocked_number_target, Span<const Name>(number_target_types, 1));
    context->end_element();

    ElementHandle drag_blocker = context->begin_element(605, Name("drag blocker"));
    context->set_layout_result(drag_blocker, blocked_number_target_layout);
    Interactable drag_blocker_interactable;
    drag_blocker_interactable.blocks_pointer_input = true;
    context->set_interactable(drag_blocker, drag_blocker_interactable);
    context->end_element();

    context->end_element();
    context->pop_layer();

    i32 drag_value = 42;
    lutest(failed(context->start_drag_drop(drag_source, text_payload, &drag_value, sizeof(drag_value))));
    lupanic_if_failed(context->start_drag_drop(drag_source, number_payload, &drag_value, sizeof(drag_value)));
    lutest(context->is_drag_drop_active());
    const DragDropPayload* active_payload = context->get_drag_drop_payload();
    lutest(active_payload);
    lutest(active_payload->type == number_payload);
    lutest(active_payload->source.id == drag_source.id);
    lutest(*active_payload->data_as<i32>() == 42);
    lutest(context->hit_test_drag_drop_target(number_payload, Float2U(240.0f, 20.0f)).id == 0);
    lutest(context->hit_test_drag_drop_target(number_payload, Float2U(120.0f, 20.0f)).id == number_target.id);
    lutest(context->hit_test_drag_drop_target(number_payload, Float2U(340.0f, 20.0f)).id == 0);
    DebugInfo drag_debug = context->dump_debug_info();
    const DebugElementInfo* drag_source_debug = nullptr;
    const DebugElementInfo* number_target_debug = nullptr;
    const DebugElementInfo* text_target_debug = nullptr;
    for(const DebugElementInfo& element_info : drag_debug.elements)
    {
        if(element_info.id == drag_source.id)
        {
            drag_source_debug = &element_info;
        }
        if(element_info.id == number_target.id)
        {
            number_target_debug = &element_info;
        }
        if(element_info.id == text_target.id)
        {
            text_target_debug = &element_info;
        }
    }
    lutest(drag_source_debug);
    lutest(drag_source_debug->drag_source_types.size() == 1);
    lutest(drag_source_debug->drag_source_types[0] == number_payload);
    lutest(number_target_debug);
    lutest(number_target_debug->drag_target_types.size() == 1);
    lutest(number_target_debug->drag_target_types[0] == number_payload);
    lutest(text_target_debug);
    lutest(text_target_debug->drag_target_types.size() == 1);
    lutest(text_target_debug->drag_target_types[0] == text_payload);

    InputEvent drop_event;
    drop_event.type = InputEventType::pointer_up;
    drop_event.position = Float2U(120.0f, 20.0f);
    drop_event.button = PointerButton::left;
    context->add_input_event(drop_event);
    context->route_input();
    lutest(!context->is_drag_drop_active());
    const DragDropPayload* delivery = context->get_drag_drop_delivery(number_target, number_payload);
    lutest(delivery);
    lutest(delivery->delivery);
    lutest(delivery->source.id == drag_source.id);
    lutest(delivery->target.id == number_target.id);
    lutest(*delivery->data_as<i32>() == 42);
    lutest(!context->get_drag_drop_delivery(text_target, number_payload));

    context->begin_frame(frame);
    InputEvent key_event;
    key_event.type = InputEventType::key_down;
    key_event.key = KeyCode::a;
    context->add_input_event(key_event);
    InputEvent text_event;
    text_event.type = InputEventType::text_utf8;
    text_event.text = "a";
    context->add_input_event(text_event);
    InputEvent key_up_event;
    key_up_event.type = InputEventType::key_up;
    key_up_event.key = KeyCode::a;
    context->add_input_event(key_up_event);
    InputEvent wheel_event;
    wheel_event.type = InputEventType::pointer_wheel;
    wheel_event.position = Float2U(24.0f, 24.0f);
    wheel_event.wheel_delta = Float2U(0.0f, -120.0f);
    context->add_input_event(wheel_event);
    InputEvent routed_move_event;
    routed_move_event.type = InputEventType::pointer_move;
    routed_move_event.position = Float2U(26.0f, 26.0f);
    context->add_input_event(routed_move_event);
    context->push_layer(70, Float2U(0.0f), Name("input delivery"));
    ElementHandle delivery_root = context->begin_element(700, Name("delivery root"));
    context->set_layout_result(delivery_root, root_layout);
    ElementHandle focused_input = context->begin_element(701, Name("focused input"));
    context->set_layout_result(focused_input, button_layout);
    Interactable input_interactable;
    input_interactable.hit_test = true;
    input_interactable.hoverable = true;
    input_interactable.activatable = true;
    input_interactable.focusable = true;
    context->set_interactable(focused_input, input_interactable);
    context->end_element();
    ElementHandle wheel_target = context->begin_element(702, Name("wheel target"));
    LayoutResult wheel_layout;
    wheel_layout.rect = RectF(16.0f, 16.0f, 120.0f, 32.0f);
    wheel_layout.clip_rect = wheel_layout.rect;
    context->set_layout_result(wheel_target, wheel_layout);
    Interactable wheel_interactable;
    wheel_interactable.hit_test = true;
    wheel_interactable.hoverable = true;
    context->set_interactable(wheel_target, wheel_interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->focus_element(701);
    context->route_input();
    Span<const InputEvent> focused_events = context->get_delivered_input_events(701);
    lutest(focused_events.size() == 3);
    lutest(focused_events[0].type == InputEventType::key_down);
    lutest(focused_events[1].type == InputEventType::text_utf8);
    lutest(focused_events[2].type == InputEventType::key_up);
    Span<const RoutedInputEvent> routed_focused_events = context->get_routed_input_events(701);
    lutest(routed_focused_events.size() == 3);
    lutest(routed_focused_events[0].event.type == InputEventType::key_down);
    lutest(!routed_focused_events[0].has_element_position);
    lutest(routed_focused_events[1].event.type == InputEventType::text_utf8);
    lutest(!routed_focused_events[1].has_element_position);
    lutest(routed_focused_events[2].event.type == InputEventType::key_up);
    lutest(!routed_focused_events[2].has_element_position);
    Span<const InputEvent> wheel_events = context->get_delivered_input_events(702);
    lutest(wheel_events.size() == 2);
    lutest(wheel_events[0].type == InputEventType::pointer_wheel);
    lutest(wheel_events[1].type == InputEventType::pointer_move);
    Span<const RoutedInputEvent> routed_wheel_events = context->get_routed_input_events(702);
    lutest(routed_wheel_events.size() == 2);
    lutest(routed_wheel_events[0].event.type == InputEventType::pointer_wheel);
    lutest(routed_wheel_events[0].event.position.x == 24.0f);
    lutest(routed_wheel_events[0].event.position.y == 24.0f);
    lutest(routed_wheel_events[0].has_element_position);
    lutest(routed_wheel_events[0].element_position.x == 8.0f);
    lutest(routed_wheel_events[0].element_position.y == 8.0f);
    lutest(routed_wheel_events[1].event.type == InputEventType::pointer_move);
    lutest(routed_wheel_events[1].event.position.x == 26.0f);
    lutest(routed_wheel_events[1].event.position.y == 26.0f);
    lutest(routed_wheel_events[1].has_element_position);
    lutest(routed_wheel_events[1].element_position.x == 10.0f);
    lutest(routed_wheel_events[1].element_position.y == 10.0f);
    DebugInfo delivery_debug = context->dump_debug_info();
    lutest(delivery_debug.counters.delivered_input_event_count == 5);
    lutest(delivery_debug.input_deliveries.size() == 2);
    bool found_routed_delivery = false;
    for(const DebugInputDeliveryInfo& delivery_info : delivery_debug.input_deliveries)
    {
        if(delivery_info.element_id == 702)
        {
            found_routed_delivery = true;
            lutest(delivery_info.events.size() == 2);
            lutest(delivery_info.routed_events.size() == 2);
            lutest(delivery_info.routed_events[0].has_element_position);
            lutest(delivery_info.routed_events[0].element_position.x == 8.0f);
        }
    }
    lutest(found_routed_delivery);

    context->begin_frame(frame);
    InputEvent nested_wheel_event;
    nested_wheel_event.type = InputEventType::pointer_wheel;
    nested_wheel_event.position = Float2U(30.0f, 30.0f);
    nested_wheel_event.wheel_delta = Float2U(0.0f, -1.0f);
    context->add_input_event(nested_wheel_event);
    context->push_layer(71, Float2U(0.0f), Name("scroll routing"));
    ElementHandle scroll_route_root = context->begin_element(710, Name("scroll route root"));
    context->set_layout_result(scroll_route_root, root_layout);
    ElementHandle scroll_parent = context->begin_element(711, Name("scroll parent"));
    LayoutResult scroll_parent_layout;
    scroll_parent_layout.rect = RectF(10.0f, 10.0f, 200.0f, 160.0f);
    scroll_parent_layout.clip_rect = scroll_parent_layout.rect;
    context->set_layout_result(scroll_parent, scroll_parent_layout);
    Interactable scroll_parent_interactable;
    scroll_parent_interactable.hit_test = true;
    scroll_parent_interactable.hoverable = true;
    scroll_parent_interactable.scrollable = true;
    context->set_interactable(scroll_parent, scroll_parent_interactable);
    ElementHandle scroll_child = context->begin_element(712, Name("scroll child"));
    LayoutResult scroll_child_layout;
    scroll_child_layout.rect = RectF(20.0f, 20.0f, 50.0f, 50.0f);
    scroll_child_layout.clip_rect = scroll_child_layout.rect;
    context->set_layout_result(scroll_child, scroll_child_layout);
    Interactable scroll_child_interactable;
    scroll_child_interactable.hit_test = true;
    scroll_child_interactable.hoverable = true;
    context->set_interactable(scroll_child, scroll_child_interactable);
    context->end_element();
    context->end_element();
    context->end_element();
    context->pop_layer();
    lutest(context->hit_test(Float2U(30.0f, 30.0f)).id == 712);
    context->route_input();
    Span<const InputEvent> routed_scroll_parent_raw = context->get_delivered_input_events(711);
    Span<const InputEvent> routed_scroll_child_raw = context->get_delivered_input_events(712);
    lutest(routed_scroll_parent_raw.size() == 1);
    lutest(routed_scroll_parent_raw[0].type == InputEventType::pointer_wheel);
    lutest(routed_scroll_parent_raw[0].position.x == 30.0f);
    lutest(routed_scroll_parent_raw[0].position.y == 30.0f);
    lutest(routed_scroll_child_raw.empty());
    Span<const RoutedInputEvent> routed_scroll_parent = context->get_routed_input_events(711);
    lutest(routed_scroll_parent.size() == 1);
    lutest(routed_scroll_parent[0].has_element_position);
    lutest(routed_scroll_parent[0].element_position.x == 20.0f);
    lutest(routed_scroll_parent[0].element_position.y == 20.0f);
    DebugInfo scroll_route_debug = context->dump_debug_info();
    bool found_scroll_parent_debug = false;
    for(const DebugElementInfo& element_info : scroll_route_debug.elements)
    {
        if(element_info.id == 711)
        {
            found_scroll_parent_debug = true;
            lutest(element_info.scrollable);
        }
    }
    lutest(found_scroll_parent_debug);

    context->begin_frame(frame);
    nested_wheel_event.position = Float2U(30.0f, 30.0f);
    context->add_input_event(nested_wheel_event);
    context->push_layer(72, Float2U(0.0f), Name("child scroll routing"));
    ElementHandle child_scroll_root = context->begin_element(720, Name("child scroll route root"));
    context->set_layout_result(child_scroll_root, root_layout);
    ElementHandle child_scroll_parent = context->begin_element(721, Name("child scroll parent"));
    context->set_layout_result(child_scroll_parent, scroll_parent_layout);
    context->set_interactable(child_scroll_parent, scroll_parent_interactable);
    ElementHandle child_scroll_child = context->begin_element(722, Name("child scroll child"));
    context->set_layout_result(child_scroll_child, scroll_child_layout);
    scroll_child_interactable.scrollable = true;
    context->set_interactable(child_scroll_child, scroll_child_interactable);
    context->end_element();
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    lutest(context->get_delivered_input_events(721).empty());
    Span<const RoutedInputEvent> child_scroll_events = context->get_routed_input_events(722);
    lutest(child_scroll_events.size() == 1);
    lutest(child_scroll_events[0].has_element_position);
    lutest(child_scroll_events[0].element_position.x == 10.0f);
    lutest(child_scroll_events[0].element_position.y == 10.0f);

    context->begin_frame(frame);
    InputEvent replay_move_event;
    replay_move_event.type = InputEventType::pointer_move;
    replay_move_event.position = Float2U(32.0f, 32.0f);
    context->add_input_event(replay_move_event);
    InputEvent replay_down_event;
    replay_down_event.type = InputEventType::pointer_down;
    replay_down_event.position = Float2U(32.0f, 32.0f);
    replay_down_event.button = PointerButton::left;
    context->add_input_event(replay_down_event);
    InputEvent replay_up_event;
    replay_up_event.type = InputEventType::pointer_up;
    replay_up_event.position = Float2U(32.0f, 32.0f);
    replay_up_event.button = PointerButton::left;
    context->add_input_event(replay_up_event);
    context->push_layer(73, Float2U(0.0f), Name("replay source"));
    ElementHandle replay_source_root = context->begin_element(730, Name("replay source root"));
    context->set_layout_result(replay_source_root, root_layout);
    ElementHandle replay_source_button = context->begin_element(731, Name("replay source button"));
    context->set_layout_result(replay_source_button, button_layout);
    context->set_interactable(replay_source_button, interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    context->route_input();
    InteractionState replay_source_state = context->get_interaction_state(731);
    lutest(replay_source_state.hovered);
    lutest(replay_source_state.clicked);
    DebugInfo replay_source_debug = context->dump_debug_info();
    lutest(replay_source_debug.input_events.size() == 3);

    context->begin_frame(frame);
    context->push_layer(74, Float2U(0.0f), Name("replay target"));
    ElementHandle replay_target_root = context->begin_element(740, Name("replay target root"));
    context->set_layout_result(replay_target_root, root_layout);
    ElementHandle replay_target_button = context->begin_element(741, Name("replay target button"));
    context->set_layout_result(replay_target_button, button_layout);
    context->set_interactable(replay_target_button, interactable);
    context->end_element();
    context->end_element();
    context->pop_layer();
    lupanic_if_failed(replay_input_events(context.get(), replay_source_debug));
    context->route_input();
    InteractionState replay_target_state = context->get_interaction_state(741);
    lutest(replay_target_state.hovered);
    lutest(replay_target_state.clicked);
    lutest(replay_target_state.clicked_screen_position.x == 32.0f);
    lutest(replay_target_state.clicked_screen_position.y == 32.0f);
    DebugInfo replay_target_debug = context->dump_debug_info();
    lutest(replay_target_debug.input_events.size() == replay_source_debug.input_events.size());
    lutest(replay_target_debug.input_events[0].type == replay_source_debug.input_events[0].type);
    lutest(replay_target_debug.input_events[0].position.x == replay_source_debug.input_events[0].position.x);
    lutest(failed(replay_input_events(nullptr, replay_source_debug)));

    DebugFrameTimeline timeline;
    lutest(current_debug_frame(timeline) == nullptr);
    push_debug_frame(timeline, replay_source_debug, 2);
    push_debug_frame(timeline, replay_target_debug, 2);
    lutest(timeline.frames.size() == 2);
    lutest(timeline.cursor == 1);
    const DebugInfo* current_frame = current_debug_frame(timeline);
    lutest(current_frame && current_frame->counters.frame_generation == replay_target_debug.counters.frame_generation);
    const DebugInfo* previous_frame = step_debug_frame(timeline, -1);
    lutest(previous_frame && previous_frame->counters.frame_generation == replay_source_debug.counters.frame_generation);
    lutest(step_debug_frame(timeline, -100)->counters.frame_generation == replay_source_debug.counters.frame_generation);
    lutest(step_debug_frame(timeline, 100)->counters.frame_generation == replay_target_debug.counters.frame_generation);
    lutest(seek_debug_frame(timeline, 0)->counters.frame_generation == replay_source_debug.counters.frame_generation);
    lutest(seek_debug_frame(timeline, 999)->counters.frame_generation == replay_target_debug.counters.frame_generation);
    push_debug_frame(timeline, replay_target_debug, 2);
    lutest(timeline.frames.size() == 2);
    lutest(timeline.cursor == 1);
    lutest(timeline.frames[0].counters.frame_generation == replay_target_debug.counters.frame_generation);
    clear_debug_frames(timeline);
    lutest(timeline.frames.empty());
    lutest(timeline.cursor == 0);
    lutest(current_debug_frame(timeline) == nullptr);

    text_draw_list.reset();
    context.reset();
    Luna::close();
    return 0;
}
