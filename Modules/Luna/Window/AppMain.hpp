/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMain.hpp
* @author JXMaster
* @date 2025/3/19
*/
#pragma once

#ifdef LUNA_PLATFORM_WINDOWS
#include "Windows/AppMainWindows.inl"
#elif defined(LUNA_PLATFORM_MACOS)
#include "Cocoa/AppMainCocoa.inl"
#elif defined(LUNA_PLATFORM_IOS)
#include "UIKit/AppMainUIKit.inl"
#endif