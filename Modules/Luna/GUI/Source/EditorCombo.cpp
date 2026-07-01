/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorCombo.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            context->set_interactable(element, interactable);
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

        static GUICore::LayoutConfig fixed_layout(f32 width, f32 height)
        {
            GUICore::LayoutConfig layout;
            if(width > 0.0f)
            {
                layout.width.kind = GUICore::SizeKind::fixed;
                layout.width.value = width;
            }
            if(height > 0.0f)
            {
                layout.height.kind = GUICore::SizeKind::fixed;
                layout.height.value = height;
            }
            return layout;
        }

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = horizontal_alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static u64 hash_u64_local(u64 value, u64 h = 14695981039346656037ull)
        {
            const byte_t* p = (const byte_t*)&value;
            for(usize i = 0; i < sizeof(value); ++i)
            {
                h ^= (u64)p[i];
                h *= 1099511628211ull;
            }
            return h;
        }

        static u64 hash_cstr_local(const c8* str, u64 h)
        {
            if(str)
            {
                while(*str)
                {
                    h ^= (u64)(byte_t)*str;
                    h *= 1099511628211ull;
                    ++str;
                }
            }
            return h;
        }

        static GUICore::id_t derived_id(GUICore::id_t id, const c8* salt)
        {
            return hash_cstr_local(salt, hash_u64_local(id));
        }

        static Ref<CoreComboState> combo_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<CoreComboState>(id);
            Ref<CoreComboState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreComboState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static const c8* combo_selected_text(i32* current_item, Span<const c8*> items)
        {
            if(!current_item || *current_item < 0 || (usize)*current_item >= items.size())
            {
                return "";
            }
            return items[(usize)*current_item] ? items[(usize)*current_item] : "";
        }

        static GUICore::id_t combo_popup_id(GUICore::id_t id)
        {
            return derived_id(id, "combo_popup");
        }

        static GUICore::id_t combo_item_id(GUICore::id_t id, usize item_index)
        {
            return hash_u64_local((u64)item_index, derived_id(id, "combo_item"));
        }

        LUNA_GUI_API GUICore::ElementHandle combo(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            i32* current_item, Span<const c8*> items, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            if(current_item && !items.empty())
            {
                *current_item = clamp(*current_item, 0, (i32)items.size() - 1);
            }

            GUICore::id_t popup_id = combo_popup_id(id);
            Ref<CoreComboState> state = combo_state(context, id);
            for(usize i = 0; i < items.size(); ++i)
            {
                if(context->get_interaction_state(combo_item_id(id, i)).clicked)
                {
                    if(current_item)
                    {
                        *current_item = (i32)i;
                    }
                    close_popup(context, popup_id);
                    break;
                }
            }

            GUICore::InteractionState preview_interaction = context->get_interaction_state(id);
            if(preview_interaction.clicked && current_item && !items.empty())
            {
                if(is_popup_open(context, popup_id))
                {
                    close_popup(context, popup_id);
                }
                else
                {
                    f32 preview_x = preview_interaction.clicked_screen_position.x - preview_interaction.clicked_element_position.x;
                    f32 preview_y = preview_interaction.clicked_screen_position.y - preview_interaction.clicked_element_position.y;
                    state->popup_position = Float2U(preview_x, preview_y + preview_interaction.clicked_element_rect.height);
                    open_popup(context, popup_id);
                }
            }

            bool open = is_popup_open(context, popup_id);
            GUICore::ElementHandle preview = context->begin_element(id, label ? Name(label) : Name("combo"));
            context->set_layout_config(preview, layout);
            set_basic_interactable(context, preview);
            Float4U background = style_value(context, open ? Name("gui.editor.combo.background_open") :
                (preview_interaction.hovered ? Name("gui.editor.combo.background_hovered") : Name("gui.editor.combo.background")),
                open ? GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f)) :
                (preview_interaction.hovered ? GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)))).number;
            Float4U border = style_value(context, Name("gui.editor.combo.border"),
                GUICore::style_f32x4(Float4U(0.18f, 0.25f, 0.34f, 1.0f))).number;
            Float4U text_color = style_value(context, Name("gui.editor.combo.text"),
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f))).number;
            f32 radius = style_value(context, Name("gui.editor.combo.radius"), GUICore::style_f32(4.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.combo.font_size"), GUICore::style_f32(15.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                border, radius);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(1.0f, 1.0f, -2.0f, -2.0f),
                background, max(radius - 1.0f, 0.0f));
            draw_relative_text(context, RectF(8.0f, 0.0f, -34.0f, 0.0f), combo_selected_text(current_item, items),
                text_color, font_size);
            draw_relative_text(context, RectF(-28.0f, 0.0f, 20.0f, 0.0f), open ? "^" : "v", text_color,
                font_size, VG::TextAlignment::center);
            context->end_element();

            if(open)
            {
                PopupDesc popup_desc;
                popup_desc.position = state->popup_position;
                f32 popup_height = max(28.0f, min((f32)items.size() * 26.0f + 12.0f, 320.0f));
                popup_desc.layout = fixed_layout(220.0f, popup_height);
                GUICore::ElementHandle popup;
                if(begin_popup(context, popup_id, popup_desc, &popup))
                {
                    GUICore::LayoutConfig item_layout;
                    item_layout.width.kind = GUICore::SizeKind::percent;
                    item_layout.width.value = 1.0f;
                    item_layout.height.kind = GUICore::SizeKind::fixed;
                    item_layout.height.value = 26.0f;
                    for(usize i = 0; i < items.size(); ++i)
                    {
                        const c8* item_text = items[i] ? items[i] : "";
                        selectable(context, combo_item_id(id, i), item_text, current_item && *current_item == (i32)i, item_layout);
                    }
                    lupanic_if_failed(end_popup(context, popup, RectF(0.0f, 0.0f, 220.0f, popup_height)));
                }
            }
            return preview;
        }
    }
}
