/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ECS.hpp
* @author JXMaster
* @date 2022/8/11
*/
#pragma once
#include "TaskContext.hpp"
#include "World.hpp"

namespace Luna
{
	namespace ECSError
	{
		LUNA_ECS_API errcat_t errtype();
		LUNA_ECS_API ErrCode entity_not_found();
		LUNA_ECS_API ErrCode component_not_found();
	}
}