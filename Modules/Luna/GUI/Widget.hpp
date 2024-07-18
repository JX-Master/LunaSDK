/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widget.hpp
* @author JXMaster
* @date 2024/5/6
*/
#pragma once
#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include "Attributes.hpp"
#include "DrawList.hpp"

namespace Luna
{
    namespace GUI
    {
        using widget_id_t = u32;

        enum class DesiredSizeType : u8
        {
            minimum,
            preferred,
            maximum,
        };

        struct IContext;

        //! @interface IWidget
        //! The base interface for all objects that can be attached to GUI as widgets.
        struct IWidget : virtual Interface
        {
            luiid("efe053d8-485d-48a8-80de-a6e841ecd8c5");

            //! Gets the widget ID of the widget.
            //! @details The widget ID is used to identify widgets between two updates, so that state objects can be attached to the widget correctly.
            //! This can be empty, which identifies one stateless widget that will be rebuilt in every update.
            virtual widget_id_t get_id() = 0;

            //! Sets the widget ID of the widget.
            virtual void set_id(widget_id_t id) = 0;

            //! Gets the parent widget of this widget.
            virtual IWidget* get_parent() = 0;

            //! Sets the parent widget of this widget.
            //! @remark The implementation must NOT keep a strong reference to the parent widget in order to prevent reference cycling.
            //! 
            //! This function should only be called by parent widget when setting child widgets. The end user should not call this function directly.
            virtual void set_parent(IWidget* widget) = 0;

            virtual HashMap<u32, f32>& get_sattrs() = 0;

            virtual HashMap<u32, Float4U>& get_vattrs() = 0;

            virtual HashMap<u32, Name>& get_tattrs() = 0;

            virtual HashMap<u32, ObjRef>& get_oattrs() = 0;

            //! Calculates the desired size of the specified widget.
            //virtual Float2U get_desired_size(DesiredSizeType type, const Float2U& suggested_size) = 0;

            //! Called after the widget tree is built and before the widget is rendered. The widget should handle user input and generate render data 
            //! in this call.
            virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) = 0;

            //! Called when the widget is rendered.
            virtual RV draw(IContext* ctx, IDrawList* draw_list) = 0;
        };
    }
}