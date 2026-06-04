/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Style.hpp
* @author JXMaster
* @date 2026/6/3
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Identifies the number of f32 components stored in a style value.
        enum class StyleValueType : u8
        {
            //! One f32 component.
            f32_1,
            //! Two f32 components.
            f32_2,
            //! Three f32 components.
            f32_3,
            //! Four f32 components.
            f32_4,
            //! One runtime name value.
            name
        };

        //! Describes how one style entry participates in inheritance.
        enum class StyleEntryState : u8
        {
            //! This entry is inherited from the parent style.
            inherit,
            //! This entry is set locally by this style.
            set,
            //! This entry hides an inherited parent entry.
            unset
        };

        //! Stores one style value as up to four f32 components.
        struct StyleValue
        {
            //! The active component count.
            StyleValueType type = StyleValueType::f32_4;
            //! The stored components.
            Float4U value = Float4U(0.0f);
            //! The stored name value when @ref type is @ref StyleValueType::name.
            Name name_value;

            //! Creates a scalar style value.
            //! @param[in] x The scalar component.
            //! @return Returns the created style value.
            static StyleValue f32_1(f32 x)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_1;
                ret.value = Float4U(x, 0.0f, 0.0f, 0.0f);
                return ret;
            }

            //! Creates a two-component style value.
            //! @param[in] v The two components.
            //! @return Returns the created style value.
            static StyleValue f32_2(const Float2U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_2;
                ret.value = Float4U(v.x, v.y, 0.0f, 0.0f);
                return ret;
            }

            //! Creates a three-component style value.
            //! @param[in] v The three components.
            //! @return Returns the created style value.
            static StyleValue f32_3(const Float3U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_3;
                ret.value = Float4U(v.x, v.y, v.z, 0.0f);
                return ret;
            }

            //! Creates a four-component style value.
            //! @param[in] v The four components.
            //! @return Returns the created style value.
            static StyleValue f32_4(const Float4U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_4;
                ret.value = v;
                return ret;
            }

            //! Creates a name style value.
            //! @param[in] v The name value.
            //! @return Returns the created style value.
            static StyleValue name(const Name& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::name;
                ret.name_value = v;
                return ret;
            }
        };

        //! One entry stored by a style.
        struct StyleEntry
        {
            //! The inheritance state of this entry.
            StyleEntryState state = StyleEntryState::inherit;
            //! The value used when @ref state is @ref StyleEntryState::set.
            StyleValue value;
        };

        //! A named collection of style entries with an optional parent style.
        struct Style
        {
            //! Style name.
            Name name;
            //! Optional parent style name.
            Name parent;
            //! Local entry table indexed by entry name.
            HashMap<Name, StyleEntry> entries;
        };

        //! @}
    }
}
