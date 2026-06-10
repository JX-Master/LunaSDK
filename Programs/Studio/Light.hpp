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
#include "Light.generated.hpp"
namespace Luna
{
    enum class [[luna::enum("8c1ba13f-e896-4814-9086-0d4f8e104447")]] LightType : u32
    {
        directional [[Luna::option]] = 0,
        point [[Luna::option]] = 1,
        spot [[Luna::option]] = 2,
    };
    struct [[luna::struct("1838b3c9-41c9-4ae5-8b23-4cfb17344473")]] Light
    {
        [[Luna::property]] LightType type;
        [[Luna::property]] Float3 intensity = { 0.5f, 0.5f, 0.5f };
        [[Luna::property]] f32 intensity_multiplier = 1.0f;
        // Only for point light and spot light.
        [[Luna::property]] f32 attenuation_power = 1.0f;
        // Only for spot light.
        [[Luna::property]] f32 spot_power = 64.0f;
    };
}
