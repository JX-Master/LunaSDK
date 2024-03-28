/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MessageBox.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif
namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{
        
        //! Specifies the type of the message box.
        enum class MessageBoxType : u32
        {
            //! One message box with one OK button.
            ok = 1,
            //! One message box with one OK button and one Calcel button.
            ok_cancel = 2,
            //! One message box with one Retry button and one Calcel button.
            retry_cancel = 3,
            //! One message box with one Yes button and one No button.
            yes_no = 4,
            //! One message box with one Yes button, one No button and one Cancel button.
            yes_no_cancel = 5,
        };

        //! Specifies the icon that will be displayed on the message box.
        enum class MessageBoxIcon : u32
        {
            //! No icon will be displayed.
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

        //! Indicates buttons on the message box.
        enum class MessageBoxButton : u32
        {
            //! The OK button.
            ok = 1,
            //! The Cancel button.
            cancel,
            //! The Retry button.
            retry,
            //! The Yes button.
            yes,
            //! The No button.
            no
        };

        //! Displays one message box dialog. The current thread blocks until the dialog is closed.
        //! @param[in] text The UTF-8 text that will be displayed in the message box.
        //! @param[in] caption The caption (title) of the message box.
        //! @param[in] type The type of the message box,
        //! @param[in] icon The icon of the message box. Default value is @ref MessageBoxIcon::none, which does not show
        //! any icon.
        //! @return Returns the button clicked by the user.
        //! @par Valid Usage
        //! * `text` and `caption` must specify null-terminated strings.
        LUNA_WINDOW_API R<MessageBoxButton> message_box(const c8* text, const c8* caption, MessageBoxType type, MessageBoxIcon icon = MessageBoxIcon::none);
    
        //! @}
    }
}