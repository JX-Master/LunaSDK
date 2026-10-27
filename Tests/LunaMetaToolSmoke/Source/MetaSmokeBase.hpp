/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MetaSmokeBase.hpp
* @author Codex
* @date 2026/10/05
*/
#pragma once
#include <Luna/Runtime/Runtime.hpp>
#include "MetaSmokeBase.generated.hpp"

namespace Luna::MetaToolSmoke
{
    struct [[luna::struct("{53746E16-A04F-4203-BDE9-B22EDEB2C3D4}")]] MetaSmokeReflectedBase
    {
        [[Luna::property]] i32 reflected_base_value = 0;
    };
}
