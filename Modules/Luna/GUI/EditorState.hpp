/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorState.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include "Base.hpp"
#include <Luna/GUICore/GUICore.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include "EditorState.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Bit flags controlling tree node behavior.
        enum class TreeNodeFlag : u32
        {
            //! Default tree node behavior.
            none = 0x00,
            //! Render the node as selected.
            selected = 0x01,
            //! Render the node as a leaf with no disclosure state.
            leaf = 0x02,
            //! Open the node by default when no previous state exists.
            default_open = 0x04,
            //! Toggle open state only when the arrow area is clicked.
            open_on_arrow = 0x08
        };

        //! Bit flags controlling tab bar behavior.
        enum class TabBarFlag : u32
        {
            //! Default tab bar behavior.
            none = 0x00,
            //! Allow user-driven tab reordering.
            reorderable = 0x01,
            //! Shrink tab widths to fit available space.
            fitting_shrink = 0x02,
            //! Use scroll buttons when tab headers exceed available space.
            fitting_scroll = 0x04,
            //! Select newly appearing tabs automatically.
            auto_select_new_tabs = 0x08
        };

        //! Bit flags controlling one tab item.
        enum class TabItemFlag : u32
        {
            //! Default tab item behavior.
            none = 0x00,
            //! Request this tab to be selected.
            selected = 0x01,
            //! Hide the close button even when an open pointer is supplied.
            no_close_button = 0x02,
            //! Mark the tab as containing unsaved changes.
            unsaved_document = 0x04,
            //! Prevent this tab from being reordered.
            no_reorder = 0x08,
            //! Render this item as a tab-strip button instead of a tab with content.
            button = 0x10
        };

        //! Bit flags controlling popup lifetime and input behavior.
        enum class PopupFlag : u32
        {
            //! No popup flags.
            none = 0x00,
            //! Let the popup stack manage open and close behavior.
            managed = 0x01,
            //! Close the popup when a pointer click lands outside the popup stack.
            close_on_outside_click = 0x02,
            //! Close the popup when Escape is pressed.
            close_on_escape = 0x04,
            //! Close the popup when focus is lost.
            close_on_blur = 0x08,
            //! Treat the popup as modal input layer.
            modal = 0x10
        };

        //! Bit flags controlling numeric drag editing behavior.
        enum class NumericEditFlag : u32
        {
            //! Default drag behavior.
            none = 0x00,
            //! Allow the drag widget to enter text input mode on double click.
            input_on_double_click = 0x01
        };

        //! Parameters used when creating one editor-style popup layer.
        struct PopupDesc
        {
            //! Popup top-left position in screen logical coordinates.
            Float2U position = Float2U(0.0f);
            //! Requested popup root layout. Fit axes use the popup content hug size.
            GUICore::LayoutInput layout;
            //! Popup lifetime and input flags.
            PopupFlag flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
        };

        //! Parameters used when creating an editor-style tooltip layer.
        struct TooltipDesc
        {
            //! Offset from owner hover position to tooltip layer position.
            Float2U offset = Float2U(14.0f, 18.0f);
            //! Requested tooltip root layout. Fit axes use the tooltip content hug size.
            GUICore::LayoutInput layout;
            //! Hover delay before the tooltip is displayed, in seconds.
            f32 delay = 0.35f;
            //! Preferred text wrapping width for simple text tooltips.
            f32 max_width = 360.0f;
        };

        //! Placement mode for a dock panel.
        enum class DockPanelMode : u8
        {
            //! Place the panel into the dock tree.
            docking,
            //! Place the panel as a floating panel above docked content.
            floating
        };

        //! Axis used by one dock space split node.
        enum class DockSplitAxis : u8
        {
            //! Split the region into left and right children.
            x,
            //! Split the region into upper and lower children.
            y
        };

        //! Describes one node in a dock space layout tree.
        struct DockSpaceLayoutNodeDesc
        {
            //! Whether this node splits its rectangle into two child rectangles.
            bool split = false;
            //! Split axis used when @ref split is `true`.
            DockSplitAxis split_axis = DockSplitAxis::x;
            //! Fraction of the available split axis length assigned to @ref child0.
            f32 split_ratio = 0.5f;
            //! Index of the first child node in @ref DockSpaceLayoutDesc::nodes.
            u32 child0 = U32_MAX;
            //! Index of the second child node in @ref DockSpaceLayoutDesc::nodes.
            u32 child1 = U32_MAX;
            //! Dock panel IDs stacked in this leaf as tabs when @ref split is `false`.
            Vector<id_t> tabs;
            //! Selected dock panel ID for this leaf. If zero or absent from @ref tabs, the first live tab is selected.
            id_t selected_tab = 0;
        };

        //! Describes one floating dock panel in a dock space layout.
        struct DockSpaceFloatingPanelDesc
        {
            //! Dock panel ID.
            id_t panel = 0;
            //! Floating panel rectangle in dock space coordinates.
            RectF rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
            //! Floating panel Z order. A value of 0 lets the context assign one after existing panels.
            u32 z_order = 0;
        };

        //! Describes the full layout stored by one dock space.
        struct DockSpaceLayoutDesc
        {
            //! Dock tree nodes. Split nodes reference children by index, leaf nodes hold tab panel IDs.
            Vector<DockSpaceLayoutNodeDesc> nodes;
            //! Index of the root node in @ref nodes. `U32_MAX` means the dock tree is empty.
            u32 root_node = U32_MAX;
            //! Floating panels drawn above the docked tree.
            Vector<DockSpaceFloatingPanelDesc> floating_panels;
        };

        //! Style for panels managed by an editor-style dock space.
        struct DockPanelStyle
        {
            //! Whether the panel displays a title bar.
            bool title_bar = true;
            //! Whether the panel displays a close button when an open pointer is supplied.
            bool close_button = true;
            //! Whether the floating panel has a resize border.
            bool resize_border = true;
            //! Title bar height in logical units.
            f32 title_bar_height = 28.0f;
            //! Border thickness in logical units.
            f32 border_size = 1.0f;
            //! Floating panel resize hit-test thickness in logical units.
            f32 resize_border_size = 6.0f;
            //! Minimum size for user-resized floating panels.
            Float2U min_floating_size = Float2U(120.0f, 80.0f);
            //! Panel background color used by the editor-style drawing code.
            Float4U background_color = Float4U(0.09f, 0.11f, 0.14f, 0.96f);
            //! Inactive title bar color used by the editor-style drawing code.
            Float4U title_bar_color = Float4U(0.13f, 0.17f, 0.22f, 1.0f);
            //! Active title bar color used by the editor-style drawing code.
            Float4U active_title_bar_color = Float4U(0.16f, 0.24f, 0.36f, 1.0f);
            //! Border color used by the editor-style drawing code.
            Float4U border_color = Float4U(0.24f, 0.29f, 0.36f, 1.0f);
        };

        //! Disclosure state used by editor-style collapsing headers and tree nodes.
        struct [[Luna::struct("{62263BD7-3405-493F-8DDB-B3D089412ACA}")]] DisclosureState
        {
            //! Whether the disclosure content is currently open.
            bool open = true;
            //! Whether the initial open state has been applied.
            bool open_initialized = false;
        };

        //! Visual animation state used by editor-style switch widgets.
        struct [[Luna::struct("{C8CC1A97-56C5-49B4-962D-70C1A1A7CC9F}")]] SwitchAnimationState
        {
            //! Current switch knob animation value.
            f32 animation = 0.0f;
            //! Whether @ref animation has been initialized from current widget value.
            bool initialized = false;
        };

        //! Visual animation state used by editor-style buttons.
        struct [[Luna::struct("{B00FEB21-FE08-459E-87BC-FD15468BB6B6}")]] ButtonAnimationState
        {
            //! Current animated button fill color.
            Float4U color = Float4U(0.18f, 0.28f, 0.45f, 1.0f);
            //! Whether @ref color has been initialized.
            bool initialized = false;
        };

        //! Visual animation state used by editor-style button groups.
        struct [[Luna::struct("{2FB79430-D36D-42E1-BF3D-2FDD21D351AF}")]] ButtonGroupAnimationState
        {
            //! Selection indicator animation value for single-selection groups.
            f32 selection_animation = 0.0f;
            //! Whether @ref selection_animation has been initialized.
            bool selection_animation_initialized = false;
            //! Per-item fill animation values for multi-selection groups.
            Vector<f32> item_animations;
        };

        //! Visual animation state used by editor-style tab bars.
        struct [[Luna::struct("{68A87F19-D1C3-4496-AE46-87A2DFB61E3D}")]] TabBarAnimationState
        {
            //! Current animated selected tab header rectangle.
            RectF selection_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            //! Whether @ref selection_rect has been initialized.
            bool selection_rect_initialized = false;
        };

        //! Persistent selection and ordering state used by editor-style tab bars.
        struct [[Luna::struct("{AEA771D2-1441-4CE9-9876-38F1787F2C49}")]] TabBarState
        {
            //! Currently selected tab item ID.
            id_t tab_selected_id = 0;
            //! Horizontal header scroll offset.
            f32 tab_scroll_x = 0.0f;
            //! Stable tab order captured from submitted tab items.
            Vector<id_t> tab_order;
        };

        //! Per-frame direct-core tab bar build scope.
        struct [[Luna::struct("{374C75C7-5C59-4CB9-ABB2-B9403A4B790A}")]] CoreTabBuildScope
        {
            //! Tab bar currently receiving tab items.
            id_t tab_bar_id = 0;
            //! Selected tab ID chosen for this frame.
            id_t selected_id = 0;
            //! First open tab ID submitted this frame.
            id_t first_open_id = 0;
            //! Whether a visible tab has already been chosen in this scope.
            bool visible_tab_chosen = false;
            //! Header element IDs generated for this tab bar.
            Vector<id_t> header_ids;
        };

        //! Per-frame direct-core tab bar build stack.
        struct [[Luna::struct("{DAFAAB34-B607-445B-8FD6-5E6FC9BE52E9}")]] CoreTabBuildState
        {
            //! Stack of nested tab build scopes.
            Vector<CoreTabBuildScope> stack;
        };

        //! Per-frame dock panel information collected while building a direct-core dock space.
        struct CoreDockPanelBuildInfo
        {
            //! Stable dock panel ID.
            id_t id = 0;
            //! Element handle for the dock panel root.
            GUICore::ElementHandle handle;
            //! Visual style requested for this panel.
            DockPanelStyle style;
            //! Optional open flag owned by the caller.
            bool* open = nullptr;
            //! Display label used for title rendering and debugging.
            String label;
        };

        //! Persistent layout state used by editor-style direct-core dock spaces.
        struct [[Luna::struct("{E49F6104-6A01-45B5-BD6D-16970F771216}")]] CoreDockSpaceState
        {
            //! Whether @ref layout has been explicitly supplied.
            bool has_layout = false;
            //! Dock tree and floating-panel layout description.
            DockSpaceLayoutDesc layout;
        };

        //! Per-frame direct-core dock space build scope.
        struct CoreDockBuildScope
        {
            //! Stable dock space ID.
            id_t dock_space_id = 0;
            //! Element handle for the dock space root.
            GUICore::ElementHandle dock_space;
            //! Panels submitted into this dock space during the current frame.
            Vector<CoreDockPanelBuildInfo> panels;
        };

        //! Per-frame direct-core dock build stack.
        struct [[Luna::struct("{0D98A82E-DDE0-4345-A9F8-1CC9D131BEE3}")]] CoreDockBuildState
        {
            //! Stack of nested dock spaces currently being built.
            Vector<CoreDockBuildScope> stack;
        };

        //! Per-frame direct-core table build scope.
        struct CoreTableBuildScope
        {
            //! Stable table element ID.
            id_t table_id = 0;
            //! Table element currently receiving rows.
            GUICore::ElementHandle table;
            //! Column tracks submitted by the caller.
            Vector<GUICore::TableTrackDesc> columns;
            //! Row tracks submitted by the caller.
            Vector<GUICore::TableTrackDesc> rows;
            //! Cell attachments collected from row scopes.
            Vector<GUICore::TableLayoutCell> cells;
            //! Gap between table columns and rows.
            Float2U gap = Float2U(0.0f);
            //! Padding assigned to cells collected by row scopes.
            Float4U cell_padding = Float4U(0.0f);
            //! Whether table children should be clipped.
            bool clip_children = true;
            //! Whether one row scope is currently open.
            bool row_open = false;
            //! Index of the row currently being built.
            u32 current_row = 0;
            //! Number of direct children present when the current row began.
            usize row_child_begin = 0;
            //! Maximum number of columns observed across submitted rows.
            u32 max_columns = 0;
        };

        //! Per-frame direct-core table build stack.
        struct [[Luna::struct("{037BC51A-E265-4854-818C-56CB1AF486DB}")]] CoreTableBuildState
        {
            //! Stack of nested tables currently being built.
            Vector<CoreTableBuildScope> stack;
        };

        //! Persistent open state used by editor-style popup layers.
        struct [[Luna::struct("{2E1149AD-DE84-4E34-AE28-393843D3E6B7}")]] CorePopupState
        {
            //! Whether the popup is currently open.
            bool open = false;
        };

        //! Per-frame hover timing state used by editor-style tooltip layers.
        struct [[Luna::struct("{C5ECA343-19C6-44BF-9163-DC8F0896A372}")]] CoreTooltipState
        {
            //! Owner element currently being hovered.
            id_t hovered_owner = 0;
            //! Time accumulated while hovering @ref hovered_owner.
            f32 hover_time = 0.0f;
        };

        //! Per-frame state used by editor-style combo boxes.
        struct [[Luna::struct("{84052EEB-AE79-44BD-A9EC-97F8B6BEAF4A}")]] CoreComboState
        {
            //! Screen position of the combo popup layer.
            Float2U popup_position = Float2U(0.0f);
        };

        //! Placement mode for legacy-compatible editor popup anchors.
        enum class PopupAnchorPlacement : u8
        {
            //! Anchor the popup at the pointer position captured by the owner widget.
            pointer,
            //! Anchor the popup below the owner widget.
            owner_down
        };

        //! Persistent popup anchor state shared by editor-style popup views.
        struct [[Luna::struct("{9BEED835-1593-4FAF-B0F7-FC753D462883}")]] PopupAnchorState
        {
            //! Screen-space popup anchor position.
            Float2U popup_anchor_position = Float2U(0.0f, 0.0f);
            //! Placement mode used to interpret @ref popup_anchor_position.
            PopupAnchorPlacement popup_anchor_placement = PopupAnchorPlacement::pointer;
            //! Whether @ref popup_anchor_position contains a captured anchor.
            bool popup_anchor_valid = false;
        };

        //! Persistent popup placement state used by editor-style menus.
        struct [[Luna::struct("{37A6BA4C-4FA2-43B5-A64E-6C231748366B}")]] CoreMenuPopupState
        {
            //! Screen position of the menu popup layer.
            Float2U popup_position = Float2U(0.0f);
        };

        //! Per-frame build scope used by editor-style menu popup stacks.
        struct [[Luna::struct("{A316D6D8-E933-491F-8DA6-E63017D4BA58}")]] CoreMenuBuildScope
        {
            //! Popup ID associated with this menu scope.
            id_t popup_id = 0;
            //! Root element of the popup layer.
            GUICore::ElementHandle popup_root;
        };

        //! Per-frame build state used by editor-style menu bars and menus.
        struct [[Luna::struct("{5AD9FC15-A6C0-45FF-9A24-C1116B48A588}")]] CoreMenuBuildState
        {
            //! Stack of menu bars currently being built.
            Vector<GUICore::ElementHandle> menu_bar_stack;
            //! Stack of open menu popup scopes currently being built.
            Vector<CoreMenuBuildScope> menu_stack;
        };

        //! Persistent selection state used by editor-style GUI Core debug panels.
        struct [[Luna::struct("{912EB38E-0181-44F3-9729-DC8A7D353F5D}")]] CoreDebugPanelState
        {
            //! Selected GUI Core element ID.
            GUICore::id_t selected_element = 0;
        };

        //! Persistent scroll state used by editor-style direct-core scroll views.
        struct [[Luna::struct("{75194DAF-8117-4600-BB38-7035F2C3DF88}")]] CoreScrollViewState
        {
            //! Current content scroll offset in layer logical coordinates.
            Float2U scroll = Float2U(0.0f);
            //! Last measured content size.
            Float2U content_size = Float2U(0.0f);
            //! Last arranged viewport content size.
            Float2U viewport_size = Float2U(0.0f);
        };

        //! Per-frame build state used by editor-style direct-core drag-drop scopes.
        struct [[Luna::struct("{9FB8D044-6509-4AEA-A273-8DD19267C095}")]] CoreDragDropBuildState
        {
            //! Whether a drag-drop source scope is currently open.
            bool source_scope_open = false;
            //! Source element captured by the current source scope.
            GUICore::ElementHandle source;
            //! Payload type captured by the current source scope.
            Name source_payload_type;
            //! Whether a drag-drop target scope is currently open.
            bool target_scope_open = false;
            //! Target element captured by the current target scope.
            GUICore::ElementHandle target;
            //! Payload type accepted by the current target scope.
            Name target_payload_type;
        };

        //! Returns whether @p state already contains @p id in its stable tab order.
        inline bool tab_order_contains(const TabBarState& state, id_t id)
        {
            for(id_t item : state.tab_order)
            {
                if(item == id) return true;
            }
            return false;
        }

        //! Identifies one deferred layout primitive requested by the editor-style immediate API package.
        enum class EditorLayoutRequestKind : u8
        {
            //! No layout primitive is attached.
            none,
            //! Horizontal or vertical linear layout.
            linear,
            //! Row-major grid layout.
            grid,
            //! Stack layout.
            stack,
            //! Anchor-based canvas layout.
            canvas,
            //! Scroll viewport layout.
            scroll_viewport,
            //! Editor-style scroll view with package-managed scroll state.
            scroll_view,
            //! Table track layout.
            table,
            //! Editor-style tab bar layout.
            tab_bar,
            //! Editor-style menu bar layout.
            menu_bar
        };

        //! Deferred layout data for one editor-style element.
        //! @remark This is high-level package state. The GUICore element tree remains typeless.
        struct EditorLayoutRequest
        {
            //! Requested layout primitive.
            EditorLayoutRequestKind kind = EditorLayoutRequestKind::none;
            //! Linear layout options used by @ref EditorLayoutRequestKind::linear.
            GUICore::LinearLayoutDesc linear;
            //! Grid layout options used by @ref EditorLayoutRequestKind::grid.
            GUICore::GridLayoutDesc grid;
            //! Stack layout options used by @ref EditorLayoutRequestKind::stack.
            GUICore::StackLayoutDesc stack;
            //! Canvas layout default item used by @ref EditorLayoutRequestKind::canvas.
            GUICore::CanvasLayoutItem canvas_default_item;
            //! Canvas layout item records copied from the caller.
            Vector<GUICore::CanvasLayoutItem> canvas_items;
            //! Whether canvas children are clipped.
            bool canvas_clip_children = true;
            //! Scroll viewport options used by @ref EditorLayoutRequestKind::scroll_viewport.
            GUICore::ScrollViewportLayoutDesc scroll_viewport;
            //! Table column records copied from the caller.
            Vector<GUICore::TableTrackDesc> table_columns;
            //! Table row records copied from the caller.
            Vector<GUICore::TableTrackDesc> table_rows;
            //! Table cell attachment records copied from the caller.
            Vector<GUICore::TableLayoutCell> table_cells;
            //! Table gap.
            Float2U table_gap = Float2U(0.0f);
            //! Whether table children are clipped.
            bool table_clip_children = true;
        };

        //! Per-frame deferred layout requests used by the editor-style immediate API package.
        struct [[Luna::struct("{CDBF6CAA-450E-43F4-8BD1-A715E9207F53}")]] EditorLayoutPassState
        {
            //! Requests keyed by element ID.
            HashMap<id_t, EditorLayoutRequest, GUICore::IdHash> requests;
        };

        //! Runtime edit state used by editor-style text input widgets.
        struct [[Luna::struct("{DC801B89-9DEE-4456-8036-9F8C9A7C8A8A}")]] InputEditState
        {
            //! UTF-8 byte offset of the text cursor.
            usize text_cursor = USIZE_MAX;
            //! UTF-8 byte offset of the selection anchor, or USIZE_MAX when no selection is active.
            usize text_select_anchor = USIZE_MAX;
            //! Whether the pointer is currently dragging a text selection.
            bool text_selecting = false;
            //! Time point used to animate the text cursor blink.
            f64 text_cursor_blink_start = 0.0;
            //! Numeric component currently edited through a text field.
            u32 numeric_edit_component = 0;
            //! Temporary text used while editing numeric widgets as text.
            String numeric_edit_text;
            //! Whether a numeric widget is currently in text edit mode.
            bool numeric_editing = false;
        };

        //! Runtime edit state used by editor-style drag numeric widgets.
        struct [[Luna::struct("{EF79C913-424D-4467-80C4-0852F5388782}")]] DragEditState
        {
            //! Whether the widget is currently dragging a numeric value.
            bool dragging = false;
            //! Component index captured when the drag started.
            u8 component = 0;
            //! Pointer X coordinate captured when the drag started.
            f32 start_pointer_x = 0.0f;
            //! Float component values captured when the drag started.
            Vector<f32> start_f32_values;
            //! Integer component values captured when the drag started.
            Vector<i32> start_i32_values;
        };

        //! Runtime edit state used by editor-style slider-with-input views.
        struct [[Luna::struct("{97EAEF60-5536-46C5-A39B-986823532D55}")]] SliderWithInputState
        {
            //! Text values currently shown in component input fields.
            Vector<String> texts;
            //! Last synchronized float component values.
            Vector<f32> last_f32_values;
            //! Last synchronized integer component values.
            Vector<i32> last_i32_values;
        };

        //! Binds a color-edit view or picker to caller-owned color storage.
        struct ColorBinding
        {
            //! Pointer to float color channels in normalized 0.0-1.0 range.
            f32* f32_value = nullptr;
            //! Pointer to byte color channels in 0-255 range.
            u8* u8_value = nullptr;
            //! Pointer to one RGBA8 value stored as 0xAABBGGRR.
            u32* u32_value = nullptr;
            //! Backing storage kind.
            ColorValueType type = ColorValueType::f32;
            //! Number of channels used by the binding.
            u8 value_count = 3;
            //! Optional owner ID used by legacy color picker numeric nodes.
            id_t owner_id = 0;
            //! Optional channel group used by legacy color picker numeric nodes.
            ColorChannelPart part = ColorChannelPart::none;
        };

        //! Persistent edit state used by editor-style color picker views.
        struct [[Luna::struct("{A9483A32-872C-47B0-9AAF-26468F6D411F}")]] ColorPickerState
        {
            //! Selected picker axis. 0-2 are H/S/V, 3-5 are R/G/B.
            Vector<i32> color_picker_axis;
            //! Current RGB(A) channel edit values in 0-255 range.
            Vector<i32> color_picker_rgb;
            //! Current HSV channel edit values in 0-255 range.
            Vector<i32> color_picker_hsv;
            //! Original color captured when the picker popup opened.
            Float4U color_picker_original = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
            //! Whether @ref color_picker_original contains a captured value.
            bool color_picker_original_valid = false;
        };

        //! Per-frame pointer interaction state used by color picker views.
        struct [[Luna::struct("{E8F4622D-7EBF-46FE-9689-FA7AE1C3CC37}")]] ColorPickerInteractionState
        {
            //! Active color picker sub-region: 0 means none, 1 square, 2 bar, 3 original swatch.
            u32 active_color_part = 0;
        };

        //! Converts one normalized color channel to an 8-bit color channel.
        inline u8 color_channel_to_u8(f32 value)
        {
            return (u8)clamp(value * 255.0f + 0.5f, 0.0f, 255.0f);
        }

        //! Converts one 8-bit color channel to a normalized color channel.
        inline f32 color_u8_to_channel(u8 value)
        {
            return (f32)value / 255.0f;
        }

        //! Returns the effective number of channels in @p binding.
        inline u8 color_value_count(const ColorBinding& binding)
        {
            return (u8)clamp((u32)binding.value_count, 1u, 4u);
        }

        //! Reads @p binding as a normalized RGBA color.
        inline Float4U read_color_value(const ColorBinding& binding)
        {
            u8* u8_values = binding.u8_value;
            u32* rgba8_value = binding.u32_value;
            f32* f32_values = binding.f32_value;
            u32 value_count = color_value_count(binding);
            if(binding.type == ColorValueType::u8 && u8_values)
            {
                return Float4U(
                    color_u8_to_channel(u8_values[0]),
                    color_u8_to_channel(u8_values[1]),
                    color_u8_to_channel(u8_values[2]),
                    value_count > 3 ? color_u8_to_channel(u8_values[3]) : 1.0f);
            }
            if(binding.type == ColorValueType::rgba8 && rgba8_value)
            {
                u32 value = *rgba8_value;
                return Float4U(
                    color_u8_to_channel((u8)(value & 0xffu)),
                    color_u8_to_channel((u8)((value >> 8) & 0xffu)),
                    color_u8_to_channel((u8)((value >> 16) & 0xffu)),
                    value_count > 3 ? color_u8_to_channel((u8)((value >> 24) & 0xffu)) : 1.0f);
            }
            if(f32_values)
            {
                return Float4U(
                    clamp(f32_values[0], 0.0f, 1.0f),
                    clamp(value_count > 1 ? f32_values[1] : 0.0f, 0.0f, 1.0f),
                    clamp(value_count > 2 ? f32_values[2] : 0.0f, 0.0f, 1.0f),
                    clamp(value_count > 3 ? f32_values[3] : 1.0f, 0.0f, 1.0f));
            }
            return Float4U(0.0f, 0.0f, 0.0f, 1.0f);
        }

        //! Writes normalized @p color to @p binding.
        inline void write_color_value(ColorBinding& binding, Float4U color)
        {
            color.x = clamp(color.x, 0.0f, 1.0f);
            color.y = clamp(color.y, 0.0f, 1.0f);
            color.z = clamp(color.z, 0.0f, 1.0f);
            u32 value_count = color_value_count(binding);
            color.w = value_count > 3 ? clamp(color.w, 0.0f, 1.0f) : 1.0f;
            u8* u8_values = binding.u8_value;
            u32* rgba8_value = binding.u32_value;
            f32* f32_values = binding.f32_value;
            if(binding.type == ColorValueType::u8 && u8_values)
            {
                u8_values[0] = color_channel_to_u8(color.x);
                u8_values[1] = color_channel_to_u8(color.y);
                u8_values[2] = color_channel_to_u8(color.z);
                if(value_count > 3) u8_values[3] = color_channel_to_u8(color.w);
            }
            else if(binding.type == ColorValueType::rgba8 && rgba8_value)
            {
                u32 r = (u32)color_channel_to_u8(color.x);
                u32 g = (u32)color_channel_to_u8(color.y);
                u32 b = (u32)color_channel_to_u8(color.z);
                u32 a = value_count > 3 ? (u32)color_channel_to_u8(color.w) : 255u;
                *rgba8_value = r | (g << 8) | (b << 16) | (a << 24);
            }
            else if(f32_values)
            {
                f32_values[0] = color.x;
                f32_values[1] = color.y;
                f32_values[2] = color.z;
                if(value_count > 3) f32_values[3] = color.w;
            }
        }

        //! Converts RGB color channels in normalized range to HSV channels in normalized range.
        inline void color_rgb_to_hsv(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v)
        {
            f32 max_value = max(max(r, g), b);
            f32 min_value = min(min(r, g), b);
            f32 delta = max_value - min_value;
            v = max_value;
            s = max_value <= 0.0f ? 0.0f : delta / max_value;
            if(delta <= 0.000001f)
            {
                h = 0.0f;
            }
            else if(max_value == r)
            {
                h = (g - b) / delta;
                if(h < 0.0f) h += 6.0f;
                h /= 6.0f;
            }
            else if(max_value == g)
            {
                h = ((b - r) / delta + 2.0f) / 6.0f;
            }
            else
            {
                h = ((r - g) / delta + 4.0f) / 6.0f;
            }
            h = clamp(h, 0.0f, 1.0f);
        }

        //! Converts HSV color channels in normalized range to an RGBA color.
        inline Float4U color_hsv_to_rgb(f32 h, f32 s, f32 v, f32 a = 1.0f)
        {
            h = clamp(h, 0.0f, 1.0f);
            s = clamp(s, 0.0f, 1.0f);
            v = clamp(v, 0.0f, 1.0f);
            f32 r = v;
            f32 g = v;
            f32 b = v;
            if(s > 0.0f)
            {
                f32 scaled = h * 6.0f;
                i32 sector = (i32)floorf(scaled);
                f32 f = scaled - (f32)sector;
                f32 p = v * (1.0f - s);
                f32 q = v * (1.0f - s * f);
                f32 t = v * (1.0f - s * (1.0f - f));
                switch(sector % 6)
                {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                default: r = v; g = p; b = q; break;
                }
            }
            return Float4U(r, g, b, clamp(a, 0.0f, 1.0f));
        }

        //! Converts a normalized color into color picker square and bar coordinates for one picker axis.
        inline void color_picker_channels_from_color(i32 axis, const Float4U& color, f32& x, f32& y, f32& bar)
        {
            axis = clamp(axis, 0, 5);
            if(axis < 3)
            {
                f32 h = 0.0f;
                f32 s = 0.0f;
                f32 v = 0.0f;
                color_rgb_to_hsv(color.x, color.y, color.z, h, s, v);
                if(axis == 0) { x = s; y = v; bar = h; }
                else if(axis == 1) { x = h; y = v; bar = s; }
                else { x = h; y = s; bar = v; }
            }
            else
            {
                if(axis == 3) { x = color.y; y = color.z; bar = color.x; }
                else if(axis == 4) { x = color.x; y = color.z; bar = color.y; }
                else { x = color.x; y = color.y; bar = color.z; }
            }
        }

        //! Converts color picker square and bar coordinates to a normalized RGBA color.
        inline Float4U color_from_picker_channels(i32 axis, f32 x, f32 y, f32 bar, f32 alpha)
        {
            axis = clamp(axis, 0, 5);
            x = clamp(x, 0.0f, 1.0f);
            y = clamp(y, 0.0f, 1.0f);
            bar = clamp(bar, 0.0f, 1.0f);
            alpha = clamp(alpha, 0.0f, 1.0f);
            if(axis == 0) return color_hsv_to_rgb(bar, x, y, alpha);
            if(axis == 1) return color_hsv_to_rgb(x, bar, y, alpha);
            if(axis == 2) return color_hsv_to_rgb(x, y, bar, alpha);
            if(axis == 3) return Float4U(bar, x, y, alpha);
            if(axis == 4) return Float4U(x, bar, y, alpha);
            return Float4U(x, y, bar, alpha);
        }

        //! Returns the square selector rectangle inside a color picker work area.
        inline RectF color_picker_square_rect(const RectF& rect)
        {
            f32 right_width = 112.0f;
            f32 bar_width = 24.0f;
            f32 gap = 10.0f;
            f32 square_size = min(rect.height, max(rect.width - right_width - bar_width - gap * 2.0f, 1.0f));
            return RectF(rect.offset_x, rect.offset_y, square_size, square_size);
        }

        //! Returns the one-dimensional color bar rectangle inside a color picker work area.
        inline RectF color_picker_bar_rect(const RectF& rect)
        {
            RectF square = color_picker_square_rect(rect);
            return RectF(square.offset_x + square.width + 10.0f, square.offset_y, 24.0f, square.height);
        }

        //! Returns the current color preview rectangle inside a color picker work area.
        inline RectF color_picker_current_rect(const RectF& rect)
        {
            RectF bar = color_picker_bar_rect(rect);
            return RectF(bar.offset_x + bar.width + 10.0f, bar.offset_y + 28.0f, 102.0f, 58.0f);
        }

        //! Returns the original color preview rectangle inside a color picker work area.
        inline RectF color_picker_original_rect(const RectF& rect)
        {
            RectF cur = color_picker_current_rect(rect);
            return RectF(cur.offset_x, cur.offset_y + cur.height + 44.0f, cur.width, cur.height);
        }

        //! Ensures all color picker channel arrays are initialized to their expected sizes.
        inline void ensure_color_picker_state_channels(ColorPickerState& state)
        {
            if(state.color_picker_axis.size() != 1)
            {
                state.color_picker_axis.resize(1, 0);
            }
            if(state.color_picker_rgb.size() != 4)
            {
                state.color_picker_rgb.resize(4, 0);
                state.color_picker_rgb[3] = 255;
            }
            if(state.color_picker_hsv.size() != 3)
            {
                state.color_picker_hsv.resize(3, 0);
            }
        }

        //! Returns a mutable reference to the color picker axis channel.
        inline i32& color_picker_axis_ref(ColorPickerState& state)
        {
            ensure_color_picker_state_channels(state);
            return state.color_picker_axis[0];
        }

        //! Clamps a UTF-8 byte cursor to a valid code point boundary.
        inline usize clamp_utf8_cursor(const String& value, usize cursor)
        {
            if(cursor == USIZE_MAX || cursor > value.size())
            {
                return value.size();
            }
            while(cursor > 0 && cursor < value.size() && (((u8)value[cursor]) & 0xC0) == 0x80)
            {
                --cursor;
            }
            return cursor;
        }

        //! Returns the previous valid UTF-8 byte cursor before @p cursor.
        inline usize previous_utf8_cursor(const String& value, usize cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(!cursor) return 0;
            --cursor;
            while(cursor > 0 && (((u8)value[cursor]) & 0xC0) == 0x80)
            {
                --cursor;
            }
            return cursor;
        }

        //! Returns the next valid UTF-8 byte cursor after @p cursor.
        inline usize next_utf8_cursor(const String& value, usize cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(cursor >= value.size()) return value.size();
            usize len = utf8_charlen(value.c_str() + cursor);
            return min(cursor + len, value.size());
        }

        //! Erases the UTF-8 code point before @p cursor and moves @p cursor to the erase position.
        inline void erase_previous_utf8_codepoint(String& value, usize& cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            usize begin = previous_utf8_cursor(value, cursor);
            if(begin == cursor) return;
            value.erase(begin, cursor - begin);
            cursor = begin;
        }

        //! Erases the UTF-8 code point at @p cursor.
        inline void erase_utf8_codepoint_at(String& value, usize& cursor)
        {
            cursor = clamp_utf8_cursor(value, cursor);
            if(cursor >= value.size()) return;
            usize end = next_utf8_cursor(value, cursor);
            value.erase(cursor, end - cursor);
        }

        //! Computes the normalized byte range of the active text selection.
        inline void input_text_selection_range(const String& value, const InputEditState& state, usize& out_begin, usize& out_end)
        {
            usize cursor = clamp_utf8_cursor(value, state.text_cursor);
            usize anchor = state.text_select_anchor == USIZE_MAX ? cursor : clamp_utf8_cursor(value, state.text_select_anchor);
            out_begin = min(cursor, anchor);
            out_end = max(cursor, anchor);
        }

        //! Returns whether @p state currently selects a non-empty text range.
        inline bool input_text_has_selection(const String& value, const InputEditState& state)
        {
            usize begin = 0;
            usize end = 0;
            input_text_selection_range(value, state, begin, end);
            return begin != end;
        }

        //! Clears the active text selection metadata.
        inline void input_text_clear_selection(InputEditState& state)
        {
            state.text_select_anchor = USIZE_MAX;
            state.text_selecting = false;
        }

        //! @}
    }
}
