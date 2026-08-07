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
        //! @param[in] context The GUI Core context that owns the popup state.
        //! @param[in] id Stable popup ID.
        LUNA_GUI_API void open_popup(GUICore::IContext* context, id_t id);
        //! Closes a popup and clears its persistent open state.
        //! @param[in] context The GUI Core context that owns the popup state.
        //! @param[in] id Stable popup ID.
        LUNA_GUI_API void close_popup(GUICore::IContext* context, id_t id);
        //! Checks whether a popup is open.
        //! @param[in] context The GUI Core context that owns the popup state.
        //! @param[in] id Stable popup ID.
        //! @return Returns `true` if @p id currently has open popup state.
        LUNA_GUI_API bool is_popup_open(GUICore::IContext* context, id_t id);
        //! Begins an open popup as a new GUI Core layer.
        //! @param[in] context The GUI Core context receiving the popup layer.
        //! @param[in] id Stable popup and layer ID previously passed to @ref open_popup.
        //! @param[in] desc Popup position, layout and closing behavior.
        //! @param[out] out_handle Optional destination for the popup root element. Receives an invalid handle when
        //! the popup is closed.
        //! @return Returns `true` when popup content should be submitted. Call @ref end_popup only in this case.
        LUNA_GUI_API bool begin_popup(GUICore::IContext* context, id_t id, const PopupDesc& desc,
            GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a popup layer and arranges its content vertically inside @p rect.
        //! @param[in] context The GUI Core context containing the open popup scope.
        //! @param[in] popup The popup root returned by @ref begin_popup.
        //! @param[in] rect Root rectangle in popup-layer coordinates.
        //! @return Returns success or failure code from layout.
        LUNA_GUI_API RV end_popup(GUICore::IContext* context, const GUICore::ElementHandle& popup, const RectF& rect);

        //! Begins a delayed tooltip layer for an owner element.
        //! @param[in] context The GUI Core context receiving the tooltip layer.
        //! @param[in] id Stable tooltip and layer ID.
        //! @param[in] owner Element whose continuous hover state controls tooltip visibility.
        //! @param[in] desc Tooltip delay, offset and layout behavior.
        //! @param[out] out_handle Optional destination for the tooltip root element. Receives an invalid handle when
        //! the tooltip is not visible.
        //! @return Returns `true` when tooltip content should be submitted. Call @ref end_tooltip only in this case.
        LUNA_GUI_API bool begin_tooltip(GUICore::IContext* context, id_t id,
            const GUICore::ElementHandle& owner, const TooltipDesc& desc = TooltipDesc(),
            GUICore::ElementHandle* out_handle = nullptr);
        //! Ends a tooltip layer and arranges its content vertically inside @p rect.
        //! @param[in] context The GUI Core context containing the open tooltip layer.
        //! @param[in] tooltip The tooltip root returned by @ref begin_tooltip.
        //! @param[in] rect Root rectangle in tooltip-layer coordinates.
        //! @return Returns success or failure code from layout.
        LUNA_GUI_API RV end_tooltip(GUICore::IContext* context, const GUICore::ElementHandle& tooltip,
            const RectF& rect);
        //! Adds a delayed single-text tooltip for an owner element.
        //! @param[in] context The GUI Core context receiving the tooltip when visible.
        //! @param[in] id Stable tooltip and layer ID.
        //! @param[in] owner Element whose continuous hover state controls tooltip visibility.
        //! @param[in] content Null-terminated UTF-8 tooltip text.
        //! @param[in] desc Tooltip delay, offset and sizing behavior.
        //! @return Returns the tooltip root when visible, or an invalid handle otherwise.
        LUNA_GUI_API GUICore::ElementHandle set_item_tooltip(GUICore::IContext* context, id_t id,
            const GUICore::ElementHandle& owner, const c8* content, const TooltipDesc& desc = TooltipDesc());

        //! Adds a combo box backed by an integer item index.
        //! @param[in] context The GUI Core context receiving the element and popup.
        //! @param[in] id Stable combo ID.
        //! @param[in] label Optional debug label.
        //! @param[in,out] current_item Selected zero-based item index. Non-empty lists clamp this value during submission.
        //! @param[in] items Null-terminated UTF-8 item labels.
        //! @param[in] layout Layout configuration for the combo preview.
        //! @param[in] desc Combo interaction and popup sizing.
        //! @return Returns the created preview element.
        LUNA_GUI_API GUICore::ElementHandle combo(GUICore::IContext* context, id_t id, const c8* label,
            i32* current_item, Span<const c8*> items,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const ComboDesc& desc = ComboDesc());

        //! Begins a horizontally arranged menu bar.
        //! @param[in] context The GUI Core context receiving the element.
        //! @param[in] id Stable menu-bar ID.
        //! @param[in] label Optional debug label.
        //! @param[in] layout Layout configuration for the bar.
        //! @param[in] desc Top-level item spacing.
        //! @return Returns the created menu-bar element. Call @ref end_menu_bar after submitting its menus.
        LUNA_GUI_API GUICore::ElementHandle begin_menu_bar(GUICore::IContext* context, id_t id,
            const c8* label = nullptr, const GUICore::LayoutConfig& layout = GUICore::LayoutConfig(),
            const MenuBarDesc& desc = MenuBarDesc());
        //! Ends a menu bar and installs its deferred horizontal layout.
        //! @param[in] context The GUI Core context containing the open menu-bar scope.
        //! @param[in] menu_bar The element returned by @ref begin_menu_bar.
        LUNA_GUI_API void end_menu_bar(GUICore::IContext* context, const GUICore::ElementHandle& menu_bar);
        //! Begins a top-level menu or submenu.
        //! @param[in] context The GUI Core context receiving the menu item and optional popup.
        //! @param[in] id Stable menu ID.
        //! @param[in] label Null-terminated UTF-8 menu label.
        //! @param[in] desc Menu interaction behavior.
        //! @param[out] out_handle Optional destination for the menu-header element.
        //! @param[in] layout Layout configuration for the menu header.
        //! @return Returns `true` when menu contents should be submitted. Call @ref end_menu only in this case.
        LUNA_GUI_API bool begin_menu(GUICore::IContext* context, id_t id, const c8* label,
            const MenuItemDesc& desc = MenuItemDesc(), GUICore::ElementHandle* out_handle = nullptr,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends the current menu popup.
        //! @param[in] context The GUI Core context containing the open menu popup.
        //! @param[in] rect Popup root rectangle in menu-layer coordinates.
        //! @return Returns success or failure code from popup layout.
        LUNA_GUI_API RV end_menu(GUICore::IContext* context, const RectF& rect);
        //! Begins one menu item as a generic horizontal child container.
        //! @param[in] context The GUI context receiving the element.
        //! @param[in] id Element ID.
        //! @param[in] label Debug label. This value is not rendered automatically.
        //! @param[in] desc Menu-item behavior and appearance.
        //! @param[in] layout Layout configuration.
        //! @return Returns the created menu-item element. Call @ref end_menu_item after submitting its children.
        //! @remark Icons, text, shortcut labels and other content are ordinary children. The menu item does not
        //! inspect or special-case child element types.
        LUNA_GUI_API GUICore::ElementHandle begin_menu_item(GUICore::IContext* context, id_t id,
            const c8* label = nullptr, const MenuItemDesc& desc = MenuItemDesc(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Ends a menu-item child container begun by @ref begin_menu_item.
        //! @param[in] context The GUI Core context containing the open menu-item scope.
        LUNA_GUI_API void end_menu_item(GUICore::IContext* context);
        //! Adds one menu item.
        //! @param[in] context The GUI Core context receiving the item.
        //! @param[in] id Stable item ID.
        //! @param[in] label Null-terminated UTF-8 item label.
        //! @param[in] selected Whether to render the item as selected.
        //! @param[in] desc Item interaction, shortcut and disabled-state behavior.
        //! @param[in] layout Layout configuration for the item.
        //! @return Returns the created item element. Query the returned handle for click state.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool selected = false, const MenuItemDesc& desc = MenuItemDesc(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds one checkable menu item.
        //! @param[in] context The GUI Core context receiving the item.
        //! @param[in] id Stable item ID.
        //! @param[in] label Null-terminated UTF-8 item label.
        //! @param[in,out] selected Optional checked value toggled when the item is clicked.
        //! @param[in] desc Item interaction, shortcut and disabled-state behavior.
        //! @param[in] layout Layout configuration for the item.
        //! @return Returns the created item element.
        LUNA_GUI_API GUICore::ElementHandle menu_item(GUICore::IContext* context, id_t id, const c8* label,
            bool* selected, const MenuItemDesc& desc = MenuItemDesc(),
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
        //! Adds a separator to the current menu.
        //! @param[in] context The GUI Core context receiving the separator.
        //! @param[in] id Stable separator ID.
        //! @param[in] layout Layout configuration for the separator.
        //! @return Returns the created separator element.
        LUNA_GUI_API GUICore::ElementHandle menu_separator(GUICore::IContext* context, id_t id,
            const GUICore::LayoutConfig& layout = GUICore::LayoutConfig());
    }
}
