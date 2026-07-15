/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorPopups.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "EditorInternal.hpp"
#include <Luna/GUI/Legacy/EditorState.hpp>
#include <Luna/GUI/Legacy/EditorWidgets.hpp>
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV set_editor_flex_layout_config(GUICore::IContext* context,
            const GUICore::ElementHandle& layout, const GUICore::FlexLayoutDesc& desc);

        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static GUICore::id_t derived_id(GUICore::id_t id, const c8* salt)
        {
            u64 h = 14695981039346656037ull;
            const byte_t* p = (const byte_t*)&id;
            for(usize i = 0; i < sizeof(id); ++i)
            {
                h ^= (u64)p[i];
                h *= 1099511628211ull;
            }
            if(salt)
            {
                while(*salt)
                {
                    h ^= (u64)(byte_t)*salt;
                    h *= 1099511628211ull;
                    ++salt;
                }
            }
            return h;
        }

        static Ref<CorePopupState> popup_state(GUICore::IContext* context, GUICore::id_t id, bool create)
        {
            id_t state_id = GUICore::make_state_id<CorePopupState>(id);
            if(object_t state_obj = context->get_state(state_id))
            {
                Ref<CorePopupState> state;
                object_retain(state_obj);
                state.attach(state_obj);
                return state;
            }
            if(!create)
            {
                return nullptr;
            }
            Ref<CorePopupState> state = new_object<CorePopupState>();
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::process));
            return state;
        }

        static Ref<CoreTooltipState> tooltip_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<CoreTooltipState>(id);
            Ref<CoreTooltipState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreTooltipState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static GUICore::LayoutConfig layer_panel_layout(GUICore::IContext* context, const c8* prefix,
            const GUICore::LayoutConfig& input)
        {
            GUICore::LayoutConfig layout = input;
            String padding_entry;
            strprintf(padding_entry, "gui.editor.%s.padding", prefix);
            f32 padding = style_value(context, Name(padding_entry.c_str()), GUICore::style_f32(6.0f)).number.x;
            layout.padding = Float4U(padding, padding, padding, padding);
            return layout;
        }

        static void draw_layer_panel_chrome(GUICore::IContext* context, const c8* prefix,
            const Float4U& default_background, const Float4U& default_border, f32 default_radius)
        {
            String entry;
            strprintf(entry, "gui.editor.%s.background", prefix);
            Float4U background = style_value(context, Name(entry.c_str()), GUICore::style_f32x4(default_background)).number;
            strprintf(entry, "gui.editor.%s.border", prefix);
            Float4U border = style_value(context, Name(entry.c_str()), GUICore::style_f32x4(default_border)).number;
            strprintf(entry, "gui.editor.%s.radius", prefix);
            f32 radius = style_value(context, Name(entry.c_str()), GUICore::style_f32(default_radius)).number.x;
            strprintf(entry, "gui.editor.%s.border_size", prefix);
            f32 border_size = style_value(context, Name(entry.c_str()), GUICore::style_f32(1.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                background, radius);
            if(border_size > 0.0f)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(0.0f, 0.0f, 0.0f, 0.0f), border, radius);
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(border_size, border_size, -border_size * 2.0f, -border_size * 2.0f),
                    background, max(radius - border_size, 0.0f));
            }
        }

        static RV end_layer_panel(GUICore::IContext* context, const GUICore::ElementHandle& panel, const RectF& rect,
            const c8* prefix)
        {
            GUICore::FlexLayoutDesc layout_desc;
            layout_desc.axis = GUICore::LayoutAxis::y;
            String gap_entry;
            strprintf(gap_entry, "gui.editor.%s.gap", prefix);
            layout_desc.main_axis_gap = style_value(context, Name(gap_entry.c_str()), GUICore::style_f32(4.0f)).number.x;
            RV r = set_editor_flex_layout_config(context, panel, layout_desc);
            context->end_element();
            context->pop_layer();
            if(succeeded(r))
            {
                r = context->apply_layout(panel, rect);
            }
            return r;
        }

        LUNA_GUI_API void open_popup(GUICore::IContext* context, GUICore::id_t id)
        {
            luassert(context && id);
            Ref<CorePopupState> state = popup_state(context, id, true);
            state->open = true;
            lupanic_if_failed(context->set_state(GUICore::make_state_id<CorePopupState>(id), state.object(), GUICore::StateLifetime::process));
        }

        LUNA_GUI_API void close_popup(GUICore::IContext* context, GUICore::id_t id)
        {
            luassert(context && id);
            context->clear_state(GUICore::make_state_id<CorePopupState>(id));
        }

        LUNA_GUI_API bool is_popup_open(GUICore::IContext* context, GUICore::id_t id)
        {
            luassert(context && id);
            Ref<CorePopupState> state = popup_state(context, id, false);
            return state && state->open;
        }

        LUNA_GUI_API bool begin_popup(GUICore::IContext* context, GUICore::id_t id, const PopupDesc& desc,
            GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            if(!is_popup_open(context, id))
            {
                if(out_handle)
                {
                    *out_handle = GUICore::ElementHandle();
                }
                return false;
            }
            Internal::push_layer(context, id, desc.position, "popup");
            GUICore::ElementHandle popup = Internal::begin_element(context, derived_id(id, "popup_root"), "popup");
            context->set_layout_config(popup, layer_panel_layout(context, "popup", desc.layout));
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            context->set_interactable(popup, interactable);
            draw_layer_panel_chrome(context, "popup", Float4U(0.08f, 0.10f, 0.13f, 0.98f),
                Float4U(0.24f, 0.30f, 0.38f, 1.0f), 5.0f);
            if(out_handle)
            {
                *out_handle = popup;
            }
            return true;
        }

        LUNA_GUI_API RV end_popup(GUICore::IContext* context, const GUICore::ElementHandle& popup, const RectF& rect)
        {
            luassert(context);
            return end_layer_panel(context, popup, rect, "popup");
        }

        LUNA_GUI_API bool begin_tooltip(GUICore::IContext* context, GUICore::id_t id, const GUICore::ElementHandle& owner,
            const TooltipDesc& desc, GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            GUICore::InteractionState owner_state = context->get_interaction_state(owner.id);
            bool hovered = owner.id && (owner_state.hovered || owner_state.subtree_hovered);
            Ref<CoreTooltipState> state = tooltip_state(context, id);
            if(hovered)
            {
                if(state->hovered_owner == owner.id)
                {
                    state->hover_time += context->get_frame_desc().delta_time;
                }
                else
                {
                    state->hovered_owner = owner.id;
                    state->hover_time = 0.0f;
                }
            }
            else
            {
                state->hovered_owner = 0;
                state->hover_time = 0.0f;
            }
            if(!hovered || state->hover_time < max(desc.delay, 0.0f))
            {
                if(out_handle)
                {
                    *out_handle = GUICore::ElementHandle();
                }
                return false;
            }
            Float2U position(owner_state.pointer_screen_position.x + desc.offset.x,
                owner_state.pointer_screen_position.y + desc.offset.y);
            Internal::push_layer(context, id, position, "tooltip");
            GUICore::ElementHandle tooltip = Internal::begin_element(context, derived_id(id, "tooltip_root"), "tooltip");
            context->set_layout_config(tooltip, layer_panel_layout(context, "tooltip", desc.layout));
            draw_layer_panel_chrome(context, "tooltip", Float4U(0.05f, 0.06f, 0.07f, 0.97f),
                Float4U(0.28f, 0.33f, 0.40f, 1.0f), 4.0f);
            if(out_handle)
            {
                *out_handle = tooltip;
            }
            return true;
        }

        LUNA_GUI_API RV end_tooltip(GUICore::IContext* context, const GUICore::ElementHandle& tooltip, const RectF& rect)
        {
            luassert(context);
            return end_layer_panel(context, tooltip, rect, "tooltip");
        }

        LUNA_GUI_API GUICore::ElementHandle set_item_tooltip(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::ElementHandle& owner, const c8* content, const TooltipDesc& desc)
        {
            luassert(context && id);
            GUICore::ElementHandle tooltip;
            if(!begin_tooltip(context, id, owner, desc, &tooltip))
            {
                return GUICore::ElementHandle();
            }
            usize text_len = content ? strlen(content) : 0;
            f32 width = min(max((f32)text_len * 8.0f + 20.0f, 80.0f), max(desc.max_width, 80.0f));
            f32 height = 30.0f;
            GUICore::LayoutConfig text_layout;
            text_layout.width.kind = GUICore::SizeKind::fixed;
            text_layout.width.value = width - 12.0f;
            text_layout.height.kind = GUICore::SizeKind::fixed;
            text_layout.height.value = 22.0f;
            text(context, derived_id(id, "tooltip_text"), content ? content : "", text_layout);
            lupanic_if_failed(end_tooltip(context, tooltip, RectF(0.0f, 0.0f, width, height)));
            return tooltip;
        }
    }
}
