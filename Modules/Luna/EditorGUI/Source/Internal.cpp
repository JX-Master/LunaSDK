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
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cstring>

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            Ref<FrameState> frame_state(GUI::IContext* context)
            {
                luassert(context);
                id_t state_id = GUI::make_state_id<FrameState>(0);
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
                lupanic_if_failed(context->set_state(state_id, state.object(), GUI::StateLifetime::next_frame));
                return state;
            }

            void* allocate_frame_raw(GUI::IContext* context, usize size, usize alignment)
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

            c8* copy_frame_string(GUI::IContext* context, const c8* string, usize* out_size)
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

            GUI::ElementHandle begin_element(GUI::IContext* context, id_t id, const c8* debug_name,
                const GUI::LayoutConfig& layout)
            {
                luassert(context && id);
                GUI::ElementHandle element = context->begin_element(id);
                context->set_element_debug_name(element, Name(debug_name ? debug_name : "GUI element"));
                context->set_layout_config(element, layout);
                return element;
            }

            void set_interactable(GUI::IContext* context, const GUI::ElementHandle& element,
                bool enabled, bool read_only, bool scrollable)
            {
                GUI::Interactable interactable;
                interactable.pointer_hit_behavior = GUI::PointerHitBehavior::target;
                set_flags(interactable.flags, GUI::InteractableFlag::hoverable);
                set_flags(interactable.flags, GUI::InteractableFlag::activatable);
                set_flags(interactable.flags, GUI::InteractableFlag::focusable);
                set_flags(interactable.flags, GUI::InteractableFlag::scrollable, scrollable);
                set_flags(interactable.flags, GUI::InteractableFlag::disabled, !enabled);
                set_flags(interactable.flags, GUI::InteractableFlag::read_only, read_only);
                context->set_interactable(element, interactable);
            }

            void add_action(GUI::IContext* context, ActionType type, id_t id, void* data)
            {
                Ref<FrameState> state = frame_state(context);
                Action action;
                action.type = type;
                action.id = id;
                action.data = data;
                state->actions.push_back(action);
            }

            GUI::StyleValue style_value(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const GUI::StyleValue& fallback)
            {
                const GUI::Element* data = context ? context->get_element(element.index) : nullptr;
                if(!context) return fallback;
                Name style = data && !data->style.empty() ? data->style : Name(DEFAULT_STYLE_NAME);
                return context->get_style_value(style, Name(entry), fallback);
            }

            Float4U style_color(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Float4U& fallback)
            {
                return style_value(context, element, entry, GUI::style_f32x4(fallback)).number;
            }

            Float2U style_vector2(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Float2U& fallback)
            {
                Float4U value = style_value(context, element, entry, GUI::style_f32x2(fallback)).number;
                return Float2U(value.x, value.y);
            }

            f32 style_scalar(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, f32 fallback)
            {
                return style_value(context, element, entry, GUI::style_f32(fallback)).number.x;
            }

            Name style_name(GUI::IContext* context, const GUI::ElementHandle& element,
                const c8* entry, const Name& fallback)
            {
                return style_value(context, element, entry, GUI::style_name(fallback)).name;
            }

            RV draw_rounded_rect_effects(GUI::IContext* context,
                const GUI::ElementHandle& element, const RectF& rect,
                const Float4U& rect_layout_scale, f32 radius, Span<const RoundedRectEffect> effects)
            {
                if(!context || effects.empty()) return E_BAD_ARGUMENTS;
                const GUI::Element* element_data = context->get_element(element.index);
                if(!element_data) return E_BAD_ARGUMENTS;
                const RectF& element_rect = element_data->layout_result.rect;
                auto resolve_extent = [](f32 element_extent, f32 offset, f32 extent, f32 scale)
                {
                    f32 scaled_extent = extent + element_extent * scale;
                    if(scaled_extent > 0.0f) return scaled_extent;
                    return extent < 0.0f ? max(element_extent + scaled_extent, 1.0f) :
                        max(element_extent - offset, 1.0f);
                };
                f32 width = resolve_extent(element_rect.width, rect.offset_x, rect.width,
                    rect_layout_scale.z);
                f32 height = resolve_extent(element_rect.height, rect.offset_y, rect.height,
                    rect_layout_scale.w);

                Vector<f32> shape_floats;
                GUI::sdf_shape_add_rounded_rectangle(shape_floats,
                    RectF(0.0f, 0.0f, width, height), Float4U(max(radius, 0.0f)));
                auto shape = context->append_sdf_shape_program(shape_floats.cspan());
                if(failed(shape)) return shape.errcode();

                Vector<f32> color_floats;
                for(const RoundedRectEffect& effect : effects)
                {
                    GUI::SDFBufferRange range;
                    if(effect.shadow)
                    {
                        GUI::SDFClipDesc clip = effect.shadow_desc.mode == GUI::ShadowMode::inner ?
                            GUI::SDFClipDesc::outer(0.0f) : GUI::SDFClipDesc::inner(0.0f);
                        range = GUI::sdf_color_add_shadow(color_floats, effect.color,
                            effect.shadow_desc.offset, effect.shadow_desc.softness,
                            effect.shadow_desc.spread, clip);
                    }
                    else
                    {
                        range = GUI::sdf_color_add_solid(color_floats, effect.color);
                    }
                    if(!range.valid()) return E_BAD_DATA;
                }
                auto color = context->append_sdf_color_program(color_floats.cspan());
                if(failed(color)) return color.errcode();

                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::sdf;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.rect_layout_scale = rect_layout_scale;
                command.sdf.shape = shape.get();
                command.sdf.color = color.get();
                context->draw(command);
                return ok;
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

        LUNA_EDITOR_GUI_API RV layout_tree(GUI::IContext* context, const GUI::ElementHandle& root,
            const RectF& rect)
        {
            if(!context)
            {
                return E_BAD_ARGUMENTS;
            }
            return context->apply_layout(root, rect);
        }

        LUNA_EDITOR_GUI_API bool is_item_valid(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            if(!context || item.generation != context->generation())
            {
                return false;
            }
            const GUI::Element* element = context->get_element(item.index);
            return element && element->id == item.id;
        }

        LUNA_EDITOR_GUI_API bool is_item_clicked(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).clicked;
        }

        LUNA_EDITOR_GUI_API bool is_item_right_clicked(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return false;
            for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(item.id))
            {
                if(routed.event.type == GUI::InputEventType::pointer_up &&
                    routed.event.button == GUI::PointerButton::right)
                {
                    return true;
                }
            }
            return false;
        }

        LUNA_EDITOR_GUI_API bool is_item_double_clicked(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).double_clicked;
        }

        LUNA_EDITOR_GUI_API bool is_item_hovered(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).hovered;
        }

        LUNA_EDITOR_GUI_API bool is_item_active(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).active;
        }

        LUNA_EDITOR_GUI_API bool is_item_focused(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            return is_item_valid(context, item) && context->get_interaction_state(item.id).focused;
        }

        LUNA_EDITOR_GUI_API RectF get_item_rect(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return RectF();
            const GUI::Element* element = context->get_element(item.index);
            return element ? element->layout_result.rect : RectF();
        }

        LUNA_EDITOR_GUI_API RectF get_item_clip_rect(GUI::IContext* context, const GUI::ElementHandle& item)
        {
            if(!is_item_valid(context, item)) return RectF();
            const GUI::Element* element = context->get_element(item.index);
            return element ? element->layout_result.clip_rect : RectF();
        }
    }
}
