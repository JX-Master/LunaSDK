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
        LUNA_GUI_API GUICore::ElementHandle begin_h_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a horizontal flex layout and attaches its layout descriptor.
        LUNA_GUI_API void end_h_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc = GUICore::FlexLayoutDesc());

        //! Begins a vertical flex layout.
        LUNA_GUI_API GUICore::ElementHandle begin_v_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a vertical flex layout and attaches its layout descriptor.
        LUNA_GUI_API void end_v_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::FlexLayoutDesc& desc = GUICore::FlexLayoutDesc());

        //! Begins a row-major grid layout.
        LUNA_GUI_API GUICore::ElementHandle begin_grid_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a grid layout and attaches its layout descriptor.
        LUNA_GUI_API void end_grid_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::GridLayoutDesc& desc);

        //! Begins an anchor-based canvas layout.
        LUNA_GUI_API GUICore::ElementHandle begin_canvas_layout(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a canvas layout and copies child placement records for this frame.
        LUNA_GUI_API void end_canvas_layout(GUICore::IContext* context, const GUICore::ElementHandle& element,
            const GUICore::CanvasLayoutDesc& desc);

        //! Begins a scroll view and its package-owned content root.
        LUNA_GUI_API GUICore::ElementHandle begin_scroll_view(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ScrollViewDesc& desc = ScrollViewDesc());
        //! Ends the current scroll view.
        LUNA_GUI_API void end_scroll_view(GUICore::IContext* context);
        //! Gets the previous-frame visible content rectangle for a scroll view begun in the current frame.
        LUNA_GUI_API RectF get_scroll_view_visible_rect(GUICore::IContext* context,
            const GUICore::ElementHandle& scroll_view);

        //! Begins a tab bar bound to a selected tab index.
        LUNA_GUI_API GUICore::ElementHandle begin_tab_bar(GUICore::IContext* context, id_t id, i32* selected_index,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(), const TabBarDesc& desc = TabBarDesc());
        //! Begins one tab item and returns whether its content should be submitted this frame.
        LUNA_GUI_API bool begin_tab_item(GUICore::IContext* context, id_t id, const c8* label);
        //! Ends the current selected tab item content scope.
        LUNA_GUI_API void end_tab_item(GUICore::IContext* context);
        //! Ends the current tab bar.
        LUNA_GUI_API void end_tab_bar(GUICore::IContext* context);
    }
}
