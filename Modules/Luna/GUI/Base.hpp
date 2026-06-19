/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Any.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/KeyCode.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/GUICore/Base.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Font/Font.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace VG
    {
        struct IShapeBuffer;
    }

    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! The stable identifier type used by editor-style GUI APIs.
        using id_t = GUICore::id_t;

        //! Aligns text inside the rectangle passed to text rendering helpers.
        enum class TextAlignment : u8
        {
            //! Align to the beginning edge of the axis.
            begin,
            //! Align to the center of the axis.
            center,
            //! Align to the ending edge of the axis.
            end
        };

        //! Bit flags controlling image rendering.
        enum class ImageFlag : u32
        {
            //! Default image rendering.
            none = 0x00,
            //! Flip image sampling vertically.
            flip_y = 0x01,
            //! Use nearest-neighbor texture sampling.
            nearest = 0x02
        };

        //! Identifies the storage representation used by color edit views.
        enum class ColorValueType : u8
        {
            //! Floating point channels in the 0-1 range.
            f32,
            //! 8-bit integer channels in the 0-255 range.
            u8,
            //! Packed RGBA8 value.
            rgba8
        };

        //! Identifies a color channel group.
        enum class ColorChannelPart : u8
        {
            //! No channel group.
            none,
            //! RGB color channels.
            rgb,
            //! HSV color channels.
            hsv
        };

        //! Gets the GUI editor-style immediate package module object.
        //! @return Returns the GUI module object.
        LUNA_GUI_API Module* module_gui();

        //! @}
    }
}
