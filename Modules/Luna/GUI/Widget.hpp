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
        
        //! @interface IAnchor
        //! Created by the parent widget to specify the per-element placement information.
        struct IAnchor : virtual Interface
        {
            
        };

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

            //! Gets the scalar attribute of the specified widget.
            virtual f32 get_sattr(u32 key, bool recursive = true, f32 default_value = 0, bool* found = nullptr) = 0;

            //! Sets the scalar attribute of the specified widget.
            virtual void set_sattr(u32 key, f32 value) = 0;

            //! Removes the scalar attribute of the specified widget.
            virtual void remove_sattr(u32 key) = 0;

            //! Gets vector attributes of the specified widget.
            virtual Float4U get_vattr(u32 key, bool recursive = true, const Float4U& default_value = Float4U(0), bool* found = nullptr) = 0;

            virtual void set_vattr(u32 key, const Float4U& value) = 0;

            virtual void remove_vattr(u32 key) = 0;

            //! Gets text attributes of the specified widget.
            virtual Name get_tattr(u32 key, bool recursive = true, const Name& default_value = Name(), bool* found = nullptr) = 0;

            virtual void set_tattr(u32 key, const Name& value) = 0;

            virtual void remove_tattr(u32 key) = 0;

            //! Gets object attributes of the specified widget.
            virtual object_t get_oattr(u32 key, bool recursive = true, object_t default_value = nullptr, bool* found = nullptr) = 0;

            virtual void set_oattr(u32 key, object_t value) = 0;

            virtual void remove_oattr(u32 key) = 0;

            //! Calculates the desired size of the specified widget.
            virtual Float2U get_desired_size(DesiredSizeType type, const Float2U& suggested_size) = 0;
        };

        struct IContext;
        struct Widget
        {
            lustruct("GUI::Widget", "{b6eb9d49-be6b-4afb-9a53-09449217d00d}");

            // The id of the widget. Used to transfer states between widgets. Can be empty.
            widget_id_t id;
            // Parent widget.
            Widget* parent = nullptr;
            // Child widgets.
            Vector<Ref<Widget>> children;
            // Attribute values.
            HashMap<u32, f32> sattrs;
            HashMap<u32, Float4U> vattrs;
            HashMap<u32, Name> tattrs;
            HashMap<u32, ObjRef> oattrs;

            // The following properties are widget computed data that will get updates every frame based on widget states.

            // The offset to place this widget in screen coordinates.
            OffsetRectF bounding_rect;

            virtual ~Widget() {}
            LUNA_GUI_API f32 get_sattr(u32 key, bool recursive = false, f32 default_value = 0, bool* found = nullptr);
            LUNA_GUI_API Float4U get_vattr(u32 key, bool recursive = false, const Float4U& default_value = Float4U(0), bool* found = nullptr);
            LUNA_GUI_API Name get_tattr(u32 key, bool recursive = false, const Name& default_value = Name(), bool* found = nullptr);
            LUNA_GUI_API object_t get_oattr(u32 key, bool recursive = false, object_t default_value = nullptr, bool* found = nullptr);

            //! Called after the widget tree is built and before the widget is rendered. The widget should handle user input and generate render data 
            //! in this call.
            LUNA_GUI_API virtual RV update(IContext* ctx, const OffsetRectF& layout_rect);

            //! Called when the widget is rendered.
            LUNA_GUI_API virtual RV draw(IContext* ctx, IDrawList* draw_list);
        };
    }
}