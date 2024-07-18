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
#include "../Widgets/Widget.hpp"
#include "../Context.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 get_sattr(IWidget* widget, u32 key, bool recursive, f32 default_value, bool* found)
        {
            auto& sattrs = widget->get_sattrs();
            auto iter = sattrs.find(key);
            if (iter != sattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                IWidget* cur = widget->get_parent();
                while(cur)
                {
                    iter = cur->get_sattrs().find(key);
                    if (iter != cur->get_sattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API Float4U get_vattr(IWidget* widget, u32 key, bool recursive, const Float4U& default_value, bool* found)
        {
            auto& vattrs = widget->get_vattrs();
            auto iter = vattrs.find(key);
            if (iter != vattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                IWidget* cur = widget->get_parent();
                while(cur)
                {
                    iter = cur->get_vattrs().find(key);
                    if (iter != cur->get_vattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API Name get_tattr(IWidget* widget, u32 key, bool recursive, const Name& default_value, bool* found)
        {
            auto& tattrs = widget->get_tattrs();
            auto iter = tattrs.find(key);
            if (iter != tattrs.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive)
            {
                IWidget* cur = widget->get_parent();
                while(cur)
                {
                    iter = cur->get_tattrs().find(key);
                    if (iter != cur->get_tattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second;
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
        LUNA_GUI_API object_t get_oattr(IWidget* widget, u32 key, bool recursive, object_t default_value, bool* found)
        {
            auto& oattrs = widget->get_oattrs();
            auto iter = oattrs.find(key);
            if (iter != oattrs.end())
            {
                if (found) *found = true;
                return iter->second.get();
            }
            // not found in current node.
            if (recursive)
            {
                IWidget* cur = widget->get_parent();
                while(cur)
                {
                    iter = cur->get_oattrs().find(key);
                    if (iter != cur->get_oattrs().end())
                    {
                        // found.
                        if (found) *found = true;
                        return iter->second.get();
                    }
                    cur = cur->get_parent();
                }
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }
    }
}