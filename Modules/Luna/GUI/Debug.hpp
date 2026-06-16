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
#include "Debug.generated.hpp"

#ifdef LUNA_GUI_ENABLE_DEBUG

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        struct IContext;

        //! Identifies the input routing stage that selected one hit-test result.
        enum class DebugHitTestStage : u8
        {
            //! No hit-test stage.
            none,
            //! Generic node hit testing.
            generic,
            //! Scrollbar hit testing.
            scrollbar,
            //! Dock splitter hit testing.
            dock_splitter,
            //! Dock panel tab hit testing.
            dock_panel_tab,
            //! Dock panel title bar, close button or resize border hit testing.
            dock_panel_chrome,
            //! Tab header hit testing.
            tab_header,
            //! Tab scroll button hit testing.
            tab_scroll_button,
            //! Drag-drop source hit testing.
            drag_drop_source,
            //! Drag-drop target hit testing.
            drag_drop_target,
            //! Scroll view content hit testing.
            scroll_view
        };

        //! Stable debug key that identifies a node within one submitted frame.
        struct DebugNodeKey
        {
            //! Context generation of the debug snapshot.
            u64 generation = 0;
            //! Layer ID that owns the node.
            id_t layer_id = 0;
            //! Node index in the description.
            u32 node_index = U32_MAX;
            //! Stable node ID.
            id_t node_id = 0;
        };

        //! Serializable representation of an arbitrary boxed or typed debug value.
        struct DebugValue
        {
            //! Human-readable type name.
            String type_name;
            //! Type GUID when available.
            Guid type_guid;
            //! Human-readable value string.
            String value;
            //! Whether the value could not be decoded into a public text representation.
            bool opaque = false;
        };

        //! One raw style entry stored in a style.
        struct DebugStyleEntryInfo
        {
            //! Entry name.
            Name name;
            //! Entry inheritance state.
            StyleEntryState state = StyleEntryState::inherit;
            //! Entry value.
            StyleValue value;
        };

        //! Raw style information stored by the context style system.
        struct DebugStyleInfo
        {
            //! Style name.
            Name name;
            //! Parent style name.
            Name parent;
            //! Local style entries.
            Vector<DebugStyleEntryInfo> entries;
        };

        //! One style entry resolved for a specific node.
        struct DebugResolvedStyleEntryInfo
        {
            //! Entry name.
            Name name;
            //! Whether a value was found through the inheritance chain.
            bool found = false;
            //! Whether the entry was explicitly unset.
            bool unset = false;
            //! Resolved value.
            StyleValue value;
        };

        //! One style entry declared by a node's render proxy and resolved against the node style.
        struct DebugStyleUsageInfo
        {
            //! Entry name.
            Name name;
            //! Expected entry value type.
            StyleValueType type = StyleValueType::f32_4;
            //! Fallback value declared by the render proxy.
            StyleValue default_value;
            //! Optional human-readable display name.
            String display_name;
            //! Optional UI category.
            String category;
            //! Optional description.
            String description;
            //! Whether a value was found through the node style inheritance chain.
            bool found = false;
            //! Whether the entry was explicitly unset.
            bool unset = false;
            //! Resolved value, or @ref default_value when @ref uses_default is `true`.
            StyleValue value;
            //! Whether rendering falls back to the render proxy default value.
            bool uses_default = true;
        };

        //! Layout and subsystem-specific rectangles captured for one node.
        struct DebugLayoutInfo
        {
            //! Node rectangle in layer coordinates.
            RectF layer_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Node clip rectangle in layer coordinates.
            RectF layer_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Node rectangle in screen coordinates.
            RectF screen_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Node clip rectangle in screen coordinates.
            RectF screen_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Measured layout metrics.
            LayoutMetrics metrics;
            //! Whether @ref metrics contains valid data.
            bool metrics_valid = false;

            //! Table column offsets.
            Vector<f32> table_column_offsets;
            //! Table column widths.
            Vector<f32> table_column_widths;
            //! Table row offsets.
            Vector<f32> table_row_offsets;
            //! Table row heights.
            Vector<f32> table_row_heights;
            //! Number of table columns.
            u32 table_columns = 0;
            //! Number of table rows.
            u32 table_rows = 0;

            //! Tab header rectangle.
            RectF tab_header_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Tab header clip rectangle.
            RectF tab_header_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Tab close button rectangle.
            RectF tab_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Left tab scroll button rectangle.
            RectF tab_scroll_left_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Right tab scroll button rectangle.
            RectF tab_scroll_right_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether the tab bar is scrollable.
            bool tab_scrollable = false;
            //! Maximum tab scroll offset.
            f32 tab_scroll_max = 0.0f;
            //! Whether the tab content is visible.
            bool tab_content_visible = true;

            //! Total content size of a scroll view.
            Float2U scroll_content_size = Float2U(0.0f);
            //! Viewport size of a scroll view.
            Float2U scroll_viewport_size = Float2U(0.0f);
            //! Whether vertical scrolling is available.
            bool scroll_has_vertical = false;
            //! Whether horizontal scrolling is available.
            bool scroll_has_horizontal = false;

            //! Whether this node is managed as a dock panel child.
            bool dock_panel_child = false;
            //! Whether the dock panel is visible.
            bool dock_panel_visible = true;
            //! Whether the dock panel is floating.
            bool dock_panel_floating = false;
            //! Owning dock space ID.
            id_t dock_space_id = 0;
            //! Dock panel outer rectangle.
            RectF dock_panel_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel clip rectangle.
            RectF dock_panel_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel title bar rectangle.
            RectF dock_panel_title_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel close button rectangle.
            RectF dock_panel_close_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Dock panel resize hit rectangle.
            RectF dock_panel_resize_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Resolved dock panel style.
            DockPanelStyle dock_panel_style;
            //! Floating panel z-order.
            u32 dock_panel_z_order = 0;
            //! Dock leaf index for docked panels.
            u32 dock_leaf_index = U32_MAX;
        };

        //! Debug snapshot for one GUI node.
        struct DebugNodeInfo
        {
            //! Stable debug node key.
            DebugNodeKey key;
            //! Concrete node type GUID.
            Guid type_guid;
            //! Concrete node type name.
            String type_name;
            //! Node label or text.
            String text;
            //! Parent node index.
            u32 parent = U32_MAX;
            //! First child node index.
            u32 first_child = U32_MAX;
            //! Last child node index.
            u32 last_child = U32_MAX;
            //! Next sibling node index.
            u32 next_sibling = U32_MAX;
            //! Node depth in the tree.
            u32 depth = 0;
            //! Whether this node is interactive.
            bool interactive = false;
            //! Whether the node type is interactive by default.
            bool default_interactive = false;
            //! Whether the node is enabled.
            bool enabled = true;
            //! Whether the node is visible.
            bool visible = true;
            //! Whether the node can be hit-tested.
            bool hit_testable = false;
            //! Whether the node belongs to the debug panel layer.
            bool debug_layer_node = false;
            //! Whether the node is absolutely positioned.
            bool absolute_position = false;
            //! Absolute position when enabled.
            Float2U position = Float2U(0.0f);
            //! Whether a user clip rectangle is assigned.
            bool has_user_clip_rect = false;
            //! User clip rectangle.
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Style bound to the node.
            Name style;
            //! Layout style assigned to the node.
            LayoutStyle layout_style;
            //! Layout descriptor assigned to the node.
            LayoutDesc layout_desc;
            //! Requested size assigned to the node.
            Size requested_size;
            //! Whether the render proxy has a pre-children draw callback.
            bool render_proxy_has_draw = false;
            //! Whether the render proxy has an after-children draw callback.
            bool render_proxy_has_draw_after_children = false;
            //! Debug pointer value for the pre-children draw callback.
            usize render_proxy_draw_ptr = 0;
            //! Debug pointer value for the after-children draw callback.
            usize render_proxy_draw_after_children_ptr = 0;
            //! Debug pointer value for render proxy user data.
            usize render_proxy_userdata_ptr = 0;
            //! Drag-drop source payload types.
            Vector<Name> drag_drop_source_types;
            //! Drag-drop target payload types.
            Vector<Name> drag_drop_target_types;
            //! Style entries resolved for this node.
            Vector<DebugResolvedStyleEntryInfo> resolved_style;
            //! Style entries declared by this node's render proxy, with their resolved values.
            Vector<DebugStyleUsageInfo> style_usage;
            //! Public item query states stored for this node.
            Vector<Pair<Name, DebugValue>> item_query_states;
            //! Layout data for this node.
            DebugLayoutInfo layout;
        };

        //! Debug snapshot for one layer.
        struct DebugLayerInfo
        {
            //! Layer array index.
            u32 index = U32_MAX;
            //! Stable layer ID.
            id_t id = 0;
            //! Root node index.
            u32 root = U32_MAX;
            //! Layer top-left position in screen coordinates.
            Float2U screen_position = Float2U(0.0f);
            //! Root rectangle in screen coordinates.
            RectF root_screen_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether this is the debug panel layer.
            bool debug_layer = false;
        };

        //! Debug snapshot for one context state object.
        struct DebugStateInfo
        {
            //! State ID.
            id_t id = 0;
            //! State lifetime.
            StateLifetime lifetime = StateLifetime::next_frame;
            //! Last generation in which this state was set.
            u64 last_set_generation = 0;
            //! Serialized state data.
            DebugValue data;
        };

        //! Debug snapshot for one processed input event.
        struct DebugInputEventInfo
        {
            //! Original input event.
            InputEvent event;
            //! Hovered item before processing this event.
            id_t hovered_before = 0;
            //! Active item before processing this event.
            id_t active_before = 0;
            //! Focused item before processing this event.
            id_t focused_before = 0;
            //! Hovered item after processing this event.
            id_t hovered_after = 0;
            //! Active item after processing this event.
            id_t active_after = 0;
            //! Focused item after processing this event.
            id_t focused_after = 0;
            //! Node hit by this event.
            id_t hit_node = 0;
            //! Layer hit by this event.
            u32 hit_layer = U32_MAX;
            //! Hit-test stage that selected the hit node.
            DebugHitTestStage stage = DebugHitTestStage::none;
        };

        //! Debug snapshot for context-level state.
        struct DebugContextInfo
        {
            //! Current frame descriptor.
            FrameDesc frame_desc;
            //! Current context generation.
            u64 generation = 0;
            //! Accumulated context time in seconds.
            f64 time = 0.0;
            //! Whether a description has been submitted.
            bool submitted = false;
            //! Whether the pointer is inside the GUI screen.
            bool pointer_inside = false;
            //! Current pointer position.
            Float2U pointer_position = Float2U(0.0f);
            //! Current pointer delta.
            Float2U pointer_delta = Float2U(0.0f);
            //! Current pointer button states.
            bool pointer_button_down[5] = {};
            //! Current key states.
            bool key_down[256] = {};
            //! Current key modifiers.
            KeyModifierFlag key_modifiers = KeyModifierFlag::none;
            //! Current hovered item ID.
            id_t hovered_id = 0;
            //! Current active item ID.
            id_t active_id = 0;
            //! Current focused item ID.
            id_t focused_id = 0;
            //! Whether drag-drop is active.
            bool drag_drop_active = false;
            //! Active drag-drop source ID.
            id_t drag_drop_source_id = 0;
            //! Active drag-drop payload type.
            Name drag_drop_type;
            //! Open popup stack, ordered from parent to child.
            Vector<id_t> popup_stack;
            //! Whether the main UI has a hovered node.
            bool has_main_hovered_node = false;
            //! Main UI hovered node key.
            DebugNodeKey main_hovered_node;
        };

        //! Complete serializable debug snapshot returned by @ref IContext::dump_debug_info.
        struct DebugInfo
        {
            //! Context-level state.
            DebugContextInfo context;
            //! Layer snapshots.
            Vector<DebugLayerInfo> layers;
            //! Node snapshots.
            Vector<DebugNodeInfo> nodes;
            //! Raw style table snapshots.
            Vector<DebugStyleInfo> styles;
            //! Context state object snapshots.
            Vector<DebugStateInfo> states;
            //! Processed input event snapshots.
            Vector<DebugInputEventInfo> input_events;
            //! Warnings generated while collecting debug information.
            Vector<String> warnings;
        };

        //! Parameters for the built-in debug panel visualization.
        struct DebugPanelDesc
        {
            //! Initial debug panel position in screen coordinates.
            Float2U screen_position = Float2U(16.0f, 16.0f);
            //! Initial debug panel size.
            Size size = Size::fixed(760.0f, 560.0f);
            //! Stable layer ID used by the debug panel.
            id_t layer_id = 0xD36D0E6B6F71A11Full;
            //! Whether debug panel nodes should be shown in the tree.
            bool show_debug_layer_nodes = false;
        };

        //! UI state used by the built-in debug panel visualization.
        struct [[Luna::struct("{1A73D379-3531-4791-9392-B379EF34B0A9}")]] DebugInspectorState
        {
            //! Whether a node is selected in the inspector.
            bool has_selected_node = false;
            //! Selected node key.
            DebugNodeKey selected_node;
            //! Whether the tree is hovering a node row.
            bool has_tree_hovered_node = false;
            //! Tree-hovered node key.
            DebugNodeKey tree_hovered_node;
            //! Whether the main UI hover mirror has a node.
            bool has_main_hovered_node = false;
            //! Main UI hovered node key.
            DebugNodeKey main_hovered_node;
            //! Whether the debug panel dock layout has been initialized.
            bool dock_layout_initialized = false;
        };

        //! Gets the default debug panel layer ID.
        //! @return Returns the debug panel layer ID.
        LUNA_GUI_API id_t debug_layer_id();
        //! Builds the built-in debug panel UI from a debug snapshot.
        //! @param[in] context The GUI context to build the panel in.
        //! @param[in] info The debug information snapshot to visualize.
        //! @param[in] desc Debug panel placement and filtering options.
        //! @return Returns the debug panel item handle.
        //! @remark This is a high-level visualization helper. The lower-level debug data contract is @ref DebugInfo.
        LUNA_GUI_API ItemHandle show_debug_info(IContext* context, const DebugInfo& info, const DebugPanelDesc& desc = DebugPanelDesc());

        //! @}
    }
}

#endif
