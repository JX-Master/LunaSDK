/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGui.hpp
* @author JXMaster
* @date 2024/7/13
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/FontAtlas.hpp>

namespace Luna
{
    namespace GUI
    {
        //! @interface IImGuiContext
        //! A stateless stack-based GUI context.
        struct IImGuiContext : virtual Interface
        {
            luiid("a7772f5e-4351-4f86-a4fb-4b2568674136");

            //! Resets the context.
            //! @param[in] draw_list The draw list to record draw commands.
            virtual void reset(VG::IShapeDrawList* draw_list) = 0;

            //! Gets the size of the viewport.
            virtual Float2U get_viewport_size() = 0;

            //! Sets the size of the viewport.
            virtual void set_viewport_size(f32 width, f32 height) = 0;

            //! Sets the position of the next widget in viewport space.
            virtual void set_next_widget_pos(f32 x, f32 y) = 0;

            //! Sets the width limitation of the next widget.
            virtual void set_next_widget_width(f32 width) = 0;

            //! Removes the width limitation of the next widget.
            virtual void reset_next_width_width() = 0;

            //! Sets the height limitation of the next widget.
            virtual void set_next_widget_height(f32 height) = 0;

            //! Removes the height limitation of the next widget.
            virtual void reset_next_widget_height() = 0;

            
        };
    }
}