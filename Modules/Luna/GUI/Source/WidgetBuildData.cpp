/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widgets.hpp
* @author JXMaster
* @date 2024/5/7
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Widget.hpp"
#include "../Context.hpp"
#include "../Widgets.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API void WidgetBuildData::update(IContext* ctx)
        {
            for(auto& c : children)
            {
                c->update(ctx);
            }
        }
        LUNA_GUI_API RV WidgetBuildData::build(IContext* ctx)
        {
            lutry
            {
                // Calculate bounding rect.
                if(parent)
                {
                    Float4U anthor = widget->get_vattr(VATTR_ANTHOR, false, {0, 0, 1, 1});
                    Float4U offset = widget->get_vattr(VATTR_OFFSET, false, {0, 0, 0, 0});
                    bounding_rect = calc_widget_bounding_rect(parent->bounding_rect, 
                        OffsetRectF{anthor.x, anthor.y, anthor.z, anthor.w}, 
                        OffsetRectF{offset.x, offset.y, offset.z, offset.w});
                }
                else
                {
                    auto& io = ctx->get_io();
                    bounding_rect = OffsetRectF{0, 0, (f32)io.width, (f32)io.height};
                }
                // Build child widgets.
                for(usize i = 0; i < children.size(); ++i)
                {
                    luexp(children[i]->build(ctx));
                }
                dirty = false;
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV WidgetBuildData::render(IContext* ctx, VG::IShapeDrawList* draw_list)
        {
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->render(ctx, draw_list));
                }
            }
            lucatchret;
            return ok;
        }
    }
}