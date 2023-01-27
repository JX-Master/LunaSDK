/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2022/10/31
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <Runtime/Module.hpp>
namespace Luna
{
	namespace Window
	{
		StartupParams g_startup_params;
		c8 g_name[260];
		Version g_version;

		RV init()
		{
			if (g_startup_params.name)
			{
				auto len = strlen(g_startup_params.name);
				memcpy(g_name, g_startup_params.name, min<usize>(len, 260));
			}
			g_version = g_startup_params.version;
			return platform_init();
		}

		void close()
		{
			platform_close();
		}

		StaticRegisterModule mod("Window", "", init, close);

		LUNA_WINDOW_API const c8* get_name()
		{
			return g_name;
		}

		LUNA_WINDOW_API Version get_version()
		{
			return g_version;
		}
	}
}