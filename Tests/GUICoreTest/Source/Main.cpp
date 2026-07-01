/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/6/17
*/
#include "../../Shared/InteractiveGUIDemo.hpp"
#include <Luna/Window/AppMain.hpp>
#include <cstdio>

using namespace Luna;

namespace
{
    constexpr u32 ROUTING_DEMO_NODE_COUNT = 11;
    constexpr u32 ROUTING_DEMO_LAYER_COUNT = 2;
    constexpr u32 ROUTING_DEMO_NO_PARENT = U32_MAX;
    constexpr u32 ROUTING_DEMO_CIRCLE_NODE = 10;

    struct RoutingDemoNodeSetting
    {
        GUICore::InteractableFlag flags = GUICore::InteractableFlag::none;
        GUICore::PointerHitBehavior hit_behavior = GUICore::PointerHitBehavior::none;
    };

    struct RoutingDemoNodeDesc
    {
        const c8* name;
        u32 layer;
        u32 parent;
        RectF rect;
        Float4U color;
        RoutingDemoNodeSetting default_setting;
    };

    struct RoutingHitTraversal
    {
        GUICore::ElementHandle target;
        Vector<GUICore::HitTestVisit> visits;
    };

    struct CoreDemoState
    {
        bool option = true;
        bool readonly = false;
        bool input_routing_initialized = false;
        u32 selected_routing_node = 1;
        RoutingDemoNodeSetting input_routing_nodes[ROUTING_DEMO_NODE_COUNT];
        f32 linear_gap = 8.0f;
        f32 grid_cell_width = 96.0f;
        f32 grid_cell_height = 64.0f;
        i32 grid_columns = 4;
        f32 split = 0.35f;
        f32 scroll_offset_x = 0.0f;
        f32 scroll_offset_y = 0.0f;
        bool grid_fixed_columns = false;
        bool stack_center = true;
        bool style_enabled = true;
        bool style_unset_text = false;
        bool capture_debug_timeline = false;
        GUICore::DebugFrameTimeline timeline;
    };

    RoutingDemoNodeSetting routing_setting(GUICore::PointerHitBehavior hit_behavior,
        GUICore::InteractableFlag flags = GUICore::InteractableFlag::none)
    {
        RoutingDemoNodeSetting setting;
        setting.flags = flags;
        setting.hit_behavior = hit_behavior;
        return setting;
    }

    const RoutingDemoNodeDesc ROUTING_DEMO_NODES[ROUTING_DEMO_NODE_COUNT] = {
        { "Base Layer Root", 0, ROUTING_DEMO_NO_PARENT, RectF(0.0f, 0.0f, 620.0f, 330.0f), Float4U(0.08f, 0.11f, 0.15f, 0.94f),
            routing_setting(GUICore::PointerHitBehavior::block) },
        { "Left Panel", 0, 0, RectF(24.0f, 50.0f, 300.0f, 190.0f), Float4U(0.10f, 0.20f, 0.30f, 0.90f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable) },
        { "Focusable Button", 0, 1, RectF(52.0f, 84.0f, 170.0f, 46.0f), Float4U(0.08f, 0.36f, 0.58f, 0.95f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable |
                GUICore::InteractableFlag::activatable | GUICore::InteractableFlag::focusable) },
        { "Clipped Child", 0, 1, RectF(206.0f, 172.0f, 170.0f, 72.0f), Float4U(0.38f, 0.22f, 0.12f, 0.88f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable |
                GUICore::InteractableFlag::activatable) },
        { "Input Blocker", 0, 0, RectF(364.0f, 72.0f, 190.0f, 94.0f), Float4U(0.34f, 0.11f, 0.16f, 0.72f),
            routing_setting(GUICore::PointerHitBehavior::block) },
        { "Pass-through Overlay", 0, 0, RectF(384.0f, 210.0f, 202.0f, 64.0f), Float4U(0.54f, 0.42f, 0.08f, 0.56f),
            routing_setting(GUICore::PointerHitBehavior::pass_through, GUICore::InteractableFlag::hoverable) },
        { "Floating Layer Root", 1, ROUTING_DEMO_NO_PARENT, RectF(0.0f, 0.0f, 360.0f, 210.0f), Float4U(0.08f, 0.10f, 0.14f, 0.92f),
            routing_setting(GUICore::PointerHitBehavior::block) },
        { "Floating Panel", 1, 6, RectF(32.0f, 42.0f, 260.0f, 122.0f), Float4U(0.12f, 0.17f, 0.30f, 0.92f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable) },
        { "Top Button", 1, 7, RectF(62.0f, 76.0f, 160.0f, 44.0f), Float4U(0.04f, 0.40f, 0.65f, 0.96f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable |
                GUICore::InteractableFlag::activatable | GUICore::InteractableFlag::focusable) },
        { "Top Blocker", 1, 7, RectF(150.0f, 116.0f, 180.0f, 70.0f), Float4U(0.46f, 0.10f, 0.28f, 0.62f),
            routing_setting(GUICore::PointerHitBehavior::block) },
        { "Circle Hit Button", 0, 0, RectF(484.0f, 228.0f, 86.0f, 86.0f), Float4U(0.08f, 0.42f, 0.58f, 0.94f),
            routing_setting(GUICore::PointerHitBehavior::target, GUICore::InteractableFlag::hoverable |
                GUICore::InteractableFlag::activatable | GUICore::InteractableFlag::focusable) }
    };

    const c8* ROUTING_DEMO_LAYER_NAMES[ROUTING_DEMO_LAYER_COUNT] = {
        "Base Layer",
        "Floating Layer"
    };

    GUICore::LayoutInput row_layout()
    {
        return Test::fill_width_layout(30.0f);
    }

    GUICore::LayoutInput scroll_content_layout(f32 min_height)
    {
        GUICore::LayoutInput layout;
        layout.width.kind = GUICore::SizeKind::expand;
        layout.height.kind = GUICore::SizeKind::fit;
        layout.height.min = min_height;
        return layout;
    }

    void add_text(GUICore::IContext* context, GUICore::id_t id, const c8* text)
    {
        GUI::text(context, id, text, row_layout());
    }

    void add_status(GUICore::IContext* context, GUICore::id_t id, const c8* label, const c8* value)
    {
        Test::label_value(context, id, label, value);
    }

    RectF intersect_local_rect(const RectF& a, const RectF& b)
    {
        f32 min_x = max(a.offset_x, b.offset_x);
        f32 min_y = max(a.offset_y, b.offset_y);
        f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
        f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
        return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
    }

    GUICore::id_t routing_node_id(u32 node)
    {
        return Test::demo_id("core.input.routing.node", node);
    }

    u32 routing_node_index_from_id(GUICore::id_t id)
    {
        for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
        {
            if(routing_node_id(i) == id)
            {
                return i;
            }
        }
        return U32_MAX;
    }

    void initialize_input_routing_state(CoreDemoState& state)
    {
        if(state.input_routing_initialized)
        {
            return;
        }
        for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
        {
            state.input_routing_nodes[i] = ROUTING_DEMO_NODES[i].default_setting;
        }
        state.input_routing_initialized = true;
    }

    void draw_element_line(GUICore::IContext* context, const GUICore::ElementHandle& element,
        const Float2U& begin, const Float2U& end, const Float4U& color, f32 width)
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::line;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
        command.point1 = end;
        command.color = color;
        command.line_width = width;
        context->draw_for_element(element, command);
    }

    void draw_element_outline(GUICore::IContext* context, const GUICore::ElementHandle& element,
        const RectF& local_rect, const Float4U& color, f32 width)
    {
        if(local_rect.width <= 0.0f || local_rect.height <= 0.0f)
        {
            return;
        }
        Float2U p0(local_rect.offset_x, local_rect.offset_y);
        Float2U p1(local_rect.offset_x + local_rect.width, local_rect.offset_y);
        Float2U p2(local_rect.offset_x + local_rect.width, local_rect.offset_y + local_rect.height);
        Float2U p3(local_rect.offset_x, local_rect.offset_y + local_rect.height);
        draw_element_line(context, element, p0, p1, color, width);
        draw_element_line(context, element, p1, p2, color, width);
        draw_element_line(context, element, p2, p3, color, width);
        draw_element_line(context, element, p3, p0, color, width);
    }

    void draw_element_background(GUICore::IContext* context, const GUICore::ElementHandle& element,
        const Float4U& color, f32 radius = 4.0f)
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::rounded_rect;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        command.color = color;
        command.radius = radius;
        context->draw_for_element(element, command);
    }

    void draw_element_label(GUICore::IContext* context, const GUICore::ElementHandle& element, const c8* label,
        const Float4U& color = Float4U(0.94f, 0.96f, 0.98f, 1.0f))
    {
        GUICore::DrawCommand command;
        command.type = GUICore::DrawCommandType::text;
        command.rect_reference = GUICore::DrawCommandRectReference::element;
        command.rect = RectF(8.0f, 4.0f, -16.0f, 22.0f);
        command.text = label ? label : "";
        command.color = color;
        command.font_size = 13.0f;
        context->draw_for_element(element, command);
    }

    void set_routing_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
        const RoutingDemoNodeSetting& setting)
    {
        GUICore::Interactable interactable;
        interactable.flags = setting.flags;
        interactable.pointer_hit_behavior = setting.hit_behavior;
        context->set_interactable(element, interactable);
    }

    bool circle_hit_test(const GUICore::IContext*, const GUICore::ElementHitTestRequest& request, void*)
    {
        f32 radius = min(request.element_rect.width, request.element_rect.height) * 0.5f;
        Float2U center(request.element_rect.width * 0.5f, request.element_rect.height * 0.5f);
        f32 dx = request.element_position.x - center.x;
        f32 dy = request.element_position.y - center.y;
        return dx * dx + dy * dy <= radius * radius;
    }

    void build_routing_demo_node(GUICore::IContext* context, CoreDemoState& state, u32 node,
        const RectF& parent_clip)
    {
        const RoutingDemoNodeDesc& desc = ROUTING_DEMO_NODES[node];
        GUICore::ElementHandle element = context->begin_element(routing_node_id(node), Name(desc.name));
        GUICore::LayoutInput layout;
        layout.width.kind = GUICore::SizeKind::pixels;
        layout.width.value = desc.rect.width;
        layout.height.kind = GUICore::SizeKind::pixels;
        layout.height.value = desc.rect.height;
        context->set_layout(element, layout);

        RectF clip = parent_clip.width > 0.0f || parent_clip.height > 0.0f ? intersect_local_rect(desc.rect, parent_clip) : desc.rect;
        GUICore::LayoutResult result;
        result.rect = desc.rect;
        result.clip_rect = clip;
        result.content_size = Float2U(desc.rect.width, desc.rect.height);
        context->set_layout_result(element, result);
        set_routing_interactable(context, element, state.input_routing_nodes[node]);
        if(node == ROUTING_DEMO_CIRCLE_NODE)
        {
            GUICore::ElementHitTestConfig hit_test;
            hit_test.mode = GUICore::ElementHitTestMode::callback;
            hit_test.callback = circle_hit_test;
            context->set_hit_test_config(element, hit_test);
        }

        draw_element_background(context, element, desc.color, node == ROUTING_DEMO_CIRCLE_NODE ? desc.rect.width * 0.5f : 4.0f);
        draw_element_label(context, element, desc.name);
        draw_element_outline(context, element, RectF(0.0f, 0.0f, desc.rect.width, desc.rect.height),
            Float4U(0.08f, 0.86f, 1.0f, 0.95f), 1.5f);
        RectF local_clip(clip.offset_x - desc.rect.offset_x, clip.offset_y - desc.rect.offset_y, clip.width, clip.height);
        draw_element_outline(context, element, local_clip, Float4U(1.0f, 0.72f, 0.10f, 0.95f), 1.5f);

        for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
        {
            if(ROUTING_DEMO_NODES[i].layer == desc.layer && ROUTING_DEMO_NODES[i].parent == node)
            {
                build_routing_demo_node(context, state, i, clip);
            }
        }
        context->end_element();
    }

    void draw_routing_target_overlay(GUICore::IContext* context, const GUICore::ElementHandle& target)
    {
        if(routing_node_index_from_id(target.id) == U32_MAX)
        {
            return;
        }
        const GUICore::Element* element = context->get_element(target.index);
        if(!element)
        {
            return;
        }
        draw_element_outline(context, target, RectF(-3.0f, -3.0f, element->layout_result.rect.width + 6.0f,
            element->layout_result.rect.height + 6.0f), Float4U(0.20f, 1.0f, 0.38f, 1.0f), 3.0f);
    }

    RoutingHitTraversal collect_routing_hit_traversal(GUICore::IContext* context, const Float2U& screen_position)
    {
        RoutingHitTraversal traversal;
        traversal.target = context->hit_test(screen_position, [&](const GUICore::HitTestVisit& visit) {
            if(routing_node_index_from_id(visit.element.id) != U32_MAX)
            {
                traversal.visits.push_back(visit);
            }
        });
        if(routing_node_index_from_id(traversal.target.id) == U32_MAX)
        {
            traversal.target = GUICore::ElementHandle();
        }
        return traversal;
    }

    void build_routing_demo_layers(GUICore::IContext* context, CoreDemoState& state)
    {
        initialize_input_routing_state(state);
        const Float2U layer_positions[ROUTING_DEMO_LAYER_COUNT] = {
            Float2U(34.0f, 100.0f),
            Float2U(330.0f, 70.0f)
        };
        for(u32 layer = 0; layer < ROUTING_DEMO_LAYER_COUNT; ++layer)
        {
            context->push_layer(Test::demo_id("core.input.routing.layer", layer), layer_positions[layer],
                Name(ROUTING_DEMO_LAYER_NAMES[layer]));
            for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
            {
                if(ROUTING_DEMO_NODES[i].layer == layer && ROUTING_DEMO_NODES[i].parent == ROUTING_DEMO_NO_PARENT)
                {
                    build_routing_demo_node(context, state, i, RectF(0.0f, 0.0f, 0.0f, 0.0f));
                }
            }
            context->pop_layer();
        }
        draw_routing_target_overlay(context, collect_routing_hit_traversal(context, context->get_pointer_position()).target);
    }

    const c8* routing_node_name_from_id(GUICore::id_t id)
    {
        u32 index = routing_node_index_from_id(id);
        if(index != U32_MAX)
        {
            return ROUTING_DEMO_NODES[index].name;
        }
        return "(none)";
    }

    void build_routing_traversal_view(GUICore::IContext* context, const RoutingHitTraversal& traversal)
    {
        add_text(context, Test::demo_id("core.input.routing.traversal.title"), "Hit traversal");
        if(traversal.visits.empty())
        {
            add_text(context, Test::demo_id("core.input.routing.traversal.empty"), "(none inside the routing demo graph)");
            return;
        }
        char text[256];
        for(usize i = 0; i < traversal.visits.size(); ++i)
        {
            const GUICore::HitTestVisit& visit = traversal.visits[i];
            const c8* role = visit.routing_stop ? (visit.event_target ? "event target" : "blocker") :
                (visit.pointer_hit_behavior == GUICore::PointerHitBehavior::pass_through ? "pass-through" : "visited");
            snprintf(text, sizeof(text), "%02u  %s  layer=%u  %s",
                (u32)i,
                routing_node_name_from_id(visit.element.id),
                visit.element_data ? visit.element_data->layer : 0,
                role);
            add_text(context, Test::demo_id("core.input.routing.traversal.item", (u64)i), text);
        }
    }

    const c8* pointer_hit_behavior_name(GUICore::PointerHitBehavior behavior)
    {
        switch(behavior)
        {
        case GUICore::PointerHitBehavior::pass_through: return "Pass";
        case GUICore::PointerHitBehavior::target: return "Target";
        case GUICore::PointerHitBehavior::block: return "Block";
        default: return "None";
        }
    }

    i32 pointer_hit_behavior_index(GUICore::PointerHitBehavior behavior)
    {
        switch(behavior)
        {
        case GUICore::PointerHitBehavior::pass_through: return 1;
        case GUICore::PointerHitBehavior::target: return 2;
        case GUICore::PointerHitBehavior::block: return 3;
        default: return 0;
        }
    }

    GUICore::PointerHitBehavior pointer_hit_behavior_from_index(i32 index)
    {
        switch(index)
        {
        case 1: return GUICore::PointerHitBehavior::pass_through;
        case 2: return GUICore::PointerHitBehavior::target;
        case 3: return GUICore::PointerHitBehavior::block;
        default: return GUICore::PointerHitBehavior::none;
        }
    }

    void set_flag_from_bool(GUICore::InteractableFlag& flags, GUICore::InteractableFlag flag, bool value)
    {
        set_flags(flags, flag, value);
    }

    void edit_routing_flag(GUICore::IContext* context, CoreDemoState& state, GUICore::InteractableFlag flag,
        const c8* label, u32 row)
    {
        RoutingDemoNodeSetting& setting = state.input_routing_nodes[state.selected_routing_node];
        bool value = test_flags(setting.flags, flag);
        GUI::checkbox(context, Test::demo_id("core.input.routing.setting", row), label, &value, row_layout());
        set_flag_from_bool(setting.flags, flag, value);
    }

    void build_routing_tree_node(GUICore::IContext* context, CoreDemoState& state, u32 node, u32 depth)
    {
        GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::default_open;
        bool leaf = true;
        for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
        {
            if(ROUTING_DEMO_NODES[i].layer == ROUTING_DEMO_NODES[node].layer && ROUTING_DEMO_NODES[i].parent == node)
            {
                leaf = false;
                break;
            }
        }
        if(leaf)
        {
            set_flags(flags, GUI::TreeNodeFlag::leaf);
        }
        if(state.selected_routing_node == node)
        {
            set_flags(flags, GUI::TreeNodeFlag::selected);
        }
        GUICore::ElementHandle handle;
        bool open = GUI::tree_node(context, Test::demo_id("core.input.routing.tree.node", node),
            ROUTING_DEMO_NODES[node].name, flags, depth, row_layout(), &handle);
        if(GUI::is_item_clicked(context, handle))
        {
            state.selected_routing_node = node;
        }
        if(open)
        {
            for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
            {
                if(ROUTING_DEMO_NODES[i].layer == ROUTING_DEMO_NODES[node].layer && ROUTING_DEMO_NODES[i].parent == node)
                {
                    build_routing_tree_node(context, state, i, depth + 1);
                }
            }
        }
    }

    void build_routing_tree_view(GUICore::IContext* context, CoreDemoState& state)
    {
        add_text(context, Test::demo_id("core.input.routing.tree.title"), "Element Tree");
        for(u32 layer = 0; layer < ROUTING_DEMO_LAYER_COUNT; ++layer)
        {
            if(GUI::tree_node(context, Test::demo_id("core.input.routing.tree.layer", layer),
                ROUTING_DEMO_LAYER_NAMES[layer], GUI::TreeNodeFlag::default_open, 0, row_layout()))
            {
                for(u32 i = 0; i < ROUTING_DEMO_NODE_COUNT; ++i)
                {
                    if(ROUTING_DEMO_NODES[i].layer == layer && ROUTING_DEMO_NODES[i].parent == ROUTING_DEMO_NO_PARENT)
                    {
                        build_routing_tree_node(context, state, i, 1);
                    }
                }
            }
        }
    }

    void build_routing_inspector(GUICore::IContext* context, CoreDemoState& state)
    {
        add_text(context, Test::demo_id("core.input.routing.inspector.title"), "Selected Interactable");
        u32 selected = min(state.selected_routing_node, ROUTING_DEMO_NODE_COUNT - 1);
        state.selected_routing_node = selected;
        char text[256];
        snprintf(text, sizeof(text), "%s  id=%llu", ROUTING_DEMO_NODES[selected].name,
            (unsigned long long)routing_node_id(selected));
        add_text(context, Test::demo_id("core.input.routing.inspector.name"), text);
        snprintf(text, sizeof(text), "PointerHitBehavior = %s",
            pointer_hit_behavior_name(state.input_routing_nodes[selected].hit_behavior));
        add_text(context, Test::demo_id("core.input.routing.inspector.behavior.label"), text);
        add_text(context, Test::demo_id("core.input.routing.inspector.hit.shape"),
            selected == ROUTING_DEMO_CIRCLE_NODE ? "Hit Test = circle callback inside layout rect" : "Hit Test = default layout rectangle");
        const c8* behavior_items[] = { "None", "Pass", "Target", "Block" };
        i32 behavior_index = pointer_hit_behavior_index(state.input_routing_nodes[selected].hit_behavior);
        GUI::button_group(context, Test::demo_id("core.input.routing.inspector.behavior"),
            &behavior_index, Span<const c8*>(behavior_items, 4), Test::fill_width_layout(34.0f));
        state.input_routing_nodes[selected].hit_behavior = pointer_hit_behavior_from_index(behavior_index);
        edit_routing_flag(context, state, GUICore::InteractableFlag::hoverable, "hoverable", 2);
        edit_routing_flag(context, state, GUICore::InteractableFlag::activatable, "activatable", 3);
        edit_routing_flag(context, state, GUICore::InteractableFlag::focusable, "focusable", 4);
        edit_routing_flag(context, state, GUICore::InteractableFlag::scrollable, "scrollable", 5);
        edit_routing_flag(context, state, GUICore::InteractableFlag::disabled, "disabled", 6);
        edit_routing_flag(context, state, GUICore::InteractableFlag::read_only, "read_only", 7);
    }

    void end_page_body(GUICore::IContext* context, const GUICore::ElementHandle& body, const GUICore::ElementHandle& scroll)
    {
        GUICore::LinearLayoutDesc body_desc;
        body_desc.axis = GUICore::LayoutAxis::y;
        body_desc.gap = 8.0f;
        lupanic_if_failed(GUI::end_v_layout(context, body, body_desc));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));
    }

    void build_input_page(GUICore::IContext* context, CoreDemoState& state)
    {
        initialize_input_routing_state(state);
        state.selected_routing_node = min(state.selected_routing_node, ROUTING_DEMO_NODE_COUNT - 1);
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.input.scroll"), "Input page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.input.body"), "Input body", Test::fill_layout());
        add_text(context, Test::demo_id("core.input.title"), "Input routing: layers, hierarchy, custom hit testing and interactable flags");
        add_text(context, Test::demo_id("core.input.legend"),
            "Cyan outline = layout rect, amber outline = clip rect, green outline = routing stop. Circle Hit Button only hits inside the circle.");
        GUI::text(context, Test::demo_id("core.input.routing.canvas.placeholder"),
            "Routing graph is drawn in two GUI Core layers above this reserved area.", Test::fill_width_layout(260.0f));
        build_routing_demo_layers(context, state);

        char text[256];
        Float2U pointer = context->get_pointer_position();
        RoutingHitTraversal traversal = collect_routing_hit_traversal(context, pointer);

        GUICore::ElementHandle editor_row = GUI::begin_h_layout(context, Test::demo_id("core.input.routing.editor.row"),
            "Routing editor", Test::fill_width_layout(420.0f));
        GUICore::ElementHandle tree_scroll = GUI::begin_scroll_view(context, Test::demo_id("core.input.routing.tree.scroll"),
            "Routing tree scroll", Test::fixed_layout(360.0f, 410.0f));
        GUICore::ElementHandle tree_column = GUI::begin_v_layout(context, Test::demo_id("core.input.routing.tree.column"),
            "Routing tree", scroll_content_layout(410.0f));
        build_routing_tree_view(context, state);
        build_routing_traversal_view(context, traversal);
        GUICore::LinearLayoutDesc column_desc;
        column_desc.axis = GUICore::LayoutAxis::y;
        column_desc.gap = 4.0f;
        lupanic_if_failed(GUI::end_v_layout(context, tree_column, column_desc));
        lupanic_if_failed(GUI::end_scroll_view(context, tree_scroll));

        GUICore::ElementHandle inspector_scroll = GUI::begin_scroll_view(context, Test::demo_id("core.input.routing.inspector.scroll"),
            "Routing inspector scroll", Test::fill_width_layout(410.0f));
        GUICore::ElementHandle inspector_column = GUI::begin_v_layout(context, Test::demo_id("core.input.routing.inspector.column"),
            "Routing inspector", scroll_content_layout(410.0f));
        build_routing_inspector(context, state);
        lupanic_if_failed(GUI::end_v_layout(context, inspector_column, column_desc));
        lupanic_if_failed(GUI::end_scroll_view(context, inspector_scroll));

        GUICore::LinearLayoutDesc editor_desc;
        editor_desc.axis = GUICore::LayoutAxis::x;
        editor_desc.gap = 16.0f;
        lupanic_if_failed(GUI::end_h_layout(context, editor_row, editor_desc));

        snprintf(text, sizeof(text), "x=%.1f y=%.1f inside=%s", pointer.x, pointer.y, context->is_pointer_inside() ? "yes" : "no");
        add_status(context, Test::demo_id("core.input.pointer"), "Pointer", text);
        snprintf(text, sizeof(text), "%s  id=%llu", routing_node_name_from_id(traversal.target.id),
            (unsigned long long)traversal.target.id);
        add_status(context, Test::demo_id("core.input.hit.test"), "Routing stop", text);
        snprintf(text, sizeof(text), "focused=%llu captured=%llu",
            (unsigned long long)context->focused_element(), (unsigned long long)context->captured_element());
        add_status(context, Test::demo_id("core.input.focus.state"), "Focus/capture", text);
        GUICore::id_t selected_id = routing_node_id(state.selected_routing_node);
        GUICore::InteractionState interaction = context->get_interaction_state(selected_id);
        snprintf(text, sizeof(text), "hovered=%s active=%s focused=%s clicked=%s double=%s delivered=%u routed=%u",
            interaction.hovered ? "yes" : "no", interaction.active ? "yes" : "no",
            interaction.focused ? "yes" : "no", interaction.clicked ? "yes" : "no",
            interaction.double_clicked ? "yes" : "no",
            (u32)context->get_delivered_input_events(selected_id).size(),
            (u32)context->get_routed_input_events(selected_id).size());
        add_status(context, Test::demo_id("core.input.selected.state"), "Selected state", text);
        snprintf(text, sizeof(text), "tab_down=%s primary_down=%s",
            context->is_key_down(KeyCode::tab) ? "yes" : "no",
            context->is_pointer_button_down(GUICore::PointerButton::left) ? "yes" : "no");
        add_status(context, Test::demo_id("core.input.routed.events"), "Delivered events", text);

        end_page_body(context, body, scroll);
    }

    void build_layout_page(GUICore::IContext* context, CoreDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.layout.scroll"), "Layout page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.layout.body"), "Layout body", Test::fill_layout());
        add_text(context, Test::demo_id("core.layout.title"), "Layout primitives");
        GUI::slider_float_with_input(context, Test::demo_id("core.layout.gap"), "Linear gap", &state.linear_gap, 0.0f, 32.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));

        GUICore::ElementHandle h = GUI::begin_h_layout(context, Test::demo_id("core.layout.h"), "HLayout", Test::fill_width_layout(44.0f));
        GUI::text_button(context, Test::demo_id("core.layout.h.a"), "Left", Test::fixed_layout(120.0f, 36.0f));
        GUI::text_button(context, Test::demo_id("core.layout.h.b"), "Middle expands", Test::fill_width_layout(36.0f));
        GUI::text_button(context, Test::demo_id("core.layout.h.c"), "Right", Test::fixed_layout(120.0f, 36.0f));
        GUICore::LinearLayoutDesc h_desc;
        h_desc.axis = GUICore::LayoutAxis::x;
        h_desc.gap = state.linear_gap;
        lupanic_if_failed(GUI::end_h_layout(context, h, h_desc));

        GUI::slider_float_with_input(context, Test::demo_id("core.layout.cell.w"), "Grid cell width", &state.grid_cell_width, 48.0f, 180.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUI::slider_float_with_input(context, Test::demo_id("core.layout.cell.h"), "Grid cell height", &state.grid_cell_height, 32.0f, 120.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUI::checkbox(context, Test::demo_id("core.layout.grid.mode"), "Grid uses fixed column count", &state.grid_fixed_columns, row_layout());
        GUI::slider_int_with_input(context, Test::demo_id("core.layout.grid.columns"), "Grid columns", &state.grid_columns, 1, 8,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUICore::ElementHandle grid = GUI::begin_grid_layout(context, Test::demo_id("core.layout.grid"), "Grid", Test::fill_width_layout(180.0f));
        for(u32 i = 0; i < 12; ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), "Cell %02u", i);
            GUI::text_button(context, Test::demo_id("core.layout.grid.cell", i), label, Test::fill_layout());
        }
        GUICore::GridLayoutDesc grid_desc;
        grid_desc.mode = state.grid_fixed_columns ? GUICore::GridLayoutMode::fixed_column_count : GUICore::GridLayoutMode::fixed_cell_size;
        grid_desc.cell_size = Float2U(state.grid_cell_width, state.grid_cell_height);
        grid_desc.column_count = (u32)max(state.grid_columns, 1);
        grid_desc.gap = Float2U(8.0f, 8.0f);
        lupanic_if_failed(GUI::end_grid_layout(context, grid, grid_desc));

        GUI::checkbox(context, Test::demo_id("core.layout.stack.center"), "Stack children centered", &state.stack_center, row_layout());
        GUICore::ElementHandle stack = GUI::begin_stack_layout(context, Test::demo_id("core.layout.stack"), "Stack", Test::fill_width_layout(130.0f));
        GUI::text_button(context, Test::demo_id("core.layout.stack.back"), "Back layer", Test::fixed_layout(260.0f, 96.0f));
        GUI::text_button(context, Test::demo_id("core.layout.stack.front"), "Front layer", Test::fixed_layout(150.0f, 48.0f));
        GUICore::StackLayoutDesc stack_desc;
        stack_desc.alignment = state.stack_center ? Float2U(0.5f, 0.5f) : Float2U(0.0f, 0.0f);
        lupanic_if_failed(GUI::end_stack_layout(context, stack, stack_desc));

        GUI::slider_float_with_input(context, Test::demo_id("core.layout.canvas.split"), "Canvas anchor X", &state.split, 0.0f, 1.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUICore::ElementHandle canvas = GUI::begin_canvas_layout(context, Test::demo_id("core.layout.canvas"), "Canvas", Test::fill_width_layout(150.0f));
        GUICore::ElementHandle anchored_a = GUI::text_button(context, Test::demo_id("core.layout.canvas.a"), "Anchored A", Test::fixed_layout(120.0f, 32.0f));
        GUICore::ElementHandle anchored_b = GUI::text_button(context, Test::demo_id("core.layout.canvas.b"), "Stretch B", Test::fixed_layout(120.0f, 32.0f));
        GUICore::CanvasLayoutItem canvas_items[2];
        canvas_items[0].element_id = anchored_a.id;
        canvas_items[0].anchor_min = Float2U(state.split, 0.0f);
        canvas_items[0].anchor_max = Float2U(state.split, 0.0f);
        canvas_items[0].offset = Float4U(0.0f, 12.0f, 0.0f, 0.0f);
        canvas_items[0].pivot = Float2U(0.5f, 0.0f);
        canvas_items[1].element_id = anchored_b.id;
        canvas_items[1].anchor_min = Float2U(0.05f, 0.45f);
        canvas_items[1].anchor_max = Float2U(0.95f, 0.45f);
        canvas_items[1].offset = Float4U(0.0f, 0.0f, 0.0f, 36.0f);
        GUICore::CanvasLayoutDesc canvas_desc;
        canvas_desc.items = Span<const GUICore::CanvasLayoutItem>(canvas_items, 2);
        lupanic_if_failed(GUI::end_canvas_layout(context, canvas, canvas_desc));

        GUI::slider_float_with_input(context, Test::demo_id("core.layout.scroll.x"), "Manual scroll X", &state.scroll_offset_x, 0.0f, 260.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUI::slider_float_with_input(context, Test::demo_id("core.layout.scroll.y"), "Manual scroll Y", &state.scroll_offset_y, 0.0f, 260.0f,
            RectF(0.0f, 0.0f, 420.0f, 30.0f), Test::fill_width_layout(34.0f));
        GUICore::ElementHandle viewport = GUI::begin_scroll_viewport(context, Test::demo_id("core.layout.viewport"), "Manual viewport",
            Test::fill_width_layout(130.0f));
        GUICore::ElementHandle viewport_content = GUI::begin_v_layout(context, Test::demo_id("core.layout.viewport.content"),
            "Manual viewport content", Test::fixed_layout(320.0f, 280.0f));
        for(u32 i = 0; i < 8; ++i)
        {
            char label[32];
            snprintf(label, sizeof(label), "Manual item %u", i);
            GUI::text_button(context, Test::demo_id("core.layout.viewport.item", i), label, Test::fixed_layout(220.0f, 30.0f));
        }
        GUICore::LinearLayoutDesc viewport_content_desc;
        viewport_content_desc.axis = GUICore::LayoutAxis::y;
        viewport_content_desc.gap = 6.0f;
        lupanic_if_failed(GUI::end_v_layout(context, viewport_content, viewport_content_desc));
        GUICore::ScrollViewportLayoutDesc viewport_desc;
        viewport_desc.scroll_offset = Float2U(state.scroll_offset_x, state.scroll_offset_y);
        lupanic_if_failed(GUI::end_scroll_viewport(context, viewport, viewport_desc));

        GUICore::ElementHandle table = GUI::begin_table_layout(context, Test::demo_id("core.layout.table"), "Table", Test::fill_width_layout(132.0f));
        GUICore::TableTrackDesc columns[3];
        columns[0].kind = GUICore::TableTrackSizeKind::pixels;
        columns[0].value = 120.0f;
        columns[1].kind = GUICore::TableTrackSizeKind::ratio;
        columns[1].value = 1.0f;
        columns[2].kind = GUICore::TableTrackSizeKind::pixels;
        columns[2].value = 180.0f;
        GUI::set_table_columns(context, Span<const GUICore::TableTrackDesc>(columns, 3));
        GUI::set_table_gap(context, Float2U(4.0f, 4.0f));
        GUI::set_table_cell_padding(context, Float4U(4.0f, 2.0f, 4.0f, 2.0f));
        for(u32 row = 0; row < 4; ++row)
        {
            GUI::begin_table_row(context, GUICore::TableTrackDesc { GUICore::TableTrackSizeKind::pixels, 28.0f, 0.0f, -1.0f });
            for(u32 col = 0; col < 3; ++col)
            {
                char label[32];
                snprintf(label, sizeof(label), "R%u C%u", row, col);
                GUI::text(context, Test::demo_id("core.layout.table.cell", row * 8 + col), label, Test::fill_layout());
            }
            GUI::end_table_row(context);
        }
        lupanic_if_failed(GUI::end_table_layout(context, table));

        end_page_body(context, body, scroll);
    }

    void build_draw_page(GUICore::IContext* context)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.draw.scroll"), "Draw page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.draw.body"), "Draw body", Test::fill_layout());
        add_text(context, Test::demo_id("core.draw.title"), "Primitive draw commands");
        GUICore::ElementHandle canvas = context->begin_element(Test::demo_id("core.draw.canvas"), Name("Draw canvas"));
        context->set_layout(canvas, Test::fill_width_layout(360.0f));
        GUICore::DrawCommand bg;
        bg.type = GUICore::DrawCommandType::rounded_rect;
        bg.rect_reference = GUICore::DrawCommandRectReference::element;
        bg.rect = RectF(16.0f, 16.0f, -32.0f, -60.0f);
        bg.color = Float4U(0.08f, 0.11f, 0.15f, 1.0f);
        bg.radius = 6.0f;
        context->draw(bg);
        GUICore::DrawCommand rect;
        rect.type = GUICore::DrawCommandType::rounded_rect;
        rect.rect_reference = GUICore::DrawCommandRectReference::element;
        rect.rect = RectF(36.0f, 42.0f, 150.0f, 80.0f);
        rect.color = Float4U(0.20f, 0.48f, 0.82f, 1.0f);
        rect.radius = 8.0f;
        context->draw(rect);
        GUICore::DrawCommand circle;
        circle.type = GUICore::DrawCommandType::rounded_rect;
        circle.rect_reference = GUICore::DrawCommandRectReference::element;
        circle.rect = RectF(258.0f, 48.0f, 84.0f, 84.0f);
        circle.color = Float4U(0.95f, 0.64f, 0.20f, 1.0f);
        circle.radius = 42.0f;
        context->draw(circle);
        GUICore::DrawCommand line;
        line.type = GUICore::DrawCommandType::line;
        line.rect_reference = GUICore::DrawCommandRectReference::element;
        line.rect = RectF(48.0f, 180.0f, 0.0f, 0.0f);
        line.point1 = Float2U(460.0f, 260.0f);
        line.color = Float4U(0.78f, 0.95f, 0.42f, 1.0f);
        line.line_width = 4.0f;
        context->draw(line);
        GUICore::DrawCommand label;
        label.type = GUICore::DrawCommandType::text;
        label.rect_reference = GUICore::DrawCommandRectReference::element;
        label.rect = RectF(48.0f, 250.0f, 420.0f, 34.0f);
        label.color = Float4U(0.9f, 0.93f, 0.97f, 1.0f);
        label.font_size = 18.0f;
        label.text = "All shapes are GUICore draw commands compiled to VG.";
        context->draw(label);
        context->end_element();
        end_page_body(context, body, scroll);
    }

    void build_element_page(GUICore::IContext* context)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.element.scroll"), "Element page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.element.body"), "Element body", Test::fill_layout());
        add_text(context, Test::demo_id("core.element.title"), "Typeless element tree, data scopes and raw draw commands");
        context->push_data_scope(Test::demo_id("core.element.scope"));
        GUICore::id_t scoped_id = context->make_id("scoped-local-id");
        context->pop_data_scope();
        char text[128];
        snprintf(text, sizeof(text), "%llu", (unsigned long long)scoped_id);
        add_status(context, Test::demo_id("core.element.scoped"), "Scoped ID", text);

        GUICore::ElementHandle raw = context->begin_element(Test::demo_id("core.element.raw"), Name("Raw GUICore Element"));
        context->set_layout(raw, Test::fill_width_layout(82.0f));
        GUICore::Interactable interactable;
        interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
        set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
        set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
        set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
        context->set_interactable(raw, interactable);
        GUICore::DrawCommand bg;
        bg.type = GUICore::DrawCommandType::rounded_rect;
        bg.rect_reference = GUICore::DrawCommandRectReference::element;
        bg.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        bg.color = Float4U(0.12f, 0.22f, 0.34f, 1.0f);
        bg.radius = 6.0f;
        context->draw_for_element(raw, bg);
        GUICore::DrawCommand label;
        label.type = GUICore::DrawCommandType::text;
        label.rect_reference = GUICore::DrawCommandRectReference::element;
        label.rect = RectF(12.0f, 0.0f, -24.0f, 0.0f);
        label.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        label.color = Float4U(0.95f, 0.97f, 1.0f, 1.0f);
        label.font_size = 17.0f;
        label.text = "Raw element: layout + interactable + draw commands";
        context->draw_for_element(raw, label);
        context->end_element();

        GUICore::InteractionState raw_state = context->get_interaction_state(raw.id);
        snprintf(text, sizeof(text), "hovered=%s active=%s clicked=%s focused=%s",
            raw_state.hovered ? "yes" : "no", raw_state.active ? "yes" : "no",
            raw_state.clicked ? "yes" : "no", raw_state.focused ? "yes" : "no");
        add_status(context, Test::demo_id("core.element.raw.state"), "Raw state", text);

        add_text(context, Test::demo_id("core.element.propagation.title"), "Overlapped hit-test: front passes through to back.");
        GUICore::ElementHandle stack = GUI::begin_stack_layout(context, Test::demo_id("core.element.propagation.stack"),
            "Pointer propagation stack", Test::fill_width_layout(88.0f));
        GUICore::ElementHandle back = GUI::hit_box(context, Test::demo_id("core.element.propagation.back"),
            Test::fixed_layout(420.0f, 72.0f));
        GUICore::DrawCommand back_bg;
        back_bg.type = GUICore::DrawCommandType::rounded_rect;
        back_bg.rect_reference = GUICore::DrawCommandRectReference::element;
        back_bg.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        back_bg.color = context->get_interaction_state(back.id).hovered ?
            Float4U(0.12f, 0.36f, 0.30f, 1.0f) : Float4U(0.08f, 0.18f, 0.16f, 1.0f);
        back_bg.radius = 6.0f;
        context->draw_for_element(back, back_bg);
        GUICore::DrawCommand back_label;
        back_label.type = GUICore::DrawCommandType::text;
        back_label.rect_reference = GUICore::DrawCommandRectReference::element;
        back_label.rect = RectF(12.0f, 0.0f, -24.0f, 0.0f);
        back_label.rect_layout_scale = Float4U(0.0f, 0.0f, 1.0f, 1.0f);
        back_label.color = Float4U(0.9f, 0.96f, 0.92f, 1.0f);
        back_label.font_size = 16.0f;
        back_label.text = "Back target receives routed pointer events";
        context->draw_for_element(back, back_label);

        GUICore::ElementHandle front = GUI::hit_box(context, Test::demo_id("core.element.propagation.front"),
            Test::fixed_layout(300.0f, 48.0f));
        GUICore::Interactable front_interactable;
        front_interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::pass_through;
        set_flags(front_interactable.flags, GUICore::InteractableFlag::hoverable);
        set_flags(front_interactable.flags, GUICore::InteractableFlag::activatable);
        context->set_interactable(front, front_interactable);
        GUICore::DrawCommand front_bg;
        front_bg.type = GUICore::DrawCommandType::rounded_rect;
        front_bg.rect_reference = GUICore::DrawCommandRectReference::element;
        front_bg.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
        front_bg.color = Float4U(0.38f, 0.20f, 0.42f, 0.82f);
        front_bg.radius = 6.0f;
        context->draw_for_element(front, front_bg);
        GUICore::DrawCommand front_label = back_label;
        front_label.text = "Front hit box is pass-through";
        front_label.color = Float4U(0.98f, 0.90f, 1.0f, 1.0f);
        context->draw_for_element(front, front_label);
        GUICore::StackLayoutDesc propagation_desc;
        propagation_desc.alignment = Float2U(0.5f, 0.5f);
        lupanic_if_failed(GUI::end_stack_layout(context, stack, propagation_desc));
        snprintf(text, sizeof(text), "back_hover=%s front_hover=%s routed_back=%u routed_front=%u",
            context->get_interaction_state(back.id).hovered ? "yes" : "no",
            context->get_interaction_state(front.id).hovered ? "yes" : "no",
            (u32)context->get_routed_input_events(back.id).size(),
            (u32)context->get_routed_input_events(front.id).size());
        add_status(context, Test::demo_id("core.element.propagation.state"), "Propagation", text);
        end_page_body(context, body, scroll);
    }

    void build_state_style_page(GUICore::IContext* context, CoreDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.state.scroll"), "State page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.state.body"), "State body", Test::fill_layout());
        add_text(context, Test::demo_id("core.state.title"), "State store and style system");
        GUI::checkbox(context, Test::demo_id("core.state.option"), "Process option stored by demo state", &state.option, row_layout());
        GUI::checkbox(context, Test::demo_id("core.state.style.enabled"), "Use custom style scope", &state.style_enabled, row_layout());
        GUI::checkbox(context, Test::demo_id("core.state.style.unset"), "Unset child text color instead of inheriting it",
            &state.style_unset_text, row_layout());

        GUICore::id_t current_state_id = GUICore::make_state_id<GUI::CoreScrollViewState>(Test::demo_id("core.state.current"));
        GUICore::id_t next_state_id = GUICore::make_state_id<GUI::CoreScrollViewState>(Test::demo_id("core.state.next"));
        GUICore::id_t process_state_id = GUICore::make_state_id<GUI::CoreScrollViewState>(Test::demo_id("core.state.process"));
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.state.set.current"),
            "Set current_frame state", row_layout())))
        {
            Ref<GUI::CoreScrollViewState> state_obj = new_object<GUI::CoreScrollViewState>();
            state_obj->scroll = Float2U(1.0f, 1.0f);
            lupanic_if_failed(context->set_state(current_state_id, state_obj.object(), GUICore::StateLifetime::current_frame));
        }
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.state.set.next"),
            "Set next_frame state", row_layout())))
        {
            Ref<GUI::CoreScrollViewState> state_obj = new_object<GUI::CoreScrollViewState>();
            state_obj->scroll = Float2U(2.0f, 2.0f);
            lupanic_if_failed(context->set_state(next_state_id, state_obj.object(), GUICore::StateLifetime::next_frame));
        }
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.state.set.process"),
            "Set process state", row_layout())))
        {
            Ref<GUI::CoreScrollViewState> state_obj = new_object<GUI::CoreScrollViewState>();
            state_obj->scroll = Float2U(3.0f, 3.0f);
            lupanic_if_failed(context->set_state(process_state_id, state_obj.object(), GUICore::StateLifetime::process));
        }
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.state.clear.process"),
            "Clear process state", row_layout())))
        {
            context->clear_state(process_state_id);
        }
        char text[256];
        snprintf(text, sizeof(text), "current=%s next=%s process=%s",
            context->get_state(current_state_id) ? "present" : "missing",
            context->get_state(next_state_id) ? "present" : "missing",
            context->get_state(process_state_id) ? "present" : "missing");
        add_status(context, Test::demo_id("core.state.lifetime"), "State lifetimes", text);

        context->define_style(Name("core_demo_style"));
        context->define_style(Name("core_demo_child_style"), Name("core_demo_style"));
        GUICore::StyleEntrySchema schema;
        schema.owner = Name("core.demo");
        schema.entry = Name("core.demo.accent");
        schema.type = GUICore::StyleValueType::f32x4;
        schema.default_value = GUICore::style_f32x4(Float4U(0.2f, 0.4f, 0.7f, 1.0f));
        schema.category = "Demo";
        schema.description = "Schema entry registered by GUICoreTest.";
        context->register_style_entry_schema(schema);
        context->set_style_value(Name("core_demo_style"), Name("gui.editor.button.background"),
            GUICore::style_f32x4(Float4U(0.20f, 0.38f, 0.22f, 1.0f)));
        context->set_style_value(Name("core_demo_style"), Name("gui.editor.text.color"),
            GUICore::style_f32x4(Float4U(0.90f, 1.0f, 0.84f, 1.0f)));
        context->set_style_value(Name("core_demo_child_style"), Name("gui.editor.button.background.hovered"),
            GUICore::style_f32x4(Float4U(0.30f, 0.48f, 0.32f, 1.0f)));
        context->set_style_value(Name("core_demo_child_style"), Name("core.demo.accent"),
            GUICore::style_f32x4(Float4U(0.80f, 0.55f, 0.16f, 1.0f)));
        if(state.style_unset_text)
        {
            context->unset_style_entry(Name("core_demo_child_style"), Name("gui.editor.text.color"));
        }
        else
        {
            context->inherit_style_entry(Name("core_demo_child_style"), Name("gui.editor.text.color"));
        }
        if(state.style_enabled)
        {
            context->push_style(Name("core_demo_child_style"));
        }
        GUI::text_button(context, Test::demo_id("core.state.styled.button"), "Button bound to inherited core_demo_child_style", Test::fill_width_layout(36.0f));
        add_text(context, Test::demo_id("core.state.styled.text"), "This text inherits style stack binding on creation.");
        if(state.style_enabled)
        {
            context->pop_style();
        }

        GUICore::PerformanceCounters counters = context->get_performance_counters();
        snprintf(text, sizeof(text), "states=%u styles=%u schemas=%u", counters.state_count, counters.style_count, counters.style_schema_count);
        add_status(context, Test::demo_id("core.state.counters"), "Counters", text);
        GUICore::StyleValue style_value = context->get_style_value(Name("core_demo_child_style"),
            Name("gui.editor.button.background"), GUICore::style_f32x4(Float4U(0.0f)));
        snprintf(text, sizeof(text), "type=%u rgba=(%.2f, %.2f, %.2f, %.2f)", (u32)style_value.type,
            style_value.number.x, style_value.number.y, style_value.number.z, style_value.number.w);
        add_status(context, Test::demo_id("core.state.resolved.style"), "Inherited background", text);
        GUICore::StyleValue text_style_value = context->get_style_value(Name("core_demo_child_style"),
            Name("gui.editor.text.color"), GUICore::style_f32x4(Float4U(0.5f, 0.5f, 0.5f, 1.0f)));
        snprintf(text, sizeof(text), "text rgba=(%.2f, %.2f, %.2f, %.2f) schemas=%u",
            text_style_value.number.x, text_style_value.number.y, text_style_value.number.z, text_style_value.number.w,
            (u32)context->get_style_entry_schemas().size());
        add_status(context, Test::demo_id("core.state.resolved.text"), "Inherited/unset text", text);
        end_page_body(context, body, scroll);
    }

    void build_debug_page(GUICore::IContext* context, CoreDemoState& state)
    {
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, Test::demo_id("core.debug.scroll"), "Debug page", Test::fill_layout());
        GUICore::ElementHandle body = GUI::begin_v_layout(context, Test::demo_id("core.debug.body"), "Debug body", Test::fill_layout());
        add_text(context, Test::demo_id("core.debug.title"), "Debug and instrumentation");
        context->log_debug_issue(GUICore::DebugIssueSeverity::info, Name("core.test"),
            "Sample issue logged by GUICoreTest.", Test::demo_id("core.debug.title"));
        context->log_debug_pass(GUICore::DebugPassKind::custom, Name("core.test.debug_page"), Name("sample"),
            Test::demo_id("core.debug.title"), "Sample debug pass logged by GUICoreTest.", 0.01);
        GUICore::PerformanceCounters counters = context->get_performance_counters();
        char text[192];
        snprintf(text, sizeof(text), "frame=%u layers=%u elements=%u interactables=%u draw_commands=%u",
            counters.frame_generation, counters.layer_count, counters.element_count,
            counters.interactable_count, counters.draw_command_count);
        add_status(context, Test::demo_id("core.debug.frame"), "Previous frame", text);
        snprintf(text, sizeof(text), "input=%.3f ms draw=%.3f ms state_gc=%.3f ms debug=%.3f ms",
            counters.input_route_ms, counters.draw_compile_ms, counters.state_gc_ms, counters.debug_dump_ms);
        add_status(context, Test::demo_id("core.debug.times"), "Timings", text);
        snprintf(text, sizeof(text), "issues=%u passes=%u", counters.debug_issue_count, counters.debug_pass_count);
        add_status(context, Test::demo_id("core.debug.logged"), "Logged diagnostics", text);
        GUI::checkbox(context, Test::demo_id("core.debug.timeline.capture"), "Capture debug timeline", &state.capture_debug_timeline, row_layout());
        if(state.capture_debug_timeline)
        {
            GUICore::push_debug_frame(state.timeline, context->dump_debug_info(), 16);
        }
        GUICore::ElementHandle timeline_buttons = GUI::begin_h_layout(context, Test::demo_id("core.debug.timeline.buttons"),
            "Timeline buttons", Test::fill_width_layout(34.0f));
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.debug.timeline.prev"), "Prev",
            Test::fixed_layout(96.0f, 30.0f))))
        {
            GUICore::step_debug_frame(state.timeline, -1);
        }
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.debug.timeline.next"), "Next",
            Test::fixed_layout(96.0f, 30.0f))))
        {
            GUICore::step_debug_frame(state.timeline, 1);
        }
        if(GUI::is_item_clicked(context, GUI::text_button(context, Test::demo_id("core.debug.timeline.clear"), "Clear",
            Test::fixed_layout(96.0f, 30.0f))))
        {
            GUICore::clear_debug_frames(state.timeline);
        }
        GUICore::LinearLayoutDesc timeline_button_desc;
        timeline_button_desc.axis = GUICore::LayoutAxis::x;
        timeline_button_desc.gap = 8.0f;
        lupanic_if_failed(GUI::end_h_layout(context, timeline_buttons, timeline_button_desc));
        const GUICore::DebugInfo* frame = GUICore::current_debug_frame(state.timeline);
        snprintf(text, sizeof(text), "frames=%u cursor=%u current_elements=%u",
            (u32)state.timeline.frames.size(), (u32)state.timeline.cursor,
            frame ? frame->counters.element_count : 0);
        add_status(context, Test::demo_id("core.debug.timeline.status"), "Timeline", text);
        add_text(context, Test::demo_id("core.debug.note"), "Dumped frames can be visualized by higher-level debug panels or external tools.");
        end_page_body(context, body, scroll);
    }

    void build_demo(GUICore::IContext* context, const GUICore::ElementHandle& root, const Float2U& surface_size, void* userdata)
    {
        CoreDemoState& state = *(CoreDemoState*)userdata;
        (void)root;
        GUI::draw_rect(context, Test::demo_id("core.background"), RectF(0.0f, 0.0f, surface_size.x, surface_size.y),
            Float4U(0.045f, 0.055f, 0.070f, 1.0f));
        GUI::draw_text(context, Test::demo_id("core.title"), RectF(14.0f, 8.0f, surface_size.x - 28.0f, 28.0f),
            "Luna GUICore Interactive Test", Float4U(0.92f, 0.94f, 0.96f, 1.0f), 18.0f);

        GUICore::ElementHandle tabs = GUI::begin_tab_bar(context, Test::demo_id("core.tabs"), "Core Tabs");
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.input"), "Input"))
        {
            build_input_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.layout"), "Layout"))
        {
            build_layout_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.draw"), "Draw"))
        {
            build_draw_page(context);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.element"), "Element"))
        {
            build_element_page(context);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.state"), "State/Style"))
        {
            build_state_style_page(context, state);
            GUI::end_tab_item(context);
        }
        if(GUI::begin_tab_item(context, Test::demo_id("core.tab.debug"), "Debug"))
        {
            build_debug_page(context, state);
            GUI::end_tab_item(context);
        }
        lupanic_if_failed(GUI::end_tab_bar(context, tabs, RectF(12.0f, 44.0f,
            max(surface_size.x - 24.0f, 1.0f), max(surface_size.y - 56.0f, 1.0f))));
    }
}

int luna_main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;
    if(!Luna::init())
    {
        return -1;
    }
    i32 exit_code = 0;
    {
        CoreDemoState state;
        Test::InteractiveGUIDemoDesc desc;
        desc.title = "Luna GUICore Interactive Test";
        desc.build = build_demo;
        desc.userdata = &state;
        RV r = Test::run_interactive_gui_demo(desc);
        if(failed(r))
        {
            log_error("GUICoreTest", "%s", explain(r.errcode()));
            exit_code = -1;
        }
    }
    Luna::close();
    return exit_code;
}
