/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MessageBox.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
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

#ifdef LUNA_PLATFORM_DESKTOP
        
        //! Specifies the semantic icon requested for the message box. The platform may substitute or omit icons that
        //! do not have a native equivalent.
        enum class MessageBoxIcon : u32
        {
            //! Requests no status icon. The platform may still display application identity artwork.
            none = 0,
            //! Displays information icon.
            //! The shape of the icon is platform-specific, usually one icon with letter "i".
            information = 1,
            //! Displays warning icon.
            //! The shape of the icon is platform-specific, usually one yellow icon with symbol "!".
            warning = 2,
            //! Displays question icon.
            //! The shape of the icon is platform-specific, usually one icon with symbol "?".
            question = 3,
            //! Displays error icon.
            //! The shape of the icon is platform-specific, usually one red icon with symbol "X".
            error = 4
        };

        //! Displays one native modal message box. The current thread blocks until the dialog is closed.
        //! @param[in] text The UTF-8 text that will be displayed in the message box.
        //! @param[in] title The UTF-8 title of the message box.
        //! @param[in] buttons The UTF-8 titles of buttons in semantic priority order. The platform determines their
        //! physical layout.
        //! @param[in] icon The semantic icon requested for the message box. Default value is
        //! @ref MessageBoxIcon::none.
        //! @param[in] default_button_index The index of the button activated by the Return key.
        //! @param[in] cancel_button_index The index of the button activated by the Escape key and, on platforms that
        //! provide one, the native window-close action. Specify @ref USIZE_MAX to prevent closing the dialog without
        //! explicitly selecting a button.
        //! @return Returns the zero-based index of the selected button, @ref E_BAD_ARGUMENTS if the arguments are
        //! invalid, @ref E_DATA_TOO_BIG if the platform cannot represent the button count, @ref E_BAD_CALLING_TIME if
        //! the calling thread cannot present native UI, @ref E_OUT_OF_MEMORY if native dialog allocation fails,
        //! @ref E_INTERRUPTED if the native modal session ends without selecting a button, or @ref E_BAD_PLATFORM_CALL
        //! if the native dialog call fails.
        //! @remark The function may run a nested native event loop while it blocks.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        //! * `text`, `title` and every element of `buttons` must specify null-terminated, valid UTF-8 strings.
        //! * `buttons` must not be empty, and every button title must not be empty.
        //! * `default_button_index` must be smaller than `buttons.size()`.
        //! * `cancel_button_index` must be @ref USIZE_MAX or smaller than `buttons.size()`. It must not equal
        //! `default_button_index`.
        LUNA_WINDOW_API R<usize> message_box(const c8* text, const c8* title, Span<const c8*> buttons,
            MessageBoxIcon icon = MessageBoxIcon::none, usize default_button_index = 0,
            usize cancel_button_index = USIZE_MAX);
    
#endif

        //! @}
    }
}
