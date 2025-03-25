/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMain.hpp
* @author JXMaster
* @date 2025/3/19
*/
#pragma once
#include "AppMainCallbacks.hpp"

#ifdef LUNA_PLATFORM_WINDOWS
#include "Windows/AppMainWindows.inl"
#elif LUNA_PLATFORM_MACOS
#include "SDL/AppMainSDL.inl"
#endif