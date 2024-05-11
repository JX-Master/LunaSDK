/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ResizableWindow.cpp
* @author JXMaster
* @date 2024/5/10
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "ResizableWindow.hpp"
#include "../../Context.hpp"

namespace Luna
{
    namespace GUI
    {
        void ResizableWindowBuildData::update(IContext* ctx)
        {
            auto& io = ctx->get_io();
            if(io.width != ctx_width || io.height != ctx_height)
            {
                dirty = true;
            }
        }
        RV ResizableWindowBuildData::build(IContext* ctx)
        {
            auto& io = ctx->get_io();
            ctx_width = io.width;
            ctx_height = io.height;
            bounding_rect = OffsetRectF(0.0f, 0.0f, (f32)ctx_width, (f32)ctx_height);
            lutry
            {
                for(auto& c : children)
                {
                    luexp(c->build(ctx));
                }
            }
            lucatchret;
            dirty = false;
            return ok;
        }
        Ref<WidgetBuildData> ResizableWindow::new_build_data()
        {
            return new_object<ResizableWindowBuildData>();
        }
    }
}