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
        LUNA_GUI_API bool Widget::equal_to(Widget *rhs)
        {
            return get_object_type(rhs) == get_object_type(this) && (id == rhs->id) &&
                sattrs_equal(sattrs, rhs->sattrs) &&
                vattrs_equal(vattrs, rhs->vattrs);
        }
        LUNA_GUI_API f32 Widget::get_sattr(u32 key, f32 default_value, bool* found)
        {
            auto iter = sattrs.find(key);
            if (found) *found = (iter != sattrs.end());
            return iter == sattrs.end() ? default_value : iter->second;
        }
        LUNA_GUI_API Float4U Widget::get_vattr(u32 key, const Float4U& default_value, bool* found)
        {
            auto iter = vattrs.find(key);
            if (found) *found = (iter != vattrs.end());
            return iter == vattrs.end() ? default_value : iter->second;
        }
    }
}