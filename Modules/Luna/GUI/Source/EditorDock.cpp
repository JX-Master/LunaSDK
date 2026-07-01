/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDock.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV set_editor_flex_layout_config(GUICore::IContext* context,
            const GUICore::ElementHandle& layout, const GUICore::FlexLayoutDesc& desc);

        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static RectF intersect_rect_local(const RectF& a, const RectF& b)
        {
            f32 min_x = max(a.offset_x, b.offset_x);
            f32 min_y = max(a.offset_y, b.offset_y);
            f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
            f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
            return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = horizontal_alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static Ref<CoreDockBuildState> dock_build_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreDockBuildState>(0);
            Ref<CoreDockBuildState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreDockBuildState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::current_frame));
            return state;
        }

        static Ref<CoreDockSpaceState> dock_space_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<CoreDockSpaceState>(id);
            Ref<CoreDockSpaceState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreDockSpaceState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::process));
            return state;
        }

        static GUICore::ElementHandle begin_core_layout(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const c8* default_label, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, label ? Name(label) : Name(default_label));
            context->set_layout_config(element, layout);
            return element;
        }

        static bool dock_scope_has_panel(const CoreDockBuildScope& scope, id_t panel)
        {
            for(const CoreDockPanelBuildInfo& info : scope.panels)
            {
                if(info.id == panel)
                {
                    return true;
                }
            }
            return false;
        }

        static bool dock_live_tab(const CoreDockBuildScope& scope, const DockSpaceLayoutNodeDesc& node, id_t& selected)
        {
            if(node.selected_tab && dock_scope_has_panel(scope, node.selected_tab))
            {
                selected = node.selected_tab;
                return true;
            }
            for(id_t tab : node.tabs)
            {
                if(dock_scope_has_panel(scope, tab))
                {
                    selected = tab;
                    return true;
                }
            }
            selected = 0;
            return false;
        }

        static void dock_assign_layout_node(const CoreDockBuildScope& scope, const DockSpaceLayoutDesc& layout,
            u32 node_index, const RectF& rect, HashMap<id_t, RectF, GUICore::IdHash>& panel_rects)
        {
            if(node_index >= layout.nodes.size())
            {
                return;
            }
            const DockSpaceLayoutNodeDesc& node = layout.nodes[node_index];
            if(node.split)
            {
                f32 ratio = clamp(node.split_ratio, 0.05f, 0.95f);
                if(node.split_axis == DockSplitAxis::x)
                {
                    f32 child_width = rect.width * ratio;
                    dock_assign_layout_node(scope, layout, node.child0,
                        RectF(rect.offset_x, rect.offset_y, child_width, rect.height), panel_rects);
                    dock_assign_layout_node(scope, layout, node.child1,
                        RectF(rect.offset_x + child_width, rect.offset_y, max(rect.width - child_width, 0.0f), rect.height), panel_rects);
                }
                else
                {
                    f32 child_height = rect.height * ratio;
                    dock_assign_layout_node(scope, layout, node.child0,
                        RectF(rect.offset_x, rect.offset_y, rect.width, child_height), panel_rects);
                    dock_assign_layout_node(scope, layout, node.child1,
                        RectF(rect.offset_x, rect.offset_y + child_height, rect.width, max(rect.height - child_height, 0.0f)), panel_rects);
                }
                return;
            }

            id_t selected = 0;
            if(dock_live_tab(scope, node, selected))
            {
                panel_rects.insert(make_pair(selected, rect));
            }
        }

        static void dock_default_layout(const CoreDockBuildScope& scope, const RectF& rect,
            HashMap<id_t, RectF, GUICore::IdHash>& panel_rects)
        {
            if(scope.panels.empty())
            {
                return;
            }
            f32 panel_height = rect.height / (f32)scope.panels.size();
            f32 cursor_y = rect.offset_y;
            for(usize i = 0; i < scope.panels.size(); ++i)
            {
                f32 height = i + 1 == scope.panels.size() ? max(rect.offset_y + rect.height - cursor_y, 0.0f) : panel_height;
                panel_rects.insert(make_pair(scope.panels[i].id, RectF(rect.offset_x, cursor_y, rect.width, height)));
                cursor_y += height;
            }
        }

        static RectF dock_panel_content_rect(const RectF& rect, const DockPanelStyle& style)
        {
            f32 border = max(style.border_size, 0.0f);
            f32 title = style.title_bar ? max(style.title_bar_height, 0.0f) : 0.0f;
            return RectF(rect.offset_x + border, rect.offset_y + title + border,
                max(rect.width - border * 2.0f, 0.0f), max(rect.height - title - border * 2.0f, 0.0f));
        }

        LUNA_GUI_API GUICore::ElementHandle begin_dock_space(GUICore::IContext* context, GUICore::id_t id,
            const c8* label, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = begin_core_layout(context, id, label, "dock_space", layout);
            CoreDockBuildScope scope;
            scope.dock_space_id = id;
            scope.dock_space = element;
            dock_build_state(context)->stack.push_back(move(scope));
            return element;
        }

        LUNA_GUI_API void set_dockspace_layout(GUICore::IContext* context, GUICore::id_t dock_space,
            const DockSpaceLayoutDesc& desc)
        {
            luassert(context && dock_space);
            Ref<CoreDockSpaceState> state = dock_space_state(context, dock_space);
            state->has_layout = true;
            state->layout = desc;
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CoreDockSpaceState>(dock_space), state.object(),
                GUICore::StateLifetime::process));
        }

        LUNA_GUI_API bool begin_dock_panel(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool* open, const DockPanelStyle& style, const GUICore::LayoutConfig& layout, GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            if(open && !*open)
            {
                if(out_handle)
                {
                    *out_handle = GUICore::ElementHandle();
                }
                return false;
            }
            Ref<CoreDockBuildState> build_state = dock_build_state(context);
            luassert(!build_state->stack.empty());

            GUICore::LayoutConfig panel_layout = layout;
            f32 padding = style_value(context, Name("gui.editor.dock_panel.padding"), GUICore::style_f32(6.0f)).number.x;
            f32 border = max(style.border_size, 0.0f);
            f32 title = style.title_bar ? max(style.title_bar_height, 0.0f) : 0.0f;
            panel_layout.padding.x += border + padding;
            panel_layout.padding.y += border + title + padding;
            panel_layout.padding.z += border + padding;
            panel_layout.padding.w += border + padding;

            GUICore::ElementHandle panel = context->begin_element(id, label ? Name(label) : Name("dock_panel"));
            context->set_layout_config(panel, panel_layout);
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            context->set_interactable(panel, interactable);

            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(0.0f, 0.0f, 0.0f, 0.0f), style.border_color, 4.0f);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(border, border, -border * 2.0f, -border * 2.0f), style.background_color, 3.0f);
            if(style.title_bar && title > 0.0f)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rect,
                    RectF(border, border, -border * 2.0f, title), style.title_bar_color);
                Float4U title_color = style_value(context, Name("gui.editor.dock_panel.title_text"),
                    GUICore::style_f32x4(Float4U(0.92f, 0.94f, 0.97f, 1.0f))).number;
                f32 font_size = style_value(context, Name("gui.editor.dock_panel.title_font_size"),
                    GUICore::style_f32(15.0f)).number.x;
                draw_relative_text(context, RectF(10.0f + border, border, -20.0f - border * 2.0f, title),
                    label, title_color, font_size, VG::TextAlignment::begin);
            }

            CoreDockPanelBuildInfo info;
            info.id = id;
            info.handle = panel;
            info.style = style;
            info.open = open;
            info.label = label ? label : "";
            build_state->stack.back().panels.push_back(move(info));
            if(out_handle)
            {
                *out_handle = panel;
            }
            return true;
        }

        LUNA_GUI_API void end_dock_panel(GUICore::IContext* context)
        {
            luassert(context);
            context->end_element();
        }

        LUNA_GUI_API RV end_dock_space(GUICore::IContext* context, const GUICore::ElementHandle& dock_space,
            const RectF& rect)
        {
            luassert(context);
            Ref<CoreDockBuildState> build_state = dock_build_state(context);
            luassert(!build_state->stack.empty());
            CoreDockBuildScope scope = move(build_state->stack.back());
            build_state->stack.pop_back();
            HashMap<id_t, RectF, GUICore::IdHash> panel_rects;
            Ref<CoreDockSpaceState> state = dock_space_state(context, scope.dock_space_id);
            if(state->has_layout && state->layout.root_node < state->layout.nodes.size())
            {
                dock_assign_layout_node(scope, state->layout, state->layout.root_node, rect, panel_rects);
                for(const DockSpaceFloatingPanelDesc& floating_panel : state->layout.floating_panels)
                {
                    if(dock_scope_has_panel(scope, floating_panel.panel))
                    {
                        RectF floating_rect = floating_panel.rect;
                        floating_rect.offset_x += rect.offset_x;
                        floating_rect.offset_y += rect.offset_y;
                        auto iter = panel_rects.find(floating_panel.panel);
                        if(iter != panel_rects.end())
                        {
                            iter->second = floating_rect;
                        }
                        else
                        {
                            panel_rects.insert(make_pair(floating_panel.panel, floating_rect));
                        }
                    }
                }
            }
            else
            {
                dock_default_layout(scope, rect, panel_rects);
            }

            RV result = ok;
            for(const CoreDockPanelBuildInfo& panel : scope.panels)
            {
                RectF panel_rect = state->has_layout ? RectF(rect.offset_x, rect.offset_y, 0.0f, 0.0f) : rect;
                auto iter = panel_rects.find(panel.id);
                if(iter != panel_rects.end())
                {
                    panel_rect = iter->second;
                }
                GUICore::FlexLayoutDesc layout_desc;
                layout_desc.axis = GUICore::LayoutAxis::y;
                layout_desc.main_axis_gap = style_value(context, Name("gui.editor.dock_panel.gap"),
                    GUICore::style_f32(4.0f)).number.x;
                RV r = set_editor_flex_layout_config(context, panel.handle, layout_desc);
                if(succeeded(r))
                {
                    r = layout_editor_tree(context, panel.handle, panel_rect);
                }
                if(failed(r) && succeeded(result))
                {
                    result = r;
                }
            }

            GUICore::LayoutResult dock_result;
            dock_result.rect = rect;
            dock_result.clip_rect = rect;
            dock_result.content_size = Float2U(rect.width, rect.height);
            context->set_layout_result(dock_space, dock_result);
            context->end_element();
            return result;
        }
    }
}
