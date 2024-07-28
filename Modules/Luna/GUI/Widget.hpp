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
            //! The required size is the size that must be satisfied in otder to correctly display 
            //! this widget. The system will always allocate required size for every widget, even if
            //! child widgets will overflow the bounding rectangle of the parent widget.
            //! 
            //! The required size is specified as absolute units. If this widget does not have a required size, set this `0`.
            required,
            //! The preferred size is the size that is preferred by the widget. After required sizes of
            //! all child widgets are allocated, the system allocates rest space to each widget so that
            //! every widget will be closer to its preferred size in equal ratio.
            //! 
            //! The preferred size is specified as absolute units. The preferred size must be greater than or equal to
            //! the required size, if the preferred size is smaller than the required size, the layout widget
            //! must clamp the preferred size using `preferred_size = max(preferred_size, required_size)` internally.
            //! If this widget does not have a preferred size, set this to `0` and the system will clamp the returned value.
            preferred,
            //! The filling size is used to fill the parent space if there is still space available after allocating 
            //! required sizes and preferred sizes for widgets. The filling size is specified in relative weights instead of
            //! absolute units. When calculating spaces for every widget, the layout will firstly sum up weights of all child
            //! widgets, divide total free space by total weights to compute space per weight unit, then allocate space for 
            //! every widget by multiplying its weights with space per weight unit.
            //! 
            //! If this widget does not have a filling size, set this to `0` and the widget will not get any filling space allocated.
            filling,
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

            virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) = 0;

            virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) = 0;

            //! Called firstly when a new frame update is performed.
            virtual RV begin_update(IContext* ctx) = 0;

            //! Called to update the layout of the widget.
            virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) = 0;

            //! Gets the bounding rectangle of this widget.
            //! @return Returns the bounding rectangle of this widget.
            //! @par Valid Usage
            //! * This function must be called after @ref layout, since the bounding rectangle is calculated based on the 
            //! layout rectangle.
            virtual OffsetRectF get_bounding_rect() = 0;

            //! Tests whether the specified point is in the widget boundary.
            //! @param[in] x The X position of the point in screen coordinates.
            //! @param[in] y The Y position of the point in screen coordinates.
            //! @return Returns `true` if the specified point is in the widget boundary, returns `false` otherwise.
            //! @remark This function is used to determine whether on position-based event should be handled by this widget.
            virtual bool contains_point(f32 x, f32 y) = 0;

            //! Called when one event is sent to this widget.
            //! @param[in] ctx The GUI context.
            //! @param[in] e The event to be handled.
            //! @param[in] handled Set to `true` will prevent event from broadcasting to the following widgets.
            //! This is set to `false` when passed in.
            virtual RV handle_event(IContext* ctx, object_t e, bool& handled) = 0;

            //! Called after the widget tree is built and before the widget is rendered. The widget should handle user input and generate render data 
            //! in this call.
            virtual RV update(IContext* ctx) = 0;

            //! Called when the widget is rendered.
            virtual RV draw(IContext* ctx, IDrawList* draw_list) = 0;
        };

        LUNA_GUI_API f32 get_sattr(IWidget* widget, u32 key, bool recursive = false, f32 default_value = 0, bool* found = nullptr);
        LUNA_GUI_API Float4U get_vattr(IWidget* widget, u32 key, bool recursive = false, const Float4U& default_value = Float4U(0), bool* found = nullptr);
        LUNA_GUI_API Name get_tattr(IWidget* widget, u32 key, bool recursive = false, const Name& default_value = Name(), bool* found = nullptr);
        LUNA_GUI_API object_t get_oattr(IWidget* widget, u32 key, bool recursive = false, object_t default_value = nullptr, bool* found = nullptr);
        LUNA_GUI_API f32 get_desired_size_x_attr(IWidget* widget, DesiredSizeType type, bool* found = nullptr);
        LUNA_GUI_API f32 get_desired_size_y_attr(IWidget* widget, DesiredSizeType type, bool* found = nullptr);
    }
}