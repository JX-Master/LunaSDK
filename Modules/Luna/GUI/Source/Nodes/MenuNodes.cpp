/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/MenuNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        MenuSeparatorNode::MenuSeparatorNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid MenuSeparatorNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> MenuSeparatorNode::clone() const
        {
            return new_object<MenuSeparatorNode>(*this);
        }

        LayoutMetrics MenuSeparatorNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(1.0f, 7.0f);
            metrics.preferred_size = Float2U(96.0f, 7.0f);
            metrics.max_size = Float2U(F32_MAX, 7.0f);
            return metrics;
        }

        void MenuSeparatorNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            f32 y = rect.offset_y + rect.height * 0.5f;
            ctx.draw_line(Float2U(rect.offset_x + 8.0f, y), Float2U(rect.offset_x + max(rect.width - 8.0f, 8.0f), y),
                clip_rect, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 1.0f);
        }

        MenuItemNode::MenuItemNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid MenuItemNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> MenuItemNode::clone() const
        {
            return new_object<MenuItemNode>(*this);
        }

        bool MenuItemNode::enabled_state() const
        {
            return enabled;
        }

        bool MenuItemNode::checked() const
        {
            return selected_value ? *selected_value : selected;
        }

        LayoutMetrics MenuItemNode::measure() const
        {
            f32 text_width = (f32)text.size() * 15.0f * 0.52f;
            f32 shortcut_width = shortcut.empty() ? 0.0f : (f32)shortcut.size() * 15.0f * 0.52f + 28.0f;
            f32 width = top_level_menu ? max(text_width + 22.0f, 42.0f) : max(text_width + shortcut_width + (popup_id ? 62.0f : 42.0f), 132.0f);
            f32 height = top_level_menu ? 24.0f : 26.0f;
            LayoutMetrics metrics;
            metrics.min_size = Float2U(top_level_menu ? min(width, 42.0f) : 96.0f, height);
            metrics.preferred_size = Float2U(width, height);
            metrics.max_size = Float2U(F32_MAX, height);
            return metrics;
        }

        LayoutMetrics MenuItemNode::measure(NodeMeasureContext& ctx) const
        {
            return measure();
        }

        void MenuItemNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            bool open = popup_id && ctx.is_popup_open(popup_id);
            Float4U text_color = enabled ? Float4U(1.0f) : Float4U(0.55f, 0.59f, 0.65f, 1.0f);
            if(open || state.hovered || state.active)
            {
                ctx.draw_rect(rect, clip_rect, state.active || open ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : Float4U(0.20f, 0.30f, 0.44f, 1.0f),
                    top_level_menu ? 4.0f : 3.0f);
            }
            if(checked() && !top_level_menu)
            {
                f32 x0 = rect.offset_x + 8.0f;
                f32 y0 = rect.offset_y + rect.height * 0.56f;
                f32 x1 = rect.offset_x + 13.0f;
                f32 y1 = rect.offset_y + rect.height * 0.72f;
                f32 x2 = rect.offset_x + 22.0f;
                f32 y2 = rect.offset_y + rect.height * 0.32f;
                ctx.draw_line(Float2U(x0, y0), Float2U(x1, y1), clip_rect, text_color, 2.0f);
                ctx.draw_line(Float2U(x1, y1), Float2U(x2, y2), clip_rect, text_color, 2.0f);
            }
            f32 text_x = top_level_menu ? 10.0f : 30.0f;
            f32 text_pad_right = top_level_menu ? 20.0f : (popup_id ? 50.0f : 74.0f);
            ctx.draw_text(RectF(rect.offset_x + text_x, rect.offset_y, max(rect.width - text_x - text_pad_right, 1.0f), rect.height),
                clip_rect, text.c_str(), 15.0f, text_color, TextAlignment::begin);
            if(popup_id && !top_level_menu)
            {
                ctx.draw_text(RectF(rect.offset_x + max(rect.width - 22.0f, 0.0f), rect.offset_y, 18.0f, rect.height),
                    clip_rect, ">", 15.0f, text_color, TextAlignment::center);
            }
            else if(!shortcut.empty() && !top_level_menu)
            {
                ctx.draw_text(RectF(rect.offset_x + max(rect.width - 88.0f, 0.0f), rect.offset_y, 80.0f, rect.height),
                    clip_rect, shortcut.c_str(), 14.0f, Float4U(text_color.x, text_color.y, text_color.z, 0.72f), TextAlignment::end);
            }
        }

        void MenuItemNode::update_state(NodeInputContext& ctx) const
        {
            if(popup_id)
            {
                ctx.set_state(Name("gui.open"), Any(ctx.is_popup_open(popup_id)));
            }
        }

        void MenuItemNode::on_click(NodeInputContext& ctx)
        {
            if(!enabled) return;
            if(popup_id)
            {
                if(ctx.is_popup_open(popup_id))
                {
                    ctx.close_popup(popup_id);
                    ctx.set_state(Name("gui.open"), Any(false));
                }
                else
                {
                    ctx.open_menu_popup(id);
                    ctx.set_state(Name("gui.open"), Any(true));
                }
                return;
            }
            if(selected_value)
            {
                *selected_value = !*selected_value;
                selected = *selected_value;
                ctx.set_state(Name("gui.value_changed"), Any(true));
            }
            ctx.close_all_popups();
        }

    }
}
