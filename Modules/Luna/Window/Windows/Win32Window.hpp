/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file IWindowsWindow.hpp
* @author JXMaster
* @date 2022/4/5
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>
#ifdef LUNA_PLATFORM_WINDOWS
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#include "../Window.hpp"

namespace Luna
{
	namespace Window
	{
		//! Implemented by window object on Windows platform.
		struct IWin32Window : virtual IWindow
		{
			luiid("{939C8832-C687-4F8E-811B-506B62C872F0}");

			//! Fetches the native handle of this window.
			virtual HWND get_hwnd() = 0;
		};
	}
}

#endif