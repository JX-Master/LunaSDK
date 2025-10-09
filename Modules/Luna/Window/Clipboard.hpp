/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Clipboard.hpp
* @author JXMaster
* @date 2025/10/9
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/String.hpp>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

        //! Reads UTF-8 text from clipboard.
        //! @param[out] out_text The UTF-8 text read from clipboard.
        LUNA_WINDOW_API RV get_clipboard_text(String& out_text);

        //! Writes UTF-8 text to clipboard.
        //! @param[in] text The UTF-8 text to be written to clipboard.
        //! @param[in] size The maximum number of @ref c8 elements to be written to clipboard.
        //! The write will be stopped if the size is reached or a null terminator is encountered.
        LUNA_WINDOW_API RV set_clipboard_text(const c8* text, usize size = USIZE_MAX);

        //! @}
    }
}