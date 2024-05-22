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

namespace Luna
{
    namespace GUI
    {
        inline bool sattrs_equal(const HashMap<u32, f32>& a, const HashMap<u32, f32>& b)
        {
            if (a.size() != b.size()) return false;
            for(auto& p : a)
            {
                auto iter = b.find(p.first);
                if(iter == b.end()) return false;
                if(p.second != iter->second) return false;
            }
            return true;
        }
        inline bool vattrs_equal(const HashMap<u32, Float4U>& a, const HashMap<u32, Float4U>& b)
        {
            if (a.size() != b.size()) return false;
            for(auto& p : a)
            {
                auto iter = b.find(p.first);
                if(iter == b.end()) return false;
                if(p.second != iter->second) return false;
            }
            return true;
        }
        inline bool tattrs_equal(const HashMap<u32, Name>& a, const HashMap<u32, Name>& b)
        {
            if (a.size() != b.size()) return false;
            for(auto& p : a)
            {
                auto iter = b.find(p.first);
                if(iter == b.end()) return false;
                if(p.second != iter->second) return false;
            }
            return true;
        }
        LUNA_GUI_API bool Widget::equal_to(Widget *rhs)
        {
            return get_object_type(rhs) == get_object_type(this) && (id == rhs->id) &&
                sattrs_equal(sattrs, rhs->sattrs) &&
                vattrs_equal(vattrs, rhs->vattrs) &&
                tattrs_equal(tattrs, rhs->tattrs);
        }
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
    }
}