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

        LUNA_GUI_API void SetNextItemLayout(const GUILayoutStyle& style)
        {
            require_current_context()->set_next_item_layout(style);
        }

        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label, const GUILayoutDesc& desc)
        {
            GUIItemHandle handle;
            GUIContext* ctx = require_current_context();
            ctx->begin_container(GUINodeKind::h_layout, label ? label : "HLayout", GUISize(), &handle);
            ctx->m_build_desc.nodes.back().layout_desc = desc;
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

        LUNA_GUI_API void EndWindow()
        {
            require_current_context()->end_container();
        }

        LUNA_GUI_API GUIItemHandle Button(const c8* label)
        {
            return require_current_context()->add_node(GUINodeKind::button, label ? label : "", true);
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

        LUNA_GUI_API GUIItemHandle SliderFloat(const c8* label, f32* value, f32 min_value, f32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::slider_float, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.f32_value = value;
            node.min_value = min_value;
            node.max_value = max_value;
            if(value) *value = clamp(*value, min_value, max_value);
            return handle;
        }

        LUNA_GUI_API GUIItemHandle DragFloat(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value)
        {
            GUIContext* ctx = require_current_context();
            GUIItemHandle handle = ctx->add_node(GUINodeKind::drag_float, label ? label : "", true);
            GUINode& node = ctx->m_build_desc.nodes.back();
            node.f32_value = value;
            node.min_value = min_value;
            node.max_value = max_value;
            node.step_value = speed;
            if(value) *value = clamp(*value, min_value, max_value);
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
    }
}
