/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Model.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include "../Model.hpp"

namespace Luna
{
	Name get_model_asset_type();
	void register_model_asset_type();
	void register_model_editor();
	void register_model_importer();
}