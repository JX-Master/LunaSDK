/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Light.hpp
* @author JXMaster
* @date 2020/5/14
*/
#pragma once
#include <Luna/Runtime/Math/Vector.hpp>
namespace Luna
{
    enum class LightType : u32
    {
        directional = 0,
        point = 1,
        spot = 2,
    };

    luenum(LightType, "LightType", "8c1ba13f-e896-4814-9086-0d4f8e104447");

    struct Light
    {
        lustruct("Light", "1838b3c9-41c9-4ae5-8b23-4cfb17344473");

        LightType type;
        Float3 intensity = { 0.5f, 0.5f, 0.5f };
        f32 intensity_multiplier = 1.0f;
        // Only for point light and spot light.
        f32 attenuation_power = 1.0f;
        // Only for spot light.
        f32 spot_power = 64.0f;
    };
}