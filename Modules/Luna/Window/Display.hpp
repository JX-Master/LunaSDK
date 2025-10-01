/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Display.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Event.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{
        
        //! The opaque handle that represents one display screen.
        using display_t = opaque_t;

        //! Gets the primary display of the platform.
        LUNA_WINDOW_API display_t get_primary_display();

        //! Gets the number of displays currently connected to the platform.
        LUNA_WINDOW_API u32 count_displays();

        //! Gets the display at specified index.
        //! @param[in] index The index of the display to get.
        //! @return Returns the display handle that can be used to query for display information.
        //! @par Valid Usage
        //! * `index` must be in range [0, @ref count_displays).
        LUNA_WINDOW_API display_t get_display(u32 index);

        //! Specifies the display orientation.
        enum class DisplayOrientation : u32
        {
            //! The orientation is not specified.
            unknown = 0,
            //! The display is in landscape orientation.
            landscape,
            //! The display is in landscape flipped orientation.
            landscape_flipped,
            //! The display is in portrait orientation.
            portrait,
            //! The display is in portrait flipped orientation.
            portrait_flipped,
        };

        //! Callbacks for display events.
        struct DisplayEvents
        {
            //! Called when the orientation of the display is changed.
            //! @param[in] display The display whose orientation is changed.
            //! @param[in] orientation The new display orientation after change.
            Event<void(display_t display, DisplayOrientation orientation)> orientation;

            //! Called when a new display is connected to the platform.
            //! @param[in] display The display that is connected.
            Event<void(display_t display)> connect;

            //! Called when a display is disconnected from the platform.
            //! @param[in] display The display that is disconnected.
            //! Since the display is disconnected, this handle is actually invalid, and the only thing that can be done 
            //! with this handle is to remove information attached to it in user application.
            Event<void(display_t display)> disconnect;

            //! Called when the position of the display in desktop coordinates 
            //! is changed. This usually happens when the user changes the display settings in the
            //! system settings.
            //! @param[in] display The display whose position is changed.
            Event<void(display_t display)> move;

            void reset()
            {
                orientation.clear();
                connect.clear();
                disconnect.clear();
                move.clear();
            }
        };

        //! Gets the display events set, so that user application can register handlers to 
        //! monitor such events.
        //! @return Returns one reference to the display events set.
        LUNA_WINDOW_API DisplayEvents& get_display_events();

        //! Describes one video mode of one display.
        struct VideoMode
        {
            //! The width of the display in screen coordinates.
            u32 width;
            //! The height of the display in screen coordinates.
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

        //! Gets the supported video modes of the display.
        //! @param[in] display The display to query.
        //! @param[out] out_video_modes Returns the queried video modes.
        //! New elements will be pushed to the end of this vector, and existing elements will not be changed.
        LUNA_WINDOW_API RV get_display_supported_video_modes(display_t display, Vector<VideoMode>& out_video_modes);

        //! Gets the current video mode of the display.
        //! @param[in] display The display to query.
        //! @return Returns the current video mode of the display.
        LUNA_WINDOW_API R<VideoMode> get_display_video_mode(display_t display);

        //! Gets the native resolution of the display.
        //! @param[in] display The display to query.
        //! @return Returns the native resolution (width, height) of the display.
        LUNA_WINDOW_API R<UInt2U> get_display_native_resolution(display_t display);

        //! Gets the virtual position of the display in screen coordinates.
        //! @param[in] display The display to query.
        //! @return Returns the virtual position of the display (x, y) in screen coordinates.
        LUNA_WINDOW_API R<Int2U> get_display_position(display_t display);

        //! Gets the working area of the display, that is, the area not occupied by the system UI
        //! like taskbars or menu bars.
        //! @param[in] display The display to query.
        //! @return Returns the integer rectangle that represents the working area of the display.
        //! The offset of the working area is represented in screen coordinates, so you may
        //! subtract the offset of the working area with the display position to get the offset
        //! relative to the current display.
        LUNA_WINDOW_API R<RectI> get_display_working_area(display_t display);

        //! Gets the name of the display.
        //! @param[in] display The display to query.
        //! @return Returns the name of the display.
        LUNA_WINDOW_API Name get_display_name(display_t display);

        //! @}
    }
}