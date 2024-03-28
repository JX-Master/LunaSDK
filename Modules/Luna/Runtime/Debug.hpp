/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.hpp
* @author JXMaster
* @date 2023/11/24
*/
#pragma once
#include "Base.hpp"
#include "Span.hpp"
#include "Name.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeDebug Debugging
    //! @}

    //! @addtogroup RuntimeDebug
    //! @{
    
    //! Captures function call stack information of the current thread.
    //! @param[out] frames One buffer that receives captured frames. Every frame is represented by 
    //! one opaque handle in the buffer.
    //! @return Returns the number of captured frames written to `frames`.
    LUNA_RUNTIME_API u32 stack_backtrace(Span<opaque_t> frames);

    //! Gets symbolic names for frames returned by @ref stack_backtrace.
    //! @param[in] frames One buffer that contains frames to query.
    //! @return Returns one array of strings that store symbolic names for frames. Strings are stored in 
    //! the same order as `frames`. If the symbolic name of one frame is not found, `nullptr` will be written.
    //! @par Valid Usage
    //! 1. All frames in `frames` must be valid frames returned by @ref stack_backtrace. In particular, if the return
    //! value of @ref stack_backtrace is smaller than the size of the frame buffer passed to @ref stack_backtrace, only 
    //! valid frames, not the whole buffer, shall be specified in this call.
    LUNA_RUNTIME_API const c8** stack_backtrace_symbols(Span<const opaque_t> frames);

    //! Frees symbols returned by @ref stack_backtrace_symbols.
    //! @param[in] symbols The symbol array returned by @ref stack_backtrace_symbols.
    LUNA_RUNTIME_API void free_backtrace_symbols(const c8** symbols);

    //! @}
}