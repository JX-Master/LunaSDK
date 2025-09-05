/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHIUtility.cpp
* @author JXMaster
* @date 2025/9/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "../RHIUtility.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include "ResourceReadContext.hpp"
#include "ResourceWriteContext.hpp"

namespace Luna
{
    namespace RHIUtility
    {
        struct RHIUtilityModule : public Module
        {
            virtual const c8* get_name() override { return "RHIUtility"; }
            virtual RV on_register() override
            {
                return add_dependency_module(this, module_rhi());
            }
            virtual RV on_init() override
            {
                register_boxed_type<ResourceReadContext>();
                impl_interface_for_type<ResourceReadContext, IResourceReadContext, RHI::IDeviceChild>();
                register_boxed_type<ResourceWriteContext>();
                impl_interface_for_type<ResourceWriteContext, IResourceWriteContext, RHI::IDeviceChild>();
                return ok;
            }
        };
    }
    LUNA_RHI_UTILITY_API Module* module_rhi_utility()
    {
        static RHIUtility::RHIUtilityModule m;
        return &m;
    }
}