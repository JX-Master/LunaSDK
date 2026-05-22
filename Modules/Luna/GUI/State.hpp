/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file State.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace GUIState
        {
            LUNA_GUI_API GUIStateKey<bool> clicked();
            LUNA_GUI_API GUIStateKey<bool> double_clicked();
            LUNA_GUI_API GUIStateKey<bool> hovered();
            LUNA_GUI_API GUIStateKey<bool> active();
            LUNA_GUI_API GUIStateKey<bool> focused();
            LUNA_GUI_API GUIStateKey<bool> open();
            LUNA_GUI_API GUIStateKey<bool> value_changed();
            LUNA_GUI_API GUIStateKey<RectF> rect();
            LUNA_GUI_API GUIStateKey<RectF> clip_rect();
        }

        LUNA_GUI_API const Any* get_item_state_any(GUIItemHandle handle, const Name& key);
        LUNA_GUI_API void set_item_state_any(GUIItemHandle handle, const Name& key, const Any& value);
        LUNA_GUI_API void remove_item_state(GUIItemHandle handle, const Name& key);

        template <typename _Ty>
        _Ty GetItemState(GUIItemHandle handle, const GUIStateKey<_Ty>& key)
        {
            const Any* value = get_item_state_any(handle, key.name);
            if(!value) return key.default_value;
            const _Ty* typed_value = value->as<_Ty>();
            return typed_value ? *typed_value : key.default_value;
        }

        template <typename _Ty>
        void SetItemState(GUIItemHandle handle, const GUIStateKey<_Ty>& key, const _Ty& value)
        {
            set_item_state_any(handle, key.name, Any(value));
        }

        LUNA_GUI_API bool IsItemClicked(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemDoubleClicked(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemHovered(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemActive(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemFocused(GUIItemHandle handle);
    }
}
