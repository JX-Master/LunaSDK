/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layout.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Context.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Identifies one linear layout axis.
        enum class LayoutAxis : u8
        {
            //! Lays out children from left to right.
            x,
            //! Lays out children from top to bottom.
            y
        };

        //! Describes one linear layout pass.
        struct LinearLayoutDesc
        {
            //! Child placement axis.
            LayoutAxis axis = LayoutAxis::y;
            //! Gap between adjacent children.
            f32 gap = 0.0f;
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Applies one linear layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] desc Linear layout options.
        //! @return Returns success or failure code.
        //! @remark This is a data-oriented layout helper. It operates through public element records and writes
        //! layout results back to the supplied context without knowing any widget type.
        LUNA_GUICORE_API RV layout_linear(IContext* context, const ElementHandle& element,
            const RectF& rect, const LinearLayoutDesc& desc = LinearLayoutDesc());

        //! Identifies how grid layout derives its column count and cell size.
        enum class GridLayoutMode : u8
        {
            //! Uses @ref GridLayoutDesc::cell_size as an absolute cell size and derives the column count from available width.
            fixed_cell_size,
            //! Uses @ref GridLayoutDesc::column_count and derives the cell width from available width.
            fixed_column_count
        };

        //! Describes one row-major grid layout pass.
        struct GridLayoutDesc
        {
            //! Grid sizing mode.
            GridLayoutMode mode = GridLayoutMode::fixed_cell_size;
            //! Cell size in layer logical coordinates.
            //! @remark In @ref GridLayoutMode::fixed_column_count mode, `x` is ignored and `y` is used as row height.
            Float2U cell_size = Float2U(64.0f, 64.0f);
            //! Number of columns used by @ref GridLayoutMode::fixed_column_count mode.
            u32 column_count = 1;
            //! Gap between adjacent cells.
            Float2U gap = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Applies one row-major grid layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] desc Grid layout options.
        //! @return Returns success or failure code.
        //! @remark Grid layout is intended for tiled content such as asset browsers and icon palettes. It does not
        //! provide scrolling by itself; callers can combine it with scroll viewport primitives.
        LUNA_GUICORE_API RV layout_grid(IContext* context, const ElementHandle& element,
            const RectF& rect, const GridLayoutDesc& desc = GridLayoutDesc());

        //! Describes one stack layout pass.
        struct StackLayoutDesc
        {
            //! Child alignment inside the parent content rectangle. `(0, 0)` means top-left and `(0.5, 0.5)` means center.
            Float2U alignment = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Applies one stack layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] desc Stack layout options.
        //! @return Returns success or failure code.
        //! @remark Stack layout overlays all children in the same parent content rectangle. It is useful for
        //! overlays, layered chrome, popup bodies and as a building block for future canvas-style placement.
        LUNA_GUICORE_API RV layout_stack(IContext* context, const ElementHandle& element,
            const RectF& rect, const StackLayoutDesc& desc = StackLayoutDesc());

        //! Describes one child placement rule for canvas layout.
        struct CanvasLayoutItem
        {
            //! Child element ID this rule applies to. Zero means the rule is ignored for ID matching.
            id_t element_id = 0;
            //! Minimum anchor in parent content rectangle normalized coordinates.
            Float2U anchor_min = Float2U(0.0f);
            //! Maximum anchor in parent content rectangle normalized coordinates.
            //! @remark If one axis has equal min/max anchors, that axis uses the child layout size and pivot.
            //! If one axis has different anchors, that axis stretches between the two anchored edges.
            Float2U anchor_max = Float2U(0.0f);
            //! Offset in left, top, right, bottom order.
            //! @remark For non-stretched axes, only the left/top component for that axis is used as anchored position offset.
            Float4U offset = Float4U(0.0f);
            //! Pivot used when an axis is not stretched. `(0, 0)` means top-left and `(0.5, 0.5)` means center.
            Float2U pivot = Float2U(0.0f);
        };

        //! Describes one canvas layout pass.
        struct CanvasLayoutDesc
        {
            //! Placement records matched by child element ID.
            Span<const CanvasLayoutItem> items;
            //! Fallback placement used when no item matches a child.
            CanvasLayoutItem default_item;
            //! Whether child clip rectangles should be intersected with the parent content rectangle.
            bool clip_children = true;
        };

        //! Applies one anchor-based canvas layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The parent element whose direct children will be arranged.
        //! @param[in] rect The parent rectangle in layer coordinates.
        //! @param[in] desc Canvas layout options.
        //! @return Returns success or failure code.
        //! @remark Canvas layout is designed for freely positioned overlay content such as viewports, gizmo overlays,
        //! HUD elements and editor chrome. Child placement data belongs to the parent layout algorithm, not to
        //! the typeless child element itself.
        LUNA_GUICORE_API RV layout_canvas(IContext* context, const ElementHandle& element,
            const RectF& rect, const CanvasLayoutDesc& desc = CanvasLayoutDesc());

        //! Describes one scroll viewport layout pass.
        struct ScrollViewportLayoutDesc
        {
            //! Current content scroll offset in layer logical coordinates.
            Float2U scroll_offset = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the viewport content rectangle.
            bool clip_children = true;
        };

        //! Applies one scroll viewport layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The viewport element whose direct children will be arranged as scroll content.
        //! @param[in] rect The viewport rectangle in layer coordinates.
        //! @param[in] desc Scroll viewport layout options.
        //! @return Returns success or failure code.
        //! @remark This primitive only translates and clips content. It does not implement scrollbar rendering,
        //! scroll input handling or offset clamping.
        LUNA_GUICORE_API RV layout_scroll_viewport(IContext* context, const ElementHandle& element,
            const RectF& rect, const ScrollViewportLayoutDesc& desc = ScrollViewportLayoutDesc());

        //! Identifies how one table track size is resolved.
        enum class TableTrackSizeKind : u8
        {
            //! Uses the largest measured cell content on this track.
            fit,
            //! Uses an absolute pixel size.
            pixels,
            //! Uses a percentage of the table content size.
            percent,
            //! Consumes remaining space using a weighted ratio.
            ratio
        };

        //! Describes one table row or column track.
        struct TableTrackDesc
        {
            //! Track sizing mode.
            TableTrackSizeKind kind = TableTrackSizeKind::fit;
            //! Numeric value used by @ref TableTrackSizeKind::pixels, @ref TableTrackSizeKind::percent and
            //! @ref TableTrackSizeKind::ratio.
            f32 value = 0.0f;
            //! Minimum resolved track size.
            f32 min = 0.0f;
            //! Maximum resolved track size. Values less than zero mean no maximum.
            f32 max = -1.0f;
        };

        //! Describes one child-to-cell attachment for table layout.
        struct TableLayoutCell
        {
            //! Child element ID this cell applies to.
            id_t element_id = 0;
            //! Zero-based row index.
            u32 row = 0;
            //! Zero-based column index.
            u32 column = 0;
            //! Number of rows occupied by the cell. Zero is treated as one.
            u32 row_span = 1;
            //! Number of columns occupied by the cell. Zero is treated as one.
            u32 column_span = 1;
            //! Cell padding in left, top, right, bottom order.
            Float4U padding = Float4U(0.0f);
        };

        //! Describes one table track layout pass.
        struct TableLayoutDesc
        {
            //! Column tracks.
            Span<const TableTrackDesc> columns;
            //! Row tracks.
            Span<const TableTrackDesc> rows;
            //! Child-to-cell attachments.
            Span<const TableLayoutCell> cells;
            //! Gap between adjacent columns and rows.
            Float2U gap = Float2U(0.0f);
            //! Whether child clip rectangles should be intersected with the table content rectangle.
            bool clip_children = true;
        };

        //! Applies one table track layout algorithm to the direct children of one element.
        //! @param[in] context The GUI Core context that owns @p element.
        //! @param[in] element The table element whose direct children will be arranged by cell attachments.
        //! @param[in] rect The table rectangle in layer coordinates.
        //! @param[in] desc Table layout options.
        //! @return Returns success or failure code.
        //! @remark Table layout uses explicit cell attachments instead of child order. This keeps the core primitive
        //! compatible with virtualized row submission and editor-authored table descriptions.
        LUNA_GUICORE_API RV layout_table(IContext* context, const ElementHandle& element,
            const RectF& rect, const TableLayoutDesc& desc = TableLayoutDesc());
    }
}
