/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Monitor.cpp
* @author JXMaster
* @date 2022/10/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#ifdef LUNA_WINDOW_GLFW
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Monitor.hpp"
#include <GLFW/glfw3.h>

namespace Luna
{
	namespace Window
	{
		LUNA_WINDOW_API monitor_t get_primary_monitor()
		{
			return glfwGetPrimaryMonitor();
		}
		LUNA_WINDOW_API u32 count_monitors()
		{
			int ret;
			glfwGetMonitors(&ret);
			return ret;
		}
		LUNA_WINDOW_API monitor_t get_monitor(u32 index)
		{
			int ret;
			auto monitors = glfwGetMonitors(&ret);
			lucheck(index < ret);
			return monitors[index];
		}
		Event<monitor_event_handler_t> g_monitor_change_event;
		LUNA_WINDOW_API Event<monitor_event_handler_t>& get_monitor_event()
		{
			return g_monitor_change_event;
		}
		static void glfw_monitor_callback(GLFWmonitor* monitor, int event)
		{
			if (event == GLFW_CONNECTED)
			{
				g_monitor_change_event(monitor, MonitorEvent::connected);
			}
			else if (event == GLFW_DISCONNECTED)
			{
				g_monitor_change_event(monitor, MonitorEvent::disconnected);
			}
		}
		void monitor_init()
		{
			glfwSetMonitorCallback(glfw_monitor_callback);
		}
		void monitor_close()
		{
			g_monitor_change_event.clear();
		}
		LUNA_WINDOW_API u32 count_monitor_supported_video_modes(monitor_t monitor)
		{
			int count;
			glfwGetVideoModes((GLFWmonitor*)monitor, &count);
			return count;
		}
		LUNA_WINDOW_API VideoMode get_monitor_supported_video_mode(monitor_t monitor, u32 index)
		{
			int count;
			auto modes = glfwGetVideoModes((GLFWmonitor*)monitor, &count);
			lucheck(index < count);
			VideoMode mode;
			auto& src = modes[index];
			mode.width = (u32)src.width;
			mode.height = (u32)src.height;
			mode.red_bits = (u32)src.redBits;
			mode.green_bits = (u32)src.greenBits;
			mode.blue_bits = (u32)src.blueBits;
			mode.refresh_rate = (u32)src.refreshRate;
			return mode;
		}
		LUNA_WINDOW_API VideoMode get_monitor_video_mode(monitor_t monitor)
		{
			auto src = glfwGetVideoMode((GLFWmonitor*)monitor);
			VideoMode mode;
			mode.width = (u32)src->width;
			mode.height = (u32)src->height;
			mode.red_bits = (u32)src->redBits;
			mode.green_bits = (u32)src->greenBits;
			mode.blue_bits = (u32)src->blueBits;
			mode.refresh_rate = (u32)src->refreshRate;
			return mode;
		}
		LUNA_WINDOW_API UInt2U get_monitor_physical_size(monitor_t monitor)
		{
			int w, h;
			glfwGetMonitorPhysicalSize((GLFWmonitor*)monitor, &w, &h);
			return UInt2U((u32)w, (u32)h);
		}
		LUNA_WINDOW_API f32 get_monitor_dpi_scale_factor(monitor_t monitor)
		{
			f32 xscale, yscale;
			glfwGetMonitorContentScale((GLFWmonitor*)monitor, &xscale, &yscale);
			return xscale;
		}
		LUNA_WINDOW_API Int2U get_monitor_position(monitor_t monitor)
		{
			int x, y;
			glfwGetMonitorPos((GLFWmonitor*)monitor, &x, &y);
			return Int2U(x, y);
		}
		LUNA_WINDOW_API RectI get_monitor_working_area(monitor_t monitor)
		{
			int x, y, w, h;
			glfwGetMonitorWorkarea((GLFWmonitor*)monitor, &x, &y, &w, &h);
			return RectI(x, y, w, h);
		}
		LUNA_WINDOW_API Name get_monitor_name(monitor_t monitor)
		{
			return glfwGetMonitorName((GLFWmonitor*)monitor);
		}
	}
}
#endif