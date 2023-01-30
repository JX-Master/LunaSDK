/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Light.hpp
* @author JXMaster
* @date 2020/5/14
*/
#pragma once
#include <Runtime/Math/Vector.hpp>
namespace Luna
{
	struct DirectionalLight
	{
		lustruct("DirectionalLight", "{10FA0F19-622B-41E5-84A1-914DF54877A4}");
		Float3 intensity = { 0.5f, 0.5f, 0.5f };
		f32 intensity_multiplier = 1.0f;
	};

	struct PointLight
	{
		lustruct("PointLight", "{D1537251-EBBD-41DB-83E0-3FAE204FCE4F}");
		Float3 intensity = { 0.5f, 0.5f, 0.5f };
		f32 intensity_multiplier = 1.0f;
		f32 attenuation_power = 1.0f;
	};

	struct SpotLight
	{
		lustruct("SpotLight", "{2BB45396-E0E3-433E-8794-49BEE8BD1BB5}");
		Float3 intensity = { 0.5f, 0.5f, 0.5f };
		f32 intensity_multiplier = 1.0f;
		f32 attenuation_power = 1.0f;
		f32 spot_power = 64.0f;
	};
}