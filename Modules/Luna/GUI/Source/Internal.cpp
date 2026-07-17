/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Internal.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            Ref<FrameState> frame_state(GUICore::IContext* context)
            {
                luassert(context);
                id_t state_id = GUICore::make_state_id<FrameState>(0);
                Ref<FrameState> state;
                if(object_t object = context->get_state(state_id))
                {
                    object_retain(object);
                    state.attach(object);
                }
                else
                {
                    state = new_object<FrameState>();
                }
                if(state->generation != context->generation())
                {
                    state->generation = context->generation();
                    state->block_index = 0;
                    state->offset = 0;
                    state->actions.clear();
                    state->scroll_stack.clear();
                    state->tab_stack.clear();
                    state->table_stack.clear();
                    state->popup_stack.clear();
                    state->menu_bar_stack.clear();
                    state->dock_space_stack.clear();
                }
                lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
                return state;
            }

            void* allocate_frame_raw(GUICore::IContext* context, usize size, usize alignment)
            {
                Ref<FrameState> state = frame_state(context);
                size = max(size, (usize)1);
                alignment = max(alignment, (usize)1);
                constexpr usize block_alignment = 16;
                constexpr usize default_block_size = 4096;
                usize required_size = max(default_block_size, align_upper(size, block_alignment));
                for(;;)
                {
                    if(state->block_index >= state->blocks.size())
                    {
                        state->blocks.push_back(Blob(required_size, block_alignment));
                    }
                    Blob& block = state->blocks[state->block_index];
                    usize offset = align_upper(state->offset, alignment);
                    if(offset + size <= block.size())
                    {
                        state->offset = offset + size;
                        return (byte_t*)block.data() + offset;
                    }
                    ++state->block_index;
                    state->offset = 0;
                }
            }

            c8* copy_frame_string(GUICore::IContext* context, const c8* string, usize* out_size)
            {
                usize size = string ? strlen(string) : 0;
                c8* result = (c8*)allocate_frame_raw(context, size + 1, alignof(c8));
                if(size)
                {
                    memcpy(result, string, size);
                }
                result[size] = 0;
                if(out_size)
                {
                    *out_size = size;
                }
                return result;
            }

            GUICore::ElementHandle begin_element(GUICore::IContext* context, id_t id, const c8* debug_name,
                const GUICore::LayoutConfig& layout)
            {
                luassert(context && id);
                GUICore::ElementHandle element = context->begin_element(id);
                context->set_element_debug_name(element, Name(debug_name ? debug_name : "GUI element"));
                context->set_layout_config(element, layout);
                return element;
            }

            void set_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
                bool enabled, bool read_only, bool scrollable)
            {
                GUICore::Interactable interactable;
                interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
                set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
                set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
                set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
                set_flags(interactable.flags, GUICore::InteractableFlag::scrollable, scrollable);
                set_flags(interactable.flags, GUICore::InteractableFlag::disabled, !enabled);
                set_flags(interactable.flags, GUICore::InteractableFlag::read_only, read_only);
                context->set_interactable(element, interactable);
            }

            void add_action(GUICore::IContext* context, ActionType type, id_t id, void* data)
            {
                Ref<FrameState> state = frame_state(context);
                Action action;
                action.type = type;
                action.id = id;
                action.data = data;
                state->actions.push_back(action);
            }

            GUICore::StyleValue style_value(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const GUICore::StyleValue& fallback)
            {
                const GUICore::Element* data = context ? context->get_element(element.index) : nullptr;
                if(!context) return fallback;
                Name style = data && !data->style.empty() ? data->style : Name(DEFAULT_STYLE_NAME);
                return context->get_style_value(style, Name(entry), fallback);
            }

            Float4U style_color(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const Float4U& fallback)
            {
                return style_value(context, element, entry, GUICore::style_f32x4(fallback)).number;
            }

            Float2U style_vector2(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const Float2U& fallback)
            {
                Float4U value = style_value(context, element, entry, GUICore::style_f32x2(fallback)).number;
                return Float2U(value.x, value.y);
            }

            f32 style_scalar(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, f32 fallback)
            {
                return style_value(context, element, entry, GUICore::style_f32(fallback)).number.x;
            }

            Name style_name(GUICore::IContext* context, const GUICore::ElementHandle& element,
                const c8* entry, const Name& fallback)
            {
                return style_value(context, element, entry, GUICore::style_name(fallback)).name;
            }

            VG::TextAlignment text_alignment(TextAlignment alignment)
            {
                switch(alignment)
                {
                case TextAlignment::center: return VG::TextAlignment::center;
                case TextAlignment::end: return VG::TextAlignment::end;
                default: return VG::TextAlignment::begin;
                }
            }

            f32 smooth_step(f32 current, f32 target, f32 speed, f32 delta_time)
            {
                f32 t = clamp(speed * delta_time, 0.0f, 1.0f);
                t = t * t * (3.0f - 2.0f * t);
                return current + (target - current) * t;
            }

            id_t derived_id(id_t id, const c8* salt)
            {
                u64 hash = 14695981039346656037ull;
                const byte_t* bytes = (const byte_t*)&id;
                for(usize i = 0; i < sizeof(id); ++i)
                {
                    hash ^= (u64)bytes[i];
                    hash *= 1099511628211ull;
                }
                while(salt && *salt)
                {
                    hash ^= (u64)(byte_t)*salt++;
                    hash *= 1099511628211ull;
                }
                return hash;
            }
        }

        LUNA_GUI_API RV layout_tree(GUICore::IContext* context, const GUICore::ElementHandle& root,
            const RectF& rect)
        {
            if(!context)
            {
                return BasicError::bad_arguments();
            }
            return context->apply_layout(root, rect);
        }

        LUNA_GUI_API bool is_item_valid(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            if(!context || item.generation != context->generation())
            {
                return false;
            }
            const GUICore::Element* element = context->get_element(item.index);
            return element && element->id == item.id;
        }

        LUNA_GUI_API bool is_item_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).clicked;
        }

        LUNA_GUI_API bool is_item_right_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return false;
            for(const GUICore::RoutedInputEvent& routed : context->get_routed_input_events(item.id))
            {
                if(routed.event.type == GUICore::InputEventType::pointer_up &&
                    routed.event.button == GUICore::PointerButton::right)
                {
                    return true;
                }
            }
            return false;
        }

        LUNA_GUI_API bool is_item_double_clicked(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).double_clicked;
        }

        LUNA_GUI_API bool is_item_hovered(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).hovered;
        }

        LUNA_GUI_API bool is_item_active(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).active;
        }

        LUNA_GUI_API bool is_item_focused(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).focused;
        }

        LUNA_GUI_API RectF get_item_rect(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return RectF();
            const GUICore::Element* element = context->get_element(item.index);
            return element ? element->layout_result.rect : RectF();
        }

        LUNA_GUI_API RectF get_item_clip_rect(GUICore::IContext* context, const GUICore::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return RectF();
            const GUICore::Element* element = context->get_element(item.index);
            return element ? element->layout_result.clip_rect : RectF();
        }
    }
}
