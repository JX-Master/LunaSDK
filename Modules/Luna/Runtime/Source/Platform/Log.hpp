/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Log.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Log.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Logs one message to the platform.
        //! If logging is not available on the current build, this function does nothing.
        //! @param[in] verbosity The log verbosity.
        //! @param[in] tag The log tag. Used by the implementation to filter logs.
        //! @param[in] tag_len The log tag length, not including the null terminator.
        //! @param[in] message The log message encoded in UTF-8. The implementation should insert newline character (`\n`) in its implementation.
        //! @param[in] message_len The log message length, not including the null terminator.
        void log(LogVerbosity verbosity, const c8* tag, usize tag_len, const c8* message, usize message_len);
    }
}