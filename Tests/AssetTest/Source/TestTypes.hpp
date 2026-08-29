/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TestTypes.hpp
* @author JXMaster
* @date 2026/8/29
*/
#pragma once
#include <Luna/Runtime/Reflection.hpp>
#include "TestTypes.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{25E088A6-D302-4E24-9EE4-6F9BA15911A9}")]] AssetTestData
    {
        i32 value = 0;

        AssetTestData() = default;
        explicit AssetTestData(i32 value) :
            value(value) {}
    };
}
