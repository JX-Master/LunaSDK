/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Style.cpp
* @author JXMaster
* @date 2025/6/27
*/
#include "Style.hpp"

namespace Luna
{
    namespace GUI
    {
        Variant Style::get_value(const Name& name, const Variant& default_value, bool recursive, bool* found)
        {
            auto iter = m_values.find(name);
            if (iter != m_values.end())
            {
                if (found) *found = true;
                return iter->second;
            }
            // not found in current node.
            if (recursive && m_parent)
            {
                return m_parent->get_value(name, default_value, recursive, found);
            }
            // not found.
            if (found) *found = false;
            return default_value;
        }

        void Style::set_value(const Name& name, const Variant& value)
        {
            if(value.valid())
            {
                m_values.insert_or_assign(name, value);
            }
            else
            {
                m_values.erase(name);
            }
        }
    }
}