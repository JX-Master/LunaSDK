/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Monitor.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Event.hpp>
#include <Luna/Runtime/Math/Vector.hpp>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
	namespace Window
	{
		//! The opaque handle that represents the system moditor.
		using monitor_t = opaque_t;

		//! Gets the primary monitor of the platform.
		LUNA_WINDOW_API monitor_t get_primary_monitor();

		//! Gets the number of monitors currently connected to the platform.
		LUNA_WINDOW_API u32 count_monitors();

		//! Gets the monitor at specified index.
		LUNA_WINDOW_API monitor_t get_monitor(u32 index);

		enum class MonitorEvent : u32
		{
			connected,
			disconnected,
		};

		using monitor_event_handler_t = void(monitor_t monitor, MonitorEvent e);

		//! Gets the event that will be called when one monitor event is triggered.
		LUNA_WINDOW_API Event<monitor_event_handler_t>& get_monitor_event();

		//! Describes one video mode of one monitor.
		struct VideoMode
		{
			//! The width of the monitor in screen coordinates.
			u32 width;
			//! The height of the monitor in screen coordinates.
			u32 height;
			//! The bit depth of the red channel of the video mode.
			u32 red_bits;
			//! The bit depth of the green channel of the video mode.
			u32 green_bits;
			//! The bit depth of the blue channel of the video mode.
			u32 blue_bits;
			//! The refresh rate, in Hz, of the video mode.
			u32 refresh_rate;
		};

		//! Gets the number of supported video modes of the monitor.
		LUNA_WINDOW_API u32 count_monitor_supported_video_modes(monitor_t monitor);

		//! Gets the supported video mode of the monitor at the specified index.
		LUNA_WINDOW_API VideoMode get_monitor_supported_video_mode(monitor_t monitor, u32 index);

		//! Gets the current video mode of the monitor.
		LUNA_WINDOW_API VideoMode get_monitor_video_mode(monitor_t monitor);

		//! Gets the physical size of the monitor measured in millimetres.
		LUNA_WINDOW_API UInt2U get_monitor_physical_size(monitor_t monitor);

		//! Gets the DPI scale factor of the monitor.
		LUNA_WINDOW_API f32 get_monitor_dpi_scale_factor(monitor_t monitor);

		//! Gets the virtual position of the monitor in screen coordinates.
		LUNA_WINDOW_API Int2U get_monitor_position(monitor_t monitor);

		//! Gets the working area of the monitor, that is, the area not occupied by the system UI
		//! like taskbars or menu bars.
		LUNA_WINDOW_API RectI get_monitor_working_area(monitor_t monitor);

		//! Gets the name of the monitor.
		LUNA_WINDOW_API Name get_monitor_name(monitor_t monitor);
	}
}