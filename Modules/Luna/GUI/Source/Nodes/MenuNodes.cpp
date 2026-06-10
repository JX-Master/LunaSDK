/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "MenuNodes.hpp"
#include "../RenderProxies/MenuRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        MenuSeparatorNode::MenuSeparatorNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_menu_separator_render_proxy();
        }

        Guid MenuSeparatorNode::type_guid() const
        {
            return Meta::StructMetaData<MenuSeparatorNode>::__guid;
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

        MenuItemNode::MenuItemNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_menu_item_render_proxy();
        }

        Guid MenuItemNode::type_guid() const
        {
            return Meta::StructMetaData<MenuItemNode>::__guid;
        }

        Ref<Node> MenuItemNode::clone() const
        {
            return new_object<MenuItemNode>(*this);
        }

        bool MenuItemNode::enabled_state() const
        {
            return item_enabled && enabled;
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
            metrics.min_size = Float2U(top_level_menu ? max(width, 42.0f) : 96.0f, height);
            metrics.preferred_size = Float2U(width, height);
            metrics.max_size = Float2U(F32_MAX, height);
            return metrics;
        }

        LayoutMetrics MenuItemNode::measure(NodeMeasureContext& ctx) const
        {
            return measure();
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
            if(!enabled_state()) return;
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
