/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "DockRenderProxies.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static bool point_in_rect_local(const Float2U& pos, const RectF& rect)
        {
            return pos.x >= rect.offset_x && pos.x <= rect.offset_x + rect.width &&
                pos.y >= rect.offset_y && pos.y <= rect.offset_y + rect.height;
        }

        static bool item_query_bool(NodeRenderContext& ctx, id_t item_id, const Name& state_name)
        {
            ItemQueryState* query_state = ctx.get_widget_state<ItemQueryState>(item_id);
            if(!query_state) return false;
            auto iter = query_state->states.find(state_name);
            return iter != query_state->states.end() && iter->second.as<bool>() && *iter->second.as<bool>();
        }

        static void draw_dock_panel_border(NodeRenderContext& ctx, const RectF& rect, const RectF& clip,
            const DockPanelStyle& style)
        {
            if(style.border_size <= 0.0f) return;
            f32 b = style.border_size;
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, rect.width, b), clip, style.border_color, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + rect.height - b, rect.width, b), clip, style.border_color, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, b, rect.height), clip, style.border_color, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x + rect.width - b, rect.offset_y, b, rect.height), clip, style.border_color, 0.0f);
        }

        static void draw_dock_panel_resize_mark(NodeRenderContext& ctx, const Node& node, const NodeRenderLayout& layout,
            const NodeRenderState& state)
        {
            if(!layout.dock_panel_style.resize_border) return;
            RectF resize_rect = layout.dock_panel_resize_rect;
            if(resize_rect.width <= 0.0f || resize_rect.height <= 0.0f) return;
            bool hovered = point_in_rect_local(state.pointer_position, resize_rect);
            DockInteractionState* dock_interaction = ctx.get_widget_state<DockInteractionState>(0);
            bool active = dock_interaction && dock_interaction->active_dock_panel_id == node.id;
            RectF clip = layout.dock_panel_clip_rect;
            if(layout.dock_panel_floating)
            {
                Float4U color = (hovered || active) ? Float4U(0.55f, 0.68f, 0.86f, 1.0f) : Float4U(0.36f, 0.42f, 0.50f, 0.85f);
                ctx.draw_line(Float2U(resize_rect.offset_x + resize_rect.width - 2.0f, resize_rect.offset_y + 2.0f),
                    Float2U(resize_rect.offset_x + 2.0f, resize_rect.offset_y + resize_rect.height - 2.0f), clip, color, 1.5f);
            }
            else
            {
                Float4U color = (hovered || active) ? Float4U(0.55f, 0.68f, 0.86f, 1.0f) : Float4U(0.30f, 0.35f, 0.42f, 0.85f);
                f32 y = resize_rect.offset_y + resize_rect.height * 0.5f;
                ctx.draw_line(Float2U(resize_rect.offset_x + 8.0f, y),
                    Float2U(resize_rect.offset_x + max(resize_rect.width - 8.0f, 8.0f), y), clip, color, 1.5f);
            }
        }

        static void draw_dock_panel_title_tabs(NodeRenderContext& ctx, const NodeRenderLayout& layout, bool active,
            const DockTreeNode& leaf, const NodeRenderState& state)
        {
            const DockPanelStyle& style = layout.dock_panel_style;
            RectF clip = layout.dock_panel_clip_rect;
            for(usize tab_index = 0; tab_index < leaf.tabs.size(); ++tab_index)
            {
                id_t tab_id = leaf.tabs[tab_index];
                RectF tab_rect = dock_panel_tab_rect(layout.dock_panel_title_rect, tab_index, leaf.tabs.size(), style.close_button);
                bool tab_selected = tab_id == leaf.selected_tab;
                bool tab_hovered = point_in_rect_local(state.pointer_position, tab_rect);
                Float4U tab_color = tab_selected ? (active ? style.active_title_bar_color : Float4U(0.16f, 0.21f, 0.28f, 1.0f)) :
                    (tab_hovered ? Float4U(0.18f, 0.24f, 0.32f, 1.0f) : Float4U(0.10f, 0.13f, 0.17f, 1.0f));
                ctx.draw_rect(tab_rect, clip, tab_color, 4.0f);
                const Node* tab_node = ctx.find_node(tab_id);
                const c8* label = tab_node ? tab_node->text.c_str() : "";
                ctx.draw_text(RectF(tab_rect.offset_x + 7.0f, tab_rect.offset_y, max(tab_rect.width - 14.0f, 1.0f), tab_rect.height),
                    clip, label, 14.0f, Color::white(), TextAlignment::begin);
            }
        }

        void draw_dock_panel_chrome(NodeRenderContext& ctx, const Node& node, const NodeRenderState& state)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            if(!layout.dock_panel_child || !layout.dock_panel_visible) return;

            const DockPanelStyle& style = layout.dock_panel_style;
            const RectF& panel_rect = layout.dock_panel_rect;
            const RectF& clip = layout.dock_panel_clip_rect;
            DockInteractionState* dock_interaction = ctx.get_widget_state<DockInteractionState>(0);
            bool active = (dock_interaction && dock_interaction->active_dock_panel_id == node.id) ||
                item_query_bool(ctx, node.id, Name("gui.focused"));

            ctx.draw_rect(panel_rect, clip, style.background_color, 5.0f);
            if(style.title_bar)
            {
                ctx.draw_rect(layout.dock_panel_title_rect, clip, active ? style.active_title_bar_color : style.title_bar_color, 5.0f);
                const DockTreeNode* leaf = nullptr;
                DockSpaceState* dock_state = ctx.get_widget_state<DockSpaceState>(layout.dock_space_id);
                if(dock_state && layout.dock_leaf_index < dock_state->dock_nodes.size())
                {
                    const DockTreeNode& dock_leaf = dock_state->dock_nodes[layout.dock_leaf_index];
                    if(!dock_leaf.split && dock_leaf.tabs.size() > 1)
                    {
                        leaf = &dock_leaf;
                    }
                }
                if(leaf && dock_state)
                {
                    draw_dock_panel_title_tabs(ctx, layout, active, *leaf, state);
                }
                else
                {
                    ctx.draw_text(RectF(layout.dock_panel_title_rect.offset_x + 8.0f, layout.dock_panel_title_rect.offset_y,
                        max(layout.dock_panel_title_rect.width - 40.0f, 1.0f), layout.dock_panel_title_rect.height),
                        clip, node.text.c_str(), 15.0f, Color::white(), TextAlignment::begin);
                }
                if(style.close_button)
                {
                    bool close_hovered = point_in_rect_local(state.pointer_position, layout.dock_panel_close_rect);
                    ctx.draw_rect(layout.dock_panel_close_rect, clip,
                        close_hovered ? Float4U(0.55f, 0.18f, 0.18f, 1.0f) : Float4U(0.23f, 0.27f, 0.33f, 1.0f),
                        4.0f);
                    ctx.draw_text(layout.dock_panel_close_rect, clip, "X", 13.0f, Color::white(), TextAlignment::center);
                }
            }
            draw_dock_panel_border(ctx, panel_rect, clip, style);
            draw_dock_panel_resize_mark(ctx, node, layout, state);
        }

        void draw_dock_space_splitters(NodeRenderContext& ctx, const Node& node, const NodeRenderState& state)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            DockSpaceState* dock_state = ctx.get_widget_state<DockSpaceState>(node.id);
            if(!dock_state || dock_state->dock_root_node == U32_MAX || dock_state->dock_root_node >= dock_state->dock_nodes.size()) return;
            DockInteractionState* dock_interaction = ctx.get_widget_state<DockInteractionState>(0);
            Vector<u32> stack;
            stack.push_back(dock_state->dock_root_node);
            while(!stack.empty())
            {
                u32 dock_node_index = stack.back();
                stack.pop_back();
                if(dock_node_index >= dock_state->dock_nodes.size()) continue;
                const DockTreeNode& dock_node = dock_state->dock_nodes[dock_node_index];
                if(!dock_node.split) continue;
                stack.push_back(dock_node.child1);
                stack.push_back(dock_node.child0);
                if(dock_node.split_rect.width <= 0.0f || dock_node.split_rect.height <= 0.0f) continue;
                bool hovered_splitter = point_in_rect_local(state.pointer_position, dock_node.split_rect);
                bool active_splitter = dock_interaction &&
                    dock_interaction->active_dock_split_space_id == node.id &&
                    dock_interaction->active_dock_split_node == dock_node_index;
                Float4U splitter_color = active_splitter ? Float4U(0.36f, 0.58f, 0.90f, 1.0f) :
                    (hovered_splitter ? Float4U(0.28f, 0.42f, 0.62f, 0.95f) : Float4U(0.11f, 0.14f, 0.18f, 0.90f));
                ctx.draw_rect(dock_node.split_rect, layout.clip_rect, splitter_color, 0.0f);
            }
        }
    }
}
