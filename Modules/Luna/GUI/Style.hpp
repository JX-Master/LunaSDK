/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Style.hpp
* @author JXMaster
* @date 2025/6/23
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Ref.hpp>

namespace Luna
{
    namespace GUI
    {
        struct IStyle : virtual Interface
        {
            luiid("e8400e06-57a2-4d79-a59e-200ae537bac6");

            virtual IStyle* get_parent() = 0;
            
            virtual void set_parent(IStyle* parent) = 0;

            virtual Variant get_value(const Name& name, const Variant& default_value = Variant(), bool recursive = false, bool* found = nullptr) = 0;

            virtual void set_value(const Name& name, const Variant& value) = 0;
        };
    }
}