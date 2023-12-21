/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Runtime.hpp
* @author JXMaster
* @date 2020/7/22
* @brief This header file contains APIs to initialize and close Luna SDK.
*/
#pragma once
#include "Result.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup Runtime Runtime
	//! The Runtime module provides the runtime environment of Luna SDK and defines core functionalities that will be used by almost all modules.
	//! @{

	//! @defgroup RuntimeInit SDK initialization and shutdown

	//! @}

	//! @addtogroup RuntimeInit
	//! @{

	//! Initializes Luna SDK.
	//! @details Call this function to initialize Luna SDK. Most features provided by Luna SDK are only available after Luna SDK is initialized, 
	//! so always initialize Luna SDK firstly on program startup. Calling this function when Luna SDK is already initialized does nothing and returns `true` directly.
	//! 
	//! Note that modules registered to Luna SDK will not be initialized by this function, they should be initialized manually using functions like @ref init_modules.
	//! @return Returns `true` if Luna SDK is succssfully initialized, returns `false` otherwise.
	LUNA_RUNTIME_API bool init();

	//! Closes Luna SDK.
	//! @details Call this function to close Luna SDK. Most features provided by Luna SDK are not available after Luna SDK is closed.
	//! Calling this function when Luna SDK is not initialized or already closed does nothing and returns directly.
	//! 
	//! Initialized modules *will* be closed by this function in the reverse order of their initialization order, so they don't need to be closed manually.
	//! @remark All dynamic memory allocated from @ref memalloc, @ref memrealloc and @ref memnew must be freed before calling @ref close,
	//! all boxed objects created from @ref new_object and @ref object_alloc must be released before calling @ref close too. 
	//! Calls to @ref memfree, @ref memdelete, @ref object_release and other functions after @ref close results in undefined behavior, and usually a program crash. 
	//! This often happens when you declare global variables that hold dynamic allocated resources (such as @ref Ref) and memory blocks (such as @ref UniquePtr, 
	//! and containers like @ref Vector, @ref HashMap, etc.). Remember to clear such resources before calling @ref close. For some containers, you should call 
	//! `clear` then `shrink_to_fit` to eventually frees the internal memory buffer used by containers.
	LUNA_RUNTIME_API void close();

	//! @}
}