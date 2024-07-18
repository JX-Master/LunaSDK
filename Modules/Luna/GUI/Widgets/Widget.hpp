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
            virtual RV update(IContext* ctx, const OffsetRectF& layout_rect) override
            {
                return ok;
            }
            virtual RV draw(IContext* ctx, IDrawList* draw_list) override
            {
                return ok;
            }
        };

        LUNA_GUI_API f32 get_sattr(IWidget* widget, u32 key, bool recursive = false, f32 default_value = 0, bool* found = nullptr);
        LUNA_GUI_API Float4U get_vattr(IWidget* widget, u32 key, bool recursive = false, const Float4U& default_value = Float4U(0), bool* found = nullptr);
        LUNA_GUI_API Name get_tattr(IWidget* widget, u32 key, bool recursive = false, const Name& default_value = Name(), bool* found = nullptr);
        LUNA_GUI_API object_t get_oattr(IWidget* widget, u32 key, bool recursive = false, object_t default_value = nullptr, bool* found = nullptr);
    }
}