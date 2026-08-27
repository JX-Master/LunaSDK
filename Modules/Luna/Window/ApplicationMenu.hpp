/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ApplicationMenu.hpp
* @author JXMaster
* @date 2026/8/26
*/
#pragma once
#include <Luna/Runtime/KeyCode.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Span.hpp>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

#if defined(LUNA_PLATFORM_MACOS)

        //! Identifies one application menu item.
        //! @details The value `0` is reserved for items that do not expose an application-defined identifier.
        using application_menu_item_id_t = u64;

        //! Specifies the type of one application menu item.
        enum class ApplicationMenuItemType : u8
        {
            //! An invocable command or standard platform action.
            command,
            //! A menu that contains child items.
            submenu,
            //! A visual separator between adjacent items.
            separator,
        };

        //! Specifies the standard platform role of one application menu item.
        enum class ApplicationMenuItemRole : u8
        {
            //! No standard role. Selecting a command with this role dispatches
            //! @ref ApplicationMenuItemInvokedEvent.
            none,
            //! Shows the platform-standard About interface.
            about,
            //! Identifies an application-defined Settings command.
            //! @details The platform supplies the conventional title. The item must have a non-zero identifier,
            //! and selecting it dispatches @ref ApplicationMenuItemInvokedEvent.
            settings,
            //! Hosts the platform Services menu.
            services,
            //! Hides this application.
            hide,
            //! Hides all applications except this application.
            hide_others,
            //! Shows all applications.
            show_all,
            //! Requests the application to quit.
            quit,
            //! Identifies the platform Window menu.
            window_menu,
            //! Identifies the platform Help menu.
            help_menu,
        };

        //! Specifies the check state of one application menu item.
        enum class ApplicationMenuItemCheckState : u8
        {
            //! The item has no check mark.
            none,
            //! The item is checked.
            checked,
            //! The item has a mixed or indeterminate check state.
            mixed,
        };

        //! Specifies modifier keys for an application menu keyboard shortcut.
        //! These values can be bitwise-OR combined.
        enum class KeyModifierFlag : u8
        {
            //! No modifier key.
            none = 0x00,
            //! The Control key.
            ctrl = 0x01,
            //! The Shift key.
            shift = 0x02,
            //! The Alt key on Windows or Option key on macOS.
            alt = 0x04,
            //! The platform system command key, such as Command on macOS.
            system = 0x08,
        };

        //! Describes the mutable presentation state of one application menu item.
        struct ApplicationMenuItemState
        {
            //! Whether the item accepts user interaction.
            bool enabled = true;
            //! The check state displayed by the item.
            ApplicationMenuItemCheckState check_state = ApplicationMenuItemCheckState::none;
            //! Whether the item is visible.
            bool visible = true;
        };

        //! Describes one item in an application menu tree.
        struct ApplicationMenuItemDesc
        {
            //! The item type.
            ApplicationMenuItemType type = ApplicationMenuItemType::command;
            //! The standard platform role, or @ref ApplicationMenuItemRole::none for an application-defined item.
            ApplicationMenuItemRole role = ApplicationMenuItemRole::none;
            //! The application-defined item identifier.
            //! @details Every non-zero identifier must be unique in the complete menu tree. An application-defined
            //! command and a Settings command must have a non-zero identifier.
            application_menu_item_id_t id = 0;
            //! The UTF-8 item title.
            //! @details This is ignored for separators. A standard role may use its platform-default title when this
            //! is `nullptr`.
            const c8* title = nullptr;
            //! The initial presentation state.
            ApplicationMenuItemState state;
            //! The non-modifier key of the optional keyboard shortcut.
            //! Specify @ref KeyCode::unknown to disable the shortcut.
            KeyCode shortcut_key = KeyCode::unknown;
            //! Modifier keys of the optional keyboard shortcut.
            KeyModifierFlag shortcut_modifiers = KeyModifierFlag::none;
            //! Child items. This must be empty unless @ref type is @ref ApplicationMenuItemType::submenu.
            Span<const ApplicationMenuItemDesc> children;
        };

        //! Describes the application main menu.
        struct ApplicationMenuDesc
        {
            //! The top-level menu items.
            //! @details Every top-level item must be a submenu. On macOS, the first item is the application menu
            //! displayed next to the Apple menu.
            Span<const ApplicationMenuItemDesc> items;
        };

        //! Replaces the application main menu.
        //! @param[in] desc The complete menu descriptor tree.
        //! @return Returns `ok` on success, @ref E_BAD_ARGUMENTS if the descriptor tree is invalid, or
        //! @ref E_OUT_OF_MEMORY if native menu allocation fails.
        //! @remark The complete descriptor tree is validated and deep-copied before this function returns. If
        //! validation or installation fails, the currently installed menu is not changed.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        LUNA_WINDOW_API RV set_application_menu(const ApplicationMenuDesc& desc);

        //! Restores the platform-default application main menu.
        //! @return Returns `ok` on success or @ref E_OUT_OF_MEMORY if native menu allocation fails.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        LUNA_WINDOW_API RV reset_application_menu();

        //! Updates the presentation state of one identified application menu item.
        //! @param[in] id The non-zero identifier of the item to update.
        //! @param[in] state The new presentation state.
        //! @return Returns `ok` on success, @ref E_BAD_ARGUMENTS if `state` is invalid, or @ref E_NOT_FOUND if no
        //! item has the specified identifier.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        LUNA_WINDOW_API RV set_application_menu_item_state(application_menu_item_id_t id,
            const ApplicationMenuItemState& state);

        //! Updates the title of one identified application menu item.
        //! @param[in] id The non-zero identifier of the item to update.
        //! @param[in] title The new null-terminated UTF-8 title.
        //! @return Returns `ok` on success, @ref E_BAD_ARGUMENTS if `title` is invalid, or @ref E_NOT_FOUND if no
        //! item has the specified identifier.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        //! * `title` must specify one null-terminated UTF-8 string.
        LUNA_WINDOW_API RV set_application_menu_item_title(application_menu_item_id_t id, const c8* title);

#endif

        //! @}
    }
}
