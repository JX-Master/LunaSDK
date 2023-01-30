/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Scene.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include "../Scene.hpp"

namespace Luna
{
	Name get_scene_asset_type();
	void register_scene_asset_type();
	RV register_scene_editor();
	void register_scene_importer();
}