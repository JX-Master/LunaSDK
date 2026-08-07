/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Workspace.hpp
* @author JXMaster
* @date 2026/7/16
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! Identifies the placement mode of one dock panel.
        enum class DockPanelMode : u8
        {
            //! The panel occupies a leaf in the dock space split tree.
            docking,
            //! The panel is displayed in a separate movable GUI Core layer.
            floating
        };

        //! Identifies the axis used by one dock space split node.
        enum class DockSplitAxis : u8
        {
            //! Splits the node into left and right children.
            x,
            //! Splits the node into upper and lower children.
            y
        };

        //! Describes one node in a dock space split tree.
        struct DockSpaceLayoutNodeDesc
        {
            //! Whether this node is a branch node.
            bool split = false;
            //! Split axis used by branch nodes.
            DockSplitAxis split_axis = DockSplitAxis::x;
            //! Fraction of available axis length assigned to @ref child0.
            f32 split_ratio = 0.5f;
            //! First child node index, or `U32_MAX` for an invalid child.
            u32 child0 = U32_MAX;
            //! Second child node index, or `U32_MAX` for an invalid child.
            u32 child1 = U32_MAX;
            //! Panel IDs stacked as tabs in this leaf node.
            Vector<id_t> tabs;
            //! Selected tab ID. Zero selects the first live tab.
            id_t selected_tab = 0;
        };

        //! Describes one floating panel in a dock space layout.
        struct DockSpaceFloatingPanelDesc
        {
            //! Panel ID.
            id_t panel = 0;
            //! Floating rectangle in dock space local coordinates.
            RectF rect = RectF(0.0f, 0.0f, 320.0f, 220.0f);
            //! Floating Z order. Larger values are displayed above smaller values.
            u32 z_order = 0;
        };

        //! Describes the complete persistent layout of one dock space.
        struct DockSpaceLayoutDesc
        {
            //! Dense split-tree node storage.
            Vector<DockSpaceLayoutNodeDesc> nodes;
            //! Root node index, or `U32_MAX` for an empty dock tree.
            u32 root_node = U32_MAX;
            //! Floating panel records.
            Vector<DockSpaceFloatingPanelDesc> floating_panels;
        };

        //! Configures dock space behavior and drawing.
        struct DockSpaceDesc
        {
            //! Thickness of draggable splitters.
            f32 splitter_size = 6.0f;
            //! Visible splitter line thickness. This does not affect the draggable hit region.
            f32 splitter_visual_size = 1.0f;
            //! Minimum fraction retained by either side of a split.
            f32 minimum_split_ratio = 0.08f;
            //! Color used by splitters.
            Float4U splitter_color = Float4U(0.24f, 0.29f, 0.36f, 1.0f);
            //! Color used by docking target indicators.
            Float4U docking_indicator_color = Float4U(0.10f, 0.50f, 0.82f, 0.86f);
        };

        //! Configures one panel managed by a dock space.
        struct DockPanelDesc
        {
            //! Whether the panel displays a title bar.
            bool title_bar = true;
            //! Whether the panel displays a close button when an open pointer is supplied.
            bool close_button = true;
            //! Whether a floating panel can be resized by dragging its border.
            bool resize_border = true;
            //! Title bar height in logical units. A non-positive value uses `gui.control.height` from the current Style.
            f32 title_bar_height = 0.0f;
            //! Border thickness in logical units.
            f32 border_size = 1.0f;
            //! Resize hit region thickness in logical units.
            f32 resize_border_size = 7.0f;
            //! Minimum floating panel size.
            Float2U minimum_floating_size = Float2U(140.0f, 90.0f);
            //! Panel background color. Zero alpha uses `gui.surface.1` for docked panels and
            //! `gui.dock_panel.floating.background` for floating panels from the current Style.
            Float4U background_color = Float4U(0.0f);
            //! Docked title bar color. Zero alpha uses `gui.surface.1` from the current Style.
            Float4U title_bar_color = Float4U(0.0f);
            //! Optional selected title tab fill. Zero alpha keeps the selected tab transparent.
            Float4U active_title_bar_color = Float4U(0.0f);
            //! Border color. Zero alpha uses `gui.border.strong` from the current Style.
            Float4U border_color = Float4U(0.0f);
        };

        //! Begins one dock space.
        //! @param[in] context GUI Core context.
        //! @param[in] id Stable dock space ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration of the dock space element.
        //! @param[in] desc Dock space behavior and drawing configuration.
        //! @return Returns the dock space root element.
        LUNA_GUI_API GUICore::ElementHandle begin_dock_space(GUICore::IContext* context, id_t id,
            const c8* label, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const DockSpaceDesc& desc = DockSpaceDesc());

        //! Immediately replaces the persistent layout of one dock space.
        //! @param[in] context GUI Core context.
        //! @param[in] dock_space Stable dock space ID. The dock space does not need to be submitted yet.
        //! @param[in] desc Complete replacement layout.
        //! @remark Call this only during initialization or when the application explicitly changes layouts.
        LUNA_GUI_API void set_dockspace_layout(GUICore::IContext* context, id_t dock_space,
            const DockSpaceLayoutDesc& desc);

        //! Begins one panel in the current dock space.
        //! @param[in] context GUI Core context.
        //! @param[in] id Stable panel ID referenced by @ref DockSpaceLayoutDesc.
        //! @param[in] label Displayed panel title and debug label.
        //! @param[in,out] open Optional visibility value. Closing the panel writes `false`.
        //! @param[in] desc Panel behavior and drawing configuration.
        //! @return Returns `true` when this panel's content should be submitted. Call @ref end_dock_panel only
        //! when this function returns `true`.
        LUNA_GUI_API bool begin_dock_panel(GUICore::IContext* context, id_t id, const c8* label,
            bool* open = nullptr, const DockPanelDesc& desc = DockPanelDesc());

        //! Ends the current visible dock panel.
        //! @param[in] context GUI Core context containing the open dock-panel scope.
        LUNA_GUI_API void end_dock_panel(GUICore::IContext* context);

        //! Ends the current dock space and finalizes panel topology, layout and layer order.
        //! @param[in] context GUI Core context containing the open dock-space scope.
        LUNA_GUI_API void end_dock_space(GUICore::IContext* context);
    }
}
