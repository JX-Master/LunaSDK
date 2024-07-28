/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widget.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once
#include "../Widget.hpp"
#include <Luna/Runtime/Math/Vector.hpp>
namespace Luna
{
    namespace GUI
    {
        struct IContext;
        
        //! The base class for all built-in widgets.
        struct Widget : virtual IWidget
        {
            lustruct("GUI::Widget", "{b6eb9d49-be6b-4afb-9a53-09449217d00d}");
            luiimpl();

            // The id of the widget. Used to transfer states between widgets. Can be empty.
            widget_id_t id;
            // Parent widget.
            IWidget* parent = nullptr;
            // Attribute values.
            HashMap<u32, f32> sattrs;
            HashMap<u32, Float4U> vattrs;
            HashMap<u32, Name> tattrs;
            HashMap<u32, ObjRef> oattrs;
            // Bounding rect.
            OffsetRectF bounding_rect = OffsetRectF(0, 0, 0, 0);

            virtual ~Widget() {}
            
            virtual widget_id_t get_id() override
            {
                return id;
            }
            virtual void set_id(widget_id_t id) override
            {
                this->id = id;
            }
            virtual IWidget* get_parent() override
            {
                return parent;
            }
            virtual void set_parent(IWidget* widget) override
            {
                parent = widget;
            }
            virtual HashMap<u32, f32>& get_sattrs() override
            {
                return sattrs;
            }
            virtual HashMap<u32, Float4U>& get_vattrs() override
            {
                return vattrs;
            }
            virtual HashMap<u32, Name>& get_tattrs() override
            {
                return tattrs;
            }
            virtual HashMap<u32, ObjRef>& get_oattrs() override
            {
                return oattrs;
            }
            virtual f32 get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y) override
            {
                return get_desired_size_x_attr(this, type);
            }
            virtual f32 get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x) override
            {
                return get_desired_size_y_attr(this, type);
            }
            virtual RV begin_update(IContext* ctx) override
            {
                return ok;
            }
            virtual RV layout(IContext* ctx, const OffsetRectF& layout_rect) override
            {
                bounding_rect = layout_rect;
                return ok;
            }
            virtual OffsetRectF get_bounding_rect() override
            {
                return bounding_rect;
            }
            virtual bool contains_point(f32 x, f32 y) override
            {
                return in_bounds(Float2(x, y), Float2(bounding_rect.left, bounding_rect.top), Float2(bounding_rect.right, bounding_rect.bottom));
            }
            virtual RV handle_event(IContext* ctx, object_t e, bool& handled) override
            {
                return ok;
            }
            virtual RV update(IContext* ctx) override
            {
                return ok;
            }
            virtual RV draw(IContext* ctx, IDrawList* draw_list) override
            {
                return ok;
            }
        };
    }
}