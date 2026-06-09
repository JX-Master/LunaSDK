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
        static bool is_menu_bar_node(const Node& node)
        {
            return node.type_guid() == MenuBarNode::__guid;
        }

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

        LUNA_GUI_API void define_style(IContext* context, const Name& name, const Name& parent)
        {
            context_from_interface(context)->define_style(name, parent);
        }

        LUNA_GUI_API void set_style_parent(IContext* context, const Name& name, const Name& parent)
        {
            context_from_interface(context)->set_style_parent(name, parent);
        }

        LUNA_GUI_API void set_style_value(IContext* context, const Name& style, const Name& entry, const StyleValue& value)
        {
            context_from_interface(context)->set_style_value(style, entry, value);
        }

        LUNA_GUI_API void set_style_f32(IContext* context, const Name& style, const Name& entry, f32 value)
        {
            context_from_interface(context)->set_style_value(style, entry, StyleValue::f32_1(value));
        }

        LUNA_GUI_API void set_style_f32x2(IContext* context, const Name& style, const Name& entry, const Float2U& value)
        {
            context_from_interface(context)->set_style_value(style, entry, StyleValue::f32_2(value));
        }

        LUNA_GUI_API void set_style_f32x3(IContext* context, const Name& style, const Name& entry, const Float3U& value)
        {
            context_from_interface(context)->set_style_value(style, entry, StyleValue::f32_3(value));
        }

        LUNA_GUI_API void set_style_f32x4(IContext* context, const Name& style, const Name& entry, const Float4U& value)
        {
            context_from_interface(context)->set_style_value(style, entry, StyleValue::f32_4(value));
        }

        LUNA_GUI_API void set_style_name(IContext* context, const Name& style, const Name& entry, const Name& value)
        {
            context_from_interface(context)->set_style_value(style, entry, StyleValue::name(value));
        }

        LUNA_GUI_API void inherit_style_entry(IContext* context, const Name& style, const Name& entry)
        {
            context_from_interface(context)->inherit_style_entry(style, entry);
        }

        LUNA_GUI_API void unset_style_entry(IContext* context, const Name& style, const Name& entry)
        {
            context_from_interface(context)->unset_style_entry(style, entry);
        }

        LUNA_GUI_API StyleValue get_style_value(IContext* context, const Name& style, const Name& entry, const StyleValue& default_value)
        {
            return context_from_interface(context)->get_style_value(style, entry, default_value);
        }

        LUNA_GUI_API void push_style(IContext* context, const Name& style)
        {
            context_from_interface(context)->push_style(style);
        }

        LUNA_GUI_API void pop_style(IContext* context)
        {
            context_from_interface(context)->pop_style();
        }

        LUNA_GUI_API RV register_font(IContext* context, const Name& id, Font::IFontFile* font, u32 font_index)
        {
            return context_from_interface(context)->register_font(id, font, font_index);
        }

        LUNA_GUI_API FontDesc get_font(IContext* context, const Name& id)
        {
            return context_from_interface(context)->get_font(id);
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

        LUNA_GUI_API void set_next_item_enabled(IContext* context, bool enabled)
        {
            context_from_interface(context)->set_next_item_enabled(enabled);
        }

        LUNA_GUI_API void push_enabled(IContext* context, bool enabled)
        {
            context_from_interface(context)->push_enabled(enabled);
        }

        LUNA_GUI_API void pop_enabled(IContext* context)
        {
            context_from_interface(context)->pop_enabled();
        }

        LUNA_GUI_API void set_next_item_render_proxy(IContext* context, const RenderProxyDesc& proxy)
        {
            context_from_interface(context)->set_next_item_render_proxy(proxy);
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
            Context* ctx = context_from_interface(context);
            luassert(!ctx->m_parent_stack.empty());
            u32 parent = ctx->m_parent_stack.back();
            luassert(parent < ctx->m_build_desc.nodes.size());
            TableLayoutNode* table = table_layout_node(ctx->m_build_desc.nodes[parent]);
            luassert_msg(table && table->active_row_attachment == U32_MAX, "Cannot end a table layout while a table row is open.");
            ctx->end_container();
        }

        LUNA_GUI_API bool begin_table_row(IContext* context)
        {
            return context_from_interface(context)->begin_table_row();
        }

        LUNA_GUI_API void end_table_row(IContext* context)
        {
            context_from_interface(context)->end_table_row();
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

        LUNA_GUI_API bool begin_popup(IContext* context, const c8* label, const Float2U& position, const Size& size, ItemHandle* out_handle)
        {
            PopupDesc desc;
            desc.position = position;
            desc.size = size;
            return context_from_interface(context)->begin_popup(label, desc, out_handle);
        }

        LUNA_GUI_API bool begin_popup(IContext* context, const c8* label, const PopupDesc& desc, ItemHandle* out_handle)
        {
            return context_from_interface(context)->begin_popup(label, desc, out_handle);
        }

        LUNA_GUI_API void end_popup(IContext* context)
        {
            context_from_interface(context)->end_popup();
        }

        LUNA_GUI_API void open_popup(IContext* context, const c8* label)
        {
            context_from_interface(context)->open_popup(label);
        }

        LUNA_GUI_API void open_popup(IContext* context, ItemHandle popup)
        {
            context_from_interface(context)->open_popup(popup);
        }

        LUNA_GUI_API void close_popup(IContext* context, const c8* label)
        {
            context_from_interface(context)->close_popup(label);
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

        LUNA_GUI_API bool is_popup_open(IContext* context, const c8* label)
        {
            return context_from_interface(context)->is_popup_open(label);
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

        static void prepare_menu_item_for_parent(Context* ctx, MenuItemNode& item)
        {
            if(!ctx || ctx->m_parent_stack.empty() || ctx->m_parent_stack.back() == U32_MAX)
            {
                return;
            }
            u32 parent = ctx->m_parent_stack.back();
            item.top_level_menu = parent < ctx->m_build_desc.nodes.size() && is_menu_bar_node(ctx->m_build_desc.nodes[parent]);
            if(item.top_level_menu)
            {
                item.layout_style = LayoutStyle::hug();
            }
        }

        LUNA_GUI_API bool begin_menu(IContext* context, const c8* label, bool enabled, ItemHandle* out_handle)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuItemNode> menu = new_object<MenuItemNode>();
            menu->enabled = enabled;
            prepare_menu_item_for_parent(ctx, *menu);
            ItemHandle handle = ctx->add_node(Ref<Node>(menu), label ? label : "", true);
            if(out_handle)
            {
                *out_handle = handle;
            }
            ctx->set_item_query_state_if_absent(handle.id, Name("gui.open"), Any(false));

            PopupDesc popup_desc;
            popup_desc.flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
            ctx->push_id(handle.id);
            ItemHandle popup;
            bool popup_open = ctx->begin_popup("##MenuPopup", popup_desc, &popup);
            menu->popup_id = popup.id;
            if(!popup_open)
            {
                ctx->pop_id();
                return false;
            }
            Node& popup_node = ctx->m_build_desc.nodes.back();
            set_popup_owner(popup_node, handle.id);
            popup_node.layout_desc.padding = EdgeInsets::xy(6.0f, 5.0f);
            popup_node.layout_desc.gap = 1.0f;
            popup_node.layout_desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
            return true;
        }

        LUNA_GUI_API void end_menu(IContext* context)
        {
            Context* ctx = context_from_interface(context);
            ctx->end_popup();
            ctx->pop_id();
        }

        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool selected, bool enabled)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuItemNode> node = new_object<MenuItemNode>();
            node->shortcut = shortcut ? shortcut : "";
            node->selected = selected;
            node->enabled = enabled;
            prepare_menu_item_for_parent(ctx, *node);
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle menu_item(IContext* context, const c8* label, const c8* shortcut, bool* selected, bool enabled)
        {
            Context* ctx = context_from_interface(context);
            Ref<MenuItemNode> node = new_object<MenuItemNode>();
            node->shortcut = shortcut ? shortcut : "";
            node->selected_value = selected;
            node->selected = selected ? *selected : false;
            node->enabled = enabled;
            prepare_menu_item_for_parent(ctx, *node);
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
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
            ctx->tab_build_state().stack.push_back(scope);
            return handle;
        }

        LUNA_GUI_API void end_tab_bar(IContext* context)
        {
            Context* ctx = context_from_interface(context);
            luassert(!ctx->tab_build_state().stack.empty());
            TabBuildScope scope = ctx->tab_build_state().stack.back();
            ctx->tab_build_state().stack.pop_back();
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
            luassert(!ctx->tab_build_state().stack.empty());
            Ref<TabItemNode> tab_node = new_object<TabItemNode>();
            tab_node->open = open;
            tab_node->flags = flags;
            ItemHandle handle = ctx->add_node(Ref<Node>(tab_node), label ? label : "", true);
            u32 index = (u32)ctx->m_build_desc.nodes.size() - 1;
            Node& node = ctx->m_build_desc.nodes[index];
            bool item_open = !open || *open;

            TabBuildScope& scope = ctx->tab_build_state().stack.back();
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
            TabItemNode* built_tab = tab_item_node(node);
            luassert(built_tab);
            built_tab->content_visible = visible;
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
                if(!tab_item_layout(node) || strcmp(node.text.c_str(), label ? label : "") != 0) continue;
                TabItemNode* tab = tab_item_node(node);
                if(tab && tab->open)
                {
                    *tab->open = false;
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

        LUNA_GUI_API ItemHandle progress_bar(IContext* context, const c8* label, f32 fraction, const Size& size, const c8* overlay)
        {
            Context* ctx = context_from_interface(context);
            Ref<ProgressBarNode> node = new_object<ProgressBarNode>();
            node->fraction = clamp(fraction, 0.0f, 1.0f);
            if(overlay)
            {
                node->overlay = overlay;
                node->has_overlay = true;
            }
            apply_requested_size(*node, size);
            return ctx->add_node(Ref<Node>(node), label ? label : "ProgressBar", false);
        }

        static bool has_widget_label(const c8* label)
        {
            return label && label[0];
        }

        static ItemHandle add_label_text_node(Context* ctx, const c8* label, f32 font_size)
        {
            Ref<TextNode> node = new_object<TextNode>();
            node->font_size = font_size;
            return ctx->add_node(Ref<Node>(node), label ? label : "", false);
        }

        static ItemHandle add_labeled_widget(Context* ctx, Ref<Node> node, const c8* label, f32 font_size = 16.0f)
        {
            ItemHandle handle;
            ctx->begin_container(move(node), label ? label : "", Size(), &handle);
            bool enabled = ctx->m_build_desc.nodes.back().enabled_state();
            ctx->push_enabled(enabled);
            add_label_text_node(ctx, label, font_size);
            ctx->pop_enabled();
            ctx->end_container();
            ctx->m_last_item_id = handle.id;
            return handle;
        }

        LUNA_GUI_API ItemHandle selectable(IContext* context, const c8* label, bool selected)
        {
            Context* ctx = context_from_interface(context);
            Ref<SelectableNode> node = new_object<SelectableNode>();
            node->selected = selected;
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label, 15.0f);
            }
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
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label);
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool selected)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->selected = selected;
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label);
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, bool* value)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->value = value;
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label);
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle radio_button(IContext* context, const c8* label, i32* value, i32 button_value)
        {
            Context* ctx = context_from_interface(context);
            Ref<RadioButtonNode> node = new_object<RadioButtonNode>();
            node->i32_value = value;
            node->item_value = button_value;
            node->selected = value && *value == button_value;
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label);
            }
            return ctx->add_node(Ref<Node>(node), label ? label : "", true);
        }

        LUNA_GUI_API ItemHandle toggle_switch(IContext* context, const c8* label, bool* value)
        {
            Context* ctx = context_from_interface(context);
            Ref<ToggleSwitchNode> node = new_object<ToggleSwitchNode>();
            node->value = value;
            if(has_widget_label(label))
            {
                node->label_layout = true;
                return add_labeled_widget(ctx, Ref<Node>(node), label);
            }
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

        ItemHandle add_slider_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
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

        ItemHandle add_slider_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
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

        ItemHandle add_input_float_node(IContext* context, const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
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

        ItemHandle add_input_int_node(IContext* context, const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
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

        LUNA_GUI_API bool is_key_down(IContext* context, KeyCode key)
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
