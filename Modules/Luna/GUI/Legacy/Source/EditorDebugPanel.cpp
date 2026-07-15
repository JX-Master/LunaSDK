/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDebugPanel.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/Legacy/EditorState.hpp>
#include <Luna/GUI/Legacy/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            struct InspectedLayer
            {
                GUICore::id_t id = 0;
                u32 root = GUICore::INVALID_ELEMENT;
                Float2U screen_position = Float2U(0.0f);
                Name debug_name;
            };

            struct InspectedElement
            {
                GUICore::Element element;
                u32 index = GUICore::INVALID_ELEMENT;
                GUICore::LayoutCallbackConfig layout_callbacks;
                GUICore::NavigationConfig navigation;
                GUICore::ElementHitTestConfig hit_test;
                GUICore::DrawConfig draw;
                GUICore::InteractionState interaction;
                u32 draw_command_count = 0;
            };

            struct InspectionData
            {
                Vector<InspectedLayer> layers;
                Vector<InspectedElement> elements;
                u32 draw_command_count = 0;
            };

            GUICore::LayoutConfig fixed_height(f32 height)
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
                return layout;
            }

            GUICore::LayoutConfig fixed_width(f32 width)
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = width;
                layout.height.kind = GUICore::SizeKind::percent;
                layout.height.value = 1.0f;
                return layout;
            }

            GUICore::LayoutConfig fill_layout()
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUICore::SizeKind::percent;
                layout.height.value = 1.0f;
                layout.flex_grow = 1.0f;
                return layout;
            }

            GUICore::LayoutConfig content_height(f32 height)
            {
                GUICore::LayoutConfig layout;
                layout.width.kind = GUICore::SizeKind::percent;
                layout.width.value = 1.0f;
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = max(height, 1.0f);
                return layout;
            }

            String format_id(GUICore::id_t id)
            {
                String ret;
                strprintf(ret, "%016llX", (unsigned long long)id);
                return ret;
            }

            String format_rect(const RectF& rect)
            {
                String ret;
                strprintf(ret, "(%.1f, %.1f, %.1f, %.1f)", rect.offset_x, rect.offset_y, rect.width, rect.height);
                return ret;
            }

            String format_size_value(const GUICore::SizeValue& value)
            {
                const c8* kind = "fit";
                if(value.kind == GUICore::SizeKind::fixed)
                {
                    kind = "fixed";
                }
                else if(value.kind == GUICore::SizeKind::percent)
                {
                    kind = "percent";
                }
                String ret;
                strprintf(ret, "%s %.3f min %.1f max %.1f", kind, value.value, value.min, value.max);
                return ret;
            }

            String format_style_value(const GUICore::StyleValue& value)
            {
                String ret;
                switch(value.type)
                {
                case GUICore::StyleValueType::f32:
                    strprintf(ret, "%.3f", value.number.x);
                    break;
                case GUICore::StyleValueType::f32x2:
                    strprintf(ret, "(%.3f, %.3f)", value.number.x, value.number.y);
                    break;
                case GUICore::StyleValueType::f32x3:
                    strprintf(ret, "(%.3f, %.3f, %.3f)", value.number.x, value.number.y, value.number.z);
                    break;
                case GUICore::StyleValueType::f32x4:
                    strprintf(ret, "(%.3f, %.3f, %.3f, %.3f)", value.number.x, value.number.y, value.number.z,
                        value.number.w);
                    break;
                case GUICore::StyleValueType::name:
                    ret = value.name.c_str();
                    break;
                default:
                    ret = "none";
                    break;
                }
                return ret;
            }

            const c8* format_pointer_hit_behavior(GUICore::PointerHitBehavior behavior)
            {
                switch(behavior)
                {
                case GUICore::PointerHitBehavior::pass_through:
                    return "pass_through";
                case GUICore::PointerHitBehavior::target:
                    return "target";
                case GUICore::PointerHitBehavior::block:
                    return "block";
                default:
                    return "none";
                }
            }

            InspectionData inspect_context(GUICore::IContext* context)
            {
                InspectionData ret;
                for(const GUICore::Layer& layer : context->get_layers())
                {
                    InspectedLayer inspected;
                    inspected.id = layer.id;
                    inspected.root = layer.root;
                    inspected.screen_position = layer.screen_position;
                    inspected.debug_name = layer.debug_name;
                    ret.layers.push_back(move(inspected));
                }

                Span<const GUICore::Element> elements = context->get_elements();
                ret.elements.reserve(elements.size());
                for(u32 i = 0; i < elements.size(); ++i)
                {
                    const GUICore::Element& element = elements[i];
                    GUICore::ElementHandle handle { element.id, i, context->generation() };
                    InspectedElement inspected;
                    inspected.element = element;
                    inspected.index = i;
                    inspected.layout_callbacks = context->get_layout_callback_config(handle);
                    inspected.navigation = context->get_navigation_config(handle);
                    inspected.hit_test = context->get_hit_test_config(handle);
                    inspected.draw = context->get_draw_config(handle);
                    inspected.interaction = context->get_interaction_state(element.id);
                    ret.elements.push_back(move(inspected));
                }

                Span<const GUICore::DrawCommand> commands = context->get_draw_commands();
                ret.draw_command_count = (u32)commands.size();
                for(const GUICore::DrawCommand& command : commands)
                {
                    if(command.element < ret.elements.size())
                    {
                        ++ret.elements[command.element].draw_command_count;
                    }
                }
                return ret;
            }

            const InspectedElement* find_element(const InspectionData& info, GUICore::id_t id)
            {
                for(const InspectedElement& element : info.elements)
                {
                    if(element.element.id == id)
                    {
                        return &element;
                    }
                }
                return nullptr;
            }

            Ref<CoreDebugPanelState> debug_panel_state(GUICore::IContext* context, GUICore::id_t id)
            {
                GUICore::id_t state_id = GUICore::make_state_id<CoreDebugPanelState>(id);
                Ref<CoreDebugPanelState> state;
                if(object_t state_object = context->get_state(state_id))
                {
                    object_retain(state_object);
                    state.attach(state_object);
                }
                else
                {
                    state = new_object<CoreDebugPanelState>();
                }
                lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
                return state;
            }

            void debug_text(GUICore::IContext* context, u32& row, const c8* label, const String& value)
            {
                String text_value;
                text_value.append(label);
                text_value.append(": ");
                text_value.append(value);
                GUI::text(context, context->make_id((GUICore::id_t)row++), text_value.c_str(), fixed_height(22.0f));
            }

            void debug_bool(GUICore::IContext* context, u32& row, const c8* label, bool value)
            {
                debug_text(context, row, label, String(value ? "true" : "false"));
            }

            void debug_section(GUICore::IContext* context, u32& row, const c8* title)
            {
                GUI::text(context, context->make_id((GUICore::id_t)row++), title, fixed_height(28.0f));
            }

            void draw_element_details(GUICore::IContext* context, const InspectionData& info,
                const InspectedElement* inspected)
            {
                u32 row = 400000;
                if(!inspected)
                {
                    GUI::text(context, context->make_id((GUICore::id_t)row++), "Select an element to inspect.",
                        fixed_height(24.0f));
                    return;
                }

                const GUICore::Element& element = inspected->element;
                debug_section(context, row, "Identity");
                debug_text(context, row, "ID", format_id(element.id));
                debug_text(context, row, "Name", element.debug_name.c_str());
                String index;
                strprintf(index, "%u", inspected->index);
                debug_text(context, row, "Index", index);
                String layer;
                if(element.layer < info.layers.size())
                {
                    strprintf(layer, "%u / %s", element.layer, format_id(info.layers[element.layer].id).c_str());
                }
                else
                {
                    strprintf(layer, "%u / invalid", element.layer);
                }
                debug_text(context, row, "Layer", layer);

                debug_section(context, row, "Layout");
                debug_text(context, row, "Rect", format_rect(element.layout_result.rect));
                debug_text(context, row, "Clip", format_rect(element.layout_result.clip_rect));
                debug_text(context, row, "Width", format_size_value(element.layout.width));
                debug_text(context, row, "Height", format_size_value(element.layout.height));
                debug_text(context, row, "Algorithm", inspected->layout_callbacks.algorithm.empty() ? String("none") :
                    String(inspected->layout_callbacks.algorithm.c_str()));
                debug_bool(context, row, "Measure Callback", inspected->layout_callbacks.measure_callback != nullptr);
                debug_bool(context, row, "Arrange Callback", inspected->layout_callbacks.callback != nullptr);
                debug_bool(context, row, "Finalize Callback", inspected->layout_callbacks.finalize_callback != nullptr);

                debug_section(context, row, "Input");
                debug_text(context, row, "Pointer Hit", format_pointer_hit_behavior(element.interactable.pointer_hit_behavior));
                debug_bool(context, row, "Hit Test Callback", inspected->hit_test.callback != nullptr);
                debug_bool(context, row, "Hoverable", GUICore::has_flags(element.interactable, GUICore::InteractableFlag::hoverable));
                debug_bool(context, row, "Activatable", GUICore::has_flags(element.interactable, GUICore::InteractableFlag::activatable));
                debug_bool(context, row, "Focusable", GUICore::has_flags(element.interactable, GUICore::InteractableFlag::focusable));
                debug_bool(context, row, "Disabled", GUICore::has_flags(element.interactable, GUICore::InteractableFlag::disabled));
                debug_bool(context, row, "Hovered", inspected->interaction.hovered);
                debug_bool(context, row, "Active", inspected->interaction.active);
                debug_bool(context, row, "Focused", inspected->interaction.focused);
                debug_bool(context, row, "Clicked", inspected->interaction.clicked);
                debug_bool(context, row, "Double Clicked", inspected->interaction.double_clicked);

                debug_section(context, row, "Draw");
                String draw_count;
                strprintf(draw_count, "%u", inspected->draw_command_count);
                debug_text(context, row, "Command Count", draw_count);
                debug_text(context, row, "Callback", inspected->draw.name.empty() ? String("none") :
                    String(inspected->draw.name.c_str()));
                String total_commands;
                strprintf(total_commands, "%u", info.draw_command_count);
                debug_text(context, row, "Total Commands", total_commands);

                debug_section(context, row, "Resolved Style");
                debug_text(context, row, "Bound Style", element.style.empty() ? String("none") : String(element.style.c_str()));
                for(const GUICore::StyleEntrySchema& schema : context->get_style_entry_schemas())
                {
                    String label;
                    label.append(schema.owner.empty() ? "style" : schema.owner.c_str());
                    label.append(".");
                    label.append(schema.entry.c_str());
                    debug_text(context, row, label.c_str(), format_style_value(
                        context->get_style_value(element.style, schema.entry, schema.default_value)));
                }
            }
        }

        LUNA_GUI_API GUICore::ElementHandle show_debug_panel(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            InspectionData info = inspect_context(context);
            Ref<CoreDebugPanelState> state = debug_panel_state(context, id);
            if(!state->selected_element && !info.elements.empty())
            {
                state->selected_element = info.elements.front().element.id;
            }
            if(state->selected_element && !find_element(info, state->selected_element))
            {
                state->selected_element = info.elements.empty() ? 0 : info.elements.front().element.id;
            }

            GUICore::ElementHandle root = begin_v_layout(context, id, "GUI Core Debug Panel", layout);
            context->push_data_scope(id);
            text(context, context->make_id("title"), "GUI Core Debug", fixed_height(28.0f));

            GUICore::id_t tree_viewport_id = context->make_id("tree_viewport");
            GUICore::id_t tree_content_id = context->make_id("tree_content");
            GUICore::id_t details_viewport_id = context->make_id("details_viewport");
            GUICore::id_t details_content_id = context->make_id("details_content");
            f32 tree_content_height = max((f32)info.elements.size() * 24.0f, 24.0f);
            const InspectedElement* selected = find_element(info, state->selected_element);
            f32 details_content_height = selected ?
                540.0f + (f32)context->get_style_entry_schemas().size() * 22.0f : 48.0f;

            GUICore::ElementHandle body = begin_h_layout(context, context->make_id("body"), "Debug Body", fill_layout());
            GUICore::ElementHandle tree_viewport = begin_scroll_view(context, tree_viewport_id, "Element Tree Viewport",
                fixed_width(420.0f));
            GUICore::ElementHandle tree = begin_v_layout(context, tree_content_id, "Element Tree",
                content_height(tree_content_height));
            u32 row_index = 0;
            for(const InspectedElement& inspected : info.elements)
            {
                const GUICore::Element& element = inspected.element;
                String label;
                for(u32 i = 0; i < element.depth; ++i)
                {
                    label.append("  ");
                }
                label.append(element.debug_name.empty() ? "element" : element.debug_name.c_str());
                label.append("  #");
                label.append(format_id(element.id));
                if(inspected.interaction.hovered)
                {
                    label.append("  <hover>");
                }
                GUICore::ElementHandle row = selectable(context,
                    context->make_id((GUICore::id_t)(100000 + row_index)), label.c_str(),
                    state->selected_element == element.id || inspected.interaction.hovered, fixed_height(24.0f));
                if(is_item_clicked(context, row))
                {
                    state->selected_element = element.id;
                }
                ++row_index;
            }
            lupanic_if_failed(end_v_layout(context, tree, GUICore::FlexLayoutDesc()));
            lupanic_if_failed(end_scroll_view(context, tree_viewport));

            GUICore::ElementHandle details_viewport = begin_scroll_view(context, details_viewport_id,
                "Element Details Viewport", fill_layout());
            GUICore::ElementHandle details = begin_v_layout(context, details_content_id, "Element Details",
                content_height(details_content_height));
            draw_element_details(context, info, selected);
            lupanic_if_failed(end_v_layout(context, details, GUICore::FlexLayoutDesc()));
            lupanic_if_failed(end_scroll_view(context, details_viewport));

            GUICore::FlexLayoutDesc body_layout;
            body_layout.main_axis_gap = 12.0f;
            lupanic_if_failed(end_h_layout(context, body, body_layout));
            context->pop_data_scope();

            GUICore::FlexLayoutDesc root_layout;
            root_layout.main_axis_gap = 8.0f;
            lupanic_if_failed(end_v_layout(context, root, root_layout));
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDebugPanelState>(id), state.object(),
                GUICore::StateLifetime::next_frame));
            return root;
        }
    }
}
