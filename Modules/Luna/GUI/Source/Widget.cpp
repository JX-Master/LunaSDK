/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Widget.cpp
* @author JXMaster
* @date 2024/5/8
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
        LUNA_GUI_API f32 Widget::get_sattr(u32 key, bool recursive, f32 default_value, bool* found)
        {
            auto iter = sattrs.find(key);
            if (iter != sattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = parent;
                while(cur)
                {
                    iter = cur->sattrs.find(key);
                    if (iter != cur->sattrs.end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->parent;
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API Float4U Widget::get_vattr(u32 key, bool recursive, const Float4U& default_value, bool* found)
        {
            auto iter = vattrs.find(key);
            if (iter != vattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = parent;
                while(cur)
                {
                    iter = cur->vattrs.find(key);
                    if (iter != cur->vattrs.end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->parent;
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API Name Widget::get_tattr(u32 key, bool recursive, const Name& default_value, bool* found)
        {
            auto iter = tattrs.find(key);
            if (iter != tattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                Widget* cur = parent;
                while(cur)
                {
                    iter = cur->tattrs.find(key);
                    if (iter != cur->tattrs.end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->parent;
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API RV Widget::update(IContext* ctx)
        {
            lutry
            {
                // Calculate bounding rect.
                if(parent)
                {
                    Float4U anthor = get_vattr(VATTR_ANTHOR, false, {0, 0, 1, 1});
                    Float4U offset = get_vattr(VATTR_OFFSET, false, {0, 0, 0, 0});
                    bounding_rect = calc_widget_bounding_rect(parent->bounding_rect, 
                        OffsetRectF{anthor.x, anthor.y, anthor.z, anthor.w}, 
                        OffsetRectF{offset.x, offset.y, offset.z, offset.w});
                }
                else
                {
                    auto& io = ctx->get_io();
                    bounding_rect = OffsetRectF{0, 0, (f32)io.width, (f32)io.height};
                }
                for(auto& c : children)
                {
                    luexp(c->update(ctx));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Widget::render(IContext* ctx, VG::IShapeDrawList* draw_list)
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