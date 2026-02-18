/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Result.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Captures function call stack information of the current thread.
        //! @param[out] frames One buffer that receives captured frames. Every frame is represented by 
        //! one opaque handle in the buffer.
        //! @return The number of captured frames written to `frames`.
        u32 stack_backtrace(Span<opaque_t> frames);

        //! Gets symbolic names for frames returned by @ref stack_backtrace.
        //! @param[in] frames One buffer that contains frames to query.
        //! @return Returns one array of strings that store symbolic names for frames. Strings are stored in 
        //! the same order as `frames`. If the symbolic name of one frame is not found, `nullptr` will be written.
        //! @par Valid Usage
        //! 1. All frames in `frames` must be valid frames returned by @ref stack_backtrace. In particular, if the return
        //! value of @ref stack_backtrace is smaller than the size of the frame buffer passed to @ref stack_backtrace, only 
        //! valid frames, not the whole buffer, shall be specified in this call.
        const c8** stack_backtrace_symbols(Span<const opaque_t> frames);

        //! Frees symbols returned by @ref stack_backtrace_symbols.
        //! @param[in] symbols The symbol array returned by @ref stack_backtrace_symbols.
        void free_backtrace_symbols(const c8** symbols);
    }
}