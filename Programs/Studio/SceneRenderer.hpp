/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneRenderer.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Runtime/TypeInfo.hpp>
#include <RHI/RHI.hpp>
#include <Asset/Asset.hpp>
#include <RG/RenderGraph.hpp>
namespace Luna
{
	struct Entity;
	struct SceneRenderer
	{
		lustruct("SceneRenderer", "{CE0188A0-C1A6-421E-A60C-8D4F260972A3}");

		Name camera_entity;

		Asset::asset_t skybox;

		Float3 environment_color = Float3(0.0f, 0.0f, 0.0f);

		f32 skybox_rotation = 0.0f;
		f32 exposure = 1.0f;
	};
}