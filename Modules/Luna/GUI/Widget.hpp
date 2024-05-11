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
namespace Luna
{
    namespace GUI
    {
        struct WidgetBuildData;
        struct IContext;
        struct Widget
        {
            lustruct("GUI::Widget", "{b6eb9d49-be6b-4afb-9a53-09449217d00d}");

            // The id of the widget. Used to transfer states between widgets. Can be empty.
            Name id;
            // Parent widget.
            Widget* parent = nullptr;
            // Child widgets.
            Vector<Ref<Widget>> children;
            // Attribute values.
            HashMap<u32, f32> sattrs;
            HashMap<u32, Float4U> vattrs;

            virtual ~Widget() {}
            virtual Ref<WidgetBuildData> new_build_data() = 0;
            LUNA_GUI_API virtual bool equal_to(Widget* rhs);
            LUNA_GUI_API f32 get_sattr(u32 key, f32 default_value = 0, bool* found = nullptr);
            LUNA_GUI_API Float4U get_vattr(u32 key, const Float4U& default_value = Float4U(0), bool* found = nullptr);
        };

        struct WidgetBuildData
        {
            lustruct("GUI::WidgetBuildData", "{ff4f1ef1-54c5-4a99-adc5-5b41efcd171a}");

            //! Pointer to the parent build data if any.
            WidgetBuildData* parent = nullptr;
            //! Pointer to the widget that builds this build data.
            Ref<Widget> widget;
            //! Child build data objects.
            Vector<Ref<WidgetBuildData>> children;

            // The offset to place this widget in screen coordinates.
            OffsetRectF bounding_rect;

            // The state object attached with this widget. Can be null.
            ObjRef state;

            // Whether this widget should be rebuilt.
            bool dirty = true;

            //! Called every frame to check whether this widget should be rebuilt.
            //! The derived widget may set `dirty` to `true` in this function to let `build` to be called
            //! in this frame.
            LUNA_GUI_API virtual void update(IContext* ctx);

            //! Called when the widget data should be rebuilt (`dirty` is `true`).
            LUNA_GUI_API virtual RV build(IContext* ctx);

            //! Called when the widget data is rendered.
            LUNA_GUI_API virtual RV render(IContext* ctx, VG::IShapeDrawList* draw_list);
        };

        struct Rectangle : Widget
        {
            lustruct("RHI::Rectangle", "{d0e27859-1439-4089-bf55-b8fe82b24c47}");

            LUNA_GUI_API virtual Ref<WidgetBuildData> new_build_data() override;
        };

        struct Text : Widget
        {
            lustruct("RHI::Text", "{06fa0393-d4f9-4dce-b340-d9790b36c732}");

            Name text;

            LUNA_GUI_API virtual Ref<WidgetBuildData> new_build_data() override;
            LUNA_GUI_API virtual bool equal_to(Widget* rhs) override;
        };

        struct ResizableWindow : Widget
        {
            lustruct("RHI::ResizableWindow", "{22637fc6-330d-46e9-ad80-16d2ed7ec328}");

            LUNA_GUI_API virtual Ref<WidgetBuildData> new_build_data() override;
        };
    }
}