/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Runtime.hpp
* @author JXMaster
* @date 2020/7/22
* @brief This header file contains APIs that controls Luna Runtime, including APIs to initialize and close 
* Luna Runtime, APIs to apply one assembly to Luna Runtime, etc.
*/
#pragma once
#include "Result.hpp"
#include "Name.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! Initializes and starts Luna Runtime.
	LUNA_RUNTIME_API bool init();

	//! Closes Luna Runtime.
	LUNA_RUNTIME_API void close();
}