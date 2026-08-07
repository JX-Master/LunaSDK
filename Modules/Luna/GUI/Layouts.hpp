/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Layouts.hpp
* @author JXMaster
* @date 2026/7/13
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! Begins a horizontal flex layout.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable layout ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the container.
        //! @return Returns the created container. Call @ref end_h_layout after submitting its children.
        LUNA_GUI_API GUICore::ElementHandle begin_h_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a horizontal flex layout and attaches its layout descriptor.
        //! @param[in] context The GUI Core context containing the open layout scope.
        //! @param[in] element The container returned by @ref begin_h_layout.
        //! @param[in] desc Flex behavior copied into package frame storage with its axis forced to horizontal.
        LUNA_GUI_API void end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc = GUICore::FlexLayoutDesc());

        //! Begins a vertical flex layout.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable layout ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the container.
        //! @return Returns the created container. Call @ref end_v_layout after submitting its children.
        LUNA_GUI_API GUICore::ElementHandle begin_v_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a vertical flex layout and attaches its layout descriptor.
        //! @param[in] context The GUI Core context containing the open layout scope.
        //! @param[in] element The container returned by @ref begin_v_layout.
        //! @param[in] desc Flex behavior copied into package frame storage with its axis forced to vertical.
        LUNA_GUI_API void end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc = GUICore::FlexLayoutDesc());

        //! Begins a row-major grid layout.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable layout ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the container.
        //! @return Returns the created container. Call @ref end_grid_layout after submitting its children.
        LUNA_GUI_API GUICore::ElementHandle begin_grid_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a grid layout and attaches its layout descriptor.
        //! @param[in] context The GUI Core context containing the open layout scope.
        //! @param[in] element The container returned by @ref begin_grid_layout.
        //! @param[in] desc Grid behavior copied into package frame storage.
        LUNA_GUI_API void end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::GridLayoutDesc& desc);

        //! Begins an anchor-based canvas layout.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable layout ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the container.
        //! @return Returns the created container. Call @ref end_canvas_layout after submitting its children.
        LUNA_GUI_API GUICore::ElementHandle begin_canvas_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a canvas layout and copies child placement records for this frame.
        //! @param[in] context The GUI Core context containing the open layout scope.
        //! @param[in] element The container returned by @ref begin_canvas_layout.
        //! @param[in] desc Canvas behavior and child attachments copied into package frame storage.
        LUNA_GUI_API void end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::CanvasLayoutDesc& desc);

        //! Begins a focus navigation scope.
        //! @param[in] context The GUI Core context.
        //! @param[in] id Stable scope element and focus-scope ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration used by the scope container.
        //! @return Returns the scope element.
        //! @remark Automatic navigation remains inside the scope containing the currently focused descendant.
        LUNA_GUI_API GUICore::ElementHandle begin_focus_scope(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a focus scope and arranges its direct children vertically.
        //! @param[in] context The GUI Core context containing the open focus scope.
        //! @param[in] element The scope element returned by @ref begin_focus_scope.
        LUNA_GUI_API void end_focus_scope(GUICore::IContext* context, const GUICore::ElementHandle& element);

        //! Begins a table track layout.
        //! @param[in] context The GUI Core context.
        //! @param[in] id Stable table element ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration used by the table element.
        //! @param[in] desc Package-level table behavior.
        //! @return Returns the table element.
        LUNA_GUI_API GUICore::ElementHandle begin_table_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TableDesc& desc = TableDesc());
        //! Sets the column tracks used by the current table layout.
        //! @param[in] context The GUI Core context containing the open table scope.
        //! @param[in] columns Column tracks to copy for the current frame.
        //! @remark The descriptors are copied for the current frame. When omitted, fit columns are inferred from
        //! the widest submitted row.
        LUNA_GUI_API void set_table_columns(GUICore::IContext* context,
            Span<const GUICore::TableTrackDesc> columns);
        //! Begins one row in the current table.
        //! @param[in] context The GUI Core context.
        //! @param[in] row Row track used when fixed-row-height mode is disabled.
        //! @return Returns `true` when row cells should be submitted. The caller must call @ref end_table_row
        //! regardless of the returned value.
        LUNA_GUI_API bool begin_table_row(GUICore::IContext* context,
            const GUICore::TableTrackDesc& row = GUICore::TableTrackDesc());
        //! Ends the current table row and attaches newly submitted direct children to consecutive cells.
        //! @param[in] context The GUI Core context containing the open table row.
        LUNA_GUI_API void end_table_row(GUICore::IContext* context);
        //! Ends the current table and installs its table layout, measurement and resizing callbacks.
        //! @param[in] context The GUI Core context containing the open table scope.
        //! @param[in] element The table element returned by @ref begin_table_layout.
        LUNA_GUI_API void end_table_layout(GUICore::IContext* context, const GUICore::ElementHandle& element);

        //! Begins a scroll view and its package-owned content root.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable scroll-view ID used for persistent scroll state.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the viewport.
        //! @param[in] desc Scroll axes, wheel scaling, overscan and scrollbar behavior.
        //! @return Returns the viewport element. Submit content, then call @ref end_scroll_view.
        LUNA_GUI_API GUICore::ElementHandle begin_scroll_view(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ScrollViewDesc& desc = ScrollViewDesc());
        //! Ends the current scroll view.
        //! @param[in] context The GUI Core context containing the open scroll-view scope.
        LUNA_GUI_API void end_scroll_view(GUICore::IContext* context);
        //! Gets the previous-frame visible content rectangle for a scroll view begun in the current frame.
        //! @param[in] context The GUI Core context that owns @p scroll_view.
        //! @param[in] scroll_view The viewport returned by @ref begin_scroll_view.
        //! @return Returns the previous-frame visible rectangle in unscrolled content coordinates. A new viewport
        //! returns a conservative rectangle covering the current logical screen.
        LUNA_GUI_API RectF get_scroll_view_visible_rect(GUICore::IContext* context,
            const GUICore::ElementHandle& scroll_view);

        //! Begins a tab bar bound to a selected tab index.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable tab-bar ID.
        //! @param[in,out] selected_index Selected zero-based tab index. The value is clamped by @ref end_tab_bar.
        //! @param[in] layout Layout configuration for the tab bar.
        //! @param[in] desc Header interaction and fitting behavior.
        //! @return Returns the created tab-bar element. Submit items, then call @ref end_tab_bar.
        LUNA_GUI_API GUICore::ElementHandle begin_tab_bar(GUICore::IContext* context, id_t id, i32* selected_index,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TabBarDesc& desc = TabBarDesc());
        //! Begins one tab item and returns whether its content should be submitted this frame.
        //! @param[in] context The GUI Core context containing the open tab bar.
        //! @param[in] id Stable tab ID.
        //! @param[in] label Null-terminated UTF-8 header label.
        //! @param[in] desc Per-frame selection request.
        //! @return Returns `true` for the selected tab. Submit its content and call @ref end_tab_item only in this case.
        LUNA_GUI_API bool begin_tab_item(GUICore::IContext* context, id_t id, const c8* label,
            const TabItemDesc& desc = TabItemDesc());
        //! Ends the current selected tab item content scope.
        //! @param[in] context The GUI Core context containing the open tab content scope.
        LUNA_GUI_API void end_tab_item(GUICore::IContext* context);
        //! Ends the current tab bar.
        //! @param[in] context The GUI Core context containing the open tab-bar scope.
        LUNA_GUI_API void end_tab_bar(GUICore::IContext* context);
    }
}
