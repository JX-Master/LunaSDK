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
        LUNA_GUI_API void set_anthor(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(ctx, VATTR_ANTHOR, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void set_offset(IContext* ctx, f32 left, f32 top, f32 right, f32 bottom)
        {
            set_vattr(ctx, VATTR_OFFSET, Float4U(left, top, right, bottom));
        }
        LUNA_GUI_API void set_sattr(IContext* ctx, u32 kind, f32 value)
        {
            Widget* widget = ctx->get_current_widget();
            widget->sattrs.insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_vattr(IContext* ctx, u32 kind, const Float4U& value)
        {
            Widget* widget = ctx->get_current_widget();
            widget->vattrs.insert_or_assign(kind, value);
        }
        LUNA_GUI_API void set_tattr(IContext* ctx, u32 kind, const Name& value)
        {
            Widget* widget = ctx->get_current_widget();
            widget->tattrs.insert_or_assign(kind, value);
        }
    }
}