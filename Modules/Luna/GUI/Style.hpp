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
        enum class StyleValueType : u8
        {
            f32_1,
            f32_2,
            f32_3,
            f32_4
        };

        enum class StyleEntryState : u8
        {
            inherit,
            set,
            unset
        };

        struct StyleValue
        {
            StyleValueType type = StyleValueType::f32_4;
            Float4U value = Float4U(0.0f);

            static StyleValue f32_1(f32 x)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_1;
                ret.value = Float4U(x, 0.0f, 0.0f, 0.0f);
                return ret;
            }

            static StyleValue f32_2(const Float2U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_2;
                ret.value = Float4U(v.x, v.y, 0.0f, 0.0f);
                return ret;
            }

            static StyleValue f32_3(const Float3U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_3;
                ret.value = Float4U(v.x, v.y, v.z, 0.0f);
                return ret;
            }

            static StyleValue f32_4(const Float4U& v)
            {
                StyleValue ret;
                ret.type = StyleValueType::f32_4;
                ret.value = v;
                return ret;
            }
        };

        struct StyleEntry
        {
            StyleEntryState state = StyleEntryState::inherit;
            StyleValue value;
        };

        struct Style
        {
            Name name;
            Name parent;
            HashMap<Name, StyleEntry> entries;
        };
    }
}
