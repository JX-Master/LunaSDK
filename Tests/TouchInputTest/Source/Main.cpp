/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/10/20
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/GUI/GUI.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Window/Event.hpp>
#ifdef LUNA_PLATFORM_WINDOWS
#include <Luna/Window/Windows/Win32Window.hpp>
#endif
#include <cstdio>
#include <cmath>

using namespace Luna;
#define CHECK(condition) do { if(!(condition)) { std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return false; } } while(false)

namespace
{
    bool nearly_equal(f32 a, f32 b) { return std::fabs(a - b) < 0.01f; }

    void send(GUI::IContext* gui, GUI::InputEventType type, f32 x, f32 y, u64 touch = 1, u64 device = 7)
    {
        GUI::InputEvent event;
        event.type = type;
        event.position = Float2U(x, y);
        event.touch_id = touch;
        event.device_id = device;
        gui->add_input_event(event);
    }

    void tap(GUI::IContext* gui, f32 x, f32 y, u64 touch = 1)
    {
        send(gui, GUI::InputEventType::touch_down, x, y, touch);
        send(gui, GUI::InputEventType::touch_up, x, y, touch);
    }

    struct Surface
    {
        Ref<GUI::IContext> gui;
        GUI::ElementHandle root;

        Surface(EditorGUI::InputMode mode = EditorGUI::InputMode::pointer)
        {
            gui = GUI::new_context();
            lupanic_if_failed(gui->register_font(Name("default"), Font::get_default_font()));
            EditorGUI::register_style_schemas(gui);
            EditorGUI::DefaultStyleDesc style;
            style.input_mode = mode;
            EditorGUI::set_default_style(gui, style);
        }

        void begin()
        {
            GUI::FrameDesc frame;
            frame.logical_size = Float2U(400, 300);
            frame.render_size = UInt2U(400, 300);
            frame.delta_time = 1.0f / 60.0f;
            gui->begin_frame(frame);
            gui->push_layer(1, Float2U(0));
            root = EditorGUI::begin_v_layout(gui, 2, "Test surface");
        }

        void layout()
        {
            EditorGUI::end_v_layout(gui, root);
            gui->pop_layer();
            lupanic_if_failed(EditorGUI::layout_tree(gui, root, RectF(0, 0, 400, 300)));
        }

        void resolve()
        {
            gui->route_input();
            auto result = EditorGUI::resolve_interactions(gui);
            if(result.relayout_requested)
                lupanic_if_failed(EditorGUI::layout_tree(gui, root, RectF(0, 0, 400, 300)));
        }

        Float2U center(GUI::id_t id)
        {
            const auto* element = gui->find_element(id);
            const auto& rect = element->layout_result.rect;
            const auto& layer = gui->get_layers()[element->layer];
            return Float2U(rect.offset_x + rect.width * 0.5f, rect.offset_y + rect.height * 0.5f) + layer.screen_position;
        }

        void tap_item(GUI::id_t id)
        {
            auto point = center(id);
            tap(gui, point.x, point.y);
        }
    };

    GUI::LayoutConfig control(f32 height = 40)
    {
        GUI::LayoutConfig layout;
        layout.width.kind = GUI::SizeKind::percent;
        layout.width.value = 1;
        layout.height.kind = GUI::SizeKind::fixed;
        layout.height.value = height;
        layout.flex_shrink = 0;
        return layout;
    }

    void buttons(Surface& surface, bool disable_first = false, bool include_first = true)
    {
        surface.begin();
        EditorGUI::ButtonDesc desc;
        desc.enabled = !disable_first;
        if(include_first) EditorGUI::text_button(surface.gui, 10, "First", control(), desc);
        EditorGUI::text_button(surface.gui, 11, "Second", control());
        surface.layout();
    }

    bool independent_contacts()
    {
        Surface s;
        buttons(s);
        send(s.gui, GUI::InputEventType::pointer_move, 20, 60);
        send(s.gui, GUI::InputEventType::pointer_down, 20, 60);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20, 1, 7);
        send(s.gui, GUI::InputEventType::touch_down, 30, 20, 1, 8);
        s.resolve();
        CHECK(s.gui->captured_element() == 11);
        CHECK(s.gui->is_pointer_button_down(GUI::PointerButton::left));
        CHECK(nearly_equal(s.gui->get_pointer_position().y, 60));
        CHECK(s.gui->get_interaction_state(11).hovered);
        CHECK(!s.gui->get_interaction_state(10).hovered);
        GUI::TouchState first, second;
        CHECK(s.gui->get_touch_state(7, 1, first));
        CHECK(s.gui->get_touch_state(8, 1, second));
        CHECK(first.captured_element == 10 && second.captured_element == 10);
        CHECK(s.gui->get_interaction_state(10).touch_active);

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_cancel, 20, 20, 1, 7);
        s.resolve();
        CHECK(!s.gui->get_touch_state(7, 1, first));
        CHECK(s.gui->get_touch_state(8, 1, second));
        CHECK(s.gui->get_interaction_state(10).touch_active);
        CHECK(s.gui->captured_element() == 11);
        CHECK(!s.gui->get_interaction_state(10).clicked);

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_up, 30, 20, 1, 8);
        send(s.gui, GUI::InputEventType::pointer_up, 20, 60);
        s.resolve();
        CHECK(s.gui->get_interaction_state(10).touch_clicked);
        CHECK(s.gui->get_interaction_state(11).clicked);
        CHECK(!s.gui->get_interaction_state(11).touch_clicked);
        return true;
    }

    bool cancellation_and_reuse()
    {
        Surface s;
        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        s.resolve();
        buttons(s, true);
        s.resolve();
        GUI::TouchState state;
        CHECK(!s.gui->get_touch_state(7, 1, state));
        bool canceled = false;
        for(const auto& event : s.gui->get_delivered_input_events(10))
            canceled |= event.type == GUI::InputEventType::touch_cancel;
        CHECK(canceled && !s.gui->get_interaction_state(10).clicked);

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_up, 20, 20);
        s.resolve();
        CHECK(!s.gui->get_interaction_state(10).clicked);
        buttons(s);
        tap(s.gui, 20, 20);
        s.resolve();
        CHECK(s.gui->get_interaction_state(10).touch_clicked);

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        s.resolve();
        buttons(s, false, false);
        s.resolve();
        CHECK(!s.gui->get_touch_state(7, 1, state));

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        send(s.gui, GUI::InputEventType::blur, 0, 0);
        s.resolve();
        CHECK(!s.gui->get_touch_state(7, 1, state));
        CHECK(!s.gui->get_interaction_state(10).clicked);
        return true;
    }

    bool tap_threshold_and_capture()
    {
        Surface s;
        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        send(s.gui, GUI::InputEventType::touch_move, 24, 21);
        send(s.gui, GUI::InputEventType::touch_up, 24, 21);
        s.resolve();
        CHECK(s.gui->get_interaction_state(10).touch_clicked);
        buttons(s);
        tap(s.gui, 24, 21, 2);
        s.resolve();
        CHECK(s.gui->get_interaction_state(10).touch_double_clicked);
        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        send(s.gui, GUI::InputEventType::touch_up, 120, 20);
        s.resolve();
        CHECK(!s.gui->get_interaction_state(10).clicked);

        buttons(s);
        send(s.gui, GUI::InputEventType::touch_down, 20, 20);
        s.resolve();
        s.gui->capture_touch(7, 1, 11);
        GUI::TouchState state;
        CHECK(s.gui->get_touch_state(7, 1, state) && state.captured_element == 11);
        CHECK(s.gui->captured_element() == 0);
        buttons(s);
        send(s.gui, GUI::InputEventType::touch_move, 500, 500);
        s.resolve();
        bool delivered = false;
        for(const auto& event : s.gui->get_delivered_input_events(11))
            delivered |= event.type == GUI::InputEventType::touch_move;
        CHECK(delivered);
        s.gui->release_touch_capture(7, 1);
        CHECK(s.gui->get_touch_state(7, 1, state) && state.captured_element == 0);
        return true;
    }

    GUI::ElementHandle scroll_frame(Surface& s, bool slider, f32* value = nullptr)
    {
        s.begin();
        EditorGUI::ScrollViewDesc desc;
        desc.horizontal = false;
        desc.scrollbar_mode = EditorGUI::ScrollBarMode::always_visible;
        auto viewport = EditorGUI::begin_scroll_view(s.gui, 20, "Scrollable", control(240), desc);
        if(slider) EditorGUI::slider_float(s.gui, 21, value, 0, 1, control());
        EditorGUI::text_button(s.gui, 22, "Scrollable child button", control(1000));
        EditorGUI::end_scroll_view(s.gui);
        s.layout();
        return viewport;
    }

    f32 scroll_y(Surface& s, GUI::ElementHandle viewport)
    {
        const auto* parent = s.gui->get_element(viewport.index);
        return -s.gui->get_element(parent->first_child)->layout_result.rect.offset_y;
    }

    bool scroll_and_arbitration()
    {
        for(auto mode : {EditorGUI::InputMode::pointer, EditorGUI::InputMode::touch})
        {
            Surface s(mode);
            auto viewport = scroll_frame(s, false);
            send(s.gui, GUI::InputEventType::touch_down, 100, 180);
            s.resolve();
            viewport = scroll_frame(s, false);
            send(s.gui, GUI::InputEventType::touch_move, 100, 60);
            send(s.gui, GUI::InputEventType::pointer_move, 300, 200);
            s.resolve();
            CHECK(nearly_equal(scroll_y(s, viewport), 120));
            CHECK(!s.gui->get_interaction_state(22).clicked);
            bool canceled = false;
            for(const auto& event : s.gui->get_delivered_input_events(22))
                canceled |= event.type == GUI::InputEventType::touch_cancel;
            CHECK(canceled);
            viewport = scroll_frame(s, false);
            send(s.gui, GUI::InputEventType::touch_up, 100, 60);
            s.resolve();
            CHECK(nearly_equal(scroll_y(s, viewport), 120));
            CHECK(!s.gui->get_interaction_state(22).clicked);

            Surface slider(mode);
            f32 value = 0.5f;
            scroll_frame(slider, true, &value);
            send(slider.gui, GUI::InputEventType::touch_down, 100, 20);
            slider.resolve();
            viewport = scroll_frame(slider, true, &value);
            send(slider.gui, GUI::InputEventType::touch_move, 200, 20);
            slider.resolve();
            CHECK(value > 0.5f && nearly_equal(scroll_y(slider, viewport), 0));
            scroll_frame(slider, true, &value);
            send(slider.gui, GUI::InputEventType::touch_cancel, 200, 20);
            slider.resolve();

            Surface vertical(mode);
            value = 0.5f;
            scroll_frame(vertical, true, &value);
            send(vertical.gui, GUI::InputEventType::touch_down, 100, 20);
            vertical.resolve();
            viewport = scroll_frame(vertical, true, &value);
            send(vertical.gui, GUI::InputEventType::touch_move, 100, -80);
            vertical.resolve();
            CHECK(nearly_equal(scroll_y(vertical, viewport), 100));
            GUI::TouchState state;
            CHECK(vertical.gui->get_touch_state(7, 1, state) && state.captured_element == 20);
        }
        return true;
    }

    bool widget_activation_and_ownership()
    {
        Surface s;
        bool checked = false, toggled = false;
        i32 radio = 0;
        s.begin();
        EditorGUI::checkbox(s.gui, 30, "Check", &checked, control());
        EditorGUI::toggle_switch(s.gui, 31, "Toggle", &toggled, control());
        EditorGUI::radio_button(s.gui, 32, "Radio", &radio, 3, control());
        s.layout();
        s.tap_item(30); s.tap_item(31); s.tap_item(32);
        s.resolve();
        CHECK(checked && toggled && radio == 3);

        f32 first = 0, second = 0;
        auto sliders = [&]()
        {
            s.begin();
            EditorGUI::slider_float(s.gui, 40, &first, 0, 1, control());
            EditorGUI::slider_float(s.gui, 41, &second, 0, 1, control());
            s.layout();
        };
        sliders();
        send(s.gui, GUI::InputEventType::touch_down, 80, 20, 1);
        send(s.gui, GUI::InputEventType::touch_down, 120, 60, 2);
        s.resolve();
        sliders();
        send(s.gui, GUI::InputEventType::touch_move, 240, 20, 1);
        send(s.gui, GUI::InputEventType::touch_move, 320, 60, 2);
        send(s.gui, GUI::InputEventType::pointer_down, 40, 20);
        send(s.gui, GUI::InputEventType::pointer_move, 10, 20);
        s.resolve();
        CHECK(nearly_equal(first, 0.6f) && nearly_equal(second, 0.8f));
        CHECK(s.gui->is_pointer_button_down(GUI::PointerButton::left));
        sliders();
        send(s.gui, GUI::InputEventType::touch_up, 240, 20, 1);
        send(s.gui, GUI::InputEventType::touch_cancel, 320, 60, 2);
        send(s.gui, GUI::InputEventType::pointer_up, 10, 20);
        s.resolve();
        CHECK(nearly_equal(first, 0.6f) && nearly_equal(second, 0.8f));
        return true;
    }

    bool text_drag_and_table()
    {
        Surface s;
        f32 number = 10;
        String text("abcdef");
        auto form = [&]()
        {
            s.begin();
            EditorGUI::DragDesc desc;
            desc.speed = 1;
            EditorGUI::drag_float(s.gui, 50, &number, 0, 1000, control(), desc);
            EditorGUI::input_text(s.gui, 51, text, control());
            s.layout();
        };
        form();
        send(s.gui, GUI::InputEventType::touch_down, 100, 20);
        s.resolve();
        form();
        send(s.gui, GUI::InputEventType::touch_move, 130, 20);
        send(s.gui, GUI::InputEventType::pointer_move, 390, 20);
        s.resolve();
        CHECK(nearly_equal(number, 40));
        form();
        send(s.gui, GUI::InputEventType::touch_cancel, 130, 20);
        tap(s.gui, 10, 60, 2);
        GUI::InputEvent typed;
        typed.type = GUI::InputEventType::text_utf8;
        typed.text = "X";
        s.gui->add_input_event(typed);
        s.resolve();
        CHECK(s.gui->focused_element() == 51);
        CHECK(text.find('X') != String::npos);
        CHECK(!s.gui->is_pointer_button_down(GUI::PointerButton::left));

        auto table = [&]()
        {
            s.begin();
            EditorGUI::TableDesc desc;
            desc.resizable_columns = true;
            auto table = EditorGUI::begin_table_layout(s.gui, 60, "Table", control(160), desc);
            GUI::TableTrackDesc columns[2];
            for(auto& column : columns) { column.kind = GUI::TableTrackSizeKind::pixels; column.value = 200; }
            EditorGUI::set_table_columns(s.gui, Span<const GUI::TableTrackDesc>(columns, 2));
            EditorGUI::begin_table_row(s.gui);
            EditorGUI::text_button(s.gui, 61, "Cell A", control());
            EditorGUI::text_button(s.gui, 62, "Cell B", control());
            EditorGUI::end_table_row(s.gui);
            EditorGUI::end_table_layout(s.gui, table);
            s.layout();
        };
        table();
        f32 initial_width = s.gui->find_element(61)->layout_result.rect.width;
        Float2U splitter(0);
        for(const auto& element : s.gui->get_elements())
            if(element.debug_name == Name("Table Column Splitter")) splitter = s.center(element.id);
        CHECK(splitter.x > 0);
        send(s.gui, GUI::InputEventType::touch_down, splitter.x, splitter.y, 3);
        s.resolve();
        table();
        send(s.gui, GUI::InputEventType::touch_move, splitter.x + 40, splitter.y, 3);
        send(s.gui, GUI::InputEventType::pointer_move, 10, 10);
        s.resolve();
        CHECK(nearly_equal(s.gui->find_element(61)->layout_result.rect.width, initial_width + 40));
        return true;
    }

    GUI::id_t named_element(Surface& s, const char* name)
    {
        for(const auto& element : s.gui->get_elements())
            if(element.debug_name == Name(name)) return element.id;
        return 0;
    }

    bool nested_scroll_views()
    {
        Surface s;
        GUI::ElementHandle outer;
        auto build = [&]()
        {
            s.begin();
            EditorGUI::ScrollViewDesc desc;
            desc.horizontal = false;
            outer = EditorGUI::begin_scroll_view(s.gui, 70, "Outer", control(240), desc);
            EditorGUI::begin_scroll_view(s.gui, 71, "Inner without overflow", control(100), desc);
            EditorGUI::text_button(s.gui, 72, "Inner item", control(80));
            EditorGUI::end_scroll_view(s.gui);
            EditorGUI::text_button(s.gui, 73, "Outer overflow", control(1000));
            EditorGUI::end_scroll_view(s.gui);
            s.layout();
        };
        build();
        send(s.gui, GUI::InputEventType::touch_down, 50, 50);
        s.resolve();
        build();
        send(s.gui, GUI::InputEventType::touch_move, 50, -50);
        s.resolve();
        GUI::TouchState touch;
        CHECK(s.gui->get_touch_state(7, 1, touch) && touch.captured_element == 70);
        CHECK(nearly_equal(scroll_y(s, outer), 100));
        CHECK(!s.gui->get_interaction_state(72).clicked);
        return true;
    }

    bool color_picker_and_docking()
    {
        Surface color;
        f32 rgba[] = {1, 0, 0, 1};
        auto build_color = [&]()
        {
            color.begin();
            EditorGUI::color_edit4(color.gui, 80, "Color preview", rgba, control());
            color.layout();
        };
        build_color();
        color.tap_item(80);
        color.resolve();
        build_color();
        GUI::id_t picker = named_element(color, "Color Picker");
        CHECK(picker != 0);
        auto center = color.center(picker);
        const auto* element = color.gui->find_element(picker);
        Float2U point = center - Float2U(element->layout_result.rect.width * 0.5f - 40,
            element->layout_result.rect.height * 0.5f - 40);
        send(color.gui, GUI::InputEventType::touch_down, point.x, point.y);
        color.resolve();
        CHECK(!nearly_equal(rgba[0], 1) || !nearly_equal(rgba[1], 0) || !nearly_equal(rgba[2], 0));
        build_color();
        f32 previous = rgba[0];
        send(color.gui, GUI::InputEventType::touch_move, point.x + 50, point.y + 70);
        send(color.gui, GUI::InputEventType::pointer_move, 390, 290);
        color.resolve();
        CHECK(!nearly_equal(rgba[0], previous));

        Surface dock;
        bool initialized = false;
        auto build_dock = [&]()
        {
            dock.begin();
            if(!initialized)
            {
                EditorGUI::DockSpaceLayoutDesc desc;
                EditorGUI::DockSpaceFloatingPanelDesc panel;
                panel.panel = 91;
                panel.rect = RectF(40, 40, 240, 180);
                desc.floating_panels.push_back(panel);
                EditorGUI::set_dockspace_layout(dock.gui, 90, desc);
                initialized = true;
            }
            EditorGUI::begin_dock_space(dock.gui, 90, "Dock", control(300));
            if(EditorGUI::begin_dock_panel(dock.gui, 91, "Floating test panel"))
            {
                EditorGUI::text(dock.gui, 92, "Content", control());
                EditorGUI::end_dock_panel(dock.gui);
            }
            EditorGUI::end_dock_space(dock.gui);
            dock.layout();
        };
        build_dock();
        GUI::id_t title = named_element(dock, "Dock Panel Title");
        CHECK(title != 0);
        Float2U initial = dock.center(title);
        send(dock.gui, GUI::InputEventType::touch_down, initial.x, initial.y);
        dock.resolve();
        build_dock();
        send(dock.gui, GUI::InputEventType::touch_move, initial.x + 50, initial.y + 20);
        send(dock.gui, GUI::InputEventType::pointer_move, 0, 0);
        dock.resolve();
        Float2U moved = dock.center(title);
        CHECK(nearly_equal(moved.x, initial.x + 50) && nearly_equal(moved.y, initial.y + 20));
        return true;
    }

    bool touch_menu_keeps_passive_mouse_hover()
    {
        Surface s;
        bool first_open = false, second_open = false;
        auto build = [&]()
        {
            s.begin();
            auto bar = EditorGUI::begin_menu_bar(s.gui, 100, "Menu bar", control());
            first_open = EditorGUI::begin_menu(s.gui, 101, "First");
            if(first_open)
            {
                EditorGUI::menu_item(s.gui, 103, "First action");
                lupanic_if_failed(EditorGUI::end_menu(s.gui, RectF(0, 0, 220, 100)));
            }
            second_open = EditorGUI::begin_menu(s.gui, 102, "Second");
            if(second_open)
            {
                EditorGUI::menu_item(s.gui, 104, "Second action");
                lupanic_if_failed(EditorGUI::end_menu(s.gui, RectF(0, 0, 220, 100)));
            }
            EditorGUI::end_menu_bar(s.gui, bar);
            s.layout();
        };
        build();
        Float2U first = s.center(101), second = s.center(102);
        send(s.gui, GUI::InputEventType::pointer_move, first.x, first.y);
        s.resolve();
        build();
        tap(s.gui, second.x, second.y);
        s.resolve();
        build();
        CHECK(!first_open && second_open);
        s.resolve();
        build();
        CHECK(!first_open && second_open);
        return true;
    }

    bool window_adapter_preserves_touch()
    {
        auto result = Window::new_window("Touch adapter test", 0, 0, 200, 200,
            Window::WindowStyleFlag::resizable, Window::WindowCreationFlag::hidden);
        CHECK(succeeded(result));
        Ref<Window::IWindow> window = result.get();
        Surface s;
        buttons(s);
        auto down = new_object<Window::WindowTouchDownEvent>();
        down->window = window; down->device_id = 17; down->id = 91; down->x = 20; down->y = 20;
        CHECK(GUIWindow::handle_window_event(down.object(), window, s.gui));
        auto motion = new_object<Window::WindowTouchMoveEvent>();
        motion->window = window; motion->device_id = 17; motion->id = 91; motion->x = 500; motion->y = -20;
        CHECK(GUIWindow::handle_window_event(motion.object(), window, s.gui));
        auto cancel = new_object<Window::WindowTouchCancelEvent>();
        cancel->window = window; cancel->device_id = 17; cancel->id = 91;
        CHECK(GUIWindow::handle_window_event(cancel.object(), window, s.gui));
        auto events = s.gui->get_input_events();
        CHECK(events.size() == 3);
        CHECK(events[0].type == GUI::InputEventType::touch_down);
        CHECK(events[1].type == GUI::InputEventType::touch_move && events[1].position.x == 500);
        CHECK(events[2].type == GUI::InputEventType::touch_cancel);
        for(const auto& event : events) CHECK(event.device_id == 17 && event.touch_id == 91);
        s.resolve();
        CHECK(!s.gui->is_pointer_button_down(GUI::PointerButton::left));
        CHECK(s.gui->captured_element() == 0);
        window->close();
        return true;
    }
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({EditorGUI::module_editor_gui(), GUIWindow::module_gui_window()}));
    lupanic_if_failed(init_modules());
    struct Test { const char* name; bool(*run)(); };
    const Test tests[] = {
        {"independent mouse and multiple touch contacts", independent_contacts},
        {"cancellation, disappearing targets and ID reuse", cancellation_and_reuse},
        {"tap thresholds, double tap and explicit contact capture", tap_threshold_and_capture},
        {"content scrolling, click cancellation and directional arbitration", scroll_and_arbitration},
        {"widget activation and concurrent manipulation ownership", widget_activation_and_ownership},
        {"touch text editing, scalar dragging and table resizing", text_drag_and_table},
        {"nested scrolling skips inner views without overflow", nested_scroll_views},
        {"touch color picking and floating dock manipulation", color_picker_and_docking},
        {"touch menus remain open under passive mouse hover", touch_menu_keeps_passive_mouse_hover},
        {"Window to GUI adapter keeps touch events independent", window_adapter_preserves_touch}
    };
    usize failures = 0;
    for(const Test& test : tests)
    {
        bool passed = test.run();
        std::printf("%s: %s\n", passed ? "PASS" : "FAIL", test.name);
        if(!passed) ++failures;
    }
    Luna::close();
    return failures ? 1 : 0;
}
