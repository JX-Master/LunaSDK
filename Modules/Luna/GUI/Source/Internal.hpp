/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Internal.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include "State.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            Ref<FrameState> frame_state(GUICore::IContext* context);
            void* allocate_frame_raw(GUICore::IContext* context, usize size, usize alignment);
            c8* copy_frame_string(GUICore::IContext* context, const c8* string, usize* out_size = nullptr);

            template <typename T>
            T* allocate_frame(GUICore::IContext* context)
            {
                void* memory = allocate_frame_raw(context, sizeof(T), alignof(T));
                return new (memory) T();
            }

            template <typename T>
            T* copy_frame(GUICore::IContext* context, const T& value)
            {
                T* result = allocate_frame<T>(context);
                *result = value;
                return result;
            }

            template <typename T>
            T* allocate_frame_array(GUICore::IContext* context, usize count)
            {
                if(!count) return nullptr;
                T* result = (T*)allocate_frame_raw(context, sizeof(T) * count, alignof(T));
                for(usize i = 0; i < count; ++i)
                {
                    new (result + i) T();
                }
                return result;
            }

            template <typename T>
            Ref<T> widget_state(GUICore::IContext* context, id_t owner)
            {
                id_t state_id = GUICore::make_state_id<T>(owner);
                Ref<T> state;
                if(object_t object = context->get_state(state_id))
                {
                    object_retain(object);
                    state.attach(object);
                }
                else
                {
                    state = new_object<T>();
                }
                lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
                return state;
            }

            GUICore::ElementHandle begin_element(GUICore::IContext* context, id_t id, const c8* debug_name,
                const GUICore::LayoutConfig& layout);
            void set_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
                bool enabled, bool read_only = false, bool scrollable = false);
            void add_action(GUICore::IContext* context, ActionType type, id_t id, void* data);

            GUICore::StyleValue style_value(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const GUICore::StyleValue& fallback);
            Float4U style_color(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const Float4U& fallback);
            f32 style_scalar(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, f32 fallback);
            Name style_name(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const Name& fallback = Name());

            VG::TextAlignment text_alignment(TextAlignment alignment);
            f32 smooth_step(f32 current, f32 target, f32 speed, f32 delta_time);
            void set_flex_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const GUICore::FlexLayoutDesc& source, GUICore::LayoutAxis axis);

            bool resolve_input_text_action(GUICore::IContext* context, TextInputAction& action);
            bool resolve_slider_float_action(GUICore::IContext* context, SliderFloatAction& action);
            bool resolve_slider_int_action(GUICore::IContext* context, SliderIntAction& action);
            bool resolve_choice_action(GUICore::IContext* context, ChoiceAction& action);
            bool resolve_button_group_multi_action(GUICore::IContext* context, ButtonGroupMultiAction& action);
            bool resolve_disclosure_action(GUICore::IContext* context, DisclosureAction& action);
            bool resolve_drag_float_action(GUICore::IContext* context, DragFloatAction& action);
            bool resolve_drag_int_action(GUICore::IContext* context, DragIntAction& action);
            bool resolve_scroll_action(GUICore::IContext* context, ScrollAction& action);
            bool resolve_tab_action(GUICore::IContext* context, TabAction& action);
        }
    }
}
