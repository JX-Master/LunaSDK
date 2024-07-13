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
        //! @addtogroup Window
        //! @{
        
        //! The opaque handle that represents the system moditor.
        using monitor_t = opaque_t;

        //! Gets the primary monitor of the platform.
        LUNA_WINDOW_API monitor_t get_primary_monitor();

        //! Gets the number of monitors currently connected to the platform.
        LUNA_WINDOW_API u32 count_monitors();

        //! Gets the monitor at specified index.
        //! @param[in] index The index of the monitor to get.
        //! @return Returns the monitor handle that can be used to query for monitor information.
        //! @par Valid Usage
        //! * `index` must be in range [0, @ref count_monitors).
        LUNA_WINDOW_API monitor_t get_monitor(u32 index);

        //! Specifies the monitor event types.
        enum class MonitorEventType : u32
        {
            //! One new monitor is connected to the platform and is added to the monitor list.
            connected,
            //! One existing monitor is disconnected from the platform and is about to be removed
            //! from the monitor list.
            //! 
            //! If this event is triggered, the `monitor` opaque pointer of the disconnected monitor 
            //! passed along with event is valid only before the event callback returns, and all 
            //! query operations for the disconnected monitor is disabled. The user can only use 
            //! this opaque pointer to free user-allocated resources for the monitor.
            disconnected,
            //! The orientation of the monitor has changed.
            orientation,
            //! The position of the monitor has been changed.
            moved,
        };

        //! Specifies the monitor orientation.
        enum class MonitorOrientation : u32
        {
            //! The orientation is not specified.
            unknown = 0,
            //! The monitor is in landscape orientation.
            landscape,
            //! The monitor is in landscape flipped orientation.
            landscape_flipped,
            //! The monitor is in portrait orientation.
            portrait,
            //! The monitor is in portrait flipped orientation.
            portrait_flipped,
        };

        struct MonitorEvent
        {
            //! The type of the monitor event.
            MonitorEventType type;
            //! The new orientation of the monitor if `type` is @ref MonitorEventType::orientation,
            //! otherwise @ref MonitorOrientation::unknown always.
            MonitorOrientation orientation;
        };
        
        //! The callback provided by the user to handle monitor events.
        //! @param[in] monitor The monitor that triggers the event.
        //! @param[in] e The triggered event.
        using monitor_event_handler_t = void(monitor_t monitor, const MonitorEvent& e);

        //! Gets the event that will be called when one monitor event is triggered.
        //! @details This function is not thread-safe and should only be called from the main thread.
        //! @return Returns the event that will be called when one monitor event is triggered.
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
        //! @param[in] monitor The monitor to query.
        //! @return Returns the number of supported video modes of the monitor.
        LUNA_WINDOW_API u32 count_monitor_supported_video_modes(monitor_t monitor);

        //! Gets the supported video mode of the monitor at the specified index.
        //! @param[in] monitor The monitor to query.
        //! @param[in] index The index of the video mode to query.
        //! @return Returns the queried video mode of the monitor.
        //! @par Valid Usage
        //! * `index` must be in range [0, `count_monitor_supported_video_modes(monitor)`).
        LUNA_WINDOW_API VideoMode get_monitor_supported_video_mode(monitor_t monitor, u32 index);

        //! Gets the current video mode of the monitor.
        //! @param[in] monitor The monitor to query.
        //! @return Returns the current video mode of the monitor.
        LUNA_WINDOW_API VideoMode get_monitor_video_mode(monitor_t monitor);

        //! Gets the native resolution of the monitor.
        //! @param[in] monitor The monitor to query.
        //! @return Returns the native resolution (width, height) of the monitor.
        LUNA_WINDOW_API UInt2U get_monitor_native_resolution(monitor_t monitor);

        //! Gets the virtual position of the monitor in screen coordinates.
        //! @param[in] monitor The monitor to query.
        //! @return Returns the virtual position of the monitor (x, y) in screen coordinates.
        LUNA_WINDOW_API Int2U get_monitor_position(monitor_t monitor);

        //! Gets the working area of the monitor, that is, the area not occupied by the system UI
        //! like taskbars or menu bars.
        //! @param[in] monitor The monitor to query.
        //! @return Returns the integer rectangle that represents the working area of the monitor.
        //! The offset of the working area is represented in screen coordinates, so you may
        //! subtract the offset of the working area with the monitor position to get the offset
        //! relative to the current monitor.
        LUNA_WINDOW_API RectI get_monitor_working_area(monitor_t monitor);

        //! Gets the name of the monitor.
        //! @param[in] monitor The monitor to query.
        //! @return Returns the name of the monitor.
        LUNA_WINDOW_API Name get_monitor_name(monitor_t monitor);

        //! @}
    }
}