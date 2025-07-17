/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.cpp
* @author JXMaster
* @date 2024/3/30
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Widgets.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void set_canvas_anthor(IWidgetBuilder* builder, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(builder, VATTR_ANTHOR, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void set_canvas_offset(IWidgetBuilder* builder, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(builder, VATTR_OFFSET, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void set_sattr(IWidgetBuilder* builder, u32 kind, f32 value)
        {
            Widget* widget = builder->get_current_widget();
            widget->get_sattrs().insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_vattr(IWidgetBuilder* builder, u32 kind, const Float4U& value)
        {
            Widget* widget = builder->get_current_widget();
            widget->get_vattrs().insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_tattr(IWidgetBuilder* builder, u32 kind, const Name& value)
        {
            Widget* widget = builder->get_current_widget();
            widget->get_tattrs().insert_or_assign(kind, value);
        }
        LUNA_GUI_API f32 get_desired_size_x_attr(Widget* widget, DesiredSizeType type, bool* found)
        {
            u32 attr = 0;
            switch(type)
            {
                case DesiredSizeType::required: attr = SATTR_REQUIRED_SIZE_X; break;
                case DesiredSizeType::preferred: attr = SATTR_PREFERRED_SIZE_X; break;
                case DesiredSizeType::filling: attr = SATTR_FILLING_SIZE_X; break;
            }
            return widget->get_sattr(attr, false, 0.0f, found);
        }
        LUNA_GUI_API f32 get_desired_size_y_attr(Widget* widget, DesiredSizeType type, bool* found)
        {
            u32 attr = 0;
            switch(type)
            {
                case DesiredSizeType::required: attr = SATTR_REQUIRED_SIZE_Y; break;
                case DesiredSizeType::preferred: attr = SATTR_PREFERRED_SIZE_Y; break;
                case DesiredSizeType::filling: attr = SATTR_FILLING_SIZE_Y; break;
            }
            return widget->get_sattr(attr, false, 0.0f, found);
        }
    }
}