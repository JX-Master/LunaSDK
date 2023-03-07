/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneEditor.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <Runtime/Math/Matrix.hpp>
namespace Luna
{
    struct CameraCB
	{
		Float4x4U world_to_view;
		Float4x4U view_to_proj;
		Float4x4U world_to_proj;
		Float4x4U view_to_world;
		Float4U env_color;
	};

	struct LightingParams
	{
		Float3U strength;
		f32 attenuation_power;
		Float3U direction;
		u32 type;
		Float3U position;
		f32 spot_attenuation_power;
	};
}