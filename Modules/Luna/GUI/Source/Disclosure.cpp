/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Disclosure.cpp
* @author JXMaster
* @date 2026/7/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct DisclosureData
            {
                c8* label = nullptr;
                u32 indent_depth = 0;
                bool header = false;
                bool leaf = false;
                bool enabled = true;
                bool selected = false;
                DisclosureState* state = nullptr;
            };

            static GUICore::MeasureResult measure_disclosure(GUICore::IContext* context,
                const GUICore::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 30.0f);
                GUICore::MeasureResult result;
                result.minimum = Float2U(32.0f, height);
                result.desired = Float2U(180.0f, height);
                return result;
            }

            static void disclosure_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
                const Float4U& color, f32 width)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::line;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
                command.point1 = end;
                command.rect_layout_scale = Float4U(0.0f, 0.5f, 0.0f, 0.5f);
                command.color = color;
                command.line_width = width;
                context->draw(command);
            }

            static RV draw_disclosure(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                DisclosureData* data = (DisclosureData*)userdata;
                if(!data || !data->state) return ok;
                GUICore::InteractionState interaction = context->get_interaction_state(element.id);
                if(data->header || data->selected || (data->enabled && interaction.hovered))
                {
                    GUICore::DrawCommand background;
                    background.type = GUICore::DrawCommandType::rounded_rect;
                    background.rect_reference = GUICore::DrawCommandRectReference::element;
                    background.color = data->selected ? style_color(context, element, "gui.tree.selected",
                        Float4U(0.16f, 0.35f, 0.58f, 1.0f)) : data->header ? style_color(context, element,
                        interaction.hovered ? "gui.disclosure.header_hovered" : "gui.disclosure.header",
                        interaction.hovered ? Float4U(0.20f, 0.25f, 0.32f, 1.0f) :
                        Float4U(0.16f, 0.19f, 0.24f, 1.0f)) : style_color(context, element,
                        "gui.choice.hovered", Float4U(0.13f, 0.19f, 0.27f, 1.0f));
                    background.radius = 4.0f;
                    context->draw(background);
                }
                f32 x = data->header ? 8.0f : 4.0f + 18.0f * (f32)data->indent_depth;
                Float4U icon = data->enabled ? style_color(context, element, "gui.disclosure.icon",
                    Float4U(0.86f, 0.90f, 0.96f, 1.0f)) : style_color(context, element,
                    "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                if(data->leaf)
                {
                    GUICore::DrawCommand dot;
                    dot.type = GUICore::DrawCommandType::rounded_rect;
                    dot.rect_reference = GUICore::DrawCommandRectReference::element;
                    dot.rect = RectF(x + 6.0f, -2.5f, 5.0f, 5.0f);
                    dot.rect_layout_scale = Float4U(0.0f, 0.5f, 0.0f, 0.0f);
                    dot.color = icon;
                    dot.radius = 2.5f;
                    context->draw(dot);
                }
                else if(data->state->animation > 0.5f)
                {
                    disclosure_line(context, Float2U(x + 3.0f, -2.0f), Float2U(x + 8.0f, 3.0f), icon, 1.8f);
                    disclosure_line(context, Float2U(x + 8.0f, 3.0f), Float2U(x + 13.0f, -2.0f), icon, 1.8f);
                }
                else
                {
                    disclosure_line(context, Float2U(x + 5.0f, -5.0f), Float2U(x + 11.0f, 0.0f), icon, 1.8f);
                    disclosure_line(context, Float2U(x + 11.0f, 0.0f), Float2U(x + 5.0f, 5.0f), icon, 1.8f);
                }
                GUICore::DrawCommand text;
                text.type = GUICore::DrawCommandType::text;
                text.rect_reference = GUICore::DrawCommandRectReference::element;
                text.rect = RectF(x + 20.0f, 0.0f, -(x + 24.0f), 0.0f);
                text.text = data->label ? data->label : "";
                text.font = style_name(context, element, "gui.font");
                text.font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                text.color = data->enabled ? style_color(context, element, "gui.text.color",
                    Float4U(0.86f, 0.88f, 0.92f, 1.0f)) : style_color(context, element,
                    "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                text.horizontal_alignment = VG::TextAlignment::begin;
                text.vertical_alignment = VG::TextAlignment::center;
                context->draw(text);
                return ok;
            }

            bool resolve_disclosure_action(GUICore::IContext* context, DisclosureAction& action)
            {
                bool changed = false;
                GUICore::InteractionState interaction = context->get_interaction_state(action.id);
                bool clicked_arrow = !action.open_on_arrow || (interaction.clicked_element_position.x >= action.arrow_min_x &&
                    interaction.clicked_element_position.x <= action.arrow_max_x);
                if(action.enabled && action.can_toggle && action.state && interaction.clicked && clicked_arrow)
                {
                    action.state->open = !action.state->open;
                    changed = true;
                }
                if(action.state)
                {
                    action.state->animation = smooth_step(action.state->animation,
                        action.state->open ? 1.0f : 0.0f, 14.0f,
                        max(context->get_frame_desc().delta_time, 0.0f));
                }
                return changed;
            }

            static bool disclosure(GUICore::IContext* context, id_t id, const c8* label,
                const GUICore::LayoutConfig& layout, const DisclosureDesc& desc, bool header,
                bool leaf, bool always_open, bool selected, bool open_on_arrow, u32 indent_depth,
                GUICore::ElementHandle* out_handle)
            {
                Ref<DisclosureState> state = widget_state<DisclosureState>(context, id);
                if(!state->initialized)
                {
                    state->open = always_open || (!leaf && desc.default_open);
                    state->animation = state->open ? 1.0f : 0.0f;
                    state->initialized = true;
                }
                if(always_open) state->open = true;
                GUICore::ElementHandle element = begin_element(context, id, label ? label : "Disclosure", layout);
                if(out_handle) *out_handle = element;
                set_interactable(context, element, desc.enabled && !leaf && !always_open);
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.disclosure");
                callbacks.measure_callback = measure_disclosure;
                context->set_layout_callback_config(element, callbacks);
                DisclosureData* data = allocate_frame<DisclosureData>(context);
                data->label = copy_frame_string(context, label);
                data->indent_depth = indent_depth;
                data->header = header;
                data->leaf = leaf;
                data->enabled = desc.enabled;
                data->selected = selected;
                data->state = state.get();
                GUICore::DrawConfig draw;
                draw.name = Name("gui.disclosure");
                draw.callback = draw_disclosure;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                DisclosureAction* action = allocate_frame<DisclosureAction>(context);
                action->id = id;
                action->enabled = desc.enabled;
                action->can_toggle = !leaf && !always_open;
                action->open_on_arrow = open_on_arrow;
                action->arrow_min_x = data->header ? 8.0f : 4.0f + 18.0f * (f32)indent_depth;
                action->arrow_max_x = action->arrow_min_x + 16.0f;
                action->state = state.get();
                add_action(context, ActionType::disclosure, id, action);
                return leaf ? false : state->open;
            }
        }

        LUNA_GUI_API bool collapsing_header(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout, const DisclosureDesc& desc, GUICore::ElementHandle* out_handle)
        {
            return Internal::disclosure(context, id, label, layout, desc, true, false, false, false, false, 0, out_handle);
        }

        LUNA_GUI_API bool tree_node(GUICore::IContext* context, id_t id, const c8* label,
            TreeNodeFlag flags, u32 indent_depth, const GUICore::LayoutConfig& layout,
            const DisclosureDesc& desc, GUICore::ElementHandle* out_handle)
        {
            return Internal::disclosure(context, id, label, layout, desc, false,
                test_flags(flags, TreeNodeFlag::leaf), test_flags(flags, TreeNodeFlag::always_open),
                test_flags(flags, TreeNodeFlag::selected), test_flags(flags, TreeNodeFlag::open_on_arrow),
                indent_depth, out_handle);
        }
    }
}
