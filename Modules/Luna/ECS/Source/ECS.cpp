/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ECS.cpp
* @author JXMaster
* @date 2022/8/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "WorldImpl.hpp"
#include "ECS.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>
namespace Luna
{
    static RV register_ecs_error_codes()
    {
        if (!register_error_category(ECS::ERROR_CATEGORY, "ECS") ||
            !register_error_code(ECS::E_ENTITY_NOT_FOUND, "entity_not_found", "The specified entity was not found.") ||
            !register_error_code(ECS::E_COMPONENT_NOT_FOUND, "component_not_found", "The specified component was not found."))
        {
            return set_error(E_ALREADY_EXISTS, "ECS error metadata conflicts with an existing registration.");
        }
        return ok;
    }

    namespace ECS
    {
        struct ECSModule : public Module
        {
            virtual const c8* get_name() override { return "ECS"; }
            virtual RV on_register() override
            {
                RV result = register_ecs_error_codes();
                if (failed(result.errcode())) return result;
                return add_dependency_module(this, module_job_system());
            }
            virtual RV on_init() override
            {
                Meta::register_ECS_types();
                return ok;
            }
        };
    }
    LUNA_ECS_API Module* module_ecs()
    {
        static ECS::ECSModule m;
        return &m;
    }
}
