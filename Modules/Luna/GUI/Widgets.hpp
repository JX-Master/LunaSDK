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

        LUNA_GUI_API void SetNextItemLayout(const GUILayoutStyle& style);

        LUNA_GUI_API GUIItemHandle BeginHLayout(const c8* label = nullptr, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API void EndHLayout();
        LUNA_GUI_API GUIItemHandle BeginVLayout(const c8* label = nullptr, const GUILayoutDesc& desc = GUILayoutDesc());
        LUNA_GUI_API void EndVLayout();
        LUNA_GUI_API GUIItemHandle BeginTableLayout(const c8* label, const GUITableDesc& desc);
        LUNA_GUI_API void EndTableLayout();
        LUNA_GUI_API void SetNextTableCellColor(const Float4U& color);
        LUNA_GUI_API GUIItemHandle BeginScrollView(const c8* label, const GUISize& size);
        LUNA_GUI_API void EndScrollView();
        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, const GUISize& size = GUISize());
        LUNA_GUI_API void EndWindow();

        LUNA_GUI_API GUIItemHandle Button(const c8* label);
        LUNA_GUI_API GUIItemHandle Text(const c8* text);
        LUNA_GUI_API GUIItemHandle Checkbox(const c8* label, bool* value);
        LUNA_GUI_API GUIItemHandle InputText(const c8* label, String& value);
        LUNA_GUI_API GUIItemHandle Image(RHI::ITexture* texture, const GUISize& size);
        LUNA_GUI_API GUIItemHandle CollapsingHeader(const c8* label);
        LUNA_GUI_API GUIItemHandle Combo(const c8* label, i32* current_item, Span<const c8*> items);
        LUNA_GUI_API GUIItemHandle SliderFloat(const c8* label, f32* value, f32 min_value, f32 max_value);
        LUNA_GUI_API GUIItemHandle DragFloat(const c8* label, f32* value, f32 speed, f32 min_value, f32 max_value);
    }
}
