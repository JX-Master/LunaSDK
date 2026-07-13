/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorDisclosure.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "EditorInternal.hpp"
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
            const Float4U& color, f32 radius = 0.0f, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_relative_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
            const Float4U& color, f32 width, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.rect_layout_scale = scale;
            command.color = color;
            command.line_width = width;
            context->draw(command);
        }

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin,
            const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = horizontal_alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        LUNA_GUI_API bool collapsing_header(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            bool default_open, const GUICore::LayoutConfig& layout, GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "collapsing_header");
            if(out_handle)
            {
                *out_handle = element;
            }
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element);

            id_t state_id = GUICore::make_state_id<DisclosureState>(id);
            bool open = default_open;
            if(object_t state_obj = context->get_state(state_id))
            {
                if(DisclosureState* state = cast_object<DisclosureState>(state_obj); state && state->open_initialized)
                {
                    open = state->open;
                }
            }
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(interaction.clicked)
            {
                open = !open;
            }
            Ref<DisclosureState> next_state = new_object<DisclosureState>();
            next_state->open = open;
            next_state->open_initialized = true;
            lupanic_if_failed(context->set_state(state_id, next_state.object(), GUICore::StateLifetime::next_frame));

            Float4U background = style_value(context, Name("gui.editor.collapsing_header.background"),
                GUICore::style_f32x4(Float4U(0.16f, 0.19f, 0.24f, 1.0f))).number;
            if(interaction.hovered)
            {
                background = style_value(context, Name("gui.editor.collapsing_header.background_hovered"),
                    GUICore::style_f32x4(Float4U(0.20f, 0.25f, 0.32f, 1.0f))).number;
            }
            f32 radius = style_value(context, Name("gui.editor.collapsing_header.radius"), GUICore::style_f32(4.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f),
                background, radius);

            Float4U icon_color = style_value(context, Name("gui.editor.collapsing_header.icon_color"),
                GUICore::style_f32x4(Float4U(0.86f, 0.90f, 0.96f, 1.0f))).number;
            if(open)
            {
                draw_relative_line(context, Float2U(9.0f, -2.0f), Float2U(14.0f, 3.0f), icon_color, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
                draw_relative_line(context, Float2U(14.0f, 3.0f), Float2U(19.0f, -2.0f), icon_color, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
            }
            else
            {
                draw_relative_line(context, Float2U(11.0f, -5.0f), Float2U(17.0f, 0.0f), icon_color, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
                draw_relative_line(context, Float2U(17.0f, 0.0f), Float2U(11.0f, 5.0f), icon_color, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
            }

            Float4U text_color = style_value(context, Name("gui.editor.collapsing_header.text_color"),
                GUICore::style_f32x4(Float4U(1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.collapsing_header.font_size"), GUICore::style_f32(16.0f)).number.x;
            draw_relative_text(context, RectF(28.0f, 0.0f, -34.0f, 0.0f), label, text_color, font_size,
                VG::TextAlignment::begin, Float4U(0.0f, 0.0f, 1.0f, 1.0f));
            context->end_element();
            return open;
        }

        LUNA_GUI_API bool tree_node(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            TreeNodeFlag flags, u32 indent_depth, const GUICore::LayoutConfig& layout, GUICore::ElementHandle* out_handle)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "tree_node");
            if(out_handle)
            {
                *out_handle = element;
            }
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element);

            bool leaf = test_flags(flags, TreeNodeFlag::leaf);
            bool open = !leaf && test_flags(flags, TreeNodeFlag::default_open);
            id_t state_id = GUICore::make_state_id<DisclosureState>(id);
            if(object_t state_obj = context->get_state(state_id))
            {
                if(DisclosureState* state = cast_object<DisclosureState>(state_obj); state && state->open_initialized)
                {
                    open = state->open;
                }
            }
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(interaction.clicked && !leaf)
            {
                bool should_toggle = true;
                if(test_flags(flags, TreeNodeFlag::open_on_arrow))
                {
                    f32 arrow_x = 4.0f + 18.0f * (f32)indent_depth;
                    Float2U p = interaction.clicked_element_position;
                    should_toggle = p.x >= arrow_x && p.x < arrow_x + 18.0f && p.y >= 4.0f && p.y < 22.0f;
                }
                if(should_toggle)
                {
                    open = !open;
                }
            }
            Ref<DisclosureState> next_state = new_object<DisclosureState>();
            next_state->open = open;
            next_state->open_initialized = true;
            lupanic_if_failed(context->set_state(state_id, next_state.object(), GUICore::StateLifetime::next_frame));

            bool selected = test_flags(flags, TreeNodeFlag::selected);
            if(selected)
            {
                Float4U selected_background = style_value(context, Name("gui.editor.tree.selected_background"),
                    GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f))).number;
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(0.0f, 0.0f, 0.0f, 0.0f), selected_background, 4.0f);
            }
            else if(interaction.hovered)
            {
                Float4U hovered_background = style_value(context, Name("gui.editor.tree.hovered_background"),
                    GUICore::style_f32x4(Float4U(0.13f, 0.17f, 0.22f, 1.0f))).number;
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(0.0f, 0.0f, 0.0f, 0.0f), hovered_background, 4.0f);
            }

            f32 arrow_x = 4.0f + 18.0f * (f32)indent_depth;
            Float4U icon_color = style_value(context, Name("gui.editor.tree.icon_color"),
                GUICore::style_f32x4(leaf ? Float4U(0.58f, 0.65f, 0.74f, 1.0f) : Float4U(1.0f))).number;
            if(leaf)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(arrow_x + 6.5f, -2.5f, 5.0f, 5.0f), icon_color, 2.5f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            }
            else if(open)
            {
                draw_relative_line(context, Float2U(arrow_x + 4.0f, -3.0f), Float2U(arrow_x + 9.0f, 3.0f), icon_color, 1.8f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
                draw_relative_line(context, Float2U(arrow_x + 9.0f, 3.0f), Float2U(arrow_x + 14.0f, -3.0f), icon_color, 1.8f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
            }
            else
            {
                draw_relative_line(context, Float2U(arrow_x + 6.0f, -5.0f), Float2U(arrow_x + 12.0f, 0.0f), icon_color, 1.8f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
                draw_relative_line(context, Float2U(arrow_x + 12.0f, 0.0f), Float2U(arrow_x + 6.0f, 5.0f), icon_color, 1.8f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
            }

            Float4U text_color = style_value(context, Name("gui.editor.tree.text_color"),
                GUICore::style_f32x4(Float4U(1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.tree.font_size"), GUICore::style_f32(15.0f)).number.x;
            draw_relative_text(context, RectF(arrow_x + 20.0f, 0.0f, -6.0f, 0.0f), label, text_color, font_size);
            context->end_element();
            return leaf ? false : open;
        }

        LUNA_GUI_API GUICore::ElementHandle progress_bar(GUICore::IContext* context, GUICore::id_t id, f32 fraction,
            const c8* overlay, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, "progress_bar");
            context->set_layout_config(element, layout);
            f32 progress = clamp(fraction, 0.0f, 1.0f);
            f32 border_size = style_value(context, Name("gui.editor.progress.border_size"), GUICore::style_f32(1.0f)).number.x;
            f32 radius = style_value(context, Name("gui.editor.progress.radius"), GUICore::style_f32(5.0f)).number.x;
            Float4U border = style_value(context, Name("gui.editor.progress.border"),
                GUICore::style_f32x4(Float4U(0.25f, 0.29f, 0.35f, 1.0f))).number;
            Float4U background = style_value(context, Name("gui.editor.progress.background"),
                GUICore::style_f32x4(Float4U(0.07f, 0.08f, 0.10f, 1.0f))).number;
            Float4U fill = style_value(context, Name("gui.editor.progress.fill"),
                GUICore::style_f32x4(Float4U(0.20f, 0.36f, 0.62f, 1.0f))).number;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f), border, radius);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(border_size, border_size, -border_size * 2.0f, -border_size * 2.0f),
                background, max(radius - border_size, 0.0f));
            if(progress > 0.0f)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(border_size, border_size, -border_size * 2.0f * progress, -border_size * 2.0f),
                    fill, max(radius - border_size, 0.0f), Float4U(0.0f, 0.0f, progress, 0.0f));
            }
            String percentage;
            const c8* overlay_text = overlay;
            if(!overlay_text)
            {
                strprintf(percentage, "%.0f%%", progress * 100.0f);
                overlay_text = percentage.c_str();
            }
            Float4U text_color = style_value(context, Name("gui.editor.progress.text_color"), GUICore::style_f32x4(Float4U(1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.progress.font_size"), GUICore::style_f32(14.0f)).number.x;
            draw_relative_text(context, RectF(6.0f, border_size, -12.0f, -border_size * 2.0f),
                overlay_text, text_color, font_size, VG::TextAlignment::center);
            context->end_element();
            return element;
        }
    }
}
