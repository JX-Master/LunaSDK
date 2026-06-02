/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIWidgets.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void push_id(IContext* context, u64 id)
        {
            context_from_interface(context)->push_id(id);
        }

        LUNA_GUI_API void push_id(IContext* context, const void* ptr)
        {
            context_from_interface(context)->push_id((u64)(usize)ptr);
        }

        LUNA_GUI_API void push_id(IContext* context, const c8* str)
        {
            u64 h = hash_cstr(str ? str : "", FNV_OFFSET);
            context_from_interface(context)->push_id(h);
        }

        LUNA_GUI_API void pop_id(IContext* context)
        {
            context_from_interface(context)->pop_id();
        }

        LUNA_GUI_API void push_layer(IContext* context, id_t id, const Float2U& screen_position)
        {
            context_from_interface(context)->push_layer(id, screen_position);
        }

        LUNA_GUI_API void pop_layer(IContext* context)
        {
            context_from_interface(context)->pop_layer();
        }

        LUNA_GUI_API void push_clip_rect(IContext* context, const RectF& rect)
        {
            context_from_interface(context)->push_clip_rect(rect);
        }

        LUNA_GUI_API void pop_clip_rect(IContext* context)
        {
            context_from_interface(context)->pop_clip_rect();
        }

        LUNA_GUI_API void tree_push(IContext* context)
        {
            context_from_interface(context)->tree_push();
        }

        LUNA_GUI_API void tree_push(IContext* context, ItemHandle node)
        {
            context_from_interface(context)->tree_push(node);
        }

        LUNA_GUI_API void tree_pop(IContext* context)
        {
            context_from_interface(context)->tree_pop();
        }

        LUNA_GUI_API ItemHandle custom_node(IContext* context, Ref<Node> node, const c8* label, bool interactive)
        {
            return context_from_interface(context)->add_node(move(node), label, interactive);
        }

        LUNA_GUI_API bool begin_drag_drop_source(IContext* context, ItemHandle source, const Name& payload_type)
        {
            return context_from_interface(context)->begin_drag_drop_source(source, payload_type);
        }

        LUNA_GUI_API void set_drag_drop_payload(IContext* context, const void* data, usize data_size)
        {
            context_from_interface(context)->set_drag_drop_payload(data, data_size);
        }

        LUNA_GUI_API void end_drag_drop_source(IContext* context)
        {
            context_from_interface(context)->end_drag_drop_source();
        }

        LUNA_GUI_API bool begin_drag_drop_target(IContext* context, ItemHandle target, const Name& payload_type)
        {
            return context_from_interface(context)->begin_drag_drop_target(target, payload_type);
        }

        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, const Name& payload_type)
        {
            return context_from_interface(context)->accept_drag_drop_payload(payload_type);
        }

        LUNA_GUI_API const DragDropPayload* accept_drag_drop_payload(IContext* context, ItemHandle target, const Name& payload_type)
        {
            return context_from_interface(context)->accept_drag_drop_payload(target, payload_type);
        }

        LUNA_GUI_API void end_drag_drop_target(IContext* context)
        {
            context_from_interface(context)->end_drag_drop_target();
        }

        LUNA_GUI_API bool is_drag_drop_active(IContext* context)
        {
            return context_from_interface(context)->is_drag_drop_active();
        }

        LUNA_GUI_API const DragDropPayload* get_drag_drop_payload(IContext* context)
        {
            return context_from_interface(context)->get_drag_drop_payload();
        }

        LUNA_GUI_API void set_next_item_layout(IContext* context, const LayoutStyle& style)
        {
            context_from_interface(context)->set_next_item_layout(style);
        }

        LUNA_GUI_API void set_next_canvas_item_layout(IContext* context, const CanvasItemLayout& layout)
        {
            context_from_interface(context)->set_next_canvas_item_layout(layout);
        }

        LUNA_GUI_API void set_next_dock_panel_style(IContext* context, const DockPanelStyle& style, bool* open)
        {
            context_from_interface(context)->set_next_dock_panel_style(style, open);
        }

        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<HLayoutNode>()), label ? label : "HLayout", Size(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API ItemHandle begin_h_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<HLayoutNode>()), label ? label : "HLayout", Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            Node& node = ctx->m_build_desc.nodes.back();
            node.layout_desc = desc;
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void end_h_layout(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<VLayoutNode>()), label ? label : "VLayout", Size(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API ItemHandle begin_v_layout(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<VLayoutNode>()), label ? label : "VLayout", Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            Node& node = ctx->m_build_desc.nodes.back();
            node.layout_desc = desc;
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void end_v_layout(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_table_layout(IContext* context, const c8* label, const TableDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<TableLayoutNode> node = new_object<TableLayoutNode>();
            node->desc = desc;
            ctx->begin_container(Ref<Node>(node), label ? label : "TableLayout", Size(), &handle);
            return handle;
        }

        LUNA_GUI_API void end_table_layout(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API void set_next_table_cell_color(IContext* context, const Float4U& color)
        {
            context_from_interface(context)->set_next_table_cell_color(color);
        }

        LUNA_GUI_API ItemHandle begin_grid_layout(IContext* context, const c8* label, const GridLayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<GridLayoutNode> node = new_object<GridLayoutNode>();
            node->desc = desc;
            ctx->begin_container(Ref<Node>(node), label ? label : "GridLayout", Size(), &handle);
            return handle;
        }

        LUNA_GUI_API void end_grid_layout(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label, const Size& size, const CanvasLayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<CanvasLayoutNode> node = new_object<CanvasLayoutNode>();
            node->desc = desc;
            ctx->begin_container(Ref<Node>(node), label ? label : "CanvasLayout", size, &handle);
            return handle;
        }

        LUNA_GUI_API ItemHandle begin_canvas_layout(IContext* context, const c8* label, const RectF& rect, const CanvasLayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<CanvasLayoutNode> node = new_object<CanvasLayoutNode>();
            node->desc = desc;
            ctx->begin_container(Ref<Node>(node), label ? label : "CanvasLayout", Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            Node& built_node = ctx->m_build_desc.nodes.back();
            built_node.absolute_position = true;
            built_node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void end_canvas_layout(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_dock_space(IContext* context, const c8* label, const Size& size)
        {
            ItemHandle handle;
            context_from_interface(context)->begin_container(Ref<Node>(new_object<DockSpaceNode>()), label ? label : "DockSpace", size, &handle);
            return handle;
        }

        LUNA_GUI_API void end_dock_space(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_dock_panel(IContext* context, const c8* label, bool* open, const DockPanelStyle& style, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->set_next_dock_panel_style(style, open);
            ctx->begin_container(Ref<Node>(new_object<VLayoutNode>()), label ? label : "DockPanel", Size(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API void end_dock_panel(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_scroll_view(IContext* context, const c8* label, const Size& size)
        {
            ItemHandle handle;
            context_from_interface(context)->begin_container(Ref<Node>(new_object<ScrollViewNode>()), label ? label : "ScrollView", size, &handle);
            return handle;
        }

        LUNA_GUI_API void end_scroll_view(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, const Size& size)
        {
            ItemHandle handle;
            context_from_interface(context)->begin_container(Ref<Node>(new_object<WindowNode>()), label ? label : "Window", size, &handle);
            return handle;
        }

        LUNA_GUI_API ItemHandle begin_window(IContext* context, const c8* label, bool* open, const Size& size)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<WindowNode> node = new_object<WindowNode>();
            node->open = open;
            ctx->begin_container(Ref<Node>(node), label ? label : "Window", size, &handle);
            ctx->m_build_desc.nodes.back().interactive = open != nullptr;
            return handle;
        }

        LUNA_GUI_API void end_window(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_popup(IContext* context, const c8* label, const Float2U& position, const Size& size)
        {
            PopupDesc desc;
            desc.position = position;
            desc.size = size;
            desc.flags = PopupFlag::none;
            return context_from_interface(context)->begin_popup(label, desc);
        }

        LUNA_GUI_API ItemHandle begin_popup(IContext* context, const c8* label, const PopupDesc& desc)
        {
            return context_from_interface(context)->begin_popup(label, desc);
        }

        LUNA_GUI_API void end_popup(IContext* context)
        {
            context_from_interface(context)->end_popup();
        }

        LUNA_GUI_API void open_popup(IContext* context, ItemHandle popup)
        {
            context_from_interface(context)->open_popup(popup);
        }

        LUNA_GUI_API void close_popup(IContext* context, ItemHandle popup)
        {
            context_from_interface(context)->close_popup(popup);
        }

        LUNA_GUI_API void close_current_popup(IContext* context)
        {
            context_from_interface(context)->close_current_popup();
        }

        LUNA_GUI_API void close_all_popups(IContext* context)
        {
            context_from_interface(context)->close_all_popups();
        }

        LUNA_GUI_API bool is_popup_open(IContext* context, ItemHandle popup)
        {
            return context_from_interface(context)->is_popup_open(popup);
        }

        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<MenuBarNode>()), label ? label : "MenuBar", Size(), &handle);
            Node& node = ctx->m_build_desc.nodes.back();
            LayoutDesc default_desc;
            if(desc.padding.left == 0.0f && desc.padding.top == 0.0f && desc.padding.right == 0.0f && desc.padding.bottom == 0.0f &&
                desc.gap == default_desc.gap &&
                desc.main_axis_alignment == default_desc.main_axis_alignment &&
                desc.cross_axis_alignment == default_desc.cross_axis_alignment)
            {
                node.layout_desc.padding = EdgeInsets::xy(4.0f, 2.0f);
                node.layout_desc.gap = 2.0f;
                node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::center;
            }
            else
            {
                node.layout_desc = desc;
            }
            return handle;
        }

        LUNA_GUI_API ItemHandle begin_menu_bar(IContext* context, const c8* label, const RectF& rect, const LayoutDesc& desc)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            ctx->begin_container(Ref<Node>(new_object<MenuBarNode>()), label ? label : "MenuBar", Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            Node& node = ctx->m_build_desc.nodes.back();
            LayoutDesc default_desc;
            if(desc.padding.left == 0.0f && desc.padding.top == 0.0f && desc.padding.right == 0.0f && desc.padding.bottom == 0.0f &&
                desc.gap == default_desc.gap &&
                desc.main_axis_alignment == default_desc.main_axis_alignment &&
                desc.cross_axis_alignment == default_desc.cross_axis_alignment)
            {
                node.layout_desc.padding = EdgeInsets::xy(4.0f, 2.0f);
                node.layout_desc.gap = 2.0f;
                node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::center;
            }
            else
            {
                node.layout_desc = desc;
            }
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void end_menu_bar(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle begin_menu(IContext* context, const c8* label, bool enabled)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuNode> menu = new_object<MenuNode>();
            menu->enabled = enabled;
            if(!ctx->m_parent_stack.empty() && ctx->m_parent_stack.back() != U32_MAX)
            {
                u32 parent = ctx->m_parent_stack.back();
                menu->top_level_menu = parent < ctx->m_build_desc.nodes.size() && ctx->m_build_desc.nodes[parent].is_menu_bar();
            }
            ItemHandle handle = ctx->add_node(Ref<Node>(menu), label ? label : "", enabled);
            ctx->set_item_query_state_if_absent(handle.id, Name("gui.open"), Any(false));
            u32 menu_index = (u32)ctx->m_build_desc.nodes.size() - 1;

            PopupDesc popup_desc;
            popup_desc.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            ItemHandle popup = ctx->begin_popup("##MenuPopup", popup_desc);
            Node& popup_node = ctx->m_build_desc.nodes.back();
            popup_node.set_popup_owner(handle.id);
            popup_node.layout_desc.padding = EdgeInsets::xy(6.0f, 5.0f);
            popup_node.layout_desc.gap = 1.0f;
            popup_node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ctx->m_build_desc.nodes[menu_index].set_menu_popup(popup.id);
            return handle;
        }

        LUNA_GUI_API void end_menu(IContext* context)
        {
            context_from_interface(context)->end_popup();
        }

        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool selected, bool enabled)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuItemNode> node = new_object<MenuItemNode>();
            node->shortcut = shortcut ? shortcut : "";
            node->selected = selected;
            node->enabled = enabled;
            return ctx->add_node(Ref<Node>(node), label ? label : "", enabled);
        }

        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool* selected, bool enabled)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuItemNode> node = new_object<MenuItemNode>();
            node->shortcut = shortcut ? shortcut : "";
            node->selected_value = selected;
            node->selected = selected ? *selected : false;
            node->enabled = enabled;
            return ctx->add_node(Ref<Node>(node), label ? label : "", enabled);
        }

        LUNA_GUI_API ItemHandle menu_separator(IContext* context)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuSeparatorNode> node = new_object<MenuSeparatorNode>();
            return ctx->add_node(Ref<Node>(node), "##MenuSeparator", false);
        }

        LUNA_GUI_API ItemHandle begin_tab_bar(IContext* context, const c8* label, TabBarFlag flags)
        {
            ItemHandle handle;
            Context* ctx = context_from_interface(context);
            Ref<TabBarNode> node = new_object<TabBarNode>();
            node->flags = flags;
            ctx->begin_container(Ref<Node>(node), label ? label : "TabBar", Size(), &handle);

            Ref<TabBarState> state = ctx->get_or_create_widget_state<TabBarState>(handle.id);
            TabBuildScope scope;
            scope.tab_bar_id = handle.id;
            scope.selected_id = state->tab_selected_id;
            scope.flags = flags;
            scope.had_existing_tabs = !state->tab_order.empty();
            ctx->m_tab_build_stack.push_back(scope);
            return handle;
        }

        LUNA_GUI_API void end_tab_bar(IContext* context)
        {
            Context* ctx = context_from_interface(context);
            luassert(!ctx->m_tab_build_stack.empty());
            TabBuildScope scope = ctx->m_tab_build_stack.back();
            ctx->m_tab_build_stack.pop_back();
            if(!scope.visible_tab_chosen && scope.first_open_id)
            {
                Ref<TabBarState> state = ctx->get_or_create_widget_state<TabBarState>(scope.tab_bar_id);
                state->tab_selected_id = scope.first_open_id;
            }
            ctx->end_container();
        }

        LUNA_GUI_API bool begin_tab_item(IContext* context, const c8* label, bool* open, TabItemFlag flags)
        {
            Context* ctx = context_from_interface(context);
            luassert(!ctx->m_tab_build_stack.empty());
            Ref<TabItemNode> tab_node = new_object<TabItemNode>();
            tab_node->open = open;
            tab_node->flags = flags;
            ItemHandle handle = ctx->add_node(Ref<Node>(tab_node), label ? label : "", true);
            u32 index = (u32)ctx->m_build_desc.nodes.size() - 1;
            Node& node = ctx->m_build_desc.nodes[index];
            bool item_open = !open || *open;

            TabBuildScope& scope = ctx->m_tab_build_stack.back();
            if(item_open && !scope.first_open_id)
            {
                scope.first_open_id = handle.id;
            }
            Ref<TabBarState> bar_state = ctx->get_or_create_widget_state<TabBarState>(scope.tab_bar_id);
            bool auto_select_new = item_open &&
                test_flags(scope.flags, TabBarFlag::auto_select_new_tabs) &&
                scope.had_existing_tabs &&
                !tab_order_contains(*bar_state, handle.id) &&
                !test_flags(flags, TabItemFlag::button);
            if(item_open && (test_flags(flags, TabItemFlag::selected) || auto_select_new) &&
                !test_flags(flags, TabItemFlag::button))
            {
                scope.selected_id = handle.id;
                bar_state->tab_selected_id = handle.id;
            }
            bool explicit_selected = item_open && scope.selected_id == handle.id;
            bool visible = item_open && !test_flags(flags, TabItemFlag::button) &&
                ((scope.selected_id && scope.selected_id == handle.id) ||
                    (!scope.selected_id && !scope.visible_tab_chosen) ||
                    explicit_selected);
            node.set_tab_item_selected(visible);
            if(visible)
            {
                scope.visible_tab_chosen = true;
                ctx->m_parent_stack.push_back(index);
                ctx->m_id_stack.push_back(handle.id);
            }
            return visible;
        }

        LUNA_GUI_API void end_tab_item(IContext* context)
        {
            context_from_interface(context)->end_container();
        }

        LUNA_GUI_API ItemHandle tab_item_button(IContext* context, const c8* label, TabItemFlag flags)
        {
            Context* ctx = context_from_interface(context);
            Ref<TabItemNode> tab_node = new_object<TabItemNode>();
            tab_node->flags = (TabItemFlag)((u32)flags | (u32)TabItemFlag::button);
            ItemHandle handle = ctx->add_node(Ref<Node>(tab_node), label ? label : "", true);
            return handle;
        }

        LUNA_GUI_API void set_tab_item_closed(IContext* context, const c8* label)
        {
            Context* ctx = context_from_interface(context);
            if(ctx->m_parent_stack.empty()) return;
            u32 parent = ctx->m_parent_stack.back();
            if(parent >= ctx->m_build_desc.nodes.size()) return;
            for(u32 child = ctx->m_build_desc.nodes[parent].first_child; child != U32_MAX; child = ctx->m_build_desc.nodes[child].next_sibling)
            {
                Node& node = ctx->m_build_desc.nodes[child];
                if(!node.is_tab_item() || strcmp(node.text.c_str(), label ? label : "") != 0) continue;
                if(node.bool_value())
                {
                    *node.bool_value() = false;
                }
                break;
            }
        }

        LUNA_GUI_API ItemHandle button(IContext* context, const c8* label)
        {
            Context* ctx = context_from_interface(context);
            Ref<ButtonNode> node = new_object<ButtonNode>();
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle button(IContext* context, const c8* label, const RectF& rect)
        {
            Context* ctx = context_from_interface(context);
            Ref<ButtonNode> button_node = new_object<ButtonNode>();
            ItemHandle handle = ctx->add_node(Ref<Node>(button_node), label ? label : "", true);
            Node& node = ctx->m_build_desc.nodes.back();
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            apply_requested_size(node, Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            return handle;
        }

        LUNA_GUI_API ItemHandle selectable(IContext* context, const c8* label, bool selected)
        {
            Context* ctx = context_from_interface(context);
            Ref<SelectableNode> node = new_object<SelectableNode>();
            node->selected = selected;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle text(IContext* context, const c8* text)
        {
            Context* ctx = context_from_interface(context);
            Ref<TextNode> node = new_object<TextNode>();
            return ctx->add_node(Ref<Node>(node), text ? text : "", false);
        }

        LUNA_GUI_API ItemHandle checkbox(IContext* context, const c8* label, bool* value)
        {
            Context* ctx = context_from_interface(context);
            Ref<CheckboxNode> node = new_object<CheckboxNode>();
            node->value = value;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool selected)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->selected = selected;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool* value)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->value = value;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, i32* value, i32 button_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->i32_value = value;
            node->item_value = button_value;
            node->selected = value && *value == button_value;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle toggle_switch(IContext* context, const c8* label, bool* value)
        {
            Context* ctx = context_from_interface(context);
            Ref<ToggleSwitchNode> node = new_object<ToggleSwitchNode>();
            node->value = value;
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle input_text(IContext* context, const c8* label, String& value)
        {
            Context* ctx = context_from_interface(context);
            Ref<InputTextNode> input_node = new_object<InputTextNode>();
            input_node->value = &value;
            ItemHandle handle = ctx->add_node(Ref<Node>(input_node), label ? label : "", true);
            return handle;
        }

        LUNA_GUI_API ItemHandle image(IContext* context, RHI::ITexture* texture, const Size& size, ImageFlag flags)
        {
            Context* ctx = context_from_interface(context);
            Ref<ImageNode> node = new_object<ImageNode>();
            node->image = texture;
            node->flags = flags;
            apply_requested_size(*node, size);
            ItemHandle handle = ctx->add_node(Ref<Node>(node), "Image", false);
            return handle;
        }

        LUNA_GUI_API ItemHandle collapsing_header(IContext* context, const c8* label)
        {
            Context* ctx = context_from_interface(context);
            Ref<CollapsingHeaderNode> node = new_object<CollapsingHeaderNode>();
            ItemHandle handle = ctx->add_node(Ref<Node>(node), label ? label : "", true);
            ctx->set_item_query_state_if_absent(handle.id, Name("gui.open"), Any(true));
            return handle;
        }

        LUNA_GUI_API ItemHandle tree_node(IContext* context, const c8* label, TreeNodeFlag flags)
        {
            Context* ctx = context_from_interface(context);
            Ref<TreeNodeNode> node = new_object<TreeNodeNode>();
            node->flags = flags;
            node->indent_depth = ctx->m_tree_depth;
            node->selected = test_flags(flags, TreeNodeFlag::selected);
            ItemHandle handle = ctx->add_node(Ref<Node>(node), label ? label : "", true);
            bool default_open = !test_flags(flags, TreeNodeFlag::leaf) && test_flags(flags, TreeNodeFlag::default_open);
            ctx->set_item_query_state_if_absent(handle.id, Name("gui.open"), Any(default_open));
            return handle;
        }

        LUNA_GUI_API ItemHandle combo(IContext* context, const c8* label, i32* current_item, Span<const c8*> items)
        {
            Context* ctx = context_from_interface(context);
            Ref<ComboNode> node = new_object<ComboNode>();
            node->current_item = current_item;
            node->combo_items.reserve(items.size());
            for(const c8* item : items)
            {
                node->combo_items.push_back(item ? item : "");
            }
            if(current_item && !node->combo_items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)node->combo_items.size() - 1);
            }
            ItemHandle handle = ctx->add_node(Ref<Node>(node), label ? label : "", true);
            ctx->set_item_query_state_if_absent(handle.id, Name("gui.open"), Any(false));
            return handle;
        }

        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, i32* current_item, Span<const c8*> items)
        {
            Context* ctx = context_from_interface(context);
            Ref<ButtonGroupNode> node = new_object<ButtonGroupNode>();
            node->current_item = current_item;
            node->items.reserve(items.size());
            for(const c8* item : items)
            {
                node->items.push_back(item ? item : "");
            }
            if(current_item && !node->items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)node->items.size() - 1);
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "ButtonGroup", true);
        }

        LUNA_GUI_API ItemHandle button_group(IContext* context, const c8* label, Span<bool> selected, Span<const c8*> items)
        {
            Context* ctx = context_from_interface(context);
            Ref<ButtonGroupNode> node = new_object<ButtonGroupNode>();
            node->selected = selected.data();
            usize count = min(selected.size(), items.size());
            node->items.reserve(count);
            for(usize i = 0; i < count; ++i)
            {
                node->items.push_back(items[i] ? items[i] : "");
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "ButtonGroup", true);
        }

        static ItemHandle add_slider_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<SliderFloatNode> slider_node = new_object<SliderFloatNode>();
            slider_node->binding.f32_value = value;
            slider_node->binding.value_count = count;
            slider_node->binding.min_value = min_value;
            slider_node->binding.max_value = max_value;
            ItemHandle handle = ctx->add_node(Ref<Node>(slider_node), label ? label : "", true);
            if(value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API ItemHandle slider_float(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float2(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float3(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float4(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(context, label, value, 4, min_value, max_value);
        }

        static ItemHandle add_slider_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<SliderIntNode> slider_node = new_object<SliderIntNode>();
            slider_node->binding.i32_value = value;
            slider_node->binding.value_count = count;
            slider_node->binding.min_value = (f32)min_value;
            slider_node->binding.max_value = (f32)max_value;
            ItemHandle handle = ctx->add_node(Ref<Node>(slider_node), label ? label : "", true);
            if(value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API ItemHandle slider_int(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int2(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int3(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int4(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(context, label, value, 4, min_value, max_value);
        }

        static ItemHandle add_input_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<InputFloatNode> input_node = new_object<InputFloatNode>();
            input_node->binding.f32_value = value;
            input_node->binding.value_count = count;
            input_node->binding.min_value = min_value;
            input_node->binding.max_value = max_value;
            ItemHandle handle = ctx->add_node(Ref<Node>(input_node), label ? label : "", true);
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        static ItemHandle add_input_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<InputIntNode> input_node = new_object<InputIntNode>();
            input_node->binding.i32_value = value;
            input_node->binding.value_count = count;
            input_node->binding.min_value = (f32)min_value;
            input_node->binding.max_value = (f32)max_value;
            ItemHandle handle = ctx->add_node(Ref<Node>(input_node), label ? label : "", true);
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        static ItemHandle add_slider_float_with_input_component(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ItemHandle handle = begin_h_layout(context, label ? label : "SliderFloatWithInput", row);
            set_next_item_layout(context, LayoutStyle::fill());
            add_slider_float_node(context, label, value, 1, min_value, max_value);
            set_next_item_layout(context, LayoutStyle::fixed_width(72.0f));
            add_input_float_node(context, "", value, 1, min_value, max_value);
            end_h_layout(context);
            return handle;
        }

        static ItemHandle add_slider_int_with_input_component(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            LayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            ItemHandle handle = begin_h_layout(context, label ? label : "SliderIntWithInput", row);
            set_next_item_layout(context, LayoutStyle::fill());
            add_slider_int_node(context, label, value, 1, min_value, max_value);
            set_next_item_layout(context, LayoutStyle::fixed_width(72.0f));
            add_input_int_node(context, "", value, 1, min_value, max_value);
            end_h_layout(context);
            return handle;
        }

        static ItemHandle add_slider_float_with_input_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            if(count <= 1) return add_slider_float_with_input_component(context, label, value, min_value, max_value);
            LayoutDesc column;
            column.gap = 4.0f;
            ItemHandle handle = begin_v_layout(context, label ? label : "SliderFloatWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                push_id(context, i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_float_with_input_component(context, component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                pop_id(context);
            }
            end_v_layout(context);
            return handle;
        }

        static ItemHandle add_slider_int_with_input_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            if(count <= 1) return add_slider_int_with_input_component(context, label, value, min_value, max_value);
            LayoutDesc column;
            column.gap = 4.0f;
            ItemHandle handle = begin_v_layout(context, label ? label : "SliderIntWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                push_id(context, i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_int_with_input_component(context, component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                pop_id(context);
            }
            end_v_layout(context);
            return handle;
        }

        LUNA_GUI_API ItemHandle slider_float_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float2_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float3_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_float4_with_input(IContext* context, const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(context, label, value, 4, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(context, label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int2_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(context, label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int3_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(context, label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API ItemHandle slider_int4_with_input(IContext* context, const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(context, label, value, 4, min_value, max_value);
        }

        static ItemHandle add_drag_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 speed, f32 min_value, f32 max_value, bool color, NumericEditFlag flags)
        {
            Context* ctx = context_from_interface(context);
            Ref<DragFloatNode> drag_node = new_object<DragFloatNode>();
            drag_node->binding.f32_value = value;
            drag_node->binding.value_count = count;
            drag_node->binding.f32_color = color;
            drag_node->binding.flags = flags;
            drag_node->binding.min_value = min_value;
            drag_node->binding.max_value = max_value;
            drag_node->binding.step_value = speed;
            ItemHandle handle = ctx->add_node(Ref<Node>(drag_node), label ? label : "", true);
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API ItemHandle drag_float(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags)
        {
            return add_drag_float_node(context, label, value, 1, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API ItemHandle drag_float2(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags)
        {
            return add_drag_float_node(context, label, value, 2, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API ItemHandle drag_float3(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags)
        {
            return add_drag_float_node(context, label, value, 3, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API ItemHandle drag_float4(IContext* context, const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, NumericEditFlag flags)
        {
            return add_drag_float_node(context, label, value, 4, speed, min_value, max_value, false, flags);
        }

        static ItemHandle add_drag_int_node(IContext* context, const c8* label, i32* value, u8 count, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags)
        {
            Context* ctx = context_from_interface(context);
            Ref<DragIntNode> drag_node = new_object<DragIntNode>();
            drag_node->binding.i32_value = value;
            drag_node->binding.value_count = count;
            drag_node->binding.flags = flags;
            drag_node->binding.min_value = (f32)min_value;
            drag_node->binding.max_value = (f32)max_value;
            drag_node->binding.step_value = speed;
            ItemHandle handle = ctx->add_node(Ref<Node>(drag_node), label ? label : "", true);
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API ItemHandle drag_int(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags)
        {
            return add_drag_int_node(context, label, value, 1, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API ItemHandle drag_int2(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags)
        {
            return add_drag_int_node(context, label, value, 2, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API ItemHandle drag_int3(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags)
        {
            return add_drag_int_node(context, label, value, 3, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API ItemHandle drag_int4(IContext* context, const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, NumericEditFlag flags)
        {
            return add_drag_int_node(context, label, value, 4, speed, min_value, max_value, flags);
        }

        static void sync_color_edit_build_state(ColorEditState& state, const Float4U& color)
        {
            ensure_color_edit_state_channels(state);
            state.color_edit_rgb[0] = (i32)color_channel_to_u8(color.x);
            state.color_edit_rgb[1] = (i32)color_channel_to_u8(color.y);
            state.color_edit_rgb[2] = (i32)color_channel_to_u8(color.z);
            state.color_edit_rgb[3] = (i32)color_channel_to_u8(color.w);
            f32 h = 0.0f;
            f32 s = 0.0f;
            f32 v = 0.0f;
            color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
            state.color_edit_hsv[0] = (i32)color_channel_to_u8(h);
            state.color_edit_hsv[1] = (i32)color_channel_to_u8(s);
            state.color_edit_hsv[2] = (i32)color_channel_to_u8(v);
        }

        static void assign_color_binding(ColorBinding& binding, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count)
        {
            binding.f32_value = f32_value;
            binding.u8_value = u8_value;
            binding.u32_value = u32_value;
            binding.type = type;
            binding.value_count = count;
        }

        static ItemHandle add_color_picker_node(IContext* context, const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count, id_t owner_id)
        {
            Context* ctx = context_from_interface(context);
            Ref<ColorPickerNode> picker_node = new_object<ColorPickerNode>();
            assign_color_binding(picker_node->binding, f32_value, u8_value, u32_value, type, count);
            picker_node->binding.owner_id = owner_id;
            ItemHandle handle = ctx->add_node(Ref<Node>(picker_node), label ? label : "ColorPicker", true);
            return handle;
        }

        static void tag_color_numeric_node(Context* ctx, ItemHandle handle, id_t owner_id, ColorEditPart part)
        {
            if(Node* node = ctx->find_build_node(handle))
            {
                if(!node->is_drag_numeric() || !node->is_int_numeric()) return;
                DragIntNode* drag_node = (DragIntNode*)node;
                drag_node->binding.color_owner_id = owner_id;
                drag_node->binding.color_part = part;
            }
        }

        static ItemHandle add_color_channel_drag(IContext* context, const c8* label, i32* value, id_t owner_id, ColorEditPart part)
        {
            Context* ctx = context_from_interface(context);
            ItemHandle handle = drag_int(context, label, value, 1.0f, 0, 255, NumericEditFlag::input_on_double_click);
            tag_color_numeric_node(ctx, handle, owner_id, part);
            return handle;
        }

        static ItemHandle add_color_edit_node(IContext* context, const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, ColorValueType type, u8 count)
        {
            Context* ctx = context_from_interface(context);
            Ref<ColorEditNode> edit_node = new_object<ColorEditNode>();
            assign_color_binding(edit_node->binding, f32_value, u8_value, u32_value, type, count);
            ItemHandle handle = ctx->add_node(Ref<Node>(edit_node), label ? label : "", true);
            u32 color_index = (u32)ctx->m_build_desc.nodes.size() - 1;
            Node& node = ctx->m_build_desc.nodes[color_index];
            write_color_value(node, read_color_value(node));

            Ref<ColorEditState> color_state = ctx->get_or_create_widget_state<ColorEditState>(handle.id);
            color_edit_axis_ref(*color_state) = clamp(color_edit_axis_ref(*color_state), 0, 5);
            ensure_color_edit_state_channels(*color_state);
            sync_color_edit_build_state(*color_state, read_color_value(node));

            PopupDesc popup_desc;
            popup_desc.size = Size::fixed(476.0f, count == 4 ? 470.0f : 432.0f);
            popup_desc.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            ctx->push_id(handle.id);
            ItemHandle popup = ctx->begin_popup("##ColorEditPopup", popup_desc);
            Node& popup_node = ctx->m_build_desc.nodes.back();
            popup_node.set_popup_owner(handle.id);
            popup_node.layout_desc.padding = EdgeInsets::all(10.0f);
            popup_node.layout_desc.gap = 8.0f;
            popup_node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;

            set_next_item_layout(context, LayoutStyle::fixed_height(300.0f));
            add_color_picker_node(context, "##ColorPicker", f32_value, u8_value, u32_value, type, count, handle.id);

            const c8* axis_items[] = { "H", "S", "V", "R", "G", "B" };
            set_next_item_layout(context, LayoutStyle::fixed_height(28.0f));
            button_group(context, "Channel", &color_edit_axis_ref(*color_state), Span<const c8*>(axis_items, 6));

            LayoutDesc row;
            row.gap = 6.0f;
            row.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
            begin_h_layout(context, "RGB", row);
            add_color_channel_drag(context, "R", &color_state->color_edit_rgb[0], handle.id, ColorEditPart::rgb);
            add_color_channel_drag(context, "G", &color_state->color_edit_rgb[1], handle.id, ColorEditPart::rgb);
            add_color_channel_drag(context, "B", &color_state->color_edit_rgb[2], handle.id, ColorEditPart::rgb);
            end_h_layout(context);

            set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
            begin_h_layout(context, "HSV", row);
            add_color_channel_drag(context, "H", &color_state->color_edit_hsv[0], handle.id, ColorEditPart::hsv);
            add_color_channel_drag(context, "S", &color_state->color_edit_hsv[1], handle.id, ColorEditPart::hsv);
            add_color_channel_drag(context, "V", &color_state->color_edit_hsv[2], handle.id, ColorEditPart::hsv);
            end_h_layout(context);

            if(count == 4)
            {
                set_next_item_layout(context, LayoutStyle::fixed_height(30.0f));
                begin_h_layout(context, "Alpha", row);
                add_color_channel_drag(context, "A", &color_state->color_edit_rgb[3], handle.id, ColorEditPart::rgb);
                end_h_layout(context);
            }

            ctx->end_popup();
            ctx->pop_id();
            ctx->m_build_desc.nodes[color_index].set_menu_popup(popup.id);
            return handle;
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, f32* value)
        {
            return add_color_edit_node(context, label, value, nullptr, nullptr, ColorValueType::f32, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, f32* value)
        {
            return add_color_edit_node(context, label, value, nullptr, nullptr, ColorValueType::f32, 4);
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u8* value)
        {
            return add_color_edit_node(context, label, nullptr, value, nullptr, ColorValueType::u8, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u8* value)
        {
            return add_color_edit_node(context, label, nullptr, value, nullptr, ColorValueType::u8, 4);
        }

        LUNA_GUI_API ItemHandle color_edit3(IContext* context, const c8* label, u32* value)
        {
            return add_color_edit_node(context, label, nullptr, nullptr, value, ColorValueType::rgba8, 3);
        }

        LUNA_GUI_API ItemHandle color_edit4(IContext* context, const c8* label, u32* value)
        {
            return add_color_edit_node(context, label, nullptr, nullptr, value, ColorValueType::rgba8, 4);
        }

        static bool project_gizmo_point(const Float3& point, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect, Float2U& out)
        {
            Float4 clip = mul(Float4(point.x, point.y, point.z, 1.0f), mul(view, projection));
            if(abs(clip.w) < 0.00001f || isnan(clip.w) || isinf(clip.w)) return false;
            f32 ndc_x = clip.x / clip.w;
            f32 ndc_y = clip.y / clip.w;
            if(isnan(ndc_x) || isnan(ndc_y) || isinf(ndc_x) || isinf(ndc_y)) return false;
            out.x = viewport_rect.offset_x + (ndc_x * 0.5f + 0.5f) * viewport_rect.width;
            out.y = viewport_rect.offset_y + (0.5f - ndc_y * 0.5f) * viewport_rect.height;
            return true;
        }

        static RectF gizmo_line_hit_rect(const Float2U& a, const Float2U& b, f32 padding)
        {
            f32 min_x = min(a.x, b.x) - padding;
            f32 min_y = min(a.y, b.y) - padding;
            f32 max_x = max(a.x, b.x) + padding;
            f32 max_y = max(a.y, b.y) + padding;
            return RectF(min_x, min_y, max(max_x - min_x, 1.0f), max(max_y - min_y, 1.0f));
        }

        static Float3 gizmo_normalize_axis(const Float3& axis, const Float3& fallback)
        {
            f32 len = length(axis);
            if(len <= 0.0001f) return fallback;
            return axis / len;
        }

        static f32 gizmo_axis_pixels_per_unit(const Float2U& begin, const Float2U& end)
        {
            return length(Float2(end.x - begin.x, end.y - begin.y));
        }

        static f32 gizmo_compute_screen_factor(const Float3& origin, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect)
        {
            Float4x4 camera_world = inverse(view);
            Float3 camera_right = gizmo_normalize_axis(AffineMatrix::right(camera_world), Float3(1.0f, 0.0f, 0.0f));
            Float2U origin_screen;
            Float2U right_screen;
            if(!project_gizmo_point(origin, view, projection, viewport_rect, origin_screen) ||
                !project_gizmo_point(origin + camera_right, view, projection, viewport_rect, right_screen))
            {
                return 1.0f;
            }
            f32 pixels_per_world_unit = gizmo_axis_pixels_per_unit(origin_screen, right_screen);
            if(pixels_per_world_unit <= 0.0001f || isnan(pixels_per_world_unit) || isinf(pixels_per_world_unit))
            {
                return 1.0f;
            }
            f32 target_screen_length = clamp(min(viewport_rect.width, viewport_rect.height) * 0.11f, 56.0f, 96.0f);
            return max(target_screen_length / pixels_per_world_unit, 0.0001f);
        }

        static Float2U gizmo_state_default_pointer()
        {
            return Float2U(0.0f, 0.0f);
        }

        static Float3 gizmo_matrix_row3(const Float4x4& matrix, u32 row)
        {
            return Float3(matrix.r[row].x, matrix.r[row].y, matrix.r[row].z);
        }

        static void gizmo_set_matrix_row3(Float4x4& matrix, u32 row, const Float3& value)
        {
            matrix.r[row].x = value.x;
            matrix.r[row].y = value.y;
            matrix.r[row].z = value.z;
        }

        static bool gizmo_axis_delta_units(const Float2U& origin_screen, const Float2U& axis_screen, f32 axis_world_len, const Float2& screen_delta, f32& out_delta_units)
        {
            f32 axis_pixels = gizmo_axis_pixels_per_unit(origin_screen, axis_screen);
            if(axis_pixels <= 0.0001f) return false;
            Float2 screen_axis(axis_screen.x - origin_screen.x, axis_screen.y - origin_screen.y);
            screen_axis = normalize(screen_axis);
            out_delta_units = (screen_delta.x * screen_axis.x + screen_delta.y * screen_axis.y) * axis_world_len / axis_pixels;
            return true;
        }

        static void gizmo_scale_basis(Float4x4& matrix, i32 active_axis, f32 factor)
        {
            factor = max(factor, 0.001f);
            if(active_axis >= 0 && active_axis < 3)
            {
                gizmo_set_matrix_row3(matrix, (u32)active_axis, gizmo_matrix_row3(matrix, (u32)active_axis) * factor);
                return;
            }
            for(u32 i = 0; i < 3; ++i)
            {
                gizmo_set_matrix_row3(matrix, i, gizmo_matrix_row3(matrix, i) * factor);
            }
        }

        static void gizmo_rotate_basis(Float4x4& matrix, const Float3& axis, f32 angle)
        {
            Float4x4 rotation = AffineMatrix::make_rotation_axis_angle(axis, angle);
            for(u32 i = 0; i < 3; ++i)
            {
                Float3 row = gizmo_matrix_row3(matrix, i);
                Float4 rotated = mul(Float4(row.x, row.y, row.z, 0.0f), rotation);
                gizmo_set_matrix_row3(matrix, i, Float3(rotated.x, rotated.y, rotated.z));
            }
        }

        static Float4x4 gizmo_scale_delta_matrix(i32 active_axis, f32 factor)
        {
            if(active_axis == 0) return AffineMatrix::make_scaling(factor, 1.0f, 1.0f);
            if(active_axis == 1) return AffineMatrix::make_scaling(1.0f, factor, 1.0f);
            if(active_axis == 2) return AffineMatrix::make_scaling(1.0f, 1.0f, factor);
            return AffineMatrix::make_scaling(factor, factor, factor);
        }

        static void gizmo_draw_rotation_ring(IContext* context, const Float3& origin, const Float3& axis_a, const Float3& axis_b, f32 radius, const Float4x4& view, const Float4x4& projection,
            const RectF& viewport_rect, const Float4U& color, f32 width)
        {
            constexpr u32 segments = 48;
            Float2U previous_screen;
            bool previous_visible = false;
            for(u32 i = 0; i <= segments; ++i)
            {
                f32 angle = (f32)i / (f32)segments * 6.28318530717958647692f;
                Float3 point = origin + axis_a * cos(angle) * radius + axis_b * sin(angle) * radius;
                Float2U screen;
                bool visible = project_gizmo_point(point, view, projection, viewport_rect, screen);
                if(visible && previous_visible)
                {
                    draw_line(context, previous_screen, screen, color, width);
                }
                previous_screen = screen;
                previous_visible = visible;
            }
        }

        LUNA_GUI_API ItemHandle gizmo(IContext* context, const c8* label, Float4x4& world_matrix, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect,
            GizmoOperation operation, GizmoMode mode, f32 snap, bool enabled, bool orthographic,
            Float4x4* delta_matrix, bool* is_mouse_hover, bool* is_mouse_moving, bool* edited)
        {
            (void)orthographic;
            if(delta_matrix) *delta_matrix = Float4x4::identity();
            if(is_mouse_hover) *is_mouse_hover = false;
            if(is_mouse_moving) *is_mouse_moving = false;
            if(edited) *edited = false;

            push_id(context, label ? label : "Gizmo");

            Float3 origin = AffineMatrix::translation(world_matrix);
            Float3 axes[3];
            if(mode == GizmoMode::local)
            {
                axes[0] = gizmo_normalize_axis(AffineMatrix::right(world_matrix), Float3(1.0f, 0.0f, 0.0f));
                axes[1] = gizmo_normalize_axis(AffineMatrix::up(world_matrix), Float3(0.0f, 1.0f, 0.0f));
                axes[2] = gizmo_normalize_axis(AffineMatrix::forward(world_matrix), Float3(0.0f, 0.0f, 1.0f));
            }
            else
            {
                axes[0] = Float3(1.0f, 0.0f, 0.0f);
                axes[1] = Float3(0.0f, 1.0f, 0.0f);
                axes[2] = Float3(0.0f, 0.0f, 1.0f);
            }

            Float2U origin_screen;
            Float2U axis_screen[3];
            bool visible = project_gizmo_point(origin, view, projection, viewport_rect, origin_screen);
            f32 axis_world_len = gizmo_compute_screen_factor(origin, view, projection, viewport_rect);
            for(u32 i = 0; i < 3; ++i)
            {
                visible = project_gizmo_point(origin + axes[i] * axis_world_len, view, projection, viewport_rect, axis_screen[i]) && visible;
            }
            if(!visible)
            {
                ItemHandle invalid = hit_box(context, "Invalid", RectF(-10000.0f, -10000.0f, 1.0f, 1.0f));
                pop_id(context);
                return invalid;
            }

            Float4U axis_colors[3] = {
                Float4U(0.92f, 0.22f, 0.25f, 1.0f),
                Float4U(0.24f, 0.80f, 0.28f, 1.0f),
                Float4U(0.25f, 0.46f, 0.96f, 1.0f)
            };

            ItemHandle axis_handles[3];
            for(u32 i = 0; i < 3; ++i)
            {
                c8 axis_label[8];
                snprintf(axis_label, 8, "Axis%u", i);
                axis_handles[i] = hit_box(context, axis_label, gizmo_line_hit_rect(origin_screen, axis_screen[i], 7.0f));
            }
            ItemHandle center_handle = hit_box(context, "Center", RectF(origin_screen.x - 8.0f, origin_screen.y - 8.0f, 16.0f, 16.0f));

            i32 active_axis = -1;
            for(i32 i = 0; i < 3; ++i)
            {
                if(is_item_active(axis_handles[i]))
                {
                    active_axis = i;
                    break;
                }
            }
            if(active_axis < 0 && is_item_active(center_handle))
            {
                active_axis = 3;
            }

            bool hovered = is_item_hovered(center_handle);
            for(u32 i = 0; i < 3; ++i)
            {
                hovered = hovered || is_item_hovered(axis_handles[i]);
            }
            if(is_mouse_hover) *is_mouse_hover = hovered;
            if(is_mouse_moving) *is_mouse_moving = active_axis >= 0;

            if(operation == GizmoOperation::rotate)
            {
                gizmo_draw_rotation_ring(context, origin, axes[1], axes[2], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[0]) || is_item_active(axis_handles[0]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[0], 3.0f);
                gizmo_draw_rotation_ring(context, origin, axes[2], axes[0], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[1]) || is_item_active(axis_handles[1]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[1], 3.0f);
                gizmo_draw_rotation_ring(context, origin, axes[0], axes[1], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[2]) || is_item_active(axis_handles[2]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[2], 3.0f);
            }
            for(u32 i = 0; i < 3; ++i)
            {
                bool axis_hot = is_item_hovered(axis_handles[i]) || is_item_active(axis_handles[i]);
                Float4U color = axis_hot ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[i];
                f32 width = axis_hot ? 4.0f : 3.0f;
                draw_line(context, origin_screen, axis_screen[i], color, operation == GizmoOperation::rotate ? 2.0f : width);
                draw_circle(context, axis_screen[i], axis_hot ? 6.0f : 5.0f, color);
            }
            draw_circle(context, origin_screen, is_item_hovered(center_handle) || is_item_active(center_handle) ? 8.0f : 6.0f,
                is_item_hovered(center_handle) || is_item_active(center_handle) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : Float4U(1.0f));

            StateKey<i32> active_axis_key { Name("gui.gizmo.active_axis"), -1 };
            StateKey<Float2U> last_pointer_key { Name("gui.gizmo.last_pointer"), gizmo_state_default_pointer() };
            i32 last_active_axis = get_item_state(center_handle, active_axis_key);
            Float2U pointer = get_pointer_position(context);
            Float2U last_pointer = get_item_state(center_handle, last_pointer_key);

            bool changed = false;
            if(enabled && active_axis >= 0)
            {
                if(last_active_axis == active_axis)
                {
                    Float2 screen_delta(pointer.x - last_pointer.x, pointer.y - last_pointer.y);
                    if(operation == GizmoOperation::translate)
                    {
                        Float3 world_delta(0.0f, 0.0f, 0.0f);
                        if(active_axis < 3)
                        {
                            f32 delta_units = 0.0f;
                            if(gizmo_axis_delta_units(origin_screen, axis_screen[active_axis], axis_world_len, screen_delta, delta_units))
                            {
                                if(snap > 0.0f && abs(delta_units) >= snap)
                                {
                                    delta_units = (delta_units > 0.0f ? 1.0f : -1.0f) * snap;
                                }
                                world_delta = axes[active_axis] * delta_units;
                            }
                        }
                        else
                        {
                            Float4x4 camera_world = inverse(view);
                            Float3 camera_right = gizmo_normalize_axis(AffineMatrix::right(camera_world), Float3(1.0f, 0.0f, 0.0f));
                            Float3 camera_up = gizmo_normalize_axis(AffineMatrix::up(camera_world), Float3(0.0f, 1.0f, 0.0f));
                            Float2U right_screen;
                            Float2U up_screen;
                            f32 right_ppu = project_gizmo_point(origin + camera_right, view, projection, viewport_rect, right_screen) ?
                                gizmo_axis_pixels_per_unit(origin_screen, right_screen) : 0.0f;
                            f32 up_ppu = project_gizmo_point(origin + camera_up, view, projection, viewport_rect, up_screen) ?
                                gizmo_axis_pixels_per_unit(origin_screen, up_screen) : 0.0f;
                            if(right_ppu > 0.0001f && up_ppu > 0.0001f)
                            {
                                world_delta = camera_right * (screen_delta.x / right_ppu) - camera_up * (screen_delta.y / up_ppu);
                            }
                        }
                        changed = length(world_delta) > 0.000001f;
                        if(changed)
                        {
                            world_matrix.r[3].x += world_delta.x;
                            world_matrix.r[3].y += world_delta.y;
                            world_matrix.r[3].z += world_delta.z;
                            if(delta_matrix) *delta_matrix = AffineMatrix::make_translation(world_delta);
                        }
                    }
                    else if(operation == GizmoOperation::scale)
                    {
                        f32 factor = 1.0f;
                        if(active_axis < 3)
                        {
                            f32 delta_units = 0.0f;
                            if(gizmo_axis_delta_units(origin_screen, axis_screen[active_axis], axis_world_len, screen_delta, delta_units))
                            {
                                factor += delta_units;
                            }
                        }
                        else
                        {
                            factor += (screen_delta.x - screen_delta.y) * 0.01f;
                        }
                        factor = max(factor, 0.001f);
                        changed = abs(factor - 1.0f) > 0.00001f;
                        if(changed)
                        {
                            gizmo_scale_basis(world_matrix, active_axis, factor);
                            if(delta_matrix) *delta_matrix = gizmo_scale_delta_matrix(active_axis, factor);
                        }
                    }
                    else if(operation == GizmoOperation::rotate)
                    {
                        Float3 axis = active_axis < 3 ? axes[active_axis] : gizmo_normalize_axis(AffineMatrix::forward(inverse(view)), Float3(0.0f, 0.0f, 1.0f));
                        f32 delta_angle = 0.0f;
                        if(active_axis < 3)
                        {
                            f32 pixels_per_unit = gizmo_axis_pixels_per_unit(origin_screen, axis_screen[active_axis]);
                            if(pixels_per_unit > 0.0001f)
                            {
                                Float2 screen_axis(axis_screen[active_axis].x - origin_screen.x, axis_screen[active_axis].y - origin_screen.y);
                                screen_axis = normalize(screen_axis);
                                Float2 tangent(-screen_axis.y, screen_axis.x);
                                delta_angle = (screen_delta.x * tangent.x + screen_delta.y * tangent.y) * 0.01f;
                            }
                        }
                        else
                        {
                            delta_angle = (screen_delta.x + screen_delta.y) * 0.01f;
                        }
                        if(snap > 0.0f && abs(delta_angle) >= snap)
                        {
                            delta_angle = (delta_angle > 0.0f ? 1.0f : -1.0f) * snap;
                        }
                        changed = abs(delta_angle) > 0.00001f;
                        if(changed)
                        {
                            gizmo_rotate_basis(world_matrix, axis, delta_angle);
                            if(delta_matrix) *delta_matrix = AffineMatrix::make_rotation_axis_angle(axis, delta_angle);
                        }
                    }
                }
            }

            if(changed)
            {
                if(edited) *edited = true;
            }
            set_item_state(center_handle, active_axis_key, active_axis);
            set_item_state(center_handle, last_pointer_key, pointer);

            pop_id(context);
            return center_handle;
        }

        LUNA_GUI_API ItemHandle hit_box(IContext* context, const c8* label, const RectF& rect)
        {
            Context* ctx = context_from_interface(context);
            Ref<HitBoxNode> node = new_object<HitBoxNode>();
            node->absolute_position = true;
            node->position = Float2U(rect.offset_x, rect.offset_y);
            apply_requested_size(*node, Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            ItemHandle handle = ctx->add_node(Ref<Node>(node), label ? label : "HitBox", true);
            return handle;
        }

        LUNA_GUI_API bool is_item_clicked(ItemHandle handle)
        {
            return get_item_state(handle, State::clicked());
        }

        LUNA_GUI_API bool is_item_right_clicked(ItemHandle handle)
        {
            return get_item_state(handle, State::right_clicked());
        }

        LUNA_GUI_API bool is_item_double_clicked(ItemHandle handle)
        {
            return get_item_state(handle, State::double_clicked());
        }

        LUNA_GUI_API bool is_item_hovered(ItemHandle handle)
        {
            return get_item_state(handle, State::hovered());
        }

        LUNA_GUI_API bool is_item_active(ItemHandle handle)
        {
            return get_item_state(handle, State::active());
        }

        LUNA_GUI_API bool is_item_focused(ItemHandle handle)
        {
            return get_item_state(handle, State::focused());
        }

        LUNA_GUI_API Float2U get_pointer_position(IContext* context)
        {
            return context_from_interface(context)->m_pointer_pos;
        }

        LUNA_GUI_API bool is_pointer_button_down(IContext* context, PointerButton button)
        {
            u32 index = (u32)button;
            return index < 5 ? context_from_interface(context)->m_pointer_button_down[index] : false;
        }

        LUNA_GUI_API bool is_key_down(IContext* context, Key key)
        {
            u32 index = (u32)key;
            return index < 256 ? context_from_interface(context)->m_key_down[index] : false;
        }

        LUNA_GUI_API KeyModifierFlag get_key_modifiers(IContext* context)
        {
            return context_from_interface(context)->m_key_modifiers;
        }

        LUNA_GUI_API FrameDesc get_frame_desc(IContext* context)
        {
            return context_from_interface(context)->m_frame_desc;
        }

        LUNA_GUI_API Float2U get_pointer_delta(IContext* context)
        {
            return context_from_interface(context)->m_pointer_delta;
        }

        static ItemHandle add_draw_node(IContext* context, Ref<Node> node, const c8* label, const RectF& rect)
        {
            Context* ctx = context_from_interface(context);
            node->absolute_position = true;
            node->position = Float2U(rect.offset_x, rect.offset_y);
            apply_requested_size(*node, GUI::Size::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            ItemHandle handle = ctx->add_node(move(node), label ? label : "", false);
            return handle;
        }

        LUNA_GUI_API ItemHandle draw_rect(IContext* context, const RectF& rect, const Float4U& color, f32 radius)
        {
            Ref<DrawRectNode> node = new_object<DrawRectNode>();
            node->color = color;
            node->radius = radius;
            return add_draw_node(context, Ref<Node>(node), "DrawRect", rect);
        }

        LUNA_GUI_API ItemHandle draw_circle(IContext* context, const Float2U& center, f32 radius, const Float4U& color)
        {
            f32 r = max(radius, 0.5f);
            RectF rect(center.x - r, center.y - r, r * 2.0f, r * 2.0f);
            Ref<DrawCircleNode> node = new_object<DrawCircleNode>();
            node->color = color;
            return add_draw_node(context, Ref<Node>(node), "DrawCircle", rect);
        }

        LUNA_GUI_API ItemHandle draw_line(IContext* context, const Float2U& begin, const Float2U& end, const Float4U& color, f32 width)
        {
            f32 line_width = max(width, 1.0f);
            f32 half_width = line_width * 0.5f;
            f32 min_x = min(begin.x, end.x) - half_width;
            f32 min_y = min(begin.y, end.y) - half_width;
            f32 max_x = max(begin.x, end.x) + half_width;
            f32 max_y = max(begin.y, end.y) + half_width;
            RectF rect(min_x, min_y, max(max_x - min_x, 1.0f), max(max_y - min_y, 1.0f));
            Ref<DrawLineNode> node = new_object<DrawLineNode>();
            node->begin = begin;
            node->end = end;
            node->color = color;
            node->width = line_width;
            return add_draw_node(context, Ref<Node>(node), "DrawLine", rect);
        }

        LUNA_GUI_API ItemHandle draw_text(IContext* context, const RectF& rect, const c8* text, const Float4U& color, f32 font_size,
            TextAlignment horizontal_alignment, TextAlignment vertical_alignment)
        {
            Ref<DrawTextNode> node = new_object<DrawTextNode>();
            node->text = text ? text : "";
            node->color = color;
            node->font_size = font_size;
            node->horizontal_alignment = horizontal_alignment;
            node->vertical_alignment = vertical_alignment;
            return add_draw_node(context, Ref<Node>(node), text ? text : "", rect);
        }

        LUNA_GUI_API ItemHandle draw_image(IContext* context, RHI::ITexture* texture, const RectF& rect, const Float4U& color, ImageFlag flags)
        {
            Ref<DrawImageNode> node = new_object<DrawImageNode>();
            node->image = texture;
            node->color = color;
            node->flags = flags;
            return add_draw_node(context, Ref<Node>(node), "DrawImage", rect);
        }
    }
}
