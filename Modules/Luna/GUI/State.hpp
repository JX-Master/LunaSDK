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
#include "Context.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! State object that stores public item query values for one item.
        //! @remark This is the state read by @ref get_item_state and the `is_item_*` helpers.
        struct ItemQueryState
        {
            lustruct("GUI::ItemQueryState", "{0F5DB6E4-C49A-4BDE-BDFF-E6A432146AFD}");
            //! Query values indexed by public state key name.
            HashMap<Name, Any> states;
        };

        //! General-purpose state bag available for custom widgets and views.
        struct CustomState
        {
            lustruct("GUI::CustomState", "{44E02645-6144-4A84-8736-204880C0E620}");
            //! Custom values indexed by user-defined names.
            HashMap<Name, Any> states;
        };

        //! Disclosure state used by collapsing headers and tree nodes.
        struct DisclosureState
        {
            lustruct("GUI::DisclosureState", "{62263BD7-3405-493F-8DDB-B3D089412ACA}");
            //! Whether the disclosure content is currently open.
            bool open = true;
            //! Whether the initial open state has been applied.
            bool open_initialized = false;
        };

        //! Visual animation state used by switch widgets.
        struct SwitchAnimationState
        {
            lustruct("GUI::SwitchAnimationState", "{C8CC1A97-56C5-49B4-962D-70C1A1A7CC9F}");
            //! Current switch knob animation value.
            f32 animation = 0.0f;
            //! Whether @ref animation has been initialized from current widget value.
            bool initialized = false;
        };

        //! Visual animation state used by buttons.
        struct ButtonAnimationState
        {
            lustruct("GUI::ButtonAnimationState", "{B00FEB21-FE08-459E-87BC-FD15468BB6B6}");
            //! Current animated button fill color.
            Float4U color = Float4U(0.18f, 0.28f, 0.45f, 1.0f);
            //! Whether @ref color has been initialized.
            bool initialized = false;
        };

        //! Visual animation state used by button groups.
        struct ButtonGroupAnimationState
        {
            lustruct("GUI::ButtonGroupAnimationState", "{2FB79430-D36D-42E1-BF3D-2FDD21D351AF}");
            //! Selection indicator animation value for single-selection groups.
            f32 selection_animation = 0.0f;
            //! Whether @ref selection_animation has been initialized.
            bool selection_animation_initialized = false;
            //! Per-item fill animation values for multi-selection groups.
            Vector<f32> item_animations;
        };

        //! Visual animation state used by tab bars.
        struct TabBarAnimationState
        {
            lustruct("GUI::TabBarAnimationState", "{68A87F19-D1C3-4496-AE46-87A2DFB61E3D}");
            //! Current animated selected tab header rectangle.
            RectF selection_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether @ref selection_rect has been initialized.
            bool selection_rect_initialized = false;
        };

        //! Gets a typed state object owned by the specified ID.
        //! @param[in] context The GUI context.
        //! @param[in] owner_id The state owner ID.
        //! @return Returns the typed state object, or `nullptr`.
        template <typename T>
        T* get_state(IContext* context, id_t owner_id)
        {
            if(!context) return nullptr;
            object_t obj = context->get_state(make_state_id<T>(owner_id));
            return obj ? cast_object<T>(obj) : nullptr;
        }

        //! Gets or creates a typed state object owned by the specified ID.
        //! @param[in] context The GUI context.
        //! @param[in] owner_id The state owner ID.
        //! @param[in] lifetime The lifetime used when creating or refreshing the state.
        //! @return Returns the typed state object.
        template <typename T>
        Ref<T> get_or_create_state(IContext* context, id_t owner_id, StateLifetime lifetime = StateLifetime::next_frame)
        {
            if(!context) return Ref<T>();
            id_t id = make_state_id<T>(owner_id);
            object_t existing = context->get_state(id);
            if(existing)
            {
                Ref<T> state;
                object_retain(existing);
                state.attach(existing);
                RV r = context->set_state(id, state.object(), lifetime);
                luassert_always(succeeded(r));
                return state;
            }
            Ref<T> state = new_object<T>();
            RV r = context->set_state(id, state.object(), lifetime);
            luassert_always(succeeded(r));
            return state;
        }

        //! Clears a typed state object owned by the specified ID.
        //! @param[in] context The GUI context.
        //! @param[in] owner_id The state owner ID.
        template <typename T>
        void clear_state(IContext* context, id_t owner_id)
        {
            if(context)
            {
                context->clear_state(make_state_id<T>(owner_id));
            }
        }

        //! Built-in public item query keys.
        namespace State
        {
            //! Gets the state key that reports whether the item was clicked this frame.
            LUNA_GUI_API StateKey<bool> clicked();
            //! Gets the state key that reports whether the item was right-clicked this frame.
            LUNA_GUI_API StateKey<bool> right_clicked();
            //! Gets the state key that reports whether the item was double-clicked this frame.
            LUNA_GUI_API StateKey<bool> double_clicked();
            //! Gets the state key that reports whether the item is hovered.
            LUNA_GUI_API StateKey<bool> hovered();
            //! Gets the state key that reports whether the item is active.
            LUNA_GUI_API StateKey<bool> active();
            //! Gets the state key that reports whether the item is focused.
            LUNA_GUI_API StateKey<bool> focused();
            //! Gets the state key that reports whether the item is enabled.
            LUNA_GUI_API StateKey<bool> enabled();
            //! Gets the state key that reports whether the item is open.
            LUNA_GUI_API StateKey<bool> open();
            //! Gets the state key that reports whether the item value changed this frame.
            LUNA_GUI_API StateKey<bool> value_changed();
            //! Gets the state key that reports the item rectangle in layer coordinates.
            LUNA_GUI_API StateKey<RectF> rect();
            //! Gets the state key that reports the item clip rectangle in layer coordinates.
            LUNA_GUI_API StateKey<RectF> clip_rect();
        }

        //! Reads one typed public item query value.
        //! @param[in] handle The item handle returned by a widget API.
        //! @param[in] key The state key to read.
        //! @return Returns the stored value, or the key default when absent or when the handle is invalid.
        template <typename T>
        T get_item_state(ItemHandle handle, const StateKey<T>& key)
        {
            IContext* context = handle.context ? query_interface<IContext>(handle.context) : nullptr;
            if(!context || handle.generation != context->generation()) return key.default_value;
            ItemQueryState* query_state = get_state<ItemQueryState>(context, handle.id);
            if(!query_state) return key.default_value;
            auto iter = query_state->states.find(key.name);
            if(iter == query_state->states.end()) return key.default_value;
            const Any* value = &iter->second;
            if(!value) return key.default_value;
            const T* typed_value = value->as<T>();
            return typed_value ? *typed_value : key.default_value;
        }

        //! Writes one typed public item query value.
        //! @param[in] handle The item handle returned by a widget API.
        //! @param[in] key The state key to write.
        //! @param[in] value The value to store.
        template <typename T>
        void set_item_state(ItemHandle handle, const StateKey<T>& key, const T& value)
        {
            IContext* context = handle.context ? query_interface<IContext>(handle.context) : nullptr;
            if(!context || handle.generation != context->generation()) return;
            Ref<ItemQueryState> query_state = get_or_create_state<ItemQueryState>(context, handle.id);
            if(query_state)
            {
                query_state->states.insert_or_assign(key.name, Any(value));
                RV r = context->set_state(make_state_id<ItemQueryState>(handle.id), query_state.object(), StateLifetime::next_frame);
                luassert_always(succeeded(r));
            }
        }

        //! Checks whether an item was clicked this frame.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item was clicked.
        LUNA_GUI_API bool is_item_clicked(ItemHandle handle);
        //! Checks whether an item was right-clicked this frame.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item was right-clicked.
        LUNA_GUI_API bool is_item_right_clicked(ItemHandle handle);
        //! Checks whether an item was double-clicked this frame.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item was double-clicked.
        LUNA_GUI_API bool is_item_double_clicked(ItemHandle handle);
        //! Checks whether an item is currently hovered.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item is hovered.
        LUNA_GUI_API bool is_item_hovered(ItemHandle handle);
        //! Checks whether an item is currently active.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item is active.
        LUNA_GUI_API bool is_item_active(ItemHandle handle);
        //! Checks whether an item is currently focused.
        //! @param[in] handle The item handle to query.
        //! @return Returns `true` if the item is focused.
        LUNA_GUI_API bool is_item_focused(ItemHandle handle);

        //! @}
    }
}
