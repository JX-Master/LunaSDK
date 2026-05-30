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
        namespace State
        {
            LUNA_GUI_API StateKey<bool> clicked();
            LUNA_GUI_API StateKey<bool> right_clicked();
            LUNA_GUI_API StateKey<bool> double_clicked();
            LUNA_GUI_API StateKey<bool> hovered();
            LUNA_GUI_API StateKey<bool> active();
            LUNA_GUI_API StateKey<bool> focused();
            LUNA_GUI_API StateKey<bool> open();
            LUNA_GUI_API StateKey<bool> value_changed();
            LUNA_GUI_API StateKey<RectF> rect();
            LUNA_GUI_API StateKey<RectF> clip_rect();
        }

        LUNA_GUI_API const Any* get_item_state_any(ItemHandle handle, const Name& key);
        LUNA_GUI_API void set_item_state_any(ItemHandle handle, const Name& key, const Any& value);
        LUNA_GUI_API void remove_item_state(ItemHandle handle, const Name& key);

        template <typename T>
        T get_item_state(ItemHandle handle, const StateKey<T>& key)
        {
            const Any* value = get_item_state_any(handle, key.name);
            if(!value) return key.default_value;
            const T* typed_value = value->as<T>();
            return typed_value ? *typed_value : key.default_value;
        }

        template <typename T>
        void set_item_state(ItemHandle handle, const StateKey<T>& key, const T& value)
        {
            set_item_state_any(handle, key.name, Any(value));
        }

        LUNA_GUI_API bool is_item_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_right_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_double_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_hovered(ItemHandle handle);
        LUNA_GUI_API bool is_item_active(ItemHandle handle);
        LUNA_GUI_API bool is_item_focused(ItemHandle handle);
    }
}
