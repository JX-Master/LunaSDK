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
#include "../EditorGUI.hpp"

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            RV initialize_icons();
            void close_icons();

            Ref<FrameState> frame_state(GUI::IContext* context);
            void* allocate_frame_raw(GUI::IContext* context, usize size, usize alignment);
            c8* copy_frame_string(GUI::IContext* context, const c8* string, usize* out_size = nullptr);

            template <typename T>
            T* allocate_frame(GUI::IContext* context)
            {
                void* memory = allocate_frame_raw(context, sizeof(T), alignof(T));
                return new (memory) T();
            }

            template <typename T>
            T* copy_frame(GUI::IContext* context, const T& value)
            {
                T* result = allocate_frame<T>(context);
                *result = value;
                return result;
            }

            template <typename T>
            T* allocate_frame_array(GUI::IContext* context, usize count)
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
            Ref<T> widget_state(GUI::IContext* context, id_t owner)
            {
                id_t state_id = GUI::make_state_id<T>(owner);
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
                lupanic_if_failed(context->set_state(state_id, state.object(), GUI::StateLifetime::next_frame));
                return state;
            }

            GUI::ElementHandle begin_element(GUI::IContext* context, id_t id, const c8* debug_name,
                const GUI::LayoutConfig& layout);
            void set_interactable(GUI::IContext* context, const GUI::ElementHandle& element,
                bool enabled, bool read_only = false, bool scrollable = false);
            void add_action(GUI::IContext* context, ActionType type, id_t id, void* data);

            GUI::StyleValue style_value(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const GUI::StyleValue& fallback);
            Float4U style_color(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Float4U& fallback);
            Float2U style_vector2(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Float2U& fallback);
            f32 style_scalar(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, f32 fallback);
            Name style_name(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Name& fallback = Name());

            struct RoundedRectEffect
            {
                bool shadow = false;
                Float4U color = Float4U(1.0f);
                GUI::ShadowDesc shadow_desc;
            };

            RV draw_rounded_rect_effects(GUI::IContext* context,
                const GUI::ElementHandle& element, const RectF& rect,
                const Float4U& rect_layout_scale, f32 radius, Span<const RoundedRectEffect> effects,
                GUI::paint_order_id_t paint_order_id);

            VG::TextAlignment text_alignment(TextAlignment alignment);
            f32 smooth_step(f32 current, f32 target, f32 speed, f32 delta_time);
            void set_flex_layout(GUI::IContext* context, const GUI::ElementHandle& element,
                const GUI::FlexLayoutDesc& source, GUI::LayoutAxis axis);
            id_t derived_id(id_t id, const c8* salt);

            bool resolve_input_text_action(GUI::IContext* context, TextInputAction& action);
            bool resolve_slider_float_action(GUI::IContext* context, SliderFloatAction& action);
            bool resolve_slider_int_action(GUI::IContext* context, SliderIntAction& action);
            bool resolve_choice_action(GUI::IContext* context, ChoiceAction& action);
            bool resolve_button_group_multi_action(GUI::IContext* context, ButtonGroupMultiAction& action);
            bool resolve_disclosure_action(GUI::IContext* context, DisclosureAction& action);
            bool resolve_drag_float_action(GUI::IContext* context, DragFloatAction& action);
            bool resolve_drag_int_action(GUI::IContext* context, DragIntAction& action);
            bool resolve_scroll_action(GUI::IContext* context, ScrollAction& action);
            bool resolve_tab_action(GUI::IContext* context, TabAction& action);
            bool resolve_table_action(GUI::IContext* context, TableAction& action);
            bool resolve_color_edit_action(GUI::IContext* context, ColorEditAction& action);
            bool resolve_dock_space_action(GUI::IContext* context, DockSpaceAction& action);
        }
    }
}
