/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.cpp
* @author JXMaster
* @date 2022/10/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Display.hpp"
#include <GLFW/glfw3.h>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API display_t get_primary_display()
        {
            return glfwGetPrimaryMonitor();
        }
        LUNA_WINDOW_API u32 count_displays()
        {
            int ret;
            glfwGetMonitors(&ret);
            return ret;
        }
        LUNA_WINDOW_API display_t get_display(u32 index)
        {
            int ret;
            auto displays = glfwGetMonitors(&ret);
            lucheck(index < ret);
            return displays[index];
        }
        Event<display_event_handler_t> g_display_change_event;
        LUNA_WINDOW_API Event<display_event_handler_t>& get_display_event()
        {
            return g_display_change_event;
        }
        static void glfw_display_callback(GLFWmonitor* display, int event)
        {
            MonitorEvent e;
            e.orientation = MonitorOrientation::unknown;
            if (event == GLFW_CONNECTED)
            {
                e.type = MonitorEventType::connected;
            }
            else if (event == GLFW_DISCONNECTED)
            {
                e.type = MonitorEventType::disconnected;
            }
            g_display_change_event(display, e);
        }
        void display_init()
        {
            glfwSetMonitorCallback(glfw_display_callback);
        }
        void display_close()
        {
            g_display_change_event.clear();
        }
        LUNA_WINDOW_API u32 count_display_supported_video_modes(display_t display)
        {
            int count;
            glfwGetVideoModes((GLFWmonitor*)display, &count);
            return count;
        }
        LUNA_WINDOW_API VideoMode get_display_supported_video_mode(display_t display, u32 index)
        {
            int count;
            auto modes = glfwGetVideoModes((GLFWmonitor*)display, &count);
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
        LUNA_WINDOW_API VideoMode get_display_video_mode(display_t display)
        {
            auto src = glfwGetVideoMode((GLFWmonitor*)display);
            VideoMode mode;
            mode.width = (u32)src->width;
            mode.height = (u32)src->height;
            mode.red_bits = (u32)src->redBits;
            mode.green_bits = (u32)src->greenBits;
            mode.blue_bits = (u32)src->blueBits;
            mode.refresh_rate = (u32)src->refreshRate;
            return mode;
        }
        LUNA_WINDOW_API UInt2U get_display_physical_size(display_t display)
        {
            int w, h;
            glfwGetMonitorPhysicalSize((GLFWmonitor*)display, &w, &h);
            return UInt2U((u32)w, (u32)h);
        }
        LUNA_WINDOW_API f32 get_display_dpi_scale_factor(display_t display)
        {
            f32 xscale, yscale;
            glfwGetMonitorContentScale((GLFWmonitor*)display, &xscale, &yscale);
            return xscale;
        }
        LUNA_WINDOW_API Int2U get_display_position(display_t display)
        {
            int x, y;
            glfwGetMonitorPos((GLFWmonitor*)display, &x, &y);
            return Int2U(x, y);
        }
        LUNA_WINDOW_API RectI get_display_working_area(display_t display)
        {
            int x, y, w, h;
            glfwGetMonitorWorkarea((GLFWmonitor*)display, &x, &y, &w, &h);
            return RectI(x, y, w, h);
        }
        LUNA_WINDOW_API Name get_display_name(display_t display)
        {
            return glfwGetMonitorName((GLFWmonitor*)display);
        }
    }
}