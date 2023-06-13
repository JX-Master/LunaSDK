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
	//! Initializes Luna SDK.
	//! @return Returns `true` if Luna SDK is succssfully initialized, returns `false` otherwise.
	//! @remark Calling this function when Luna SDK is already initialized does nothing and returns `true` directly.
	LUNA_RUNTIME_API bool init();

	//! Closes Luna Runtime.
	//! @remark Calling this function when Luna SDK is not initialized does nothing and returns directly.
	LUNA_RUNTIME_API void close();
}