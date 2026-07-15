/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Overlay.hpp
* @author JXMaster
* @date 2026/7/15
*/
#pragma once
#include "Widgets.hpp"

namespace Luna
{
    namespace GUI
    {
        //! Opens a popup. A matching @ref begin_popup observes this request immediately when called later in the frame.
        LUNA_GUI_API void open_popup(GUICore::IContext* context, id_t id);
        //! Closes a popup and clears its persistent open state.
        LUNA_GUI_API void close_popup(GUICore::IContext* context, id_t id);
        //! Checks whether a popup is open.
        LUNA_GUI_API bool is_popup_open(GUICore::IContext* context, id_t id);
        //! Begins an open popup as a new GUI Core layer.
        //! @return Returns `true` when popup content should be submitted. Call @ref end_popup only in this case.
        LUNA_GUI_API bool begin_popup(GUICore::IContext* context, id_t id, const PopupDesc& desc,
            GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a popup layer and arranges its content vertically inside @p rect.
        LUNA_GUI_API RV end_popup(GUICore::IContext* context, const GUICore::ElementHandle& popup, const RectF& rect);

        //! Begins a delayed tooltip layer for an owner element.
        //! @return Returns `true` when tooltip content should be submitted. Call @ref end_tooltip only in this case.
        LUNA_GUI_API bool begin_tooltip(GUICore::IContext* context, id_t id,
            const GUICore::ElementHandle& owner, const TooltipDesc& desc = TooltipDesc(),
            GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a tooltip layer and arranges its content vertically inside @p rect.
        LUNA_GUI_API RV end_tooltip(GUICore::IContext* context, const GUICore::ElementHandle& tooltip,
            const RectF& rect);
        //! Adds a delayed single-text tooltip for an owner element.
        LUNA_GUI_API GUICore::ElementHandle set_item_tooltip(GUICore::IContext* context, id_t id,
            const GUICore::ElementHandle& owner, const c8* content, const TooltipDesc& desc = TooltipDesc());

        //! Adds a combo box backed by an integer item index.
        LUNA_GUI_API GUICore::ElementHandle combo(GUICore::IContext* context, id_t id, const c8* label,
            i32* current_item, Span<const c8*> items,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ComboDesc& desc = ComboDesc());

        //! Begins a horizontally arranged menu bar.
        LUNA_GUI_API GUICore::ElementHandle begin_menu_bar(GUICore::IContext* context, id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const MenuBarDesc& desc = MenuBarDesc());
        //! Ends a menu bar and installs its deferred horizontal layout.
        LUNA_GUI_API void end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar);
        //! Begins a top-level menu or submenu.
        //! @return Returns `true` when menu contents should be submitted. Call @ref end_menu only in this case.
        LUNA_GUI_API bool begin_menu(GUICore::IContext* context, id_t id, const c8* label,
            const MenuItemDesc& desc = MenuItemDesc(), GUICore::ElementHandle* out_handle = nullptr,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current menu popup.
        LUNA_GUI_API RV end_menu(GUICore::IContext* context, const RectF& rect);
        //! Adds one menu item.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool selected = false, const MenuItemDesc& desc = MenuItemDesc(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds one checkable menu item.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool* selected, const MenuItemDesc& desc = MenuItemDesc(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a separator to the current menu.
        LUNA_GUI_API GUICore::ElementHandle menu_separator(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
    }
}
