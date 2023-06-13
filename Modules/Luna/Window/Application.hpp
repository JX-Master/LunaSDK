/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Application.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Base.hpp>

#ifdef LUNA_PLATFORM_WINDOWS
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#endif

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
	namespace Window
	{

		struct StartupParams
		{
			//! Tha application name.
			const c8* name = nullptr;
			//! The application version.
			Version version = Version(0, 0, 0);
#ifdef LUNA_PLATFORM_WINDOWS
			//! The `hInstance` passed from `WinMain`. Set to `NULL` will let the
			//! system fetch the instance handle from `GetModuleHandleW(NULL)`.
			HINSTANCE hInstance = NULL;
			//! The `hPrevInstance` passed from `WinMain`.
			HINSTANCE hPrevInstance = NULL;
			//! The `lpCmdLine` passed from `WinMain`.
			LPSTR lpCmdLine = NULL;
			//! The `nCmdShow` passed from `WinMain`.
			int nCmdShow = 0;
			//! The icon handle from `LoadIcon` if not `NULL`.
			HICON hIcon = NULL;
			//! The icon handle from `LoadIcon` if not `NULL`.
			HICON hIconSm = NULL;
#endif
		};
		//! Sets the startup parameters for Window module. This should be called
		//! before the module is initialized.
		LUNA_WINDOW_API void set_startup_params(const StartupParams& params);

		//! Gets the application name.
		//! @return Returns one string that represents the application name. The string is valid 
		//! until this module is closed.
		LUNA_WINDOW_API const c8* get_name();

		//! Gets the application version.
		LUNA_WINDOW_API Version get_version();
	}
}