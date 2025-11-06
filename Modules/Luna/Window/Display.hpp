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

        //! Gets a list of displays of the current platform.
        //! @param[out] out_displays Returns a list of displays of the platform.
        LUNA_WINDOW_API void get_displays(Vector<display_t>& out_displays);

        //! Describes one video mode of one display.
        struct VideoMode
        {
            //! The width of the display in screen coordinates.
            u32 width;
            //! The height of the display in screen coordinates.
            u32 height;
            //! The bit depth of the video mode.
            u32 bits_per_pixel;
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
        LUNA_WINDOW_API R<Name> get_display_name(display_t display);

        //! @}
    }
}