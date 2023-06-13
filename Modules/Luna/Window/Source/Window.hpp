/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include "../Application.hpp"
namespace Luna
{
	namespace Window
	{
		extern StartupParams g_startup_params;
		RV platform_init();
		void platform_close();
	}
}