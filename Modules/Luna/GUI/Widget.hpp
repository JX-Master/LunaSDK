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

namespace Luna
{
    namespace GUI
    {
        using widget_id_t = u32;

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
            LUNA_GUI_API virtual RV draw(IContext* ctx, VG::IShapeDrawList* draw_list);
        };
    }
}