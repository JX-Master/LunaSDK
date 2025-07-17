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

        //! The base class for all widgets.
        struct LUNA_GUI_API Widget
        {
            lustruct("GUI::Widget", "{b6eb9d49-be6b-4afb-9a53-09449217d00d}");

            //! Gets the widget ID of the widget.
            //! @details The widget ID is used to identify widgets between two updates, so that state objects can be attached to the widget correctly.
            //! This can be empty, which identifies one stateless widget that will be rebuilt in every update.
            virtual widget_id_t get_id();

            //! Sets the widget ID of the widget.
            virtual void set_id(widget_id_t id);

            //! Gets the parent widget of this widget.
            virtual Widget* get_parent();

            //! Adds one child widget to this widget.
            //! @param[in] child The child to add.
            //! @param[in] pos The position of the new child to insert.
            //! If this is 0 or greater, child will be inserted at `pos`.
            //! If this is smaller than 0, child will be inserted at `get_num_children() + 1 - pos`.
            //! @par Valid Usage.
            //! * `child` must not be `nullptr`.
            //! * `pos` must specify a valid position.
            virtual void add_child(Widget* child, isize pos = -1);

            //! Removes the child widget at specified widget.
            //! @param[in] index The index of the child to remove.
            //! @return Returns the removed widget.
            virtual Ref<Widget> remove_child(usize index);

            //! Gets the child at the specified index.
            //! @param[in] index The index of the child to get.
            //! @return Returns the child.
            //! @par Valid Usage
            //! * `index` must specify a valid position (between `[0, get_num_children())`).
            virtual Widget* get_child(usize index);

            //! Sets the child at the specified index.
            //! @param[in] index The index of the child to get.
            //! @return Returns the old child before the new child is set.
            //! @par Valid Usage
            //! * `index` must specify a valid position (between `[0, get_num_children())`).
            virtual Ref<Widget> set_child(usize index, Widget* new_widget);

            //! Gets a list of child widgets of this widget.
            //! @param[out] out_children The array to write child widgets to.
            //! Child widgets will be pushed to the end of this array, existing elements in the 
            //! array are not changed.
            virtual Span<const Ref<Widget>> get_children();

            //! Sets a list of child widgets to replace exisiting one.
            //! @param[in] widgets An array of widgets to set.
            //! @param[in] num_widgets The number of widgets in the array.
            //! @par Valid Usage
            //! * If `num_widgets` is not 0, `widgets` must not be `nullptr`, and every element of `widgets` must not be `nullptr`.
            virtual void set_children(Widget** widgets, usize num_widgets);

            virtual HashMap<u32, f32>& get_sattrs();

            virtual HashMap<u32, Float4U>& get_vattrs();

            virtual HashMap<u32, Name>& get_tattrs();

            virtual HashMap<u32, ObjRef>& get_oattrs();

            virtual f32 get_sattr(u32 key, bool recursive = false, f32 default_value = 0, bool* found = nullptr);

            virtual Float4U get_vattr(u32 key, bool recursive = false, const Float4U& default_value = Float4U(0, 0, 0, 0), bool* found = nullptr);

            virtual Name get_tattr(u32 key, bool recursive = false, const Name& default_value = Name(), bool* found = nullptr);

            virtual object_t get_oattr(u32 key, bool recursive = false, object_t default_value = nullptr, bool* found = nullptr);
            
            virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y);

            virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x);

            //! Called firstly when a new frame update is performed.
            virtual RV begin_update(IContext* ctx);

            //! Called to update the layout of the widget.
            virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect);

            //! Gets the bounding rectangle of this widget.
            //! @return Returns the bounding rectangle of this widget.
            //! @par Valid Usage
            //! * This function must be called after @ref layout, since the bounding rectangle is calculated based on the 
            //! layout rectangle.
            virtual OffsetRectF get_bounding_rect();

            //! Tests whether the specified point is in the widget boundary.
            //! @param[in] x The X position of the point in screen coordinates.
            //! @param[in] y The Y position of the point in screen coordinates.
            //! @return Returns `true` if the specified point is in the widget boundary, returns `false` otherwise.
            //! @remark This function is used to determine whether on position-based event should be handled by this widget.
            virtual bool contains_point(f32 x, f32 y);

            //! Called when one event is sent to this widget.
            //! @param[in] ctx The GUI context.
            //! @param[in] e The event to be handled.
            //! @param[in] handled Set to `true` will prevent event from broadcasting to the following widgets.
            //! This is set to `false` when passed in.
            virtual RV handle_event(IContext* ctx, object_t e, bool& handled);

            //! Called after the widget tree is built and before the widget is rendered. The widget should handle user input and generate render data 
            //! in this call.
            virtual RV update(IContext* ctx);

            //! Called when the widget is rendered.
            virtual RV draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list);

        protected:
            //! Sets the bounding rectangle of this widget.
            //! @param[in] bounding_rect The bounding rectangle to set.
            virtual void set_bounding_rect(const OffsetRectF& bounding_rect);

        private:
            // The id of the widget. Used to transfer states between widgets. Can be empty.
            widget_id_t m_id;
            // Parent widget.
            Widget* m_parent = nullptr;
            // Attribute values.
            HashMap<u32, f32> m_sattrs;
            HashMap<u32, Float4U> m_vattrs;
            HashMap<u32, Name> m_tattrs;
            HashMap<u32, ObjRef> m_oattrs;
            //! The children of this widget.
            Vector<Ref<Widget>> m_children;

            // Bounding rect.
            OffsetRectF m_bounding_rect = OffsetRectF(0, 0, 0, 0);
            
        };

        LUNA_GUI_API f32 get_desired_size_x_attr(Widget* widget, DesiredSizeType type, bool* found = nullptr);
        LUNA_GUI_API f32 get_desired_size_y_attr(Widget* widget, DesiredSizeType type, bool* found = nullptr);
    }
}