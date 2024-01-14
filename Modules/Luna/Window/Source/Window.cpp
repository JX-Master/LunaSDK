/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2022/10/31
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include <Luna/Runtime/Module.hpp>
#include "../Window.hpp"
namespace Luna
{
	namespace Window
	{
		StartupParams g_startup_params;
		c8 g_name[260];
		Version g_version;

		struct WindowModule : public Module
		{
			virtual const c8* get_name() override { return "Window"; }
			virtual RV on_init() override
			{
				if (g_startup_params.name)
				{
					auto len = strlen(g_startup_params.name);
					memcpy(g_name, g_startup_params.name, min<usize>(len, 260));
				}
				g_version = g_startup_params.version;
				return platform_init();
			}
			virtual void on_close() override
			{
				platform_close();
			}
		};
		
	}
	LUNA_WINDOW_API Module* module_window()
	{
		static Window::WindowModule m;
		return &m;
	}
}