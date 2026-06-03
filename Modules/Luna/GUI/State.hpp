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
        struct ItemQueryState
        {
            lustruct("GUI::ItemQueryState", "{0F5DB6E4-C49A-4BDE-BDFF-E6A432146AFD}");
            HashMap<Name, Any> states;
        };

        struct CustomState
        {
            lustruct("GUI::CustomState", "{44E02645-6144-4A84-8736-204880C0E620}");
            HashMap<Name, Any> states;
        };

        struct DisclosureState
        {
            lustruct("GUI::DisclosureState", "{62263BD7-3405-493F-8DDB-B3D089412ACA}");
            bool open = true;
            bool open_initialized = false;
        };

        struct SwitchAnimationState
        {
            lustruct("GUI::SwitchAnimationState", "{C8CC1A97-56C5-49B4-962D-70C1A1A7CC9F}");
            f32 animation = 0.0f;
            bool initialized = false;
        };

        struct ButtonAnimationState
        {
            lustruct("GUI::ButtonAnimationState", "{B00FEB21-FE08-459E-87BC-FD15468BB6B6}");
            Float4U color = Float4U(0.18f, 0.28f, 0.45f, 1.0f);
            bool initialized = false;
        };

        struct ButtonGroupAnimationState
        {
            lustruct("GUI::ButtonGroupAnimationState", "{2FB79430-D36D-42E1-BF3D-2FDD21D351AF}");
            f32 selection_animation = 0.0f;
            bool selection_animation_initialized = false;
            Vector<f32> item_animations;
        };

        struct TabBarAnimationState
        {
            lustruct("GUI::TabBarAnimationState", "{68A87F19-D1C3-4496-AE46-87A2DFB61E3D}");
            RectF selection_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool selection_rect_initialized = false;
        };

        template <typename T>
        T* get_state(IContext* context, id_t owner_id)
        {
            if(!context) return nullptr;
            object_t obj = context->get_state(make_state_id<T>(owner_id));
            return obj ? cast_object<T>(obj) : nullptr;
        }

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

        template <typename T>
        void clear_state(IContext* context, id_t owner_id)
        {
            if(context)
            {
                context->clear_state(make_state_id<T>(owner_id));
            }
        }

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

        LUNA_GUI_API bool is_item_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_right_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_double_clicked(ItemHandle handle);
        LUNA_GUI_API bool is_item_hovered(ItemHandle handle);
        LUNA_GUI_API bool is_item_active(ItemHandle handle);
        LUNA_GUI_API bool is_item_focused(ItemHandle handle);
    }
}
