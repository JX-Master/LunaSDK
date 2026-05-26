/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Widgets.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Context.hpp"
#include "State.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void PushID(u64 id);
        LUNA_GUI_API void PushID(const void* ptr);
        LUNA_GUI_API void PushID(const c8* str);
        LUNA_GUI_API void PopID();
        LUNA_GUI_API void PushClipRect(const RectF& rect);
        LUNA_GUI_API void PopClipRect();
        LUNA_GUI_API void TreePush();
        LUNA_GUI_API void TreePop();

        LUNA_GUI_API void SetNextItemLayout(const GUILayoutStyle& style);
        LUNA_GUI_API void SetNextDockPanelStyle(const GUIDockPanelStyle& style, bool* open = nullptr);

        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label = nullptr, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label, const RectF& rect, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API void EndHLayout();
        LUNA_GUI_API GUIItemHandle BeginVLayout(const c8* label = nullptr, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API GUIItemHandle BeginVLayout(const c8* label, const RectF& rect, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API void EndVLayout();
        LUNA_GUI_API GUIItemHandle BeginTableLayout(const c8* label, const GUITableDesc& desc);
        LUNA_GUI_API void EndTableLayout();
        LUNA_GUI_API void SetNextTableCellColor(const Float4U& color);
        LUNA_GUI_API GUIItemHandle BeginDockSpace(const c8* label, const GUISize& size = GUISize());
        LUNA_GUI_API void EndDockSpace();
        LUNA_GUI_API GUIItemHandle BeginDockPanel(const c8* label, bool* open = nullptr, const GUIDockPanelStyle& style = GUIDockPanelStyle(), const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API void EndDockPanel();
        LUNA_GUI_API GUIItemHandle BeginScrollView(const c8* label, const GUISize& size);
        LUNA_GUI_API void EndScrollView();
        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, const GUISize& size = GUISize());
        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, bool* open, const GUISize& size = GUISize());
        LUNA_GUI_API void EndWindow();
        LUNA_GUI_API GUIItemHandle BeginPopup(const c8* label, const Float2U& position, const GUISize& size = GUISize());
        LUNA_GUI_API void EndPopup();
        LUNA_GUI_API GUIItemHandle BeginTabBar(const c8* label, GUITabBarFlag flags = GUITabBarFlag::fitting_shrink);
        LUNA_GUI_API void EndTabBar();
        LUNA_GUI_API bool BeginTabItem(const c8* label, bool* open = nullptr, GUITabItemFlag flags = GUITabItemFlag::none);
        LUNA_GUI_API void EndTabItem();
        LUNA_GUI_API GUIItemHandle TabItemButton(const c8* label, GUITabItemFlag flags = GUITabItemFlag::none);
        LUNA_GUI_API void SetTabItemClosed(const c8* label);

        LUNA_GUI_API GUIItemHandle Button(const c8* label);
        LUNA_GUI_API GUIItemHandle Button(const c8* label, const RectF& rect);
        LUNA_GUI_API GUIItemHandle Selectable(const c8* label, bool selected = false);
        LUNA_GUI_API GUIItemHandle Text(const c8* text);
        LUNA_GUI_API GUIItemHandle Checkbox(const c8* label, bool* value);
        LUNA_GUI_API GUIItemHandle Switch(const c8* label, bool* value);
        LUNA_GUI_API GUIItemHandle InputText(const c8* label, String& value);
        LUNA_GUI_API GUIItemHandle Image(RHI::ITexture* texture, const GUISize& size);
        LUNA_GUI_API GUIItemHandle CollapsingHeader(const c8* label);
        LUNA_GUI_API GUIItemHandle TreeNode(const c8* label, GUITreeNodeFlag flags = GUITreeNodeFlag::none);
        LUNA_GUI_API GUIItemHandle Combo(const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API GUIItemHandle SliderFloat(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle DragFloat(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle DragFloat2(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle DragFloat3(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle DragFloat4(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle ColorEdit3(const c8* label, f32* value);
        LUNA_GUI_API GUIItemHandle HitBox(const c8* label, const RectF& rect);
        LUNA_GUI_API Float2U GetPointerPosition();
        LUNA_GUI_API GUIItemHandle DrawRect(const RectF& rect, const Float4U& color, f32 radius = 0.0f);
        LUNA_GUI_API GUIItemHandle DrawCircle(const Float2U& center, f32 radius, const Float4U& color);
        LUNA_GUI_API GUIItemHandle DrawLine(const Float2U& begin, const Float2U& end, const Float4U& color, f32 width = 1.0f);
        LUNA_GUI_API GUIItemHandle DrawText(const RectF& rect, const c8* text, const Float4U& color = Float4U(1.0f), f32 font_size = 16.0f,
            GUITextAlignment horizontal_alignment = GUITextAlignment::begin,
            GUITextAlignment vertical_alignment = GUITextAlignment::center);
        LUNA_GUI_API GUIItemHandle DrawImage(RHI::ITexture* texture, const RectF& rect, const Float4U& color = Float4U(1.0f));
    }
}
