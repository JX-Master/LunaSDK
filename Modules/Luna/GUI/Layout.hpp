/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! A fixed two-dimensional size in GUI logical coordinates.
        struct Size
        {
            //! The width in logical units. A value of 0 lets the widget use its default sizing behavior.
            f32 width = 0.0f;
            //! The height in logical units. A value of 0 lets the widget use its default sizing behavior.
            f32 height = 0.0f;

            //! Creates a fixed size value.
            //! @param[in] width The fixed width.
            //! @param[in] height The fixed height.
            //! @return Returns the fixed size value.
            static Size fixed(f32 width, f32 height)
            {
                Size r;
                r.width = width;
                r.height = height;
                return r;
            }
        };

        //! Describes how one layout axis chooses its size.
        enum class SizePolicy : u8
        {
            //! Use the fixed value specified in @ref LayoutStyle.
            fixed,
            //! Use the preferred size reported by the node.
            hug,
            //! Consume remaining parent space using the fill weight.
            fill
        };

        //! Aligns children along the main axis of a linear layout.
        enum class LayoutMainAxisAlignment : u8
        {
            //! Pack children toward the beginning of the main axis.
            begin,
            //! Center children on the main axis.
            center,
            //! Pack children toward the end of the main axis.
            end,
            //! Distribute remaining space between children.
            space_between
        };

        //! Aligns children along the cross axis of a linear layout.
        enum class LayoutCrossAxisAlignment : u8
        {
            //! Align to the beginning of the cross axis.
            begin,
            //! Align to the center of the cross axis.
            center,
            //! Align to the end of the cross axis.
            end,
            //! Stretch children to the cross-axis size of the layout slot.
            stretch
        };

        //! Insets from the four edges of a rectangle.
        struct EdgeInsets
        {
            //! Left inset.
            f32 left = 0.0f;
            //! Top inset.
            f32 top = 0.0f;
            //! Right inset.
            f32 right = 0.0f;
            //! Bottom inset.
            f32 bottom = 0.0f;

            //! Creates equal insets on all four edges.
            //! @param[in] value The inset value.
            //! @return Returns the created insets.
            static EdgeInsets all(f32 value)
            {
                EdgeInsets r;
                r.left = value;
                r.top = value;
                r.right = value;
                r.bottom = value;
                return r;
            }

            //! Creates symmetric horizontal and vertical insets.
            //! @param[in] x The left and right inset.
            //! @param[in] y The top and bottom inset.
            //! @return Returns the created insets.
            static EdgeInsets xy(f32 x, f32 y)
            {
                EdgeInsets r;
                r.left = x;
                r.right = x;
                r.top = y;
                r.bottom = y;
                return r;
            }
        };

        //! Intrinsic size metrics reported by one node during measure pass.
        struct LayoutMetrics
        {
            //! The minimum size required by the node.
            Float2U min_size = Float2U(0.0f);
            //! The preferred hug size of the node.
            Float2U preferred_size = Float2U(0.0f);
            //! The maximum allowed size of the node.
            Float2U max_size = Float2U(F32_MAX, F32_MAX);
        };

        //! Size policy and constraints assigned to one item by widget APIs or parent layouts.
        struct LayoutStyle
        {
            //! Width sizing policy.
            SizePolicy width_policy = SizePolicy::hug;
            //! Height sizing policy.
            SizePolicy height_policy = SizePolicy::hug;
            //! Width used when @ref width_policy is @ref SizePolicy::fixed.
            f32 fixed_width_value = 0.0f;
            //! Height used when @ref height_policy is @ref SizePolicy::fixed.
            f32 fixed_height_value = 0.0f;
            //! Fill weight used by horizontal allocation.
            f32 fill_weight_x = 1.0f;
            //! Fill weight used by vertical allocation.
            f32 fill_weight_y = 1.0f;
            //! Minimum item size clamp.
            Float2U min_size = Float2U(0.0f);
            //! Maximum item size clamp.
            Float2U max_size = Float2U(F32_MAX, F32_MAX);

            //! Creates a layout style that uses preferred size on both axes.
            //! @return Returns the layout style.
            static LayoutStyle hug()
            {
                return LayoutStyle();
            }

            //! Creates a layout style that fills both axes.
            //! @param[in] weight The fill weight for both axes.
            //! @return Returns the layout style.
            static LayoutStyle fill(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fill;
                r.height_policy = SizePolicy::fill;
                r.fill_weight_x = weight;
                r.fill_weight_y = weight;
                return r;
            }

            //! Creates a layout style that fills width and hugs height.
            //! @param[in] weight The horizontal fill weight.
            //! @return Returns the layout style.
            static LayoutStyle fill_width(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fill;
                r.fill_weight_x = weight;
                return r;
            }

            //! Creates a layout style that hugs width and fills height.
            //! @param[in] weight The vertical fill weight.
            //! @return Returns the layout style.
            static LayoutStyle fill_height(f32 weight = 1.0f)
            {
                LayoutStyle r;
                r.height_policy = SizePolicy::fill;
                r.fill_weight_y = weight;
                return r;
            }

            //! Creates a layout style with fixed width and height.
            //! @param[in] width The fixed width.
            //! @param[in] height The fixed height.
            //! @return Returns the layout style.
            static LayoutStyle fixed(f32 width, f32 height)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fixed;
                r.height_policy = SizePolicy::fixed;
                r.fixed_width_value = width;
                r.fixed_height_value = height;
                return r;
            }

            //! Creates a layout style with fixed width and hug height.
            //! @param[in] width The fixed width.
            //! @return Returns the layout style.
            static LayoutStyle fixed_width(f32 width)
            {
                LayoutStyle r;
                r.width_policy = SizePolicy::fixed;
                r.fixed_width_value = width;
                return r;
            }

            //! Creates a layout style with hug width and fixed height.
            //! @param[in] height The fixed height.
            //! @return Returns the layout style.
            static LayoutStyle fixed_height(f32 height)
            {
                LayoutStyle r;
                r.height_policy = SizePolicy::fixed;
                r.fixed_height_value = height;
                return r;
            }
        };

        //! Parameters for linear horizontal and vertical layouts.
        struct LayoutDesc
        {
            //! Padding inside the layout container.
            EdgeInsets padding;
            //! Spacing between adjacent children.
            f32 gap = 6.0f;
            //! Child distribution along the main axis.
            LayoutMainAxisAlignment main_axis_alignment = LayoutMainAxisAlignment::begin;
            //! Child alignment along the cross axis.
            LayoutCrossAxisAlignment cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        };

        //! Selects how a grid layout computes its cell width and column count.
        enum class GridSizingMode : u8
        {
            //! Use @ref GridLayoutDesc::cell_size and derive the number of columns from available width.
            fixed_cell_size,
            //! Use @ref GridLayoutDesc::columns and derive cell width from available width.
            fixed_columns
        };

        //! Parameters for a row-major grid layout.
        //! @remark Grid layout does not scroll by itself. Wrap it in a scroll view when scrollable content is required.
        struct GridLayoutDesc
        {
            //! Cell sizing mode.
            GridSizingMode sizing_mode = GridSizingMode::fixed_cell_size;
            //! Cell size used by @ref GridSizingMode::fixed_cell_size.
            Float2U cell_size = Float2U(96.0f, 118.0f);
            //! Column count used by @ref GridSizingMode::fixed_columns.
            u32 columns = 4;
            //! Padding inside the grid.
            EdgeInsets padding = EdgeInsets::all(6.0f);
            //! Horizontal and vertical gap between cells.
            Float2U gap = Float2U(8.0f, 8.0f);
            //! Alignment of each child inside its grid cell on the cross axis.
            LayoutCrossAxisAlignment cell_cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        };

        //! Parameters for a canvas layout.
        struct CanvasLayoutDesc
        {
            //! Padding inside the canvas.
            EdgeInsets padding;
            //! Whether children should be clipped to the canvas rectangle.
            bool clip_children = true;
        };

        //! Rect-transform style placement assigned to a child of a canvas layout.
        struct CanvasItemLayout
        {
            //! Minimum anchor in parent normalized coordinates.
            Float2U anchor_min = Float2U(0.0f, 0.0f);
            //! Maximum anchor in parent normalized coordinates.
            Float2U anchor_max = Float2U(0.0f, 0.0f);
            //! Offset from the minimum anchor point.
            Float2U offset_min = Float2U(0.0f, 0.0f);
            //! Offset from the maximum anchor point.
            Float2U offset_max = Float2U(0.0f, 0.0f);

            //! Creates a fixed-position canvas child rectangle.
            //! @param[in] position The top-left position in canvas coordinates.
            //! @param[in] size The fixed size.
            //! @return Returns the canvas item layout.
            static CanvasItemLayout fixed(const Float2U& position, const Float2U& size)
            {
                CanvasItemLayout r;
                r.offset_min = position;
                r.offset_max = position + size;
                return r;
            }

            //! Creates a fixed-size item placed around a single normalized anchor.
            //! @param[in] anchor The normalized anchor point.
            //! @param[in] anchored_position The offset from the anchor point.
            //! @param[in] size The fixed size.
            //! @param[in] pivot The normalized pivot inside the item.
            //! @return Returns the canvas item layout.
            static CanvasItemLayout anchored(const Float2U& anchor, const Float2U& anchored_position, const Float2U& size, const Float2U& pivot = Float2U(0.5f, 0.5f))
            {
                CanvasItemLayout r;
                r.anchor_min = anchor;
                r.anchor_max = anchor;
                Float2U min_offset = anchored_position - size * pivot;
                r.offset_min = min_offset;
                r.offset_max = min_offset + size;
                return r;
            }

            //! Creates an item that stretches to the canvas rectangle minus the provided insets.
            //! @param[in] insets The edge insets.
            //! @return Returns the canvas item layout.
            static CanvasItemLayout stretch(const EdgeInsets& insets = EdgeInsets())
            {
                CanvasItemLayout r;
                r.anchor_max = Float2U(1.0f, 1.0f);
                r.offset_min = Float2U(insets.left, insets.top);
                r.offset_max = Float2U(-insets.right, -insets.bottom);
                return r;
            }
        };

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

        enum class NumericEditFlag : u32
        {
            //! Default drag behavior.
            none = 0x00,
            //! Allow the drag widget to enter text input mode on double click.
            input_on_double_click = 0x01
        };

        //! Parameters used when creating one popup layer.
        struct PopupDesc
        {
            //! Popup top-left position in screen logical coordinates.
            Float2U position = Float2U(0.0f);
            //! Requested popup root size. A zero axis uses the popup content hug size.
            Size size;
            //! Popup lifetime and input flags.
            PopupFlag flags = PopupFlag::managed | PopupFlag::close_on_outside_click | PopupFlag::close_on_escape | PopupFlag::close_on_blur;
        };

        //! Parameters used when creating a tooltip layer.
        struct TooltipDesc
        {
            //! Offset from owner hover position to tooltip layer position.
            Float2U offset = Float2U(14.0f, 18.0f);
            //! Requested tooltip size. A zero axis uses the tooltip content hug size.
            Size size;
            //! Hover delay before the tooltip is displayed, in seconds.
            f32 delay = 0.35f;
            //! Preferred text wrapping width for simple text tooltips.
            f32 max_width = 360.0f;
        };

        //! Initial placement mode for a dock panel.
        enum class DockPanelMode : u8
        {
            //! Place the panel into the dock tree.
            docking,
            //! Place the panel as a floating panel above docked content.
            floating
        };

        //! Style and initial behavior for dock panels managed by a dock space.
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
            //! Initial mode used when no persistent dock panel state exists.
            DockPanelMode initial_mode = DockPanelMode::docking;
            //! Initial floating top-left position in dock space coordinates.
            Float2U floating_position = Float2U(24.0f, 24.0f);
            //! Initial floating size.
            Float2U floating_size = Float2U(320.0f, 220.0f);
            //! Minimum size for user-resized floating panels.
            Float2U min_floating_size = Float2U(120.0f, 80.0f);
            //! Panel background color used by the default render proxy.
            Float4U background_color = Float4U(0.09f, 0.11f, 0.14f, 0.96f);
            //! Inactive title bar color used by the default render proxy.
            Float4U title_bar_color = Float4U(0.13f, 0.17f, 0.22f, 1.0f);
            //! Active title bar color used by the default render proxy.
            Float4U active_title_bar_color = Float4U(0.16f, 0.24f, 0.36f, 1.0f);
            //! Border color used by the default render proxy.
            Float4U border_color = Float4U(0.24f, 0.29f, 0.36f, 1.0f);
        };

        //! Describes how one table row or column track determines its size.
        enum class TableTrackSizePolicy : u8
        {
            //! Use the maximum preferred size of cells in the track.
            hug,
            //! Use the explicit fixed value.
            fixed
        };

        //! Size rule for one table row or column.
        struct TableTrackSize
        {
            //! Track sizing policy.
            TableTrackSizePolicy policy = TableTrackSizePolicy::hug;
            //! Fixed track size when @ref policy is @ref TableTrackSizePolicy::fixed.
            f32 value = 0.0f;

            //! Creates a hug-sized track.
            //! @return Returns the track size rule.
            static TableTrackSize hug()
            {
                return TableTrackSize();
            }

            //! Creates a fixed-size track.
            //! @param[in] value The fixed size.
            //! @return Returns the track size rule.
            static TableTrackSize fixed(f32 value)
            {
                TableTrackSize r;
                r.policy = TableTrackSizePolicy::fixed;
                r.value = value;
                return r;
            }
        };

        //! Selects the default background fill mode for a table.
        enum class TableBackgroundMode : u8
        {
            //! Do not draw a default table background.
            none,
            //! Draw one solid background color.
            solid,
            //! Alternate background colors by row.
            alternate_rows,
            //! Alternate background colors by column.
            alternate_columns
        };

        //! Optional color override used by table rows, columns and cells.
        struct ColorOverride
        {
            //! Whether this override should be applied.
            bool enabled = false;
            //! Override color.
            Float4U color = Float4U(0.0f);

            //! Creates a disabled color override.
            //! @return Returns the override value.
            static ColorOverride none()
            {
                return ColorOverride();
            }

            //! Creates an enabled color override.
            //! @param[in] value The override color.
            //! @return Returns the override value.
            static ColorOverride make(const Float4U& value)
            {
                ColorOverride r;
                r.enabled = true;
                r.color = value;
                return r;
            }
        };

        //! Visual and interaction parameters for a table layout.
        struct TableStyle
        {
            //! Padding inside each table cell.
            EdgeInsets padding = EdgeInsets::xy(6.0f, 4.0f);
            //! Outer border thickness.
            f32 border_size = 0.0f;
            //! Outer border color.
            Float4U border_color = Float4U(0.25f, 0.28f, 0.32f, 1.0f);
            //! Default background fill mode.
            TableBackgroundMode background_mode = TableBackgroundMode::none;
            //! Primary background color.
            Float4U background_color = Float4U(0.10f, 0.12f, 0.14f, 0.72f);
            //! Alternate background color for alternating modes.
            Float4U alternate_background_color = Float4U(0.13f, 0.15f, 0.18f, 0.72f);
            //! Per-row color overrides.
            Vector<ColorOverride> row_colors;
            //! Per-column color overrides.
            Vector<ColorOverride> column_colors;
            //! Per-cell color overrides in row-major order.
            Vector<ColorOverride> cell_colors;
            //! Whether horizontal separators are drawn between rows.
            bool row_separators = false;
            //! Whether vertical separators are drawn between columns.
            bool column_separators = false;
            //! Separator thickness.
            f32 separator_size = 1.0f;
            //! Separator color.
            Float4U separator_color = Float4U(0.28f, 0.32f, 0.36f, 1.0f);
            //! Whether fixed-height rows can be resized by dragging separators.
            bool resize_fixed_rows = false;
            //! Whether fixed-width columns can be resized by dragging separators.
            bool resize_fixed_columns = false;
            //! Hit-test thickness for row and column resize handles.
            f32 resize_hit_size = 6.0f;
        };

        //! Parameters for table layout.
        struct TableDesc
        {
            //! Number of columns in the table.
            u32 columns = 1;
            //! Column size rules. Missing entries use hug size.
            Vector<TableTrackSize> column_sizes;
            //! Row size rules. Missing entries use hug size.
            Vector<TableTrackSize> row_sizes;
            //! Visual and interaction style for the table.
            TableStyle style;
        };

        //! @}
    }
}
