/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Debug.hpp
* @author JXMaster
* @date 2026/6/3
*/
#pragma once
#include "Description.hpp"

#ifdef LUNA_GUI_ENABLE_DEBUG

namespace Luna
{
    namespace GUI
    {
        struct IContext;

        enum class DebugHitTestStage : u8
        {
            none,
            generic,
            scrollbar,
            dock_splitter,
            dock_panel_tab,
            dock_panel_chrome,
            tab_header,
            tab_scroll_button,
            drag_drop_source,
            drag_drop_target,
            scroll_view
        };

        struct DebugNodeKey
        {
            u64 generation = 0;
            id_t layer_id = 0;
            u32 node_index = U32_MAX;
            id_t node_id = 0;
        };

        struct DebugValue
        {
            String type_name;
            Guid type_guid;
            String value;
            bool opaque = false;
        };

        struct DebugStyleEntryInfo
        {
            Name name;
            StyleEntryState state = StyleEntryState::inherit;
            StyleValue value;
        };

        struct DebugStyleInfo
        {
            Name name;
            Name parent;
            Vector<DebugStyleEntryInfo> entries;
        };

        struct DebugResolvedStyleEntryInfo
        {
            Name name;
            bool found = false;
            bool unset = false;
            StyleValue value;
        };

        struct DebugLayoutInfo
        {
            RectF layer_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF layer_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF screen_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF screen_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            LayoutMetrics metrics;
            bool metrics_valid = false;

            Vector<f32> table_column_offsets;
            Vector<f32> table_column_widths;
            Vector<f32> table_row_offsets;
            Vector<f32> table_row_heights;
            u32 table_columns = 0;
            u32 table_rows = 0;

            RectF tab_header_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_header_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool tab_scrollable = false;
            f32 tab_scroll_max = 0.0f;
            bool tab_content_visible = true;

            Float2U scroll_content_size = Float2U(0.0f);
            Float2U scroll_viewport_size = Float2U(0.0f);
            bool scroll_has_vertical = false;
            bool scroll_has_horizontal = false;

            bool dock_panel_child = false;
            bool dock_panel_visible = true;
            bool dock_panel_floating = false;
            id_t dock_space_id = 0;
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            DockPanelStyle dock_panel_style;
            u32 dock_panel_z_order = 0;
            u32 dock_leaf_index = U32_MAX;
        };

        struct DebugNodeInfo
        {
            DebugNodeKey key;
            Guid type_guid;
            String type_name;
            String text;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            bool interactive = false;
            bool default_interactive = false;
            bool enabled = true;
            bool visible = true;
            bool hit_testable = false;
            bool debug_layer_node = false;
            bool absolute_position = false;
            Float2U position = Float2U(0.0f);
            bool has_user_clip_rect = false;
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Name style;
            LayoutStyle layout_style;
            LayoutDesc layout_desc;
            Size requested_size;
            bool render_proxy_has_draw = false;
            bool render_proxy_has_draw_after_children = false;
            usize render_proxy_draw_ptr = 0;
            usize render_proxy_draw_after_children_ptr = 0;
            usize render_proxy_userdata_ptr = 0;
            Vector<Name> drag_drop_source_types;
            Vector<Name> drag_drop_target_types;
            Vector<DebugResolvedStyleEntryInfo> resolved_style;
            Vector<Pair<Name, DebugValue>> item_query_states;
            DebugLayoutInfo layout;
        };

        struct DebugLayerInfo
        {
            u32 index = U32_MAX;
            id_t id = 0;
            u32 root = U32_MAX;
            Float2U screen_position = Float2U(0.0f);
            RectF root_screen_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool debug_layer = false;
        };

        struct DebugStateInfo
        {
            id_t id = 0;
            StateLifetime lifetime = StateLifetime::next_frame;
            u64 last_set_generation = 0;
            DebugValue data;
        };

        struct DebugInputEventInfo
        {
            InputEvent event;
            id_t hovered_before = 0;
            id_t active_before = 0;
            id_t focused_before = 0;
            id_t hovered_after = 0;
            id_t active_after = 0;
            id_t focused_after = 0;
            id_t hit_node = 0;
            u32 hit_layer = U32_MAX;
            DebugHitTestStage stage = DebugHitTestStage::none;
        };

        struct DebugContextInfo
        {
            FrameDesc frame_desc;
            u64 generation = 0;
            f64 time = 0.0;
            bool submitted = false;
            bool pointer_inside = false;
            Float2U pointer_position = Float2U(0.0f);
            Float2U pointer_delta = Float2U(0.0f);
            bool pointer_button_down[5] = {};
            bool key_down[256] = {};
            KeyModifierFlag key_modifiers = KeyModifierFlag::none;
            id_t hovered_id = 0;
            id_t active_id = 0;
            id_t focused_id = 0;
            bool drag_drop_active = false;
            id_t drag_drop_source_id = 0;
            Name drag_drop_type;
            Vector<id_t> popup_stack;
            bool has_main_hovered_node = false;
            DebugNodeKey main_hovered_node;
        };

        struct DebugInfo
        {
            DebugContextInfo context;
            Vector<DebugLayerInfo> layers;
            Vector<DebugNodeInfo> nodes;
            Vector<DebugStyleInfo> styles;
            Vector<DebugStateInfo> states;
            Vector<DebugInputEventInfo> input_events;
            Vector<String> warnings;
        };

        struct DebugPanelDesc
        {
            Float2U screen_position = Float2U(16.0f, 16.0f);
            Size size = Size::fixed(760.0f, 560.0f);
            id_t layer_id = 0xD36D0E6B6F71A11Full;
            bool show_debug_layer_nodes = false;
        };

        struct DebugInspectorState
        {
            lustruct("GUI::DebugInspectorState", "{1A73D379-3531-4791-9392-B379EF34B0A9}");
            bool has_selected_node = false;
            DebugNodeKey selected_node;
            bool has_tree_hovered_node = false;
            DebugNodeKey tree_hovered_node;
            bool has_main_hovered_node = false;
            DebugNodeKey main_hovered_node;
        };

        LUNA_GUI_API id_t debug_layer_id();
        LUNA_GUI_API ItemHandle show_debug_info(IContext* context, const DebugInfo& info, const DebugPanelDesc& desc = DebugPanelDesc());
    }
}

#endif
