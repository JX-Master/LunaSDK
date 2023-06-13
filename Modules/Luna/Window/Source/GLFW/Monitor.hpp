/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Monitor.hpp
* @author JXMaster
* @date 2022/10/4
*/
#include "../../Window.hpp"

#ifdef LUNA_WINDOW_GLFW
namespace Luna
{
	namespace Window
	{
		void monitor_init();
		void monitor_close();
	}
}
#endif