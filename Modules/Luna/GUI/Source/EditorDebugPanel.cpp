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
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            GUICore::LayoutInput fixed_height(f32 height)
            {
                GUICore::LayoutInput layout;
                layout.width.kind = GUICore::SizeKind::expand;
                layout.height.kind = GUICore::SizeKind::pixels;
                layout.height.value = height;
                return layout;
            }

            GUICore::LayoutInput fixed_width(f32 width)
            {
                GUICore::LayoutInput layout;
                layout.width.kind = GUICore::SizeKind::pixels;
                layout.width.value = width;
                layout.height.kind = GUICore::SizeKind::expand;
                return layout;
            }

            GUICore::LayoutInput fill_layout()
            {
                GUICore::LayoutInput layout;
                layout.width.kind = GUICore::SizeKind::expand;
                layout.height.kind = GUICore::SizeKind::expand;
                return layout;
            }

            GUICore::LayoutInput content_height(f32 height)
            {
                GUICore::LayoutInput layout;
                layout.width.kind = GUICore::SizeKind::expand;
                layout.height.kind = GUICore::SizeKind::pixels;
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
                switch(value.kind)
                {
                case GUICore::SizeKind::pixels:
                    kind = "pixels";
                    break;
                case GUICore::SizeKind::percent:
                    kind = "percent";
                    break;
                case GUICore::SizeKind::ratio:
                    kind = "ratio";
                    break;
                case GUICore::SizeKind::expand:
                    kind = "expand";
                    break;
                default:
                    break;
                }
                String ret;
                strprintf(ret, "%s %.3f min %.1f max %.1f", kind, value.value, value.min, value.max);
                return ret;
            }

            String format_layer(const GUICore::DebugInfo& info, const GUICore::DebugElementInfo& element)
            {
                String ret;
                if(element.layer < info.layers.size())
                {
                    strprintf(ret, "%u / %s", element.layer, format_id(info.layers[element.layer].id).c_str());
                }
                else
                {
                    strprintf(ret, "%u / invalid", element.layer);
                }
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
                    strprintf(ret, "(%.3f, %.3f, %.3f, %.3f)", value.number.x, value.number.y, value.number.z, value.number.w);
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

            const c8* format_issue_severity(GUICore::DebugIssueSeverity severity)
            {
                switch(severity)
                {
                case GUICore::DebugIssueSeverity::warning:
                    return "warning";
                case GUICore::DebugIssueSeverity::error:
                    return "error";
                default:
                    return "info";
                }
            }

            const c8* format_pass_kind(GUICore::DebugPassKind kind)
            {
                switch(kind)
                {
                case GUICore::DebugPassKind::frame:
                    return "frame";
                case GUICore::DebugPassKind::layout:
                    return "layout";
                case GUICore::DebugPassKind::input:
                    return "input";
                case GUICore::DebugPassKind::render:
                    return "render";
                case GUICore::DebugPassKind::state:
                    return "state";
                default:
                    return "custom";
                }
            }

            const GUICore::DebugElementInfo* find_debug_element(const GUICore::DebugInfo& info, GUICore::id_t id)
            {
                for(const GUICore::DebugElementInfo& element : info.elements)
                {
                    if(element.id == id)
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
                if(object_t state_obj = context->get_state(state_id))
                {
                    object_retain(state_obj);
                    state.attach(state_obj);
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
                String text;
                text.reserve(strlen(label) + value.size() + 4);
                text.append(label);
                text.append(": ");
                text.append(value);
                GUI::text(context, context->make_id((GUICore::id_t)row++), text.c_str(), fixed_height(22.0f));
            }

            void debug_text_bool(GUICore::IContext* context, u32& row, const c8* label, bool value)
            {
                debug_text(context, row, label, String(value ? "true" : "false"));
            }

            void debug_section(GUICore::IContext* context, u32& row, const c8* title)
            {
                GUI::text(context, context->make_id((GUICore::id_t)row++), title, fixed_height(28.0f));
            }

            void draw_element_details(GUICore::IContext* context, const GUICore::DebugInfo& info,
                const GUICore::DebugElementInfo* element)
            {
                u32 row = 400000;
                if(!element)
                {
                    GUI::text(context, context->make_id((GUICore::id_t)row++), "Select an element to inspect.",
                        fixed_height(24.0f));
                    return;
                }

                debug_section(context, row, "Identity");
                debug_text(context, row, "ID", format_id(element->id));
                debug_text(context, row, "Layer", format_layer(info, *element));
                debug_text(context, row, "Index", [&]() { String s; strprintf(s, "%u", element->index); return s; }());
                debug_text(context, row, "Parent", element->parent == GUICore::INVALID_ELEMENT ?
                    String("none") : [&]() { String s; strprintf(s, "%u", element->parent); return s; }());
                debug_text(context, row, "Name", element->debug_name.c_str());

                debug_section(context, row, "Layout");
                debug_text(context, row, "Rect", format_rect(element->rect));
                debug_text(context, row, "Clip", format_rect(element->clip_rect));
                debug_text(context, row, "Width", format_size_value(element->layout.width));
                debug_text(context, row, "Height", format_size_value(element->layout.height));

                debug_section(context, row, "Input");
                debug_text_bool(context, row, "Hit Test", element->hit_test);
                debug_text_bool(context, row, "Blocks Pointer", element->blocks_pointer_input);
                debug_text_bool(context, row, "Hoverable", element->hoverable);
                debug_text_bool(context, row, "Activatable", element->activatable);
                debug_text_bool(context, row, "Focusable", element->focusable);
                debug_text_bool(context, row, "Disabled", element->disabled);
                debug_text_bool(context, row, "Hovered", element->hovered);
                debug_text_bool(context, row, "Active", element->active);
                debug_text_bool(context, row, "Captured", element->captured);
                debug_text_bool(context, row, "Focused", element->focused);
                debug_text_bool(context, row, "Clicked", element->clicked);
                debug_text_bool(context, row, "Double Clicked", element->double_clicked);
                debug_text_bool(context, row, "Subtree Hovered", element->subtree_hovered);
                debug_text_bool(context, row, "Subtree Clicked", element->subtree_clicked);
                debug_text_bool(context, row, "Subtree Double Clicked", element->subtree_double_clicked);

                debug_section(context, row, "Draw");
                debug_text(context, row, "First Command", element->first_draw_command == U32_MAX ?
                    String("none") : [&]() { String s; strprintf(s, "%u", element->first_draw_command); return s; }());
                debug_text(context, row, "Command Count", [&]() { String s; strprintf(s, "%u", element->draw_command_count); return s; }());
                debug_text(context, row, "Total Elements", [&]() { String s; strprintf(s, "%u", (u32)info.elements.size()); return s; }());
                debug_text(context, row, "Total Commands", [&]() { String s; strprintf(s, "%u", (u32)info.draw_commands.size()); return s; }());

                debug_section(context, row, "Resolved Style");
                debug_text(context, row, "Bound Style", element->style.empty() ? String("none") : String(element->style.c_str()));
                for(const GUICore::DebugResolvedStyleEntryInfo& style : element->resolved_style)
                {
                    String label;
                    label.append(style.owner.empty() ? "style" : style.owner.c_str());
                    label.append(".");
                    label.append(style.entry.empty() ? "entry" : style.entry.c_str());
                    if(style.defaulted)
                    {
                        label.append(" (default)");
                    }
                    debug_text(context, row, label.c_str(), format_style_value(style.value));
                }
            }
        }

        LUNA_GUI_API GUICore::ElementHandle show_debug_info(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::DebugInfo& info, const GUICore::LayoutInput& layout)
        {
            luassert(context && id);
            Ref<CoreDebugPanelState> state = debug_panel_state(context, id);
            if(!state->selected_element && !info.elements.empty())
            {
                state->selected_element = info.elements.front().id;
            }
            if(state->selected_element && !find_debug_element(info, state->selected_element))
            {
                state->selected_element = info.elements.empty() ? 0 : info.elements.front().id;
            }

            GUICore::ElementHandle root = begin_v_layout(context, id, "GUI Core Debug Panel", layout);
            context->push_data_scope(id);
            text(context, context->make_id("title"), "GUI Core Debug", fixed_height(28.0f));
            if(!info.issues.empty())
            {
                text(context, context->make_id("issues_title"), "Issues", fixed_height(24.0f));
                for(usize i = 0; i < info.issues.size(); ++i)
                {
                    const GUICore::DebugIssueInfo& issue = info.issues[i];
                    String issue_text;
                    if(issue.element)
                    {
                        strprintf(issue_text, "[%s] %s element %s: %s",
                            format_issue_severity(issue.severity),
                            issue.category.empty() ? "core" : issue.category.c_str(),
                            format_id(issue.element).c_str(),
                            issue.message.c_str());
                    }
                    else
                    {
                        strprintf(issue_text, "[%s] %s: %s",
                            format_issue_severity(issue.severity),
                            issue.category.empty() ? "core" : issue.category.c_str(),
                            issue.message.c_str());
                    }
                    text(context, context->make_id((GUICore::id_t)(900000 + i)), issue_text.c_str(), fixed_height(22.0f));
                }
            }
            if(!info.passes.empty())
            {
                text(context, context->make_id("passes_title"), "Passes", fixed_height(24.0f));
                for(usize i = 0; i < info.passes.size(); ++i)
                {
                    const GUICore::DebugPassInfo& pass = info.passes[i];
                    String pass_text;
                    if(pass.element)
                    {
                        strprintf(pass_text, "[%s] %s reason %s element %s %.3f ms",
                            format_pass_kind(pass.kind),
                            pass.name.empty() ? "pass" : pass.name.c_str(),
                            pass.reason.empty() ? "unknown" : pass.reason.c_str(),
                            format_id(pass.element).c_str(),
                            pass.duration_ms);
                    }
                    else
                    {
                        strprintf(pass_text, "[%s] %s reason %s %.3f ms",
                            format_pass_kind(pass.kind),
                            pass.name.empty() ? "pass" : pass.name.c_str(),
                            pass.reason.empty() ? "unknown" : pass.reason.c_str(),
                            pass.duration_ms);
                    }
                    text(context, context->make_id((GUICore::id_t)(910000 + i)), pass_text.c_str(), fixed_height(22.0f));
                }
            }

            GUICore::id_t tree_viewport_id = context->make_id("tree_viewport");
            GUICore::id_t tree_content_id = context->make_id("tree_content");
            GUICore::id_t details_viewport_id = context->make_id("details_viewport");
            GUICore::id_t details_content_id = context->make_id("details_content");
            f32 tree_content_height = max((f32)info.elements.size() * 24.0f, 24.0f);
            const GUICore::DebugElementInfo* selected_element = find_debug_element(info, state->selected_element);
            f32 details_content_height = selected_element ?
                560.0f + (f32)selected_element->resolved_style.size() * 22.0f :
                48.0f;

            GUICore::ElementHandle body = begin_h_layout(context, context->make_id("body"), "Debug Body", fill_layout());
            GUICore::ElementHandle tree_viewport = begin_scroll_view(context, tree_viewport_id, "Element Tree Viewport",
                fixed_width(420.0f));
            GUICore::ElementHandle tree = begin_v_layout(context, tree_content_id, "Element Tree", content_height(tree_content_height));
            u32 row_index = 0;
            for(const GUICore::DebugElementInfo& element : info.elements)
            {
                String label;
                for(u32 i = 0; i < element.depth; ++i)
                {
                    label.append("  ");
                }
                label.append(element.debug_name.empty() ? "element" : element.debug_name.c_str());
                label.append("  #");
                label.append(format_id(element.id));
                if(info.hovered_element == element.id)
                {
                    label.append("  <hover>");
                }
                GUICore::ElementHandle row = selectable(context, context->make_id((GUICore::id_t)(100000 + row_index)),
                    label.c_str(), state->selected_element == element.id || info.hovered_element == element.id, fixed_height(24.0f));
                if(is_item_clicked(context, row))
                {
                    state->selected_element = element.id;
                }
                ++row_index;
            }
            lupanic_if_failed(end_v_layout(context, tree, GUICore::LinearLayoutDesc()));
            lupanic_if_failed(end_scroll_view(context, tree_viewport));

            GUICore::ElementHandle details_viewport = begin_scroll_view(context, details_viewport_id, "Element Details Viewport",
                fill_layout());
            GUICore::ElementHandle details = begin_v_layout(context, details_content_id, "Element Details",
                content_height(details_content_height));
            draw_element_details(context, info, selected_element);
            lupanic_if_failed(end_v_layout(context, details, GUICore::LinearLayoutDesc()));
            lupanic_if_failed(end_scroll_view(context, details_viewport));

            GUICore::LinearLayoutDesc body_layout;
            body_layout.gap = 12.0f;
            lupanic_if_failed(end_h_layout(context, body, body_layout));
            context->pop_data_scope();

            GUICore::LinearLayoutDesc root_layout;
            root_layout.gap = 8.0f;
            lupanic_if_failed(end_v_layout(context, root, root_layout));
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDebugPanelState>(id), state.object(),
                GUICore::StateLifetime::next_frame));
            return root;
        }
    }
}
