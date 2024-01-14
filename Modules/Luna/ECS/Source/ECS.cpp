/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ECS.cpp
* @author JXMaster
* @date 2022/8/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "World.hpp"
#include "TaskContext.hpp"
#include <Luna/Runtime/Module.hpp>
namespace Luna
{
	namespace ECS
	{
		struct ECSModule : public Module
		{
			virtual const c8* get_name() override { return "ECS"; }
			virtual RV on_register() override
			{
				return add_dependency_module(this, module_job_system());
			}
			virtual RV on_init() override
			{
				register_boxed_type<World>();
				impl_interface_for_type<World, IWorld>();
				register_boxed_type<TaskContext>();
				impl_interface_for_type<TaskContext, ITaskContext>();
				return ok;
			}
		};
	}
	LUNA_ECS_API Module* module_ecs()
	{
		static ECS::ECSModule m;
		return &m;
	}
	namespace ECSError
	{
		LUNA_ECS_API errcat_t errtype()
		{
			static errcat_t v = get_error_category_by_name("ECSError");
			return v;
		}
		LUNA_ECS_API ErrCode entity_not_found()
		{
			static ErrCode v = get_error_code_by_name("ECSError", "entity_not_found");
			return v;
		}
		LUNA_ECS_API ErrCode component_not_found()
		{
			static ErrCode v = get_error_code_by_name("ECSError", "component_not_found");
			return v;
		}
	}
}