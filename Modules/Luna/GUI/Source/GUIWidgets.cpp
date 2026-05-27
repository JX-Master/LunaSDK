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
        LUNA_GUI_API void PushID(u64 id)
        {
            require_current_context()->push_id(id);
        }

        LUNA_GUI_API void PushID(const void* ptr)
        {
            require_current_context()->push_id((u64)(usize)ptr);
        }

        LUNA_GUI_API void PushID(const c8* str)
        {
            u64 h = hash_cstr(str ? str : "", FNV_OFFSET);
            require_current_context()->push_id(h);
        }

        LUNA_GUI_API void PopID()
        {
            require_current_context()->pop_id();
        }

        LUNA_GUI_API void PushClipRect(const RectF& rect)
        {
            require_current_context()->push_clip_rect(rect);
        }

        LUNA_GUI_API void PopClipRect()
        {
            require_current_context()->pop_clip_rect();
        }

        LUNA_GUI_API void TreePush()
        {
            require_current_context()->tree_push();
        }

        LUNA_GUI_API void TreePop()
        {
            require_current_context()->tree_pop();
        }

        LUNA_GUI_API bool BeginDragDropSource(GUIItemHandle source, const Name& payload_type)
        {
            return require_current_context()->begin_drag_drop_source(source, payload_type);
        }

        LUNA_GUI_API void SetDragDropPayload(const void* data, usize data_size)
        {
            require_current_context()->set_drag_drop_payload(data, data_size);
        }

        LUNA_GUI_API void EndDragDropSource()
        {
            require_current_context()->end_drag_drop_source();
        }

        LUNA_GUI_API bool BeginDragDropTarget(GUIItemHandle target, const Name& payload_type)
        {
            return require_current_context()->begin_drag_drop_target(target, payload_type);
        }

        LUNA_GUI_API const GUIDragDropPayload* AcceptDragDropPayload(const Name& payload_type)
        {
            return require_current_context()->accept_drag_drop_payload(payload_type);
        }

        LUNA_GUI_API const GUIDragDropPayload* AcceptDragDropPayload(GUIItemHandle target, const Name& payload_type)
        {
            return require_current_context()->accept_drag_drop_payload(target, payload_type);
        }

        LUNA_GUI_API void EndDragDropTarget()
        {
            require_current_context()->end_drag_drop_target();
        }

        LUNA_GUI_API bool IsDragDropActive()
        {
            return require_current_context()->is_drag_drop_active();
        }

        LUNA_GUI_API const GUIDragDropPayload* GetDragDropPayload()
        {
            return require_current_context()->get_drag_drop_payload();
        }

        LUNA_GUI_API void SetNextItemLayout(const GUILayoutStyle& style)
        {
            require_current_context()->set_next_item_layout(style);
        }

        LUNA_GUI_API void SetNextDockPanelStyle(const GUIDockPanelStyle& style, bool* open)
        {
            require_current_context()->set_next_dock_panel_style(style, open);
        }

        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::h_layout, label ? label : "HLayout", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label, const RectF& rect, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::h_layout, label ? label : "HLayout", GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.layout_desc = desc;
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void EndHLayout()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginVLayout(const c8* label, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::v_layout, label ? label : "VLayout", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle BeginVLayout(const c8* label, const RectF& rect, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::v_layout, label ? label : "VLayout", GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.layout_desc = desc;
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void EndVLayout()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginTableLayout(const c8* label, const GUITableDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::table_layout, label ? label : "TableLayout", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().table_desc = desc;
            return handle;
        }

        LUNA_GUI_API void EndTableLayout()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API void SetNextTableCellColor(const Float4U& color)
        {
            require_current_context()->set_next_table_cell_color(color);
        }

        LUNA_GUI_API GUIItemHandle BeginGridLayout(const c8* label, const GUIGridLayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::grid_layout, label ? label : "GridLayout", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().grid_desc = desc;
            return handle;
        }

        LUNA_GUI_API void EndGridLayout()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginDockSpace(const c8* label, const GUISize& size)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::dock_space, label ? label : "DockSpace", size, &handle);
            return handle;
        }

        LUNA_GUI_API void EndDockSpace()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginDockPanel(const c8* label, bool* open, const GUIDockPanelStyle& style, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->set_next_dock_panel_style(style, open);
            ctx->begin_container(GUINodeKind::v_layout, label ? label : "DockPanel", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
            return handle;
        }

        LUNA_GUI_API void EndDockPanel()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginScrollView(const c8* label, const GUISize& size)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::scroll_view, label ? label : "ScrollView", size, &handle);
            return handle;
        }

        LUNA_GUI_API void EndScrollView()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, const GUISize& size)
        {
            GUIItemHandle handle;
            require_current_context()->begin_container(GUINodeKind::window, label ? label : "Window", size, &handle);
            return handle;
        }

        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, bool* open, const GUISize& size)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::window, label ? label : "Window", size, &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.bool_value = open;
            node.interactive = open != nullptr;
            return handle;
        }

        LUNA_GUI_API void EndWindow()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginPopup(const c8* label, const Float2U& position, const GUISize& size)
        {
            GUIPopupDesc desc;
            desc.position = position;
            desc.size = size;
            desc.flags = GUIPopupFlag::none;
            return require_current_context()->begin_popup(label, desc);
        }

        LUNA_GUI_API GUIItemHandle BeginPopup(const c8* label, const GUIPopupDesc& desc)
        {
            return require_current_context()->begin_popup(label, desc);
        }

        LUNA_GUI_API void EndPopup()
        {
            require_current_context()->end_popup();
        }

        LUNA_GUI_API void OpenPopup(GUIItemHandle popup)
        {
            require_current_context()->open_popup(popup);
        }

        LUNA_GUI_API void ClosePopup(GUIItemHandle popup)
        {
            require_current_context()->close_popup(popup);
        }

        LUNA_GUI_API void CloseCurrentPopup()
        {
            require_current_context()->close_current_popup();
        }

        LUNA_GUI_API void CloseAllPopups()
        {
            require_current_context()->close_all_popups();
        }

        LUNA_GUI_API bool IsPopupOpen(GUIItemHandle popup)
        {
            return require_current_context()->is_popup_open(popup);
        }

        LUNA_GUI_API GUIItemHandle BeginMenuBar(const c8* label, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::menu_bar, label ? label : "MenuBar", GUISize(), &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            GUILayoutDesc default_desc;
            if(desc.padding.left == 0.0f && desc.padding.top == 0.0f && desc.padding.right == 0.0f && desc.padding.bottom == 0.0f &&
                desc.gap == default_desc.gap &&
                desc.main_axis_alignment == default_desc.main_axis_alignment &&
                desc.cross_axis_alignment == default_desc.cross_axis_alignment)
            {
                node.layout_desc.padding = GUIEdgeInsets::xy(4.0f, 2.0f);
                node.layout_desc.gap = 2.0f;
                node.layout_desc.cross_axis_alignment = GUILayoutCrossAxisAlignment::center;
            }
            else
            {
                node.layout_desc = desc;
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle BeginMenuBar(const c8* label, const RectF& rect, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::menu_bar, label ? label : "MenuBar", GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)), &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            GUILayoutDesc default_desc;
            if(desc.padding.left == 0.0f && desc.padding.top == 0.0f && desc.padding.right == 0.0f && desc.padding.bottom == 0.0f &&
                desc.gap == default_desc.gap &&
                desc.main_axis_alignment == default_desc.main_axis_alignment &&
                desc.cross_axis_alignment == default_desc.cross_axis_alignment)
            {
                node.layout_desc.padding = GUIEdgeInsets::xy(4.0f, 2.0f);
                node.layout_desc.gap = 2.0f;
                node.layout_desc.cross_axis_alignment = GUILayoutCrossAxisAlignment::center;
            }
            else
            {
                node.layout_desc = desc;
            }
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            return handle;
        }

        LUNA_GUI_API void EndMenuBar()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle BeginMenu(const c8* label, bool enabled)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::menu, label ? label : "", enabled);
            u32 menu_index = (u32)ctx->m_build_desc.nodes.size() - 1;
            GUINode& menu_node = ctx->m_build_desc.nodes[menu_index];
            menu_node.enabled = enabled;

            GUIPopupDesc popup_desc;
            popup_desc.flags = GUIPopupFlag::managed | GUIPopupFlag::close_on_outside_click | GUIPopupFlag::close_on_escape | GUIPopupFlag::close_on_blur;
            GUIItemHandle popup = ctx->begin_popup("##MenuPopup", popup_desc);
            GUINode& popup_node = ctx->m_build_desc.nodes.back();
            popup_node.popup_owner_id = handle.id;
            popup_node.layout_desc.padding = GUIEdgeInsets::xy(6.0f, 5.0f);
            popup_node.layout_desc.gap = 1.0f;
            popup_node.layout_desc.cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;
            ctx->m_build_desc.nodes[menu_index].menu_popup_id = popup.id;
            return handle;
        }

        LUNA_GUI_API void EndMenu()
        {
            require_current_context()->end_popup();
        }

        LUNA_GUI_API GUIItemHandle MenuItem(const c8* label, const c8* shortcut, bool selected, bool enabled)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::menu_item, label ? label : "", enabled);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.shortcut = shortcut ? shortcut : "";
            node.selected = selected;
            node.enabled = enabled;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle MenuItem(const c8* label, const c8* shortcut, bool* selected, bool enabled)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::menu_item, label ? label : "", enabled);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.shortcut = shortcut ? shortcut : "";
            node.bool_value = selected;
            node.selected = selected ? *selected : false;
            node.enabled = enabled;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle MenuSeparator()
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::menu_separator, "##MenuSeparator", false);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.enabled = false;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle BeginTabBar(const c8* label, GUITabBarFlag flags)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::tab_bar, label ? label : "TabBar", GUISize(), &handle);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.tab_bar_flags = flags;

            PersistentItemState& state = ctx->get_or_create_persistent_state(handle.id);
            TabBuildScope scope;
            scope.tab_bar_id = handle.id;
            scope.selected_id = state.tab_selected_id;
            scope.flags = flags;
            scope.had_existing_tabs = !state.tab_order.empty();
            ctx->m_tab_build_stack.push_back(scope);
            return handle;
        }

        LUNA_GUI_API void EndTabBar()
        {
            GUIContext* ctx = require_current_context();
            luassert(!ctx->m_tab_build_stack.empty());
            TabBuildScope scope = ctx->m_tab_build_stack.back();
            ctx->m_tab_build_stack.pop_back();
            if(!scope.visible_tab_chosen && scope.first_open_id)
            {
                PersistentItemState& state = ctx->get_or_create_persistent_state(scope.tab_bar_id);
                state.tab_selected_id = scope.first_open_id;
            }
            ctx->end_container();
        }

        LUNA_GUI_API bool BeginTabItem(const c8* label, bool* open, GUITabItemFlag flags)
        {
            GUIContext* ctx = require_current_context();
            luassert(!ctx->m_tab_build_stack.empty());
            GUIItemHandle handle = ctx->add_node(GUINodeKind::tab_item, label ? label : "", true);
            u32 index = (u32)ctx->m_build_desc.nodes.size() - 1;
            GUINode& node = ctx->m_build_desc.nodes[index];
            node.bool_value = open;
            node.tab_item_flags = flags;
            bool item_open = !open || *open;

            TabBuildScope& scope = ctx->m_tab_build_stack.back();
            if(item_open && !scope.first_open_id)
            {
                scope.first_open_id = handle.id;
            }
            PersistentItemState& bar_state = ctx->get_or_create_persistent_state(scope.tab_bar_id);
            bool auto_select_new = item_open &&
                test_flags(scope.flags, GUITabBarFlag::auto_select_new_tabs) &&
                scope.had_existing_tabs &&
                !tab_order_contains(bar_state, handle.id) &&
                !test_flags(flags, GUITabItemFlag::button);
            if(item_open && (test_flags(flags, GUITabItemFlag::selected) || auto_select_new) &&
                !test_flags(flags, GUITabItemFlag::button))
            {
                scope.selected_id = handle.id;
                bar_state.tab_selected_id = handle.id;
            }
            bool explicit_selected = item_open && scope.selected_id == handle.id;
            bool visible = item_open && !test_flags(flags, GUITabItemFlag::button) &&
                ((scope.selected_id && scope.selected_id == handle.id) ||
                    (!scope.selected_id && !scope.visible_tab_chosen) ||
                    explicit_selected);
            node.selected = visible;
            if(visible)
            {
                scope.visible_tab_chosen = true;
                ctx->m_parent_stack.push_back(index);
                ctx->m_id_stack.push_back(handle.id);
            }
            return visible;
        }

        LUNA_GUI_API void EndTabItem()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle TabItemButton(const c8* label, GUITabItemFlag flags)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::tab_item, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.tab_item_flags = (GUITabItemFlag)((u32)flags | (u32)GUITabItemFlag::button);
            return handle;
        }

        LUNA_GUI_API void SetTabItemClosed(const c8* label)
        {
            GUIContext* ctx = require_current_context();
            if(ctx->m_parent_stack.empty()) return;
            u32 parent = ctx->m_parent_stack.back();
            if(parent >= ctx->m_build_desc.nodes.size()) return;
            for(u32 child = ctx->m_build_desc.nodes[parent].first_child; child != U32_MAX; child = ctx->m_build_desc.nodes[child].next_sibling)
            {
                GUINode& node = ctx->m_build_desc.nodes[child];
                if(node.kind != GUINodeKind::tab_item || strcmp(node.text.c_str(), label ? label : "") != 0) continue;
                if(node.bool_value)
                {
                    *node.bool_value = false;
                }
                break;
            }
        }

        LUNA_GUI_API GUIItemHandle Button(const c8* label)
        {
            return require_current_context()->add_node(GUINodeKind::button, label ? label : "", true);
        }

        LUNA_GUI_API GUIItemHandle Button(const c8* label, const RectF& rect)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::button, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            apply_requested_size(node, GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Selectable(const c8* label, bool selected)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::selectable, label ? label : "", true);
            ctx->m_build_desc.nodes.back().selected = selected;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Text(const c8* text)
        {
            return require_current_context()->add_node(GUINodeKind::text, text ? text : "", false);
        }

        LUNA_GUI_API GUIItemHandle Checkbox(const c8* label, bool* value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::checkbox, label ? label : "", true);
            ctx->m_build_desc.nodes.back().bool_value = value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle RadioButton(const c8* label, bool selected)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::radio_button, label ? label : "", true);
            ctx->m_build_desc.nodes.back().selected = selected;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle RadioButton(const c8* label, bool* value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::radio_button, label ? label : "", true);
            ctx->m_build_desc.nodes.back().bool_value = value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle RadioButton(const c8* label, i32* value, i32 button_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::radio_button, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = value;
            node.item_value = button_value;
            node.selected = value && *value == button_value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Switch(const c8* label, bool* value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::toggle_switch, label ? label : "", true);
            ctx->m_build_desc.nodes.back().bool_value = value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle InputText(const c8* label, String& value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::input_text, label ? label : "", true);
            ctx->m_build_desc.nodes.back().string_value = &value;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Image(RHI::ITexture* texture, const GUISize& size)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::image, "Image", false);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.texture = texture;
            apply_requested_size(node, size);
            return handle;
        }

        LUNA_GUI_API GUIItemHandle CollapsingHeader(const c8* label)
        {
            return require_current_context()->add_node(GUINodeKind::collapsing_header, label ? label : "", true);
        }

        LUNA_GUI_API GUIItemHandle TreeNode(const c8* label, GUITreeNodeFlag flags)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::tree_node, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.tree_flags = flags;
            node.tree_depth = ctx->m_tree_depth;
            node.selected = test_flags(flags, GUITreeNodeFlag::selected);
            return handle;
        }

        LUNA_GUI_API GUIItemHandle Combo(const c8* label, i32* current_item, Span<const c8*> items)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::combo, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = current_item;
            node.items.reserve(items.size());
            for(const c8* item : items)
            {
                node.items.push_back(item ? item : "");
            }
            if(current_item && !node.items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)node.items.size() - 1);
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle ButtonGroup(const c8* label, i32* current_item, Span<const c8*> items)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::button_group, label ? label : "ButtonGroup", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = current_item;
            node.items.reserve(items.size());
            for(const c8* item : items)
            {
                node.items.push_back(item ? item : "");
            }
            if(current_item && !node.items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)node.items.size() - 1);
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle ButtonGroup(const c8* label, Span<bool> selected, Span<const c8*> items)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::button_group, label ? label : "ButtonGroup", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.bool_value = selected.data();
            usize count = min(selected.size(), items.size());
            node.items.reserve(count);
            for(usize i = 0; i < count; ++i)
            {
                node.items.push_back(items[i] ? items[i] : "");
            }
            return handle;
        }

        static GUIItemHandle add_slider_float_node(const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::slider_float, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.f32_value = value;
            node.f32_value_count = count;
            node.min_value = min_value;
            node.max_value = max_value;
            if(value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle SliderFloat(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat2(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat3(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat4(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_node(label, value, 4, min_value, max_value);
        }

        static GUIItemHandle add_slider_int_node(const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::slider_int, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = value;
            node.i32_value_count = count;
            node.min_value = (f32)min_value;
            node.max_value = (f32)max_value;
            if(value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle SliderInt(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt2(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt3(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt4(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_node(label, value, 4, min_value, max_value);
        }

        static GUIItemHandle add_input_float_node(const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::input_float, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.f32_value = value;
            node.f32_value_count = count;
            node.min_value = min_value;
            node.max_value = max_value;
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        static GUIItemHandle add_input_int_node(const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::input_int, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = value;
            node.i32_value_count = count;
            node.min_value = (f32)min_value;
            node.max_value = (f32)max_value;
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        static GUIItemHandle add_slider_float_with_input_component(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            GUILayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;
            GUIItemHandle handle = BeginHLayout(label ? label : "SliderFloatWithInput", row);
            SetNextItemLayout(GUILayoutStyle::fill());
            add_slider_float_node(label, value, 1, min_value, max_value);
            SetNextItemLayout(GUILayoutStyle::fixed_width(72.0f));
            add_input_float_node("", value, 1, min_value, max_value);
            EndHLayout();
            return handle;
        }

        static GUIItemHandle add_slider_int_with_input_component(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            GUILayoutDesc row;
            row.gap = 8.0f;
            row.cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;
            GUIItemHandle handle = BeginHLayout(label ? label : "SliderIntWithInput", row);
            SetNextItemLayout(GUILayoutStyle::fill());
            add_slider_int_node(label, value, 1, min_value, max_value);
            SetNextItemLayout(GUILayoutStyle::fixed_width(72.0f));
            add_input_int_node("", value, 1, min_value, max_value);
            EndHLayout();
            return handle;
        }

        static GUIItemHandle add_slider_float_with_input_node(const c8* label, f32* value, u8 count, f32 min_value, f32 max_value)
        {
            if(count <= 1) return add_slider_float_with_input_component(label, value, min_value, max_value);
            GUILayoutDesc column;
            column.gap = 4.0f;
            GUIItemHandle handle = BeginVLayout(label ? label : "SliderFloatWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                PushID(i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_float_with_input_component(component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                PopID();
            }
            EndVLayout();
            return handle;
        }

        static GUIItemHandle add_slider_int_with_input_node(const c8* label, i32* value, u8 count, i32 min_value, i32 max_value)
        {
            if(count <= 1) return add_slider_int_with_input_component(label, value, min_value, max_value);
            GUILayoutDesc column;
            column.gap = 4.0f;
            GUIItemHandle handle = BeginVLayout(label ? label : "SliderIntWithInput", column);
            const c8* components[] = { "X", "Y", "Z", "W" };
            for(u32 i = 0; i < count; ++i)
            {
                PushID(i);
                String component_label;
                if(label && label[0]) strprintf(component_label, "%s %s", label, components[i]);
                else component_label = components[i];
                add_slider_int_with_input_component(component_label.c_str(), value ? value + i : nullptr, min_value, max_value);
                PopID();
            }
            EndVLayout();
            return handle;
        }

        LUNA_GUI_API GUIItemHandle SliderFloatWithInput(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat2WithInput(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat3WithInput(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderFloat4WithInput(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            return add_slider_float_with_input_node(label, value, 4, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderIntWithInput(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(label, value, 1, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt2WithInput(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(label, value, 2, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt3WithInput(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(label, value, 3, min_value, max_value);
        }

        LUNA_GUI_API GUIItemHandle SliderInt4WithInput(const c8* label, i32* value, i32 min_value, i32 max_value)
        {
            return add_slider_int_with_input_node(label, value, 4, min_value, max_value);
        }

        static GUIItemHandle add_drag_float_node(const c8* label, f32* value, u8 count, f32 speed, f32 min_value, f32 max_value, bool color, GUINumericEditFlag flags)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::drag_float, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.f32_value = value;
            node.f32_value_count = count;
            node.f32_color = color;
            node.numeric_flags = flags;
            node.min_value = min_value;
            node.max_value = max_value;
            node.step_value = speed;
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DragFloat(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_float_node(label, value, 1, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API GUIItemHandle DragFloat2(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_float_node(label, value, 2, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API GUIItemHandle DragFloat3(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_float_node(label, value, 3, speed, min_value, max_value, false, flags);
        }

        LUNA_GUI_API GUIItemHandle DragFloat4(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_float_node(label, value, 4, speed, min_value, max_value, false, flags);
        }

        static GUIItemHandle add_drag_int_node(const c8* label, i32* value, u8 count, f32 speed, i32 min_value, i32 max_value, GUINumericEditFlag flags)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::drag_int, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.i32_value = value;
            node.i32_value_count = count;
            node.numeric_flags = flags;
            node.min_value = (f32)min_value;
            node.max_value = (f32)max_value;
            node.step_value = speed;
            if(value && max_value > min_value)
            {
                for(u32 i = 0; i < count; ++i)
                {
                    value[i] = clamp(value[i], min_value, max_value);
                }
            }
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DragInt(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_int_node(label, value, 1, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API GUIItemHandle DragInt2(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_int_node(label, value, 2, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API GUIItemHandle DragInt3(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_int_node(label, value, 3, speed, min_value, max_value, flags);
        }

        LUNA_GUI_API GUIItemHandle DragInt4(const c8* label, i32* value, f32 speed, i32 min_value, i32 max_value, GUINumericEditFlag flags)
        {
            return add_drag_int_node(label, value, 4, speed, min_value, max_value, flags);
        }

        static void sync_color_edit_build_state(PersistentItemState& state, const Float4U& color)
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

        static void assign_color_binding(GUINode& node, f32* f32_value, u8* u8_value, u32* u32_value, GUIColorValueType type, u8 count)
        {
            node.f32_value = f32_value;
            node.u8_value = u8_value;
            node.u32_value = u32_value;
            node.color_value_type = type;
            node.f32_value_count = count;
        }

        static GUIItemHandle add_color_picker_node(const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, GUIColorValueType type, u8 count, GUIID owner_id)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::color_picker, label ? label : "ColorPicker", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            assign_color_binding(node, f32_value, u8_value, u32_value, type, count);
            node.color_owner_id = owner_id;
            return handle;
        }

        static void tag_color_numeric_node(GUIContext* ctx, GUIItemHandle handle, GUIID owner_id, GUIColorEditPart part)
        {
            if(GUINode* node = ctx->find_build_node(handle))
            {
                node->color_owner_id = owner_id;
                node->color_edit_part = part;
            }
        }

        static GUIItemHandle add_color_channel_drag(const c8* label, i32* value, GUIID owner_id, GUIColorEditPart part)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = DragInt(label, value, 1.0f, 0, 255, GUINumericEditFlag::input_on_double_click);
            tag_color_numeric_node(ctx, handle, owner_id, part);
            return handle;
        }

        static GUIItemHandle add_color_edit_node(const c8* label, f32* f32_value, u8* u8_value, u32* u32_value, GUIColorValueType type, u8 count)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::color_edit, label ? label : "", true);
            u32 color_index = (u32)ctx->m_build_desc.nodes.size() - 1;
            GUINode& node = ctx->m_build_desc.nodes[color_index];
            assign_color_binding(node, f32_value, u8_value, u32_value, type, count);
            write_color_value(node, read_color_value(node));

            PersistentItemState& color_state = ctx->get_or_create_persistent_state(handle.id);
            color_edit_axis_ref(color_state) = clamp(color_edit_axis_ref(color_state), 0, 5);
            ensure_color_edit_state_channels(color_state);
            sync_color_edit_build_state(color_state, read_color_value(node));

            GUIPopupDesc popup_desc;
            popup_desc.size = GUISize::fixed(476.0f, count == 4 ? 470.0f : 432.0f);
            popup_desc.flags = GUIPopupFlag::managed | GUIPopupFlag::close_on_outside_click | GUIPopupFlag::close_on_escape | GUIPopupFlag::close_on_blur;
            ctx->push_id(handle.id);
            GUIItemHandle popup = ctx->begin_popup("##ColorEditPopup", popup_desc);
            GUINode& popup_node = ctx->m_build_desc.nodes.back();
            popup_node.popup_owner_id = handle.id;
            popup_node.layout_desc.padding = GUIEdgeInsets::all(10.0f);
            popup_node.layout_desc.gap = 8.0f;
            popup_node.layout_desc.cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;

            SetNextItemLayout(GUILayoutStyle::fixed_height(300.0f));
            add_color_picker_node("##ColorPicker", f32_value, u8_value, u32_value, type, count, handle.id);

            const c8* axis_items[] = { "H", "S", "V", "R", "G", "B" };
            SetNextItemLayout(GUILayoutStyle::fixed_height(28.0f));
            ButtonGroup("Channel", &color_edit_axis_ref(color_state), Span<const c8*>(axis_items, 6));

            GUILayoutDesc row;
            row.gap = 6.0f;
            row.cross_axis_alignment = GUILayoutCrossAxisAlignment::stretch;
            SetNextItemLayout(GUILayoutStyle::fixed_height(30.0f));
            BeginHLayout("RGB", row);
            add_color_channel_drag("R", &color_state.color_edit_rgb[0], handle.id, GUIColorEditPart::rgb);
            add_color_channel_drag("G", &color_state.color_edit_rgb[1], handle.id, GUIColorEditPart::rgb);
            add_color_channel_drag("B", &color_state.color_edit_rgb[2], handle.id, GUIColorEditPart::rgb);
            EndHLayout();

            SetNextItemLayout(GUILayoutStyle::fixed_height(30.0f));
            BeginHLayout("HSV", row);
            add_color_channel_drag("H", &color_state.color_edit_hsv[0], handle.id, GUIColorEditPart::hsv);
            add_color_channel_drag("S", &color_state.color_edit_hsv[1], handle.id, GUIColorEditPart::hsv);
            add_color_channel_drag("V", &color_state.color_edit_hsv[2], handle.id, GUIColorEditPart::hsv);
            EndHLayout();

            if(count == 4)
            {
                SetNextItemLayout(GUILayoutStyle::fixed_height(30.0f));
                BeginHLayout("Alpha", row);
                add_color_channel_drag("A", &color_state.color_edit_rgb[3], handle.id, GUIColorEditPart::rgb);
                EndHLayout();
            }

            ctx->end_popup();
            ctx->pop_id();
            ctx->m_build_desc.nodes[color_index].menu_popup_id = popup.id;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle ColorEdit3(const c8* label, f32* value)
        {
            return add_color_edit_node(label, value, nullptr, nullptr, GUIColorValueType::f32, 3);
        }

        LUNA_GUI_API GUIItemHandle ColorEdit4(const c8* label, f32* value)
        {
            return add_color_edit_node(label, value, nullptr, nullptr, GUIColorValueType::f32, 4);
        }

        LUNA_GUI_API GUIItemHandle ColorEdit3(const c8* label, u8* value)
        {
            return add_color_edit_node(label, nullptr, value, nullptr, GUIColorValueType::u8, 3);
        }

        LUNA_GUI_API GUIItemHandle ColorEdit4(const c8* label, u8* value)
        {
            return add_color_edit_node(label, nullptr, value, nullptr, GUIColorValueType::u8, 4);
        }

        LUNA_GUI_API GUIItemHandle ColorEdit3(const c8* label, u32* value)
        {
            return add_color_edit_node(label, nullptr, nullptr, value, GUIColorValueType::rgba8, 3);
        }

        LUNA_GUI_API GUIItemHandle ColorEdit4(const c8* label, u32* value)
        {
            return add_color_edit_node(label, nullptr, nullptr, value, GUIColorValueType::rgba8, 4);
        }

        LUNA_GUI_API GUIItemHandle HitBox(const c8* label, const RectF& rect)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::hit_box, label ? label : "HitBox", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            apply_requested_size(node, GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            return handle;
        }

        LUNA_GUI_API const Any* get_item_state_any(GUIItemHandle handle, const Name& key)
        {
            if(!handle.context) return nullptr;
            GUIContext* ctx = (GUIContext*)handle.context;
            return ctx->get_state(handle, key);
        }

        LUNA_GUI_API void set_item_state_any(GUIItemHandle handle, const Name& key, const Any& value)
        {
            if(!handle.context) return;
            GUIContext* ctx = (GUIContext*)handle.context;
            ctx->set_state(handle, key, value);
        }

        LUNA_GUI_API void remove_item_state(GUIItemHandle handle, const Name& key)
        {
            if(!handle.context) return;
            GUIContext* ctx = (GUIContext*)handle.context;
            ctx->remove_state(handle, key);
        }

        LUNA_GUI_API bool IsItemClicked(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::clicked());
        }

        LUNA_GUI_API bool IsItemRightClicked(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::right_clicked());
        }

        LUNA_GUI_API bool IsItemDoubleClicked(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::double_clicked());
        }

        LUNA_GUI_API bool IsItemHovered(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::hovered());
        }

        LUNA_GUI_API bool IsItemActive(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::active());
        }

        LUNA_GUI_API bool IsItemFocused(GUIItemHandle handle)
        {
            return GetItemState(handle, GUIState::focused());
        }

        LUNA_GUI_API Float2U GetPointerPosition()
        {
            return require_current_context()->m_pointer_pos;
        }

        static GUIItemHandle add_draw_node(GUINodeKind kind, const c8* label, const RectF& rect, const Float4U& color)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(kind, label ? label : "", false);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.absolute_position = true;
            node.position = Float2U(rect.offset_x, rect.offset_y);
            node.paint_rect = rect;
            node.paint_color = color;
            apply_requested_size(node, GUI::GUISize::fixed(max(rect.width, 1.0f), max(rect.height, 1.0f)));
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DrawRect(const RectF& rect, const Float4U& color, f32 radius)
        {
            GUIItemHandle handle = add_draw_node(GUINodeKind::draw_rect, "DrawRect", rect, color);
            require_current_context()->m_build_desc.nodes.back().paint_radius = radius;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DrawCircle(const Float2U& center, f32 radius, const Float4U& color)
        {
            f32 r = max(radius, 0.5f);
            RectF rect(center.x - r, center.y - r, r * 2.0f, r * 2.0f);
            GUIItemHandle handle = add_draw_node(GUINodeKind::draw_circle, "DrawCircle", rect, color);
            require_current_context()->m_build_desc.nodes.back().paint_radius = r;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DrawLine(const Float2U& begin, const Float2U& end, const Float4U& color, f32 width)
        {
            f32 line_width = max(width, 1.0f);
            f32 half_width = line_width * 0.5f;
            f32 min_x = min(begin.x, end.x) - half_width;
            f32 min_y = min(begin.y, end.y) - half_width;
            f32 max_x = max(begin.x, end.x) + half_width;
            f32 max_y = max(begin.y, end.y) + half_width;
            RectF rect(min_x, min_y, max(max_x - min_x, 1.0f), max(max_y - min_y, 1.0f));
            GUIItemHandle handle = add_draw_node(GUINodeKind::draw_line, "DrawLine", rect, color);
            GUINode& node = require_current_context()->m_build_desc.nodes.back();
            node.paint_line_begin = begin;
            node.paint_line_end = end;
            node.paint_line_width = line_width;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DrawText(const RectF& rect, const c8* text, const Float4U& color, f32 font_size,
            GUITextAlignment horizontal_alignment, GUITextAlignment vertical_alignment)
        {
            GUIItemHandle handle = add_draw_node(GUINodeKind::draw_text, text ? text : "", rect, color);
            GUINode& node = require_current_context()->m_build_desc.nodes.back();
            node.text = text ? text : "";
            node.paint_font_size = font_size;
            node.paint_horizontal_alignment = horizontal_alignment;
            node.paint_vertical_alignment = vertical_alignment;
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DrawImage(RHI::ITexture* texture, const RectF& rect, const Float4U& color)
        {
            GUIItemHandle handle = add_draw_node(GUINodeKind::draw_image, "DrawImage", rect, color);
            require_current_context()->m_build_desc.nodes.back().texture = texture;
            return handle;
        }
    }
}
