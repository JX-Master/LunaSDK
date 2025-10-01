/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneSettings.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/RG/RenderGraph.hpp>
namespace Luna
{
    struct Entity;
    struct SceneSettings
    {
        lustruct("SceneSettings", "{CE0188A0-C1A6-421E-A60C-8D4F260972A3}");

        Name camera_entity;

        Asset::asset_t skybox;

        Float3 environment_color = Float3(0.0f, 0.0f, 0.0f);

        f32 skybox_rotation = 0.0f;
        f32 exposure = 1.0f / 9.6f;
        bool auto_exposure = true;
        f32 bloom_threshold = 1.0f;
        f32 bloom_intensity = 1.0f;
    };
}